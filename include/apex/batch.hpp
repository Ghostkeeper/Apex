/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_BATCH
#define APEX_BATCH

#include <vector> //As data structure and to type-specialise batches of vectors.

#include "detail/batch_base.hpp" //Common implementation of many type-specialised Batch implementations.

namespace apex {

/*!
 * The Batch class groups a list of elements together, both conceptually and in
 * memory.
 *
 * The purpose of this batching is to improve performance. If some algorithm
 * needs to be performed on many elements simultaneously, batching these all
 * together allows them to be transferred to different compute devices in one
 * go, which reduces the latency of the transfer.
 *
 * The class behaves externally like \ref std::vector in its interface. For most
 * types of elements, it behaves like a normal vector internally as well.
 * However the template argument can be specialised for some types. This is of
 * particular interest to complex data types which point to separately allocated
 * contents. By specialising the data structure of those elements, the batch can
 * be made more efficient with large data sets.
 * \tparam Element The type of element stored in this batch.
 */
template<typename Element>
class Batch : public BatchBase<Element> {
public:
	/*!
	 * Construct an empty batch.
	 */
	Batch() : BatchBase<Element>() {}

	/*!
	 * Construct a batch containing the specified element repeated a number of
	 * times.
	 * \param count The amount of elements to fill the batch with.
	 * \param value The element to fill the batch with. This element gets
	 * repeated a number of times.
	 */
	Batch(const size_t count, const Element& value = Element()) : BatchBase<Element>(count, value) {}

	/*!
	 * Construct a batch containing default-inserted instances of the element.
	 * \param count The amount of elements to fill the batch with.
	 */
	Batch(const size_t count) : BatchBase<Element>(count) {}

	/*!
	 * Construct a batch with the contents of the range ``[first, last)``.
	 * \tparam InputIterator This constructor accepts any type of input
	 * iterator.
	 * \param first The iterator to start copying from.
	 * \param last An iterator signalling that the range of elements to copy has
	 * ended.
	 */
	template<class InputIterator>
	Batch(InputIterator first, InputIterator last) : BatchBase<Element>(first, last) {}

	/*!
	 * Construct a copy of the specified batch.
	 *
	 * All elements inside the batch will be copied as well.
	 * \param other The batch to copy.
	 */
	Batch(const Batch<Element>& other) : BatchBase<Element>(static_cast<const BatchBase<Element>&>(other)) {}

	/*!
	 * Move constructor, moving one batch to another location using move
	 * semantics.
	 *
	 * After the move, ``other`` is guaranteed to be empty.
	 * \param other The batch to move into the new batch.
	 */
	Batch(Batch<Element>&& other) : BatchBase<Element>(static_cast<BatchBase<Element>&&>(other)) {}

	/*!
	 * Construct a batch from the contents of an initialiser list.
	 * \param initialiser_list The list of elements to put in the new batch.
	 */
	Batch(std::initializer_list<Element> initializer_list) : BatchBase<Element>(initializer_list) {}

	/*!
	 * Request to reduce the memory usage of this batch to fit just the current
	 * data.
	 *
	 * This may cause the data to be reallocated. Or it may not. This depends on
	 * the implementation of the underlying data structures.
	 */
	void shrink_to_fit() noexcept {
		BatchBase<Element>::elements.shrink_to_fit();
	}
};

template<typename Element>
class SubbatchView; //Forward declare this view here so we can use it as template argument below, but implement it later so we can use Batch's fields.

/*!
 * This is a specialisation of the Batch class that handles batches of batches
 * more efficiently.
 *
 * The purpose of this batching is to improve performance. If some algorithm
 * needs to be performed on many elements simultaneously, batching these all
 * together allows them to be transferred to different compute devices in one
 * go, which reduces the latency of the transfer.
 *
 * For simple data types this performs well when the data is placed in a vector,
 * as long as the algorithm implementation knows about the vector in order to
 * allocate it in one go on the compute device. However if the data type
 * allocates additional memory on the heap, the individual allocations of each
 * element would need to be transferred to the compute devices, which is very
 * slow. That is what this template specialisation intends to solve.
 *
 * Instead of each subbatch having its own allocation of memory, this batch will
 * coalesce all data to a single array. It then creates 'view' objects that
 * point to segments of the array with a start index, a size and a capacity. The
 * encompassing batch will behave externally like a batch of these views then.
 * The views have the same interface as batches, but a different behaviour since
 * they index into a single allocation, rather than their own little piece of
 * memory. That way, if the batch as a whole needs to be processed, only that
 * single allocation and the secondary index array need to be sent to the
 * compute device. This is intended to improve performance.
 *
 * There are also disadvantages to this approach. It essentially operates as a
 * monotonic allocator, so if any of the elements have to grow in size, they
 * will need to be moved in their entirety to the end of the buffer. There is
 * also no tracking of where there might be gaps halfway in the buffer to move
 * them to, so the buffer will only grow unless it is optimised with a manual
 * call. Consider frequent modifications of the batch to be inefficient, if the
 * modifications cause the subbatches to grow.
 *
 * The subbatches introduce a level of indirection when accessing individual
 * elements. That way they can keep working if the main element array needs to
 * be reallocated. Individually accessing elements of the batch is inefficient.
 * It is advised to modify the contents only with the batch's own methods, which
 * have access to the element array and can therefore be efficient.
 *
 * The data and views together form the complete state of the batch. They are
 * intended to be sent together as two buffers to the compute devices. This
 * allows all of the batch operations to be executed on the compute devices.
 * \tparam Element The type of element stored in the subbatches.
 */
template<typename Element>
class Batch<Batch<Element>> : public BatchBase<SubbatchView<Element>> { //Specialise batches of batches.
	friend class SubbatchView<Element>; //Subbatches can access the coalesced data structure to get their own information.

public:
	/*!
	 * Creates an empty batch.
	 */
	Batch() {}

	/*!
	 * Creates a batch with a number of copies of the same subbatch.
	 * \param The number of copies to store in this batch.
	 * \param value The subbatch to copy multiple times.
	 */
	Batch(const size_t count, const Batch<Element>& value = Batch<Element>()) {
		assign(count, value);
	}

	/*!
	 * Creates a batch, filling it immediately with a range of subbatches.
	 * \param first The start of a range of subbatches to fill the batch with.
	 * \param last An iterator marking the end of a range of subbatches to fill
	 * the batch with.
	 * \tparam InputIterator This constructor works with any type of iterator.
	 */
	template<class InputIterator>
	Batch(InputIterator first, InputIterator last) {
		assign(first, last);
	}

	/*!
	 * Copy constructor, creating a copy of the specified batch.
	 * \param other The batch to copy.
	 */
	Batch(const Batch<Batch<Element>>& other) {
		//Don't simply copy the subelements buffer and subbatchviews.
		//If we're copying the data anyway, we might as well shrink the subelements buffer to fit.
		//This not only improves performance of the copy, but also of any subsequent algorithms on the batch.
		assign(other.begin(), other.end()); //Assignment deals with proper reserving of memory, since batch iterators are random access.
	}

	/*!
	 * Move constructor, moving the batch to a different location in memory.
	 * \param other The batch to move into this batch.
	 */
	Batch(Batch<Batch<Element>>&& other) : Batch<SubbatchView<Element>>(other) {
		//Don't optimise memory in this constructor.
		//It's not always necessary to copy the data, so it's not always possible to shrink to fit.
		//For cases where we can do a no-op move, allow that no-op move.
		subelements = std::move(other.subelements);
		next_position = other.next_position;
	}

	/*!
	 * Constructs a batch of batches from a list of batches.
	 * \param initialiser_list An initialiser list containing subbatches to put
	 * in this batch.
	 */
	Batch(std::initializer_list<Batch<Element>>& initialiser_list) {
		assign(initialiser_list.begin(), initialiser_list.end());
	}

	protected:
	/*!
	 * Vector containing the actual data in the subbatches.
	 *
	 * This effectively coalesces all data of all batches into one single array,
	 * which is easier to transfer to other devices in one allocation, improving
	 * performance.
	 */
	std::vector<Element> subelements;

	/*!
	 * The starting index in the element buffer of the next subbatch, if a new
	 * one would be added.
	 *
	 * This position effectively indicates where the currently allocated
	 * data ends in the most recently allocated subbatch.
	 */
	size_t next_position;
};

/*!
 * A view on a batch of batches.
 *
 * This view replaces the standard implementation of a batch if that batch is
 * contained within a different batch. The batch of batches will actually be
 * transformed into a batch of ``SubbatchView``s. These views have exactly the
 * same interface as a normal batch, so they can be used interchangeably in
 * code. However the implementation is different. It allows the actual data to
 * be coalesced with other subbatches in the batch, so that it requires fewer
 * allocations on the compute devices, improving performance.
 *
 * A view consists of a start index in the element buffer, a size and a current
 * capacity. It also tracks which batch it belongs to, so that the views can be
 * used separately from the batches as long as the original batch is not moved
 * or destroyed.
 */
template<typename Element>
class SubbatchView {
	friend class Batch<Batch<Element>>; //The parent batch is a friend class, so that it can access the hidden constructor to create new views.

	public:
	//Using the iterator types of the element buffer.
	/*!
	 * The iterator type used to iterate over elements of the subbatch.
	 */
	using iterator = typename std::vector<Element>::iterator;

	/*!
	 * The iterator type used to iterate over elements of a const subbatch.
	 */
	using const_iterator = typename std::vector<Element>::const_iterator;

	/*!
	 * The iterator type used to iterate in reverse over elements of the
	 * subbatch.
	 */
	using reverse_iterator = typename std::vector<Element>::reverse_iterator;

	/*!
	 * The iterator type used to iterate in reverse over elements of a const
	 * subbatch.
	 */
	using const_reverse_iterator = typename std::vector<Element>::const_reverse_iterator;

	/*!
	 * Return the element in a specified position in the subbatch.
	 * \param index The position of the element to retrieve.
	 * \return The element in the specified position of the subbatch.
	 */
	const Element& operator [](const size_t index) const {
		return batch.subelements[start_index + index];
	}

	/*!
	 * Return the element in a specified position in the subbatch.
	 * \param index The position of the element to retrieve.
	 * \return The element in the specified position of the subbatch.
	 */
	Element& operator [](const size_t index) {
		return batch.subelements[start_index + index];
	}

	/*!
	 * Compare two subbatches for equality.
	 *
	 * The subbatches are equal if all of their elements are equal and in the
	 * same order.
	 * \param other The subbatch to compare this subbatch to.
	 * \return ``true`` if this subbatch is equal to the other subbatch, or
	 * ``false`` if at least one of the elements is different.
	 */
	bool operator ==(const SubbatchView<Element>& other) const {
		if(size() != other.size()) {
			return false; //If they have different sizes, not all elements can have an equal in the same position in the other batch.
		}
		for(size_t element = 0; element < size(); ++element) {
			if((*this)[element] != other[element]) {
				return false;
			}
		}
		return true; //Found no element that was different in the other batch, so all elements must be the same.
	}

	/*!
	 * Compare this subbatch with a different batch for equality.
	 *
	 * The subbatch equals the other batch if all of their elements are equal
	 * and in the same order.
	 * \param other The batch to compare this subbatch to.
	 * \return ``true`` if this subbatch is equal to the other batch, or
	 * ``false`` if at least one of the elements is different.
	 */
	bool operator ==(const Batch<Element>& other) const {
		if(size() != other.size()) {
			return false; //If they have different sizes, not all elements can have an equal in the same position in the other batch.
		}
		for(size_t element = 0; element < size(); ++element) {
			if((*this)[element] != other[element]) {
				return false;
			}
		}
		return true; //Found no element that was different in the other batch, so all elements must be the same.
	}

	/*!
	 * Compare two subbatches for inequality.
	 *
	 * The subbatches are inequal if at least one of their elements is
	 * different, or in a different order.
	 * \param other The subbatch to compare this subbatch to.
	 * \return ``true`` if this subbatch is different from the other subbatch,
	 * or ``false`` if they are equal.
	 */
	bool operator !=(const SubbatchView<Element>& other) const {
		return !((*this) == other); //Inverse of equality operator.
	}

	/*!
	 * Compare a subbatch to another batch for inequality.
	 *
	 * The subbatch is inequal to the batch if at least one of their elements is
	 * different, or in a different order.
	 * \param other The batch to compare this subbatch to.
	 * \return ``true`` if this subbatch is different from the other batch, or
	 * ``false`` if they are equal.
	 */
	bool operator !=(const Batch<Element>& other) const {
		return !((*this) == other); //Inverse of equality operator.
	}

	/*!
	 * Replace the contents of the subbatch with a repeated instance of a given
	 * element.
	 * \param count How many times to repeat the element.
	 * \param value The element to repeat.
	 */
	void assign(const size_t count, const Element& value) {
		reserve(count);
		for(size_t index = 0; index < count; ++index) {
			(*this)[index] = value;
		}
		num_elements = count;
	}

	/*!
	 * Replace the contents of the subbatch with the contents of a range between
	 * two given iterators.
	 * \param begin The beginning of the range to replace with.
	 * \param end An iterator signalling the end of the range to replace with.
	 * \tparam InputIterator The type of iterator to iterate with. The ``begin``
	 * iterator needs to have the same type as the ``end`` iterator.
	 */
	template<class InputIterator>
	void assign(InputIterator begin, InputIterator end) {
		clear(); //Clear any old contents.
		assign_iterator_dispatch<InputIterator>(begin, end, typename std::iterator_traits<InputIterator>::iterator_category());
	}

	/*!
	 * Replace the contents of the subbatch with the contents of an initialiser
	 * list.
	 * \param initialiser_list The list of elements with which to replace the
	 * contents of this subbatch.
	 */
	void assign(const std::initializer_list<Element> initialiser_list) {
		clear(); //Clear contents first, so that they don't need to get copied if we need to reallocate.
		reserve(initialiser_list.size());
		size_t index = 0;
		for(const Element& element : initialiser_list) {
			(*this)[index++] = element;
		}
		num_elements = initialiser_list.size(); //Update the size, clearing any vertices that we didn't override if the original was bigger.
	}

	/*!
	 * Return a reference to an element in a certain position in the subbatch.
	 *
	 * If the given position is out of range, an ``std::out_of_range`` exception
	 * is thrown.
	 * \param position The index of the element to return.
	 * \return A reference to the element in the specified position.
	 */
	const Element& at(const size_t position) const {
		if(position >= size()) {
			throw std::out_of_range("Out of range for this view on a subbatch.");
		}
		return (*this)[position];
	}

	/*!
	 * Return a reference to an element in a certain position in the subbatch.
	 *
	 * If the given position is out of range, an ``std::out_of_range`` exception
	 * is thrown.
	 * \param position The index of the element to return.
	 * \return A reference to the element in the specified position.
	 */
	Element& at(const size_t position) {
		if(position >= size()) {
			throw std::out_of_range("Out of range for this view on a subbatch.");
		}
		return (*this)[position];
	}

	/*!
	 * Get the last element of the subbatch.
	 *
	 * Getting the back of an empty subbatch is undefined. It may return an
	 * element of a different subbatch, or run out of range of the internal
	 * memory which may cause a segfault.
	 * \return A reference to the last element of the subbatch.
	 */
	const Element& back() const {
		return (*this)[size() - 1];
	}

	/*!
	 * Get the last element of the subbatch.
	 *
	 * Getting the back of an empty subbatch is undefined. It may return an
	 * element of a different subbatch, or run out of range of the internal
	 * memory which may cause a segfault.
	 * \return A reference to the last element of the subbatch.
	 */
	Element& back() {
		return (*this)[size() - 1];
	}

	/*!
	 * Get an iterator to the first element in the subbatch.
	 *
	 * This actually returns an iterator to an element in the element buffer.
	 * You could theoretically keep iterating further, but that accesses
	 * elements of different subbatches and uninitialised memory in between the
	 * subbatches for expansion capacity. Iterating beyond the ``end()``
	 * iterator is not supported.
	 * \return An iterator pointing to the first element of the subbatch.
	 */
	const_iterator begin() const {
		return batch.subelements.begin() + start_index;
	}

	/*!
	 * Get an iterator to the first element in the subbatch.
	 *
	 * This actually returns an iterator to an element in the element buffer.
	 * You could theoretically keep iterating further, but that accesses
	 * elements of different subbatches and uninitialised memory in between the
	 * subbatches for expansion capacity. Iterating beyond the ``end()``
	 * iterator is not supported.
	 * \return An iterator pointing to the first element of the subbatch.
	 */
	iterator begin() {
		return batch.subelements.begin() + start_index;
	}

	/*!
	 * Return the number of elements that this subbatch could contain without
	 * needing to allocate more memory.
	 * \return The capacity of this subbatch.
	 */
	size_t capacity() const noexcept {
		return current_capacity;
	}

	/*!
	 * Get an iterator to the first element in the subbatch.
	 *
	 * This actually returns an iterator to an element in the element buffer.
	 * You could theoretically keep iterating further, but that accesses
	 * elements of different subbatches and uninitialised memory in between the
	 * subbatches for expansion capacity. Iterating beyond the ``cend()``
	 * iterator is not supported.
	 * \return An iterator pointing to the first element of the subbatch.
	 */
	const_iterator cbegin() const {
		return begin(); //Calls the const version, which returns a const_iterator anyway.
	}

	/*!
	 * Get an iterator signalling the end of the subbatch.
	 *
	 * This actually returns an iterator to the end of the view in the element
	 * buffer. You could theoretically keep iterating further, but that accesses
	 * elements of different subbatches and uninitialised memory in between the
	 * subbatches for expansion capacity. Iterating beyond the ``cend()``
	 * iterator is not supported.
	 * \return An iterator marking the end of the subbatch.
	 */
	const_iterator cend() const {
		return end(); //Calls the const version, which returns a const_iterator anyway.
	}

	/*!
	 * Removes all content from this subbatch.
	 */
	void clear() noexcept {
		num_elements = 0; //If we just say the subbatch ends after 0 elements, it instantly behaves as if it's empty. The old data won't do any harm where it is.
	}

	/*!
	 * Get an iterator to the first element in the subbatch when iterating in
	 * reverse (which would normally be the last element).
	 *
	 * This actually returns an iterator to the last element of the view in the
	 * element buffer. You could theoretically keep iterating further, but that
	 * accesses elements of different subbatches and uninitialised memory in
	 * between the subbatches for expansion capacity. Iterating beyond the
	 * ``crend()`` iterator is not supported.
	 * \return A reverse iterator pointing to the last element of the subbatch.
	 */
	const_reverse_iterator crbegin() const {
		return rbegin(); //Calls the const version, which returns a const_reverse_iterator anyway.
	}

	/*!
	 * Get an iterator marking the end of the subbatch when iterating in reverse
	 * (which would normally be before the first element).
	 *
	 * This actually returns an iterator to before the beginning of the view in
	 * the element buffer. You could theoretically keep iterating further, but
	 * that accesses elements of different subbatches and uninitialised memory
	 * in between the subbatches for expansion capacity. Iterating beyond the
	 * ``crend()`` iterator is not supported.
	 * \return A reverse iterator marking the end of the reversed subbatch.
	 */
	const_reverse_iterator crend() const {
		return rend(); //Calls the const version, which returns a const_reverse_iterator anyway.
	}

	/*!
	 * Return a pointer to the underlying buffer serving as element storage for
	 * this subbatch.
	 *
	 * This returns a pointer to the first element that is part of this view.
	 * If the size of the element buffer is 0, this may return a pointer that is
	 * not dereferenceable.
	 * \return A pointer to the first element that is part of this view in the
	 * underlying data structure of the view.
	 */
	const Element* data() const noexcept {
		if(batch.subelements.empty()) {
			return batch.subelements.data(); //Do whatever they do.
		}
		return &front();
	}

	/*!
	 * Return a pointer to the underlying buffer serving as element storage for
	 * this subbatch.
	 *
	 * This returns a pointer to the first element that is part of this view.
	 * If the size of the element buffer is 0, this may return a pointer that is
	 * not dereferenceable.
	 * \return A pointer to the first element that is part of this view in the
	 * underlying data structure of the view.
	 */
	Element* data() noexcept {
		if(batch.subelements.empty()) {
			return batch.subelements.data(); //Do whatever they do.
		}
		return &front();
	}

	/*!
	 * Construct a new element in-place in the subbatch.
	 *
	 * The new element is inserted in a specified position. All elements
	 * following that position will shift by 1 place.
	 * \param position The position within this subbatch to place the new
	 * element.
	 * \param arguments The constructor arguments of the element to add.
	 * \tparam Args The types of the constructor arguments of the element.
	 */
	template<class... Args>
	iterator emplace(const const_iterator position, Args&&... arguments) {
		const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the iterator).
		if(size() >= capacity()) { //Need to make sure we have enough capacity for the new element.
			reallocate(capacity() * 2 + 1); //Double the capacity, so that repeated emplacing executes in amortised linear time.
		}

		for(size_t i = size(); i > index; --i) { //Move all elements beyond the specified position by one place, to make room.
			(*this)[i] = (*this)[i - 1];
		}

		//Construct the element in-place.
		(*this)[index] = Element(arguments...);
		++num_elements; //This increased the size by 1.
		return begin() + index;
	}

	/*!
	 * Construct a new element in-place at the end of the subbatch.
	 * \param arguments The constructor arguments of the element to add.
	 * \tparam Args The types of the constructor arguments of the element.
	 */
	template<class... Args>
	void emplace_back(Args&&... arguments) {
		if(size() >= capacity()) { //Need to make sure we have enough capacity for the new element.
			reallocate(capacity() * 2 + 1); //Double in size, so that repeated emplacing executes in amortised constant time.
		}

		(*this)[size()] = Element(arguments...); //Construct the element in-place.
		++num_elements; //This increased the size by 1.
	}

	/*!
	 * Return whether this subbatch is an empty batch.
	 * \return ``true`` if there are no elements in this subbatch, or ``false``
	 * if there are.
	 */
	bool empty() const {
		return size() == 0;
	}

	/*!
	 * Get an iterator signalling the end of the subbatch.
	 *
	 * This actually returns an iterator to the end of the view in the element
	 * buffer. You could theoretically keep iterating further, but that accesses
	 * elements of different subbatches and uninitialised memory in between the
	 * subbatches for expansion capacity. Iterating beyond the ``end()``
	 * iterator is not supported.
	 * \return An iterator marking the end of the subbatch.
	 */
	const_iterator end() const {
		return batch.subelements.begin() + (start_index + size());
	}

	/*!
	 * Get an iterator signalling the end of the subbatch.
	 *
	 * This actually returns an iterator to the end of the view in the element
	 * buffer. You could theoretically keep iterating further, but that accesses
	 * elements of different subbatches and uninitialised memory in between the
	 * subbatches for expansion capacity. Iterating beyond the ``cend()``
	 * iterator is not supported.
	 * \return An iterator marking the end of the subbatch.
	 */
	iterator end() {
		return batch.subelements.begin() + (start_index + size());
	}

	/*!
	 * Erases one element from the subbatch.
	 *
	 * The iterators pointing to positions at or after this element will be
	 * invalidated. The iterators pointing to positions before this element will
	 * not be invalidated.
	 * \param position The position of the element to erase.
	 * \return An iterator to the element after the removed element. If the last
	 * element was removed, the ``end()`` iterator is returned.
	 */
	iterator erase(const const_iterator position) {
		const size_t index = position - begin();
		for(size_t i = index; i < size() - 1; ++i) { //Shift other elements over this one.
			(*this)[i] = (*this)[i + 1];
		}
		--num_elements;
		return begin() + index; //Convert iterator to non-const version by constructing a new one.
	}

	/*!
	 * Erases a range of elements from a subbatch.
	 *
	 * The iterators pointing to positions within or after the range will be
	 * invalidated. The iterators pointing to positions before the range will
	 * not be invalidated.
	 * \param first The beginning of the range of elements to remove.
	 * \param end An iterator signalling the end of the range of elements to
	 * remove.
	 * \return An iterator pointing to the element after the last removed
	 * element.
	 */
	iterator erase(const_iterator first, const const_iterator end) {
		const size_t index = first - begin();
		const size_t num_removed = end - first;
		for(size_t i = index; i < size() - num_removed; ++i) { //Shift other elements over the range.
			(*this)[i] = (*this)[i + num_removed];
		}
		num_elements -= num_removed;
		return begin() + index; //Convert iterator to non-const version by constructing a new one.
	}

	/*!
	 * Get the first element of the subbatch.
	 *
	 * Getting the front of an empty subbatch is undefined. It may return an
	 * element of a different subbatch, or run out of range of the internal
	 * memory which may cause a segfault.
	 * \return A reference to the first element of the subbatch.
	 */
	const Element& front() const {
		return (*this)[0];
	}

	/*!
	 * Get the first element of the subbatch.
	 *
	 * Getting the front of an empty subbatch is undefined. It may return an
	 * element of a different subbatch, or run out of range of the internal
	 * memory which may cause a segfault.
	 * \return A reference to the first element of the subbatch.
	 */
	Element& front() {
		return (*this)[0];
	}

	/*!
	 * Insert a new element at the specified position in the subbatch.
	 *
	 * The new element is inserted \e before the specified position.
	 * \param position The position before which the new element will be
	 * inserted. To append it to the end, the \ref end iterator may be supplied
	 * (but consider using \ref push_back instead).
	 * \param value The element to insert.
	 * \return An iterator pointing to the newly inserted element.
	 */
	iterator insert(const const_iterator position, const Element& value) {
		const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the iterator).
		if(size() >= capacity()) {
			reallocate(capacity() * 2 + 1); //Double the capacity, so that repeated inserting executes in amortised linear time.
		}

		for(size_t i = size(); i > index; --i) { //Move all elements beyond the position by one place to make room.
			(*this)[i] = (*this)[i - 1];
		}
		(*this)[index] = value; //Insert the new element.
		++num_elements;
		return begin() + index;
	}

	/*!
	 * Insert a new element at the specified position in the subbatch.
	 *
	 * This version tries to move the element rather than reallocating it. A
	 * compiler may be able to omit the move then.
	 *
	 * The new element is inserted \e before the specified position.
	 * \param position The position before which the new element will be
	 * inserted. To append it to the end, the \ref end iterator may be supplied
	 * (but consider using \ref push_back instead).
	 * \param value The element to insert.
	 * \return An iterator pointing to the newly inserted element.
	 */
	iterator insert(const const_iterator position, Element&& value) {
		const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the iterator).
		if(size() >= capacity()) {
			reallocate(capacity() * 2 + 1); //Double the capacity, so that repeated inserting executes in amortised linear time.
		}

		for(size_t i = size(); i > index; --i) { //Move all elements beyond the position by one place to make room.
			(*this)[i] = (*this)[i - 1];
		}
		(*this)[index] = std::move(value); //Insert the new element.
		++num_elements;
		return begin() + index;
	}

	/*!
	 * Insert a number of copies of an element at the specified position in the
	 * subbatch.
	 *
	 * The new elements are inserted \e before the specified position.
	 * \param position The position before which the new elements will be
	 * inserted. To append them to the end, the \ref end iterator may be
	 * supplied.
	 * \param count How many copies of the element to insert.
	 * \param value The element to insert repeatedly.
	 * \return An iterator pointing to the first of the newly inserted elements.
	 */
	iterator insert(const const_iterator position, const size_t count, const Element& value) {
		const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the iterator).
		if(size() + count > capacity()) {
			reallocate(capacity() * 2 + count); //Double the capacity, so that repeated inserting executes in amortised linear time.
		}

		for(size_t i = size() + count; i > index + count; --i) { //Move all elements beyond the position by multiple places to make room.
			(*this)[i] = (*this)[i - count];
		}
		//Insert the new elements.
		for(size_t i = 0; i < count; ++i) {
			(*this)[index + i] = value;
		}

		num_elements += count;
		return begin() + index;
	}

	/*!
	 * Insert a range of elements at the specified position in the subbatch.
	 *
	 * The new elements are inserted \e before the specified position.
	 * \param position The position before which the new elements will be
	 * inserted. To append them to the end, the \ref end iterator may be
	 * supplied.
	 * \param begin The start of the range of elements to insert.
	 * \param end An iterator signalling the end of the range of elements to
	 * insert.
	 * \return An iterator pointing to the first of the newly inserted elements.
	 * \tparam InputIterator This function works with any type of input
	 * iterator.
	 */
	template<class InputIterator>
	iterator insert(const const_iterator position, InputIterator begin, const InputIterator end) {
		//Dispatch to the most efficient implementation for the given iterator type.
		return insert_iterator_dispatch<InputIterator>(position, begin, end, typename std::iterator_traits<InputIterator>::iterator_category());
	}

	/*!
	 * Insert a list of elements at the specified position in the subbatch.
	 *
	 * The new elements are inserted \e before the specified position.
	 * \param position The position before which the new elements will be
	 * inserted. To append them to the end, the \ref end iterator may be
	 * supplied.
	 * \param initialiser_list The list of elements to insert.
	 * \return An iterator pointing to the first of the newly inserted elements.
	 */
	iterator insert(const const_iterator position, const std::initializer_list<Element>& initialiser_list) {
		return insert(position, initialiser_list.begin(), initialiser_list.end()); //Refer to the iterator-based implementation.
	}

	/*!
	 * Returns the maximum number of elements that this subbatch is
	 * theoretically able to hold due to the implementation.
	 *
	 * This maximum may be reduced if the batch also contains other subbatches.
	 * \return The maximum number of elements that this subbatch is able to
	 * hold.
	 */
	size_t max_size() const noexcept {
		return batch.subelements.max_size(); //Should really subtract the total size of all other subbatches in the batch, but that would be linear.
	}

	/*!
	 * Removes the last element from the subbatch.
	 *
	 * Calling ``pop_back`` on an empty subbatch is undefined. In fact, this
	 * might just cause an underflow on the size.
	 */
	void pop_back() {
		--num_elements; //Just reduce the size. This automatically causes the subbatch to pretend the last element isn't there.
	}

	/*!
	 * Appends an element to the end of the subbatch.
	 *
	 * The element is copied in this case. This may cause a reallocation, which
	 * will invalidate all iterators to elements in this batch.
	 * \param value The element to add to the subbatch.
	 */
	void push_back(const Element& value) {
		if(size() + 1 > capacity()) {
			reallocate(capacity() * 2 + 1); //Doubling the capacity (or multiplying with any scalar, really) guarantees an amortised constant execution time.
		}
		(*this)[size()] = value;
		++num_elements;
	}

	/*!
	 * Appends an element at the end of the subbatch.
	 *
	 * The element is moved in this case. This may cause a reallocation, which
	 * will invalidate all iterators to elements in this batch.
	 * \param value The element to add to the subbatch.
	 */
	void push_back(Element&& element) {
		if(size() + 1 > capacity()) {
			reallocate(capacity() * 2 + 1); //Doubling the capacity (or multiplying with any scalar, really) guarantees an amortised constant execution time.
		}
		(*this)[size()] = std::move(element);
		++num_elements;
	}

	/*!
	 * Get an iterator to the first element in the subbatch when iterating in
	 * reverse (which would normally be the last element).
	 *
	 * This actually returns an iterator to the last element of the view in the
	 * element buffer. You could theoretically keep iterating further, but that
	 * accesses elements of different subbatches and uninitialised memory in
	 * between the subbatches for expansion capacity. Iterating beyond the
	 * ``rend()`` iterator is not supported.
	 * \return A reverse iterator pointing to the last element of the subbatch.
	 */
	const_reverse_iterator rbegin() const {
		const_reverse_iterator beginning = batch.subelements.rbegin(); //So pointing to the very last element in the buffer.
		std::advance(beginning, batch.subelements.size() - start_index - size());
		return beginning;
	}

	/*!
	 * Get an iterator to the first element in the subbatch when iterating in
	 * reverse (which would normally be the last element).
	 *
	 * This actually returns an iterator to the last element of the view in the
	 * element buffer. You could theoretically keep iterating further, but that
	 * accesses elements of different subbatches and uninitialised memory in
	 * between the subbatches for expansion capacity. Iterating beyond the
	 * ``rend()`` iterator is not supported.
	 * \return A reverse iterator pointing to the last element of the subbatch.
	 */
	reverse_iterator rbegin() {
		reverse_iterator beginning = batch.subelements.rbegin(); //So pointing to the very last element in the buffer.
		std::advance(beginning, batch.subelements.size() - start_index - size());
		return beginning;
	}

	/*!
	 * Get an iterator marking the end of the subbatch when iterating in reverse
	 * (which would normally be before the first element).
	 *
	 * This actually returns an iterator to before the beginning of the view in
	 * the element buffer. You could theoretically keep iterating further, but
	 * that accesses elements of different subbatches and uninitialised memory
	 * in between the subbatches for expansion capacity. Iterating beyond the
	 * ``rend()`` iterator is not supported.
	 * \return A reverse iterator marking the end of the reversed subbatch.
	 */
	const_reverse_iterator rend() const {
		const_reverse_iterator ending = batch.subelements.rbegin(); //So pointing to the very last element in the buffer.
		std::advance(ending, batch.subelements.size() - start_index);
		return ending;
	}

	/*!
	 * Get an iterator marking the end of the subbatch when iterating in reverse
	 * (which would normally be before the first element).
	 *
	 * This actually returns an iterator to before the beginning of the view in
	 * the element buffer. You could theoretically keep iterating further, but
	 * that accesses elements of different subbatches and uninitialised memory
	 * in between the subbatches for expansion capacity. Iterating beyond the
	 * ``rend()`` iterator is not supported.
	 * \return A reverse iterator marking the end of the reversed subbatch.
	 */
	reverse_iterator rend() {
		reverse_iterator ending = batch.subelements.rbegin(); //So pointing to the very last element in the buffer.
		std::advance(ending, batch.subelements.size() - start_index);
		return ending;
	}

	/*!
	 * Increase the capacity of the subbatch to ensure that it can contain at
	 * least the specified number of elements, without reallocating to a new
	 * strip of memory in the element buffer.
	 *
	 * The actual size or data is not changed. However if the capacity of the
	 * view is increased, all current iterators to positions in the subbatch are
	 * invalidated. They cannot be used any more since the place they refer to
	 * is no longer where the data is held.
	 * \param new_capacity The minimum capacity that the subbatch must have to
	 * contain elements, without needing to reallocate.
	 */
	void reserve(const size_t new_capacity) {
		if(new_capacity <= current_capacity) {
			return; //Already have enough capacity for this requirement. No reallocation necessary.
		}
		reallocate(new_capacity);
	}

	/*!
	 * Resize the subbatch to a specific size.
	 *
	 * If the new size is smaller, elements are removed.
	 *
	 * If the new size is bigger, the default value is added until it has the
	 * desired size.
	 * \param new_size The new size for the subbatch.
	 * \param default_value An element to insert if the subbatch needs to become
	 * bigger.
	 */
	void resize(const size_t new_size, const Element& default_value = Element()) {
		if(new_size > size()) {
			reserve(new_size);
			for(size_t i = size(); i < new_size; ++i) {
				(*this)[i] = default_value;
			}
		}
		num_elements = new_size; //Set the new size. This may drop some elements.
	}

	/*!
	 * This function is a no-op for this implementation of the vector
	 * specification.
	 *
	 * It would normally request the memory usage to be reduced to fit exactly
	 * the amount of elements used by this subbatch. However since this subbatch
	 * is part of a bigger batch, and the batch uses a monotonic allocation
	 * algorithm to reserve memory for its members, the memory usage cannot
	 * shrink for just one individual subbatch.
	 *
	 * To actually reduce memory usage here, call the \ref Batch::shrink_to_fit
	 * function of the batch that contains this subbatch.
	 */
	inline void shrink_to_fit() noexcept {}

	/*!
	 * Return the number of elements in this subbatch.
	 * \return The number of elements in this subbatch.
	 */
	size_t size() const {
		return num_elements;
	}

	/*!
	 * Exchange the contents of this subbatch with those of another.
	 *
	 * The membership of a parent batch or the position in it is not swapped.
	 * This subbatch will still be in the same position in the same batch, but
	 * it will have different contents.
	 *
	 * If the two subbatches are part of the same batch, this will only swap
	 * indices around and complete in constant time. If the two views are not
	 * part of the same parent batch, the data itself needs to be swapped, which
	 * will take linear time and may cause reallocations in the batch with the
	 * smaller subbatch.
	 * \param other The subbatch with which to swap the contents.
	 */
	void swap(SubbatchView<Element>& other) {
		if(&batch == &other.batch) {
			//If we just swap which part of the element buffer they point to, that already works.
			std::swap(start_index, other.start_index);
			std::swap(num_elements, other.num_elements);
			std::swap(current_capacity, other.current_capacity);
		} else { //Different batches, so we need to swap the actual contents.
			const size_t my_size = size();
			const size_t other_size = other.size();

			//Find out if we need to resize either of them.
			//If so, move elements directly to the new allocation instead of moving them twice.
			size_t my_destination = start_index;
			if(other_size > capacity()) {
				my_destination = batch.next_position;
				//Make sure we have enough capacity in the element buffer itself. Grow by doubling there too.
				size_t buffer_capacity = batch.subelements.size();
				while(buffer_capacity < my_destination + other_size) {
					buffer_capacity = buffer_capacity * 2 + 1;
				}
				batch.subelements.resize(buffer_capacity, Element());
				current_capacity = other_size;
				batch.next_position += current_capacity;
			}
			size_t other_destination = other.start_index;
			if(my_size > other.capacity()) {
				other_destination = other.batch.next_position;
				//Make sure we have enough capacity in the element buffer itself. Grow by doubling there too.
				size_t buffer_capacity = other.batch.subelements.size();
				while(buffer_capacity < other_destination + my_size) {
					buffer_capacity = buffer_capacity * 2 + 1;
				}
				other.batch.subelements.resize(buffer_capacity, Element());
				other.current_capacity = my_size;
				other.batch.next_position += other.current_capacity;
			}

			//Now swap all of the data, being careful not to overwrite important data.
			if(start_index == my_destination && other.start_index == other_destination) { //Neither batch got reallocated. Use actual swaps.
				size_t i = 0;
				for(; i < std::min(my_size, other_size); ++i) {
					std::swap((*this)[i], other[i]);
				}
				//If one is bigger than the other, move the rest.
				for(; i < other_size; ++i) {
					(*this)[i] = std::move(other[i]);
				}
				for(; i < my_size; ++i) {
					other[i] = std::move((*this)[i]);
				}
			} else { //At least one got reallocated.
				//Determine which to move first. If one batch didn't get reallocated, overwrite that one last so its data doesn't get lost.
				//If both got reallocated, the order doesn't matter.
				const bool to_me_first = start_index != my_destination;
				if(to_me_first) {
					for(size_t i = 0; i < other_size; ++i) {
						batch.subelements[my_destination + i] = std::move(other[i]);
					}
				}
				for(size_t i = 0; i < my_size; ++i) {
					other.batch.subelements[other_destination + i] = std::move((*this)[i]);
				}
				if(!to_me_first) {
					for(size_t i = 0; i < other_size; ++i) {
						batch.subelements[my_destination + i] = std::move(other[i]);
					}
				}
			}

			//Update metadata.
			start_index = my_destination;
			other.start_index = other_destination;
			std::swap(num_elements, other.num_elements); //Also swap the sizes.
		}
	}

	/*!
	 * Exchange the contents of this subbatch with those of a different batch.
	 *
	 * The membership of a parent batch or the position in it is not swapped.
	 * This subbatch will still be in the same position in the same batch, but
	 * it will have different contents.
	 *
	 * If one batch is bigger than the other, the other may get reallocated in
	 * order to make space for the additional data.
	 * \param other The batch with which to swap the contents.
	 */
	void swap(Batch<Element>& other) {
		const size_t my_size = size();
		const size_t other_size = other.size();

		//Find out if we need to resize either of them.
		//If the subbatch is resized, move elements directly to the reallocated place rather than moving them twice.
		size_t my_destination = start_index;
		if(other_size > capacity()) {
			my_destination = batch.next_position;
			//Make sure we have enough capacity in the element buffer itself. Grow by doubling there too.
			size_t buffer_capacity = batch.subelements.size();
			while(buffer_capacity < my_destination + other_size) {
				buffer_capacity = buffer_capacity * 2 + 1;
			}
			batch.subelements.resize(buffer_capacity, Element());
			current_capacity = other_size;
			batch.next_position += current_capacity;
		}
		other.reserve(my_size); //For the other batch, we can't really prevent it.

		//Now swap all of the data.
		if(start_index == my_destination) { //No reallocation. We can just swap the data.
			size_t i = 0;
			for(; i < std::min(my_size, other_size); ++i) {
				std::swap((*this)[i], other[i]);
			}
			//If one is bigger than the other, move the rest.
			for(; i < other_size; ++i) {
				(*this)[i] = std::move(other[i]);
			}
			other.resize(i);
			for(; i < my_size; ++i) {
				other.push_back(std::move((*this)[i]));
			}
		} else { //We got reallocated. Move elements, making sure the order is correct to not lose data.
			for(size_t i = 0; i < other_size; ++i) {
				batch.subelements[my_destination + i] = std::move(other[i]);
			}
			other.clear();
			for(size_t i = 0; i < my_size; ++i) {
				other.push_back(std::move((*this)[i]));
			}
		}

		//Update metadata.
		start_index = my_destination;
		num_elements = other_size;
	}

	protected:
	/*!
	 * The batch this view is a part of.
	 *
	 * Operations on the data of this view should be performed in the element
	 * buffer of this batch.
	 */
	Batch<Batch<Element>>& batch;

	/*!
	 * The position in the element buffer where the data of this subbatch
	 * starts.
	 */
	size_t start_index;

	/*!
	 * The number of elements currently in the subbatch.
	 *
	 * This field can't just be called "size" since that is already an existing
	 * member, which returns this field as per the vector footprint.
	 */
	size_t num_elements;

	/*!
	 * How much space is available in this part of the element buffer for the
	 * elements in this subbatch.
	 */
	size_t current_capacity;

	/*!
	 * Construct a new view on a subbatch.
	 *
	 * A new view should only be constructed by the batch this view is part of.
	 * \param batch The batch this view is a part of.
	 * \param start_index The index in that batch's element buffer where the
	 * data of this subbatch starts.
	 * \param size The number of elements currently in the subbatch.
	 * \param capacity How much space is available in this part of the element
	 * buffer for the elements in this subbatch.
	 */
	SubbatchView(Batch<Batch<Element>>& batch, const size_t start_index, const size_t size, const size_t capacity) :
			batch(batch),
			start_index(start_index),
			num_elements(size),
			current_capacity(capacity) {};

	/*!
	 * Specialised assign operation for when assigning a range defined by random
	 * access iterators.
	 *
	 * Random access iterators have the property that the distance can be
	 * calculated between them in constant time, without actually performing the
	 * iteration. This helps to be able to reserve enough memory to contain the
	 * contents of the range.
	 * \param begin The first element of the range to assign to this batch.
	 * \param end An iterator marking the end of the range to assign to this
	 * batch.
	 * \tparam InputIterator This function works with any type of random access
	 * iterator.
	 */
	template<class InputIterator>
	void assign_iterator_dispatch(InputIterator begin, InputIterator end, const std::random_access_iterator_tag) {
		const size_t new_size = end - begin;
		reserve(new_size);

		for(size_t index = 0; index < new_size; ++index) {
			(*this)[index] = *begin;
			begin++;
		}
		num_elements = new_size;
	}

	/*!
	 * Specialised assign operation for when assigning a range defined by
	 * forward iterators.
	 *
	 * Forward iterators have the property that they can safely be iterated over
	 * multiple times. This helps to determine the size of the range before
	 * copying the contents, which can reduce the number of times the memory for
	 * this subbatch needs to be reallocated.
	 * \param begin The first element of the range to assign to this batch.
	 * \param end An iterator marking the end of the range to assign to this
	 * batch.
	 * \tparam InputIterator This function works with any type of forward
	 * iterator.
	 */
	template<class InputIterator>
	void assign_iterator_dispatch(InputIterator begin, InputIterator end, const std::forward_iterator_tag) {
		size_t new_size = 0;
		//Iterate over the range once to determine its size. Then start reallocating with the guarantee that there will be enough space.
		for(InputIterator it = begin; it != end; it++) {
			++new_size;
		}
		reserve(new_size);

		//Then a second time to actually assign the contents.
		for(size_t index = 0; index < new_size; ++index) {
			(*this)[index] = *begin;
			begin++;
		}
		num_elements = new_size;
	}

	/*!
	 * Specialised assign operation for when assigning a range defined by any
	 * input iterators.
	 *
	 * If the specialised iterator dispatches for more strict types of iterators
	 * are not applicable, the assign operation can fall back to this one. Since
	 * the range can only be iterated over once, and the size can't be computed,
	 * multiple reallocations may be necessary for this assignment to occur.
	 * \param begin The first element of the range to assign to this batch.
	 * \param end An iterator marking the end of the range to assign to this
	 * batch.
	 * \tparam InputIterator This function works with any type of input
	 * iterator.
	 */
	template<class InputIterator>
	void assign_iterator_dispatch(InputIterator begin, InputIterator end, const std::input_iterator_tag) {
		for(; begin != end; begin++) {
			push_back(*begin); //Simply append all of them. This automatically implements the amortised reallocation.
		}
	}

	/*!
	 * Specialised insert operation for when inserting a range defined by random
	 * access iterators.
	 *
	 * Random access iterators have the property that the distance can be
	 * calculated between them in constant time, without actually performing the
	 * iteration. This helps to be able to make enough space to contain the
	 * contents of the range.
	 * \param position The position before which the new elements will be
	 * inserted.
	 * \param start The first element of the range to insert into this batch.
	 * \param end An iterator marking the end of the range to insert into this
	 * batch.
	 * \return An iterator to the beginning of the newly inserted range.
	 * \tparam InputIterator This function works with any type of random access
	 * iterator.
	 */
	template<class InputIterator>
	iterator insert_iterator_dispatch(const const_iterator position, InputIterator start, const InputIterator end, const std::random_access_iterator_tag) {
		const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the iterator).
		const size_t count = end - start;
		if(size() + count >= capacity()) {
			reallocate(capacity() * 2 + count);
		}

		for(size_t i = size() + count; i > index + count; --i) { //Move all elements beyond the position by multiple places to make room.
			(*this)[i] = (*this)[i - count];
		}
		//Insert the new elements.
		for(size_t i = 0; start != end; ++i) {
			(*this)[index + i] = *start++;
		}

		num_elements += count;
		return begin() + index;
	}

	/*!
	 * Specialised insert operation for when inserting a range defined by
	 * forward iterators.
	 *
	 * Forward iterators have the property that they can safely be iterated over
	 * multiple times. This helps to determine the size of the range before
	 * copying the contents, which allows us to make enough space to insert all
	 * of the contents at once.
	 * \param position The position before which the new elements will be
	 * inserted.
	 * \param start The first element of the range to insert into this batch.
	 * \param end An iterator marking the end of the range to insert into this
	 * batch.
	 * \return An iterator to the beginning of the newly inserted range.
	 * \tparam InputIterator This function works with any type of forward
	 * iterator.
	 */
	template<class InputIterator>
	iterator insert_iterator_dispatch(const const_iterator position, InputIterator start, const InputIterator end, const std::forward_iterator_tag) {
		const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the iterator).
		//Iterate over the range once to determine its size. Then start reallocating with the guarantee that there will be enough space.
		size_t count = 0;
		for(InputIterator it = start; it != end; it++) {
			++count;
		}
		if(size() + count >= capacity()) {
			reallocate(capacity() * 2 + count);
		}

		for(size_t i = size() + count; i > index + count; --i) { //Move all elements beyond the position by multiple places to make room.
			(*this)[i] = (*this)[i - count];
		}
		//Insert the new elements.
		for(size_t i = 0; start != end; ++i) {
			(*this)[index + i] = *start++;
		}

		num_elements += count;
		return begin() + index;
	}

	/*!
	 * Specialised insert operation for when inserting a range defined by any
	 * input iterators.
	 *
	 * If the specialised iterator dispatches for more strict types of iterators
	 * are not applicable, the insert operation can fall back to this one. Since
	 * the range can only be iterated over once, and the size can't be computed,
	 * multiple reallocations may be necessary for this insertion to occur.
	 * \param position The position before which the new elements will be
	 * inserted.
	 * \param start The first element of the range to insert into this batch.
	 * \param end An iterator marking the end of the range to insert into this
	 * batch.
	 * \return An iterator to the beginning of the newly inserted range.
	 * \tparam InputIterator This function works with any type of input
	 * iterator.
	 */
	template<class InputIterator>
	iterator insert_iterator_dispatch(const const_iterator position, InputIterator start, const InputIterator end, const std::input_iterator_tag) {
		const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the iterator).

		/* Since we can't know how many elements we'll get, there is a strategy
		to inserting these elements.

		Naively, one would insert a single element at a time, shifting all
		following elements by one each time. Instead, we'll shift all following
		elements all the way to the end of the capacity, storing them
		temporarily there until the inserting is done. This gives maximum space
		for the unknown amount of elements without needing to shift them again.

		For instance, let's say you have the following elements within the
		allocation for this subbatch, with 5 numbers stored, and capacity for 8:

		``[ 1 2 3 4 5 _ _ _ ]``

		If we now want to insert two letters before the third position, this
		algorithm will first move the data after the insertion position to the
		very end of the capacity:

		``[ 1 2 _ _ _ 3 4 5 ]``

		The letters can then be inserted in the new space, without needing to
		move O(N) elements every time a new one is inserted:

		``[ 1 2 A B _ 3 4 5 ]``

		After inserting all the new elements, the trailing elements have to be
		moved back to close up the excess capacity:

		``[ 1 2 A B 3 4 5 _ ]``

		If another two elements were to be inserted here, element C would be
		inserted without problems, but there would not be any space for D. A new
		segment of the element buffer would need to be allocated. All the data
		is moved to the new segment, but the trailing elements get moved
		immediately to the end of the capacity again to maximise space for new
		elements again.*/

		//So first move the trailing elements to the end of the capacity. Only happens once.
		size_t remaining_space = capacity() - size();
		for(size_t i = index; i < size(); ++i) {
			(*this)[i + remaining_space] = (*this)[i];
		}

		size_t count = 0;
		for(; start != end; start++, ++count) {
			if(remaining_space == 0) { //Not enough space to add the next element. Allocate more.
				//This reallocation is "manual". Instead of moving all elements, we'll immediately move the trailing end to the end of the capacity.
				const size_t new_place = batch.next_position;
				const size_t new_capacity = current_capacity * 2 + 1; //Doubling the capacity ensures that repeated insertion of a new element is done in amortised constant time.
				batch.next_position += new_capacity;

				//Make sure we have enough capacity in the element buffer itself. Grow by doubling there too.
				size_t buffer_capacity = batch.subelements.size();
				while(buffer_capacity < new_place + new_capacity) {
					buffer_capacity = buffer_capacity * 2 + 1;
				}
				batch.subelements.resize(buffer_capacity, Element());

				//Copy the leading elements to the start of the allocated space.
				for(size_t i = 0; i < index + count; ++i) {
					batch.subelements[new_place + i] = (*this)[i];
				}
				//Copy the trailing elements to the end of the allocated space.
				remaining_space = new_capacity - current_capacity;
				for(size_t i = index + count; i < current_capacity; ++i) {
					batch.subelements[new_place + i + remaining_space] = (*this)[i];
				}

				start_index = new_place;
				current_capacity = new_capacity;
			}

			(*this)[index + count] = *start; //Insert the new element in the now vacant space.
			--remaining_space;
		}

		//Finally, close the remaining space, moving the trailing elements to their final positions too now that we have all elements.
		if(remaining_space > 0) {
			for(size_t i = index + count; i + remaining_space < current_capacity; ++i) {
				(*this)[i] = (*this)[i + remaining_space];
			}
		}

		num_elements += count;
		return begin() + index;
	}

	/*!
	 * Moves this subbatch to a new location inside the element buffer to make
	 * more space for new elements.
	 *
	 * This copies all of the element data to a new location, making it a linear
	 * operation. Much like the ArrayList data structure though, adding new
	 * elements won't have to call this operation very often any more as the
	 * subbatch grows bigger, resulting in an amortised constant time complexity
	 * for adding elements.
	 * \param new_capacity The amount of elements that can be stored without
	 * allocating new memory, after this operation has been completed.
	 */
	void reallocate(const size_t new_capacity) {
		const size_t new_place = batch.next_position;
		batch.next_position += new_capacity;

		//Make sure we have enough capacity in the element buffer itself. Grow by doubling there too.
		size_t buffer_capacity = batch.subelements.size();
		while(buffer_capacity < new_place + new_capacity) {
			buffer_capacity = buffer_capacity * 2 + 1;
		}
		batch.subelements.resize(buffer_capacity, Element());

		//Copy all of the data over.
		for(size_t index = 0; index < size(); ++index) {
			batch.subelements[new_place + index] = batch.subelements[start_index + index];
		}

		start_index = new_place;
		current_capacity = new_capacity;
	}
};

}

#endif //APEX_BATCH