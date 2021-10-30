/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_BATCH_BASE
#define APEX_BATCH_BASE

#include <vector> //Providing the base data structure to store elements in.
#include <numeric> //For std::accumulate.

namespace apex {

template<typename Element>
class SubbatchView;

/*!
 * This class forms the basis of all batch implementations.
 *
 * Batches are supposed to behave transparently like vectors. Inheriting from
 * vectors is unsafe if used polymorphously. Since we have no control over how
 * the (public) batch classes are used, and we want to impose no restrictions,
 * we'll inherit privately from vector and re-use all of its functions. That way
 * the user can't accidentally cast to vectors, but we still get to re-use its
 * implementation.
 *
 * This batch base doesn't support customising the allocator. If a specific
 * allocator is necessary, it should be customised in the template
 * specialisation.
 * \tparam Element The type of data stored in this batch.
 */
template<typename Element>
class BatchBase : protected std::vector<Element> {
public:
	//Inheriting all functions of vectors in this case, completely transparently.
	using iterator = typename std::vector<Element>::iterator;
	using const_iterator = typename std::vector<Element>::const_iterator;
	using reverse_iterator = typename std::vector<Element>::reverse_iterator;
	using const_reverse_iterator = typename std::vector<Element>::const_reverse_iterator;
	using std::vector<Element>::operator =;
	using std::vector<Element>::operator [];
	using std::vector<Element>::assign;
	using std::vector<Element>::at;
	using std::vector<Element>::back;
	using std::vector<Element>::begin;
	using std::vector<Element>::capacity;
	using std::vector<Element>::cbegin;
	using std::vector<Element>::cend;
	using std::vector<Element>::clear;
	using std::vector<Element>::crbegin;
	using std::vector<Element>::crend;
	using std::vector<Element>::data;
	using std::vector<Element>::emplace;
	using std::vector<Element>::emplace_back;
	using std::vector<Element>::empty;
	using std::vector<Element>::end;
	using std::vector<Element>::erase;
	using std::vector<Element>::front;
	using std::vector<Element>::insert;
	using std::vector<Element>::max_size;
	using std::vector<Element>::pop_back;
	using std::vector<Element>::push_back;
	using std::vector<Element>::rbegin;
	using std::vector<Element>::rend;
	using std::vector<Element>::reserve;
	using std::vector<Element>::resize;
	using std::vector<Element>::shrink_to_fit;
	using std::vector<Element>::size;
	using std::vector<Element>::swap;

	/*!
	 * Construct an empty batch.
	 */
	BatchBase() noexcept : std::vector<Element>() {}

	/*!
	 * Construct a batch containing the specified element repeated a number of
	 * times.
	 * \param count The amount of elements to fill the batch with.
	 * \param value The element to fill the batch with. This element gets
	 * repeated a number of times.
	 */
	BatchBase(const size_t count, const Element& value = Element()) : std::vector<Element>(count, value) {}

	/*!
	 * Construct a batch containing default-inserted instances of the element.
	 * \param count The amount of elements to fill the batch with.
	 */
	BatchBase(const size_t count) : std::vector<Element>(count) {}

	/*!
	 * Construct a batch with the contents of the range ``[first, last)``.
	 * \tparam InputIterator This constructor accepts any type of input
	 * iterator.
	 * \param first The iterator to start copying from.
	 * \param last An iterator signalling that the range of elements to copy has
	 * ended.
	 */
	template<class InputIterator>
	BatchBase(InputIterator first, InputIterator last) : std::vector<Element>(first, last) {}

	/*!
	 * Construct a batch from the contents of an initialiser list.
	 * \param initialiser_list The list of elements to put in the new batch.
	 */
	BatchBase(std::initializer_list<Element> initialiser_list) : std::vector<Element>(initialiser_list) {}

	/*!
	 * Convert a subbatch into a real, normal batch.
	 *
	 * This way, the subbatch can be used in any place where a normal batch gets
	 * used. However, this does create a copy of the data in the subbatch.
	 * \param subbatch The subbatch to convert to a normal batch.
	 */
	BatchBase(const SubbatchView<Element>& subbatch) : std::vector<Element>(subbatch.begin(), subbatch.end()) {}

	/*!
	 * Construct a copy of the specified batch.
	 *
	 * All elements inside the batch will be copied as well.
	 * \param other The batch to copy.
	 */
	BatchBase(const BatchBase& other) : std::vector<Element>(other) {}

	/*!
	 * Move constructor, moving one batch to another location using move
	 * semantics.
	 *
	 * After the move, ``other`` is guaranteed to be empty.
	 * \param other The batch to move into the new batch.
	 */
	BatchBase(BatchBase&& other) noexcept : std::vector<Element>(other) {}

	//Vector doesn't implement these comparison operators itself. They are defined as loose functions in line with the STL design.
	//However this doesn't allow us to inherit from them. We need to redefine them.

	/*!
	 * Checks if this batch is equal to another batch.
	 *
	 * The batches are equal if the element at each position in the batch equals
	 * the element at the same position in the other batch. That is, the batches
	 * contain the same elements in the same order.
	 * \param other The batch to compare this batch to.
	 * \return ``true`` if the batches are equal, or ``false`` if they are
	 * inequal.
	 */
	bool operator ==(const BatchBase<Element>& other) const {
		return (*this) == static_cast<std::vector<Element>&>(other);
	}

	/*!
	 * Checks if this batch is inequal to another batch.
	 *
	 * The batches are inequal if there is any element in this batch that is not
	 * equal to the element in the same position in the other batch. They are
	 * inequal if the batches have different sizes, contain different elements
	 * or are in a different order.
	 * \param other The batch to compare this batch to.
	 * \return ``true`` if the batches are inequal, or ``false`` if they are
	 * equal.
	 */
	bool operator !=(const BatchBase<Element>& other) const {
		return (*this) != static_cast<std::vector<Element>&>(other);
	}

	/*!
	 * Checks if the batch is lexicographically before another batch.
	 * \param other The batch to compare this batch to.
	 * \return ``true`` if this batch occurs before the other batch in
	 * lexicographic order, or ``false`` if it is equal or occurs after the
	 * other batch.
	 */
	bool operator <(const BatchBase<Element>& other) const {
		return (*this) < static_cast<std::vector<Element>&>(other);
	}

	/*!
	 * Checks if the batch is not lexicographically after another batch.
	 * \param other The batch to compare this batch to.
	 * \return ``true`` if this batch is equal to or occurs before the other
	 * batch in lexicographic order, or ``false`` if it occurs after the other
	 * batch.
	 */
	bool operator <=(const BatchBase<Element>& other) const {
		return (*this) <= static_cast<std::vector<Element>&>(other);
	}

	/*!
	 * Checks if the batch is lexicographically after another batch.
	 * \param other The batch to compare this batch to.
	 * \return ``true`` if this batch occurs after the other batch in
	 * lexicographic order, or ``false``if it is equal or occurs before the
	 * other batch.
	 */
	bool operator >(const BatchBase<Element>& other) const {
		return (*this) > static_cast<std::vector<Element>&>(other);
	}

	/*!
	 * Checks if the batch is not lexicographically before another batch.
	 * \param other The batch to compare this batch to.
	 * \return ``true`` if this batch is equal to or occurs after the other
	 * batch in lexicographic order, or ``false`` if it occurs before the other
	 * batch.
	 */
	bool operator >=(const BatchBase<Element>& other) const {
		return (*this) >= static_cast<std::vector<Element>&>(other);
	}

	/*!
	 * Swap the contents of the batch with that of another batch.
	 *
	 * This swap is made by reference and can be executed in constant time,
	 * without needing to copy or move the individual elements of the batches.
	 * \param other The batch to swap contents with.
	 */
	void swap(BatchBase<Element>& other) noexcept {
		static_cast<std::vector<Element>&>(other).swap(*this);
	}
};

template<typename Element>
class SubbatchView; //Forward declare this view here so we can use it as template argument below, but implement it later so we can use BatchBase's fields.

/*!
 * This is a specialisation of the BatchBase class that handles batches of
 * batches more efficiently.
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
class BatchBase<BatchBase<Element>> : protected std::vector<SubbatchView<Element>> { //Specialise batches of batches.
	friend class SubbatchView<Element>; //Subbatches can access the coalesced data structure to get their own information.

public:
	//Many functions can be taken over directly from the underlying vector class.
	using iterator = typename std::vector<SubbatchView<Element>>::iterator;
	using const_iterator = typename std::vector<SubbatchView<Element>>::const_iterator;
	using reverse_iterator = typename std::vector<SubbatchView<Element>>::reverse_iterator;
	using const_reverse_iterator = typename std::vector<SubbatchView<Element>>::const_reverse_iterator;
	using std::vector<SubbatchView<Element>>::operator[];
	using std::vector<SubbatchView<Element>>::at;
	using std::vector<SubbatchView<Element>>::back;
	using std::vector<SubbatchView<Element>>::begin;
	using std::vector<SubbatchView<Element>>::capacity;
	using std::vector<SubbatchView<Element>>::cbegin;
	using std::vector<SubbatchView<Element>>::cend;
	using std::vector<SubbatchView<Element>>::crbegin;
	using std::vector<SubbatchView<Element>>::crend;
	using std::vector<SubbatchView<Element>>::data; //Gets the data pointing to the subbatch views, not the subelement data.
	using std::vector<SubbatchView<Element>>::empty;
	using std::vector<SubbatchView<Element>>::erase;
	using std::vector<SubbatchView<Element>>::end;
	using std::vector<SubbatchView<Element>>::front;
	using std::vector<SubbatchView<Element>>::max_size;
	using std::vector<SubbatchView<Element>>::pop_back;
	using std::vector<SubbatchView<Element>>::rbegin;
	using std::vector<SubbatchView<Element>>::rend;
	using std::vector<SubbatchView<Element>>::reserve;
	using std::vector<SubbatchView<Element>>::size;

	/*!
	 * Creates an empty batch.
	 */
	BatchBase() : next_position(0) {
		subelements.resize(8);
	}

	/*!
	 * Creates a batch with a number of copies of the same subbatch.
	 * \param The number of copies to store in this batch.
	 * \param value The subbatch to copy multiple times.
	 */
	BatchBase(const size_t count, const BatchBase<Element>& value = BatchBase<Element>()) : next_position(0) {
		subelements.resize(8);
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
	BatchBase(InputIterator first, InputIterator last) : next_position(0) {
		subelements.resize(8);
		assign(first, last);
	}

	/*!
	 * Copy constructor, creating a copy of the specified batch.
	 * \param other The batch to copy.
	 */
	BatchBase(const BatchBase<BatchBase<Element>>& other) : next_position(0) {
		subelements.resize(8);
		//Don't simply copy the subelements buffer and subbatchviews.
		//If we're copying the data anyway, we might as well shrink the subelements buffer to fit.
		//This not only improves performance of the copy, but also of any subsequent algorithms on the batch.
		assign(other.begin(), other.end()); //Assignment deals with proper reserving of memory, since batch iterators are random access.
	}

	/*!
	 * Move constructor, moving the batch to a different location in memory.
	 *
	 * Unlike the copy constructor, this move doesn't optimise the memory usage
	 * of the subelement data. The subelement data is not moved at all, and the
	 * memory usage can only be optimised by moving it, which would defeat the
	 * purpose of the move constructor.
	 * \param other The batch to move into this batch.
	 */
	BatchBase(BatchBase<BatchBase<Element>>&& other) {
		(*this) = std::move(other); //Same as move assignment operator.
	}

	/*!
	 * Constructs a batch of batches from a list of batches.
	 * \param initialiser_list An initialiser list containing subbatches to put
	 * in this batch.
	 */
	BatchBase(std::initializer_list<BatchBase<Element>> initialiser_list) : next_position(0) {
		subelements.resize(8);
		assign(initialiser_list.begin(), initialiser_list.end());
	}

	/*!
	 * Copy-assignment operator, which copies the contents of the given batch
	 * into this batch.
	 * \param other The batch to assign to this batch.
	 * \return A reference to this batch.
	 */
	BatchBase<BatchBase<Element>>& operator =(const BatchBase<BatchBase<Element>>& other) {
		subelements.resize(8);
		assign(other.begin(), other.end());
		return (*this);
	}

	/*!
	 * Move-assignment operator, which moves the data from the given batch into
	 * this batch.
	 *
	 * This will normally not make a full copy of the data. The data stored on
	 * the heap for the given batch will remain in place, but this batch will
	 * now refer to it instead. The other batch should no longer be used.
	 * \param other The batch to assign to this batch.
	 * \return A reference to this batch.
	 */
	BatchBase<BatchBase<Element>>& operator =(BatchBase<BatchBase<Element>>&& other) noexcept {
		std::vector<SubbatchView<Element>>::operator=(std::move(other));
		//Change all back-references to the batch-of-batches in all subbatches.
		for(SubbatchView<Element>& subbatch : *this) {
			subbatch.batch = this;
		}
		subelements = std::move(other.subelements);
		next_position = other.next_position;
		return (*this);
	}

	/*!
	 * Compares two batches of batches for equality.
	 *
	 * Two batches of batches are considered equal if all of their subbatches
	 * are pairwise equal. In turn, the subbatches are equal if each of their
	 * elements are pairwise equal.
	 * \param other The batch to compare with.
	 * \return ``true`` if the two batches of batches are equal, or ``false``
	 * otherwise.
	 */
	bool operator ==(const BatchBase<BatchBase<Element>>& other) const {
		return static_cast<const std::vector<SubbatchView<Element>>&>(*this) == static_cast<const std::vector<SubbatchView<Element>>&>(other); //The base vector knows its size and lets all subbatches compare too.
	}

	/*!
	 * Compares two batches of batches for inequality.
	 *
	 * Two batches of batches are considered inequal if at least one subbatch is
	 * different from the subbatch in the same position in the other batch. In
	 * turn, the subbatches are inequal if at least one subelement is different
	 * from the subelement in the same position in the other subbatch.
	 * \param other The batch to compare with.
	 * \return ``true`` if the two batches of batches are inequal, or ``false``
	 * otherwise.
	 */
	bool operator !=(const BatchBase<BatchBase<Element>>& other) const {
		return static_cast<const std::vector<SubbatchView<Element>>&>(*this) != static_cast<const std::vector<SubbatchView<Element>>&>(other);
	}

	/*!
	 * Compares the lexicographic order of this batch of batches with another.
	 *
	 * A batch is lexicographically less if its first subbatch is less than the
	 * first subbatch of the other batch. If the first elements are equal, the
	 * second subbatches are compared, and so on. If one batch is a prefix of
	 * another, the shorter batch is considered lesser. The subbatches are in
	 * turn also lexicographically compared.
	 * \param other The batch to compare with.
	 * \return ``true`` if this batch is lexicographically less than or equal to
	 * the other batch, or ``false`` if it is greater.
	 */
	bool operator <=(const BatchBase<BatchBase<Element>>& other) const {
		return static_cast<const std::vector<SubbatchView<Element>>&>(*this) <= static_cast<const std::vector<SubbatchView<Element>>&>(other);
	}

	/*!
	 * Compares the lexicographic order of this batch of batches with another.
	 *
	 * A batch is lexicographically less if its first subbatch is less than the
	 * first subbatch of the other batch. If the first elements are equal, the
	 * second subbatches are compared, and so on. If one batch is a prefix of
	 * another, the shorter batch is considered lesser. The subbatches are in
	 * turn also lexicographically compared.
	 * \param other The batch to compare with.
	 * \return ``true`` if this batch is lexicographically less than the other
	 * batch, or ``false`` if it is greater or equal.
	 */
	bool operator <(const BatchBase<BatchBase<Element>>& other) const {
		return static_cast<const std::vector<SubbatchView<Element>>&>(*this) < static_cast<const std::vector<SubbatchView<Element>>&>(other);
	}

	/*!
	 * Compares the lexicographic order of this batch of batches with another.
	 *
	 * A batch is lexicographically less if its first subbatch is less than the
	 * first subbatch of the other batch. If the first elements are equal, the
	 * second subbatches are compared, and so on. If one batch is a prefix of
	 * another, the shorter batch is considered lesser. The subbatches are in
	 * turn also lexicographically compared.
	 * \param other The batch to compare with.
	 * \return ``true`` if this batch is lexicographically greater than or equal
	 * to the other batch, or ``false`` if it is less.
	 */
	bool operator >=(const BatchBase<BatchBase<Element>>& other) const {
		return static_cast<const std::vector<SubbatchView<Element>>&>(*this) >= static_cast<const std::vector<SubbatchView<Element>>&>(other);
	}

	/*!
	 * Compares the lexicographic order of this batch of batches with another.
	 *
	 * A batch is lexicographically less if its first subbatch is less than the
	 * first subbatch of the other batch. If the first elements are equal, the
	 * second subbatches are compared, and so on. If one batch is a prefix of
	 * another, the shorter batch is considered lesser. The subbatches are in
	 * turn also lexicographically compared.
	 * \param other The batch to compare with.
	 * \return ``true`` if this batch is lexicographically greater than the
	 * other batch, or ``false`` if it is less or equal.
	 */
	bool operator >(const BatchBase<BatchBase<Element>>& other) const {
		return static_cast<const std::vector<SubbatchView<Element>>&>(*this) > static_cast<const std::vector<SubbatchView<Element>>&>(other);
	}

	/*!
	 * Replaces the contents of the batch with a number of copies of a specified
	 * subbatch.
	 * \param count The number of copies to store in this batch of batches.
	 * \param value The subbatch to store multiple copies of.
	 */
	void assign(const size_t count, const BatchBase<Element>& value) {
		reserve_subelements(count * std::max(size_t(1), value.size()));
		reserve(count);
		clear();

		for(size_t index = 0; index < count; ++index) {
			push_back_unsafe(value);
		}
	}

	/*!
	 * Replaces the contents of the batch with the contents of a specific range
	 * of subbatches.
	 *
	 * The implementation depends on the type of iterator used. Forward
	 * iterators greatly improve performance.
	 * \param first The position of the first element in the range to store in
	 * this batch.
	 * \param last An iterator indicating the end of the range to store in this
	 * batch.
	 * \tparam InputIterator This function works with any type of input
	 * iterator.
	 */
	template<class InputIterator>
	void assign(InputIterator first, InputIterator last) {
		clear(); //Clear any old contents.
		assign_iterator_dispatch<InputIterator>(first, last, typename std::iterator_traits<InputIterator>::iterator_category());
	}

	/*!
	 * Replace the contents of the batch with the elements from the given
	 * initialiser list.
	 * \param initialiser_list A list of subbatches to store in this batch of
	 * batches.
	 */
	void assign(const std::initializer_list<BatchBase<Element>> initialiser_list) {
		assign(initialiser_list.begin(), initialiser_list.end());
	}

	/*!
	 * Erase all contents from this batch of batches.
	 *
	 * All subbatches will be eliminated.
	 */
	void clear() noexcept {
		std::vector<SubbatchView<Element>>::clear();
		next_position = 0;
	}

	/*!
	 * Get a pointer to the internal data structure where this batch-of-batches
	 * stores the data of all subbatches.
	 *
	 * All of the elements of all subbatches is coalesced into a single data
	 * structure, to make them easier to move to compute devices all at once.
	 * This function gives a pointer to the start of the data in the
	 * subelements, which is necessary to access it directly.
	 * \return A pointer to the start of the subelement buffer.
	 */
	const Element* data_subelements() const {
		return subelements.data();
	}

	/*!
	 * Get a pointer to the internal data structure where this batch-of-batches
	 * stores the data of all subbatches.
	 *
	 * All of the elements of all subbatches is coalesced into a single data
	 * structure, to make them easier to move to compute devices all at once.
	 * This function gives a pointer to the start of the data in the
	 * subelements, which is necessary to access it directly.
	 * \return A pointer to the start of the subelement buffer.
	 */
	Element* data_subelements() {
		return subelements.data();
	}

	//Since we don't actually want to call the BatchBase constructor for subbatches, we have to re-implement all of its constructor overloads for emplace and emplace_back:

	/*!
	 * Insert an empty subbatch at the specified position in the batch of
	 * batches.
	 * \param position The position to insert the new subbatch.
	 * \return An iterator pointing to the newly emplaced subbatch.
	 */
	iterator emplace(const_iterator position) {
		reserve_subelements_doubling(next_position + 1);
		return std::vector<SubbatchView<Element>>::emplace(position, *this, next_position, 0, 1);
		next_position += 1;
	}

	/*!
	 * Insert a new subbatch at the specified position, filled with a number of
	 * copies of a value.
	 * \param position The position to insert the new subbatch.
	 * \param count The number of subelements to fill the new subbatch with.
	 * \param value The subelement to copy multiple times into the new subbatch.
	 * \return An iterator pointing to the newly emplaced subbatch.
	 */
	iterator emplace(const_iterator position, const size_t count, const Element& value = Element()) {
		const size_t capacity = std::max(size_t(1), count);
		reserve_subelements_doubling(next_position + capacity);
		const iterator result = std::vector<SubbatchView<Element>>::emplace(position, *this, next_position, 0, capacity);
		next_position += capacity;
		result->assign(count, value);
		return result;
	}

	/*!
	 * Insert a new subbatch at the specified position, filled with the contents
	 * of a specified range of elements.
	 * \param position The position to insert the new subbatch.
	 * \param first The start of a range of elements to place in the subbatch.
	 * \param last An iterator signalling the end of a range of elements to
	 * place in the subbatch.
	 * \return An iterator pointing to the newly emplaced subbatch.
	 * \tparam InputIterator This function works with any type of iterator.
	 */
	template<class InputIterator>
	iterator emplace(const_iterator position, InputIterator first, InputIterator last) {
		reserve_subelements_doubling(next_position + 1);
		const iterator result = std::vector<SubbatchView<Element>>::emplace(position, *this, next_position, 0, 1);
		next_position += 1; //Capacity is 1.
		result->assign(first, last);
		return result;
	}

	/*!
	 * Insert a new subbatch at the specified position, filled with a copy of
	 * the specified batch.
	 * \param position The position to insert the new subbatch.
	 * \param original The batch to copy into the new subbatch.
	 * \return An iterator pointing to the newly emplaced subbatch.
	 */
	iterator emplace(const_iterator position, const BatchBase<Element>& original) {
		return insert(position, original); //We're just going to have to make a complete copy here.
	}

	/*!
	 * Move the specified subbatch into the specified position in this batch.
	 * \param position The position to insert the subbatch.
	 * \param original The batch to move into this batch.
	 * \return An iterator pointing to the newly emplaced subbatch.
	 */
	iterator emplace(const_iterator position, BatchBase<Element>&& original) {
		return insert(position, original); //We're just going to have to move the whole thing here.
	}

	/*!
	 * Insert a new subbatch at the specified position, filled with the contents
	 * of the specified list.
	 * \param position The position to insert the subbatch.
	 * \param initialiser_list The list of elements to fill the new subbatch
	 * with.
	 * \return An iterator pointing to the newly emplaced subbatch.
	 */
	iterator emplace(const_iterator position, const std::initializer_list<Element>& initialiser_list) {
		const size_t capacity = std::max(size_t(1), initialiser_list.size());
		reserve_subelements_doubling(next_position + capacity);
		const iterator result = std::vector<SubbatchView<Element>>::emplace(position, *this, next_position, 0, capacity);
		next_position += capacity;
		result->assign(initialiser_list);
		return result;
	}

	/*!
	 * Append an empty subbatch at the end of this batch of batches.
	 */
	void emplace_back() {
		reserve_subelements_doubling(next_position + 1);
		std::vector<SubbatchView<Element>>::emplace_back(*this, next_position, 0, 1);
		next_position += 1;
	}

	/*!
	 * Append a new subbatch at the end of this batch of batches, filled with a
	 * number of copies of a value.
	 * \param count The number of subelements to fill the new subbatch with.
	 * \param value The element to copy multiple times into the new subbatch.
	 */
	void emplace_back(const size_t count, const Element& value = Element()) {
		const size_t capacity = std::max(size_t(1), count);
		reserve_subelements_doubling(next_position + capacity);
		std::vector<SubbatchView<Element>>::emplace_back(*this, next_position, 0, capacity);
		next_position += capacity;
		back().assign(count, value);
	}

	/*!
	 * Append a new subbatch at the end of this batch of batches, filled with
	 * the contents of a specified range of elements.
	 * \param first The start of a range of elements to place in the subbatch.
	 * \param last An iterator signalling the end of a range of elements to
	 * place in the subbatch.
	 * \tparam InputIterator This function works with any type of iterator.
	 */
	template<class InputIterator>
	void emplace_back(InputIterator first, InputIterator last) {
		reserve_subelements_doubling(next_position + 1);
		std::vector<SubbatchView<Element>>::emplace_back(*this, next_position, 0, 1);
		next_position += 1;
		back().assign(first, last);
	}

	/*!
	 * Append a new subbatch at the end of this batch, filled with a copy of the
	 * specified batch.
	 * \param original The batch to copy into the new subbatch.
	 */
	void emplace_back(const BatchBase<Element>& original) {
		push_back(original);
	}

	/*!
	 * Move the specified subbatch to the end of this batch.
	 * \param The batch to move into this batch.
	 */
	void emplace_back(BatchBase<Element>&& original) {
		push_back(original);
	}

	/*!
	 * Append a new subbatch to the end of this batch, filled with the contents
	 * of the specified list.
	 * \param initialiser_list The list of elements to fill the new subbatch
	 * with.
	 */
	void emplace_back(const std::initializer_list<Element>& initialiser_list) {
		const size_t capacity = initialiser_list.size();
		reserve_subelements_doubling(next_position + capacity);
		std::vector<SubbatchView<Element>>::emplace_back(*this, next_position, 0, capacity);
		next_position += capacity;
		back().assign(initialiser_list);
	}

	/*!
	 * Insert a new batch at the specified position in this batch of batches.
	 *
	 * All subsequent batches are moved by one place to make space for the newly
	 * inserted batch. This does not cause the actual element data to move for
	 * those batches.
	 * \param position The position in this batch to insert the new subbatch.
	 * The new batch is inserted before this position.
	 * \param value The new subbatch to insert in this batch of batches.
	 * \return An iterator pointing to the newly inserted subbatch.
	 */
	iterator insert(const_iterator position, const BatchBase<Element>& value) {
		const size_t capacity = std::max(size_t(1), value.size()); //Capacity of the subbatch needs to be at least 1.
		reserve_subelements_doubling(next_position + capacity); //Grow by doubling to reduce amortised cost of repeated push_backs.
		SubbatchView<Element> subbatch(*this, next_position, 0, capacity); //Create a subbatch pointing to the new data.
		next_position += capacity;
		subbatch.assign(value.begin(), value.end()); //Insert the data into that subbatch.
		return std::vector<SubbatchView<Element>>::insert(position, subbatch);
	}

	/*!
	 * Move a batch to the specified position in this batch of batches.
	 *
	 * All subsequent batches are moved by one place to make space for the newly
	 * inserted batch. This does not cause the actual element data to move for
	 * those batches.
	 * \param position The position in this batch to insert the subbatch. The
	 * subbatch is inserted before this position.
	 * \param value The subbatch to insert into this batch of batches.
	 * \return An iterator pointing to the newly inserted subbatch.
	 */
	iterator insert(const_iterator position, BatchBase<Element>&& value) {
		const size_t capacity = std::max(size_t(1), value.size()); //Capacity of the subbatch needs to be at least 1.
		reserve_subelements_doubling(next_position + capacity); //Grow by doubling to reduce amortised cost of repeated push_backs.
		SubbatchView<Element> subbatch(*this, next_position, 0, capacity); //Create a subbatch pointing to the new data.
		next_position += capacity;

		//Move the data into that subbatch.
		for(Element& subelement : value) {
			subbatch.push_back(std::move(subelement));
		}

		return std::vector<SubbatchView<Element>>::insert(position, subbatch);
	}

	/*!
	 * Insert repeated copies of a subbatch in a specified position in this
	 * batch of batches.
	 *
	 * All subsequent batches are moved to make place for the newly inserted
	 * subbatches. This does not cause the actual element data to move for those
	 * batches.
	 * \param position The position in this batch to insert the subbatches. The
	 * subbatches are inserted before this position.
	 * \param count How often to repeat the subbatch here.
	 * \param value The subbatch to insert repeatedly into this batch of
	 * batches.
	 * \return An iterator pointing to the first of the newly inserted
	 * subbatches.
	 */
	iterator insert(const_iterator position, const size_t count, const BatchBase<Element>& value) {
		const size_t capacity = std::max(size_t(1), value.size()); //Capacity per subbatch needs to be at least 1.
		reserve_subelements(next_position + capacity * count);

		//Since we can't directly adjust the size of the views list, we'll have to insert repeated counts of the first view and adjust its fields afterwards.
		const size_t first_index = position - cbegin(); //Insert may invalidate the iterator we give it, so use indices afterwards.
		iterator result = std::vector<SubbatchView<Element>>::insert(position, count, SubbatchView(*this, next_position, 0, capacity));
		for(size_t repeat = 0; repeat < count; ++repeat) {
			SubbatchView<Element>& new_subbatch = (*this)[first_index + repeat];
			new_subbatch.start_index = next_position + repeat * capacity;
			new_subbatch.assign(value.begin(), value.end());
		}
		next_position += count * capacity;
		return result;
	}

	/*!
	 * Insert a range of subbatches at a specified position into this batch of
	 * batches.
	 *
	 * All subsequent batches are moved to make place for the newly inserted
	 * subbatches. This does not cause the actual element data to move for these
	 * batches.
	 * \param position The position in this batch to insert the subbatches. The
	 * subbatches are inserted before this position.
	 * \param first The first of the range of subbatches to insert.
	 * \param last An iterator signalling the end of the range of subbatches to
	 * insert.
	 * \return An iterator pointing to the first of the newly inserted
	 * subbatches.
	 * \tparam InputIterator This function works with any type of input
	 * iterator.
	 */
	template<class InputIterator>
	iterator insert(const_iterator position, InputIterator first, InputIterator last) {
		return insert_iterator_dispatch<InputIterator>(position, first, last, typename std::iterator_traits<InputIterator>::iterator_category());
	}

	/*!
	 * Insert a list of subbatches at a specified position into this batch of
	 * batches.
	 *
	 * All subsequent batches are moved to make place for the newly inserted
	 * subbatches. This does not cause the actual element data to move for these
	 * subbatches.
	 * \param position The position in this batch to insert the subbatches. The
	 * subbatches are inserted before this position.
	 * \param initialiser_list A list of subbatches to insert into this batch.
	 * \return An iterator pointing to the first of the newly inserted
	 * subbatches.
	 */
	iterator insert(const_iterator position, const std::initializer_list<BatchBase<Element>>& initialiser_list) {
		return insert(position, initialiser_list.begin(), initialiser_list.end());
	}

	/*!
	 * Add a new subbatch to the end of this batch of batches.
	 *
	 * The batch is copied into this batch of batches, and added to the end.
	 * \param value The batch to append to this batch of batches.
	 */
	void push_back(const BatchBase<Element>& value) {
		reserve_subelements_doubling(next_position + std::max(size_t(1), value.size())); //Grow by doubling to reduce amortised cost of repeated push_backs.
		push_back_unsafe(value);
	}

	/*!
	 * Add a new subbatch to the end of this batch of batches.
	 *
	 * The batch itself needs to be copied because it is merged into the element
	 * buffer of this batch of batches. However the individual elements in the
	 * batch will be moved. If their move constructors are more efficient than
	 * their copy constructors, this might save some data copies. For plain old
	 * data, this will have no benefit over the copy-overload of this function.
	 * \param value The batch to append to this batch of batches.
	 */
	void push_back(BatchBase<Element>&& value) {
		reserve_subelements_doubling(next_position + std::max(size_t(1), value.size())); //Grow by doubling to reduce amortised cost of repeated push_backs.
		push_back_unsafe(value);
	}

	/*!
	 * Add a new subbatch to the end of this batch of batches.
	 *
	 * The batch is copied into this batch of batches, and added to the end.
	 * \param value The batch to append to this batch of batches.
	 */
	void push_back(const SubbatchView<Element>& value) {
		reserve_subelements_doubling(next_position + std::max(size_t(1), value.size())); //Grow by doubling to reduce amortised cost of repeated push_backs.
		push_back_unsafe(value);
	}

	/*!
	 * Add a new subbatch to the end of this batch of batches.
	 *
	 * The batch itself needs to be copied because it is merged into the element
	 * buffer of this batch of batches. However the individual elements in the
	 * batch will be moved. If their move constructors are more efficient than
	 * their copy constructors, this might save some data copies. For plain old
	 * data, this will have no benefit over the copy-overload of this function.
	 * \param value The batch to append to this batch of batches.
	 */
	void push_back(SubbatchView<Element>&& value) {
		reserve_subelements_doubling(next_position + std::max(size_t(1), value.size())); //Grow by doubling to reduce amortised cost of repeated push_backs.
		push_back_unsafe(value);
	}

	/*!
	 * Reserve space for at least this many subelements to be stored without
	 * needing to reallocate them.
	 *
	 * If the current capacity is already enough to store that many subelements,
	 * this does nothing. If the capacity is insufficient, the data is moved to
	 * a bigger container such that the buffer is capable of holding that many
	 * subelements for the subbatches of this batch. In that case, iterators or
	 * pointers pointing to the subelements will be invalidated.
	 */
	void reserve_subelements(const size_t count) {
		if(subelements.size() < count) {
			subelements.resize(count);
		}
	}

	/*!
	 * Resize this batch such that it contains a specified number of subbatches.
	 *
	 * If the size is increased, empty subbatches will be appended to the end to
	 * pad the size of the batch of batches. If the size is reduced, subbatches
	 * at the end will be erased.
	 * \param count The new size of the batch of batches.
	 */
	void resize(const size_t count) {
		if(count < size()) {
			std::vector<SubbatchView<Element>>::resize(count, SubbatchView<Element>(*this, next_position, 0, 1));
		} else if(count > size()) { //If the size increases, we need to allocate memory in the subelements and assign spots for each subbatch.
			reserve(count);
			reserve_subelements(next_position + count - size());
			while(size() < count) {
				std::vector<SubbatchView<Element>>::emplace_back(*this, next_position, 0, 1);
				next_position += 1;
			}
		}
	}

	/*!
	 * Resize this batch such that it contains a specified number of subbatches.
	 *
	 * If the size is increased, the specified subbatch will be copied a number
	 * of times to pad the size of the batch of batches. If the size is reduced,
	 * subbatches at the end will be erased.
	 * \param count The new size of the batch of batches.
	 * \param value The subbatch to repeatedly copy at the end to pad the size
	 * of the batch.
	 */
	void resize(const size_t count, const BatchBase<Element>& value) {
		if(count < size()) {
			std::vector<SubbatchView<Element>>::resize(count, SubbatchView<Element>(*this, next_position, 0, 1));
		} else if(count > size()) {
			reserve(count);
			reserve_subelements(next_position + (count - size()) * std::max(size_t(1), value.size()));
			while(size() < count) {
				push_back_unsafe(value); //Append a number of copies of the given subbatch.
			}
		}
	}

	/*!
	 * Attempts to reduce memory usage by to the minimum possible by rearranging
	 * the data inside of the buffers held by this batch.
	 *
	 * The external behaviour of this batch is unchanged. Elements are not
	 * actually rearranged as far as can be seen by accessing the batch and
	 * batches. However pointers and iterators referring to subbatches or
	 * subelements will be invalidated.
	 *
	 * This function has three intended outcomes:
	 * - All subbatches will have their capacity shrunk to their size, occupying
	 *   a minimal amount of space in the subelement buffer. The minimum
	 *   capacity still holds though and is the only exception.
	 * - All subelements are rearranged so that each range is in the order again
	 *   in which it appears in the subbatch list. This improves memory locality
	 *   which improves practical performance, especially in the GPU.
	 * - The list of subbatches itself and the element buffer are shrunk to fit
	 *   the data. This actually frees up memory usage.
	 */
	void shrink_to_fit() {
		//Figure out how much space to allocate for our optimised buffer.
		const size_t num_subelements = std::accumulate(cbegin(), cend(), size_t(0), [](const size_t current, const SubbatchView<Element>& subbatch) {
			return current + std::max(subbatch.size(), size_t(1));
		});
		std::vector<Element> optimised(std::max(num_subelements, size_t(8))); //Allocate the necessary memory. We'll move this on top of our old buffer once all data is moved.

		//Move the data into the optimised buffer.
		size_t optimised_position = 0; //Position in the optimised buffer where to place next subelement.
		for(SubbatchView<Element>& subbatch : *this) { //Iterate over subbatches in order, so that they'll appear in order in the new buffer.
			for(size_t i = 0; i < subbatch.size(); ++i) {
				optimised[optimised_position + i] = std::move(subelements[subbatch.start_index + i]);
			}
			subbatch.start_index = optimised_position;
			subbatch.current_capacity = std::max(subbatch.size(), size_t(1));
			optimised_position += subbatch.capacity();
		}

		//Once optimised, swap out the new subelement buffer with the old one.
		subelements = std::move(optimised);
		next_position = optimised_position;

		std::vector<SubbatchView<Element>>::shrink_to_fit(); //Also shrink the table of subbatches.
	}

	/*!
	 * Get the total range of the subelements that are contained in the
	 * subbatches of this batch.
	 *
	 * Subelements are scattered in the subelement data referred to by
	 * \ref data_subelements since there can be free spaces leftover from
	 * removing subelements or subbatches. This function returns the position
	 * past the last slot of subelements that is actually occupied.
	 *
	 * When starting an iteration from \ref data_subelements for this many
	 * steps, it is guaranteed that all subelements will have been seen, but
	 * also some elements which were not actually valid data. Which elements are
	 * valid and which are not can be derived from the subbatch views. This size
	 * can thus be greater than the total number of subelements if you were to
	 * sum all sizes of the subbatches, but will never be smaller.
	 */
	size_t size_subelements() const {
		return next_position; //The next_position indicates the first open space we have, thus also the end of the occupied and formerly occupied spaces.
	}

	/*!
	 * Exchange the contents of this batch of batches with that of another.
	 *
	 * For this type of swap, the element data itself doesn't need to move.
	 * Iterators and pointers to the element and views data will remain valid.
	 * \param other The batch of batches to swap with.
	 */
	void swap(BatchBase<BatchBase<Element>>& other) noexcept {
		std::swap(subelements, other.subelements);
		std::swap(next_position, other.next_position);
		std::vector<SubbatchView<Element>>::swap(static_cast<std::vector<SubbatchView<Element>>&>(other)); //Swap all the views on that data too.
		for(SubbatchView<Element>& subbatch : *this) { //Update the pointers to the parent batch in each subbatch.
			subbatch.batch = this;
		}
		for(SubbatchView<Element>& subbatch : other) {
			subbatch.batch = &other;
		}
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
	void assign_iterator_dispatch(const InputIterator begin, const InputIterator end, const std::random_access_iterator_tag) {
		const size_t new_size = end - begin;
		reserve(new_size);

		//Count the total number of subelements too, to reserve enough memory there all at once.
		size_t subelements_size = 0;
		for(InputIterator iterator = begin; iterator != end; iterator++) {
			subelements_size += std::max(size_t(1), iterator->size()); //At least 1 for every subbatch.
		}
		reserve_subelements(subelements_size);

		for(InputIterator it = begin; it != end; it++) {
			push_back_unsafe(*it);
		}
	}

	/*!
	 * Specialised assign operation for when assigning a range defined by
	 * forward iterators.
	 *
	 * Forward iterators have the property that they can safely be iterated over
	 * multiple times. This helps to determine the size of the range before
	 * copying the contents, which can reduce the number of times the memory for
	 * the subbatches or their elements needs to be copied.
	 * \param begin The first element of the range to assign to this batch.
	 * \param end An iterator marking the end of the range to assign to this
	 * batch.
	 * \tparam InputIterator This function works with any type of forward
	 * iterator.
	 */
	template<class InputIterator>
	void assign_iterator_dispatch(const InputIterator begin, const InputIterator end, const std::forward_iterator_tag) {
		size_t new_size = 0;
		size_t subelements_size = 0;
		//Iterate over the range once to determine its size and the number of subelements, to reserve enough memory there all at once.
		for(InputIterator it = begin; it != end; it++) {
			++new_size;
			subelements_size += std::max(size_t(1), it->size()); //At least 1 for every subbatch.
		}
		reserve(new_size);
		reserve_subelements(subelements_size);

		//Then a second time to actually assign the contents.
		for(InputIterator it = begin; it != end; it++) {
			push_back_unsafe(*it);
		}
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
	void assign_iterator_dispatch(const InputIterator begin, const InputIterator end, const std::input_iterator_tag) {
		for(InputIterator it = begin; it != end; it++) {
			push_back(*it); //Simply append all of them. This automatically implements the amortised reallocation.
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
		const size_t index = position - cbegin(); //Convert input position to index immediately, since inserting may invalidate iterators.
		const size_t subbatch_count = end - start;
		if(subbatch_count == 0) { //We'll use the first element in special-casing below, which would fail if the range is empty. So early out before then.
			return begin() + index;
		}
		//Measure total number of subelements to prevent having to reallocate the element buffer multiple times.
		size_t subelement_count = 0;
		for(InputIterator it = start; it != end; it++) {
			subelement_count += std::max(size_t(1), it->size()); //At least 1 for every subbatch.
		}
		reserve_subelements(next_position + subelement_count);

		//Since we can't directly adjust the size of the views list, we'll have to insert repeated counts of the first view and adjust its fields afterwards.
		const iterator result = std::vector<SubbatchView<Element>>::insert(position, subbatch_count, SubbatchView(*this, next_position, 0, std::max(size_t(1), start->size())));
		iterator subbatch = begin() + index;
		for(InputIterator it = start; it != end; it++) {
			subbatch->start_index = next_position;
			subbatch->current_capacity = std::max(size_t(1), it->size());
			next_position += std::max(size_t(1), it->size());
			subbatch->assign(it->begin(), it->end()); //Copy data from original batch into subelement array.
			subbatch++;
		}
		return result;
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
		const size_t index = position - cbegin(); //Convert input position to index immediately, since inserting may invalidate iterators.
		if(start == end) { //We'll use the first element in special-casing below, which would fail if the range is empty. So early out before then.
			return begin() + index;
		}
		//Measure total number of subbatches and subelements to prevent having to reallocate the element buffer multiple times.
		size_t subbatch_count = 0;
		size_t subelement_count = 0;
		for(InputIterator it = start; it != end; it++) {
			subbatch_count++;
			subelement_count += std::max(size_t(1), it->size());
		}
		reserve_subelements(next_position + subelement_count);

		//Since we can't directly adjust the size of the views list, we'll have to insert repeated counts of the first view and adjust its fields afterwards.
		iterator result = std::vector<SubbatchView<Element>>::insert(position, subbatch_count, SubbatchView(*this, next_position, 0, std::max(size_t(1), start->size())));
		iterator subbatch = begin() + index;
		for(InputIterator it = start; it != end; it++) {
			subbatch->start_index = next_position;
			next_position += std::max(size_t(1), it->size());
			subbatch->assign(it->begin(), it->end()); //Copy data from original batch into subelement array.
			subbatch++;
		}
		return result;
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
		/* The actual subelement data can simply be appended at the end rather
		than inserted in the middle. For the views however we don't know how
		many will be inserted and we don't have control to place elements
		outside of bounds but within the capacity like we do for
		SubbatchView::insert_iterator_dispatch. So instead take a simpler
		approach: Keep the views in a separate array and then insert them all at
		once once iteration is completed. */

		std::vector<SubbatchView<Element>> views;
		for(; start != end; start++) {
			const size_t capacity = std::max(size_t(1), start->size());
			views.emplace_back(*this, next_position, 0, capacity);
			reserve_subelements_doubling(next_position + capacity);
			next_position += capacity;
			views.back().assign(start->begin(), start->end()); //Copy data from the original batch into subelement array.
		}
		//Now insert the new views directly into the views array.
		return std::vector<SubbatchView<Element>>::insert(position, views.begin(), views.end());
	}

	/*!
	 * Appends a copy of an element without checking for capacity in either the
	 * subbatch view list or the subelement buffer.
	 *
	 * Use this function only after having checked or reserved enough memory to
	 * contain a new element in the subbatch list, and the size of the subbatch
	 * in the subelement list.
	 * \param subbatch The subbatch to append to this batch of batches.
	 */
	void push_back_unsafe(const BatchBase<Element>& subbatch) {
		const size_t capacity = std::max(size_t(1), subbatch.size());
		std::vector<SubbatchView<Element>>::emplace_back(*this, next_position, 0, capacity); //Create a new subbatch with exactly enough capacity.
		next_position += capacity;
		back().assign(subbatch.begin(), subbatch.end()); //Copy all subelements.
	}

	/*!
	 * Moves the subbatch to the end of this batch of batches without checking
	 * for capacity in either the subbatch view list or the subelement buffer.
	 *
	 * Use this function only after having checked or reserved enough memory to
	 * contain a new element in the subbatch list, and the size of the subbatch
	 * in the subelement list.
	 * \param subbatch The subbatch to move to the end of this batch of batches.
	 */
	void push_back_unsafe(BatchBase<Element>&& subbatch) {
		const size_t capacity = std::max(size_t(1), subbatch.size());
		std::vector<SubbatchView<Element>>::emplace_back(*this, next_position, subbatch.size(), capacity); //Create a new subbatch with exactly enough capacity.
		next_position += capacity;
		SubbatchView<Element>& new_subbatch = back();
		for(size_t subelement = 0; subelement < subbatch.size(); ++subelement) {
			new_subbatch[subelement] = std::move(subbatch[subelement]); //Move every element individually to the new place. Might be a copy in the case of plain-old-data.
		}
	}

	/*!
	 * Appends a copy of an element without checking for capacity in either the
	 * subbatch view list or the subelement buffer.
	 *
	 * Use this function only after having checked or reserved enough memory to
	 * contain a new element in the subbatch list, and the size of the subbatch
	 * in the subelement list.
	 * \param subbatch The subbatch to append to this batch of batches.
	 */
	void push_back_unsafe(const SubbatchView<Element>& subbatch) {
		const size_t capacity = std::max(size_t(1), subbatch.size());
		std::vector<SubbatchView<Element>>::emplace_back(*this, next_position, 0, capacity); //Create a new subbatch with exactly enough capacity.
		next_position += capacity;
		back().assign(subbatch.begin(), subbatch.end()); //Copy all subelements.
	}

	/*!
	 * Moves the subbatch to the end of this batch of batches without checking
	 * for capacity in either the subbatch view list or the subelement buffer.
	 *
	 * Use this function only after having checked or reserved enough memory to
	 * contain a new element in the subbatch list, and the size of the subbatch
	 * in the subelement list.
	 * \param subbatch The subbatch to move to the end of this batch of batches.
	 */
	void push_back_unsafe(SubbatchView<Element>&& subbatch) {
		const size_t capacity = std::max(size_t(1), subbatch.size());
		std::vector<SubbatchView<Element>>::emplace_back(*this, next_position, subbatch.size(), capacity); //Create a new subbatch with exactly enough capacity.
		next_position += capacity;
		SubbatchView<Element>& new_subbatch = back();
		for(size_t subelement = 0; subelement < subbatch.size(); ++subelement) {
			new_subbatch[subelement] = std::move(subbatch[subelement]); //Move every element individually to the new place. Might be a copy in the case of plain-old-data.
		}
	}

	/*!
	 * Make sure that the subelements buffer can contain at least a certain
	 * number of subelements, but do so by doubling the subelement buffer in
	 * size.
	 *
	 * This has the advantage that if this reserve function is called frequently
	 * it will infrequently actually have to allocate new memory. If new memory
	 * is needed a linear amount of time, each allocation will on average take
	 * constant time.
	 */
	void reserve_subelements_doubling(const size_t minimum_capacity) {
		size_t buffer_size = subelements.size();
		if(buffer_size >= minimum_capacity) {
			return; //No reallocation necessary.
		}
		while(buffer_size < minimum_capacity) { //Not enough capacity in the element buffer.
			buffer_size = buffer_size * 2;
		}
		subelements.resize(buffer_size); //Resize all at once.
	}
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
	friend class BatchBase<BatchBase<Element>>; //The batch base class can modify members, e.g. for move assignment.
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
	SubbatchView(BatchBase<BatchBase<Element>>& batch, const size_t start_index, const size_t size, const size_t capacity) :
			batch(&batch),
			start_index(start_index),
			num_elements(size),
			current_capacity(capacity) {};

	/*!
	 * Return the element in a specified position in the subbatch.
	 * \param index The position of the element to retrieve.
	 * \return The element in the specified position of the subbatch.
	 */
	const Element& operator [](const size_t index) const {
		return batch->subelements[start_index + index];
	}

	/*!
	 * Return the element in a specified position in the subbatch.
	 * \param index The position of the element to retrieve.
	 * \return The element in the specified position of the subbatch.
	 */
	Element& operator [](const size_t index) {
		return batch->subelements[start_index + index];
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
	bool operator ==(const BatchBase<Element>& other) const {
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
	bool operator !=(const BatchBase<Element>& other) const {
		return !((*this) == other); //Inverse of equality operator.
	}

	/*!
	 * Checks if this subbatch is lexicographically less than the other batch.
	 *
	 * This compares the elements of both batches element-wise. So it compares
	 * the first elements of each batch, and returns if one is different from
	 * the other. If they are equal, the second elements are compared on each
	 * batch, and so on. If one batch is a prefix of the other, the shorter
	 * batch is considered lexicographically less.
	 * \param other The batch to compare this subbatch to.
	 */
	bool operator <(const BatchBase<Element>& other) const {
		const size_t smallest_size = std::min(size(), other.size());
		for(size_t index = 0; index < smallest_size; ++index) {
			if((*this)[index] < other[index]) {
				return true;
			} else if((*this)[index] > other[index]) {
				return false;
			}
			//Else, check the next element.
		}
		//One batch is a prefix of the other. So the shorter one is less.
		return size() < other.size();
	}

	/*!
	 * Checks if this subbatch is lexicographically less than the other
	 * subbatch.
	 *
	 * This compares the elements of both subbatches element-wise. So it
	 * compares the first elements of each subbatch, and returns if one is
	 * different from the other. If they are equal, the second elements are
	 * compared on each subbatch, and so on. If one subbatch is a prefix of the
	 * other, the shorter subbatch is considered lexicographically less.
	 * \param other The subbatch to compare this subbatch to.
	 */
	bool operator <(const SubbatchView<Element>& other) const {
		const size_t smallest_size = std::min(size(), other.size());
		for(size_t index = 0; index < smallest_size; ++index) {
			if((*this)[index] < other[index]) {
				return true;
			} else if((*this)[index] > other[index]) {
				return false;
			}
			//Else, check the next element.
		}
		//One batch is a prefix of the other. So the shorter one is less.
		return size() < other.size();
	}

	/*!
	 * Checks if this subbatch is lexicographically less than or equal to the
	 * other batch.
	 *
	 * This compares the elements of both batches element-wise. So it compares
	 * the first elements of each batch, and returns if one is different from
	 * the other. If they are equal, the second elements are compared on each
	 * batch, and so on. If one batch is a prefix of the other, the shorter
	 * batch is considered lexicographically less.
	 * \param other The batch to compare this subbatch to.
	 */
	bool operator <=(const BatchBase<Element>& other) const {
		const size_t smallest_size = std::min(size(), other.size());
		for(size_t index = 0; index < smallest_size; ++index) {
			if((*this)[index] < other[index]) {
				return true;
			} else if((*this)[index] > other[index]) {
				return false;
			}
			//Else, check next element.
		}
		//One batch is a prefix of the other. So the shorter one is less, or they could be equal.
		return size() <= other.size();
	}

	/*!
	 * Checks if this subbatch is lexicographically less than or equal to the
	 * other subbatch.
	 *
	 * This compares the elements of both subbatches element-wise. So it
	 * compares the first elements of each subbatch, and returns if one is
	 * different from the other. If they are equal, the second elements are
	 * compared on each subbatch, and so on. If one subbatch is a prefix of the
	 * other, the shorter subbatch is considered lexicographically less.
	 * \param other The subbatch to compare this subbatch to.
	 */
	bool operator <=(const SubbatchView<Element>& other) const {
		const size_t smallest_size = std::min(size(), other.size());
		for(size_t index = 0; index < smallest_size; ++index) {
			if((*this)[index] < other[index]) {
				return true;
			} else if((*this)[index] > other[index]) {
				return false;
			}
			//Else, check next element.
		}
		//One batch is a prefix of the other. So the shorter one is less, or they could be equal.
		return size() <= other.size();
	}

	/*!
	 * Checks if this subbatch is lexicographically greater than the other
	 * batch.
	 *
	 * It is greater if it is not lexicographically less or equal.
	 * \param other The batch to compare this subbatch to.
	 */
	bool operator >(const BatchBase<Element>& other) const {
		return !(*this <= other); //Don't just swap them, because the other batch has a different type, and we wouldn't want a recursive loop that way.
	}

	/*!
	 * Checks if this subbatch is lexicographically greater than the other
	 * subbatch.
	 *
	 * It is greater if it is not lexicographically less or equal.
	 * \param other The subbatch to compare this subbatch to.
	 */
	bool operator >(const SubbatchView<Element>& other) const {
		return !(*this <= other); //Don't just swap them, because the other batch has a different type, and we wouldn't want a recursive loop that way.
	}

	/*!
	 * Checks if this subbatch is lexicographically greater than or equal to the
	 * other batch.
	 *
	 * It is greater or equal if it is not lexicographically less.
	 * \param other The batch to compare this subbatch to.
	 */
	bool operator >=(const BatchBase<Element>& other) const {
		return !(*this < other); //Don't just swap them, because the other batch has a different type, and we wouldn't want a recursive loop that way.
	}

	/*!
	 * Checks if this subbatch is lexicographically greater than or equal to the
	 * other subbatch.
	 *
	 * It is greater or equal if it is not lexicographically less.
	 * \param other The subbatch to compare this subbatch to.
	 */
	bool operator >=(const SubbatchView<Element>& other) const {
		return !(*this < other); //Don't just swap them, because the other batch has a different type, and we wouldn't want a recursive loop that way.
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
		return batch->subelements.begin() + start_index;
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
		return batch->subelements.begin() + start_index;
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
		if(batch->subelements.empty()) {
			return batch->subelements.data(); //Do whatever they do.
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
		if(batch->subelements.empty()) {
			return batch->subelements.data(); //Do whatever they do.
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
			reallocate(capacity() * 2); //Double the capacity, so that repeated emplacing executes in amortised linear time.
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
			reallocate(capacity() * 2); //Double in size, so that repeated emplacing executes in amortised constant time.
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
		return batch->subelements.begin() + (start_index + size());
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
		return batch->subelements.begin() + (start_index + size());
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
			reallocate(capacity() * 2); //Double the capacity, so that repeated inserting executes in amortised linear time.
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
			reallocate(capacity() * 2); //Double the capacity, so that repeated inserting executes in amortised linear time.
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
		return batch->subelements.max_size(); //Should really subtract the total size of all other subbatches in the batch, but that would be linear.
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
			reallocate(capacity() * 2); //Doubling the capacity (or multiplying with any scalar, really) guarantees an amortised constant execution time.
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
			reallocate(capacity() * 2); //Doubling the capacity (or multiplying with any scalar, really) guarantees an amortised constant execution time.
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
		const_reverse_iterator beginning = batch->subelements.rbegin(); //So pointing to the very last element in the buffer.
		std::advance(beginning, batch->subelements.size() - start_index - size());
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
		reverse_iterator beginning = batch->subelements.rbegin(); //So pointing to the very last element in the buffer.
		std::advance(beginning, batch->subelements.size() - start_index - size());
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
		const_reverse_iterator ending = batch->subelements.rbegin(); //So pointing to the very last element in the buffer.
		std::advance(ending, batch->subelements.size() - start_index);
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
		reverse_iterator ending = batch->subelements.rbegin(); //So pointing to the very last element in the buffer.
		std::advance(ending, batch->subelements.size() - start_index);
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
	 * To actually reduce memory usage here, call the
	 * \ref BatchBase::shrink_to_fit function of the batch that contains this
	 * subbatch.
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
				my_destination = batch->next_position;
				//Make sure we have enough capacity in the element buffer itself. Grow by doubling there too.
				size_t buffer_capacity = batch->subelements.size();
				while(buffer_capacity < my_destination + other_size) {
					buffer_capacity = buffer_capacity * 2;
				}
				batch->subelements.resize(buffer_capacity, Element());
				current_capacity = other_size;
				batch->next_position += current_capacity;
			}
			size_t other_destination = other.start_index;
			if(my_size > other.capacity()) {
				other_destination = other.batch->next_position;
				//Make sure we have enough capacity in the element buffer itself. Grow by doubling there too.
				size_t buffer_capacity = other.batch->subelements.size();
				while(buffer_capacity < other_destination + my_size) {
					buffer_capacity = buffer_capacity * 2;
				}
				other.batch->subelements.resize(buffer_capacity, Element());
				other.current_capacity = my_size;
				other.batch->next_position += other.current_capacity;
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
						batch->subelements[my_destination + i] = std::move(other[i]);
					}
				}
				for(size_t i = 0; i < my_size; ++i) {
					other.batch->subelements[other_destination + i] = std::move((*this)[i]);
				}
				if(!to_me_first) {
					for(size_t i = 0; i < other_size; ++i) {
						batch->subelements[my_destination + i] = std::move(other[i]);
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
	void swap(BatchBase<Element>& other) {
		const size_t my_size = size();
		const size_t other_size = other.size();

		//Find out if we need to resize either of them.
		//If the subbatch is resized, move elements directly to the reallocated place rather than moving them twice.
		size_t my_destination = start_index;
		if(other_size > capacity()) {
			my_destination = batch->next_position;
			//Make sure we have enough capacity in the element buffer itself. Grow by doubling there too.
			size_t buffer_capacity = batch->subelements.size();
			while(buffer_capacity < my_destination + other_size) {
				buffer_capacity = buffer_capacity * 2;
			}
			batch->subelements.resize(buffer_capacity, Element());
			current_capacity = other_size;
			batch->next_position += current_capacity;
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
				batch->subelements[my_destination + i] = std::move(other[i]);
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
	BatchBase<BatchBase<Element>>* batch;

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
				const size_t new_place = batch->next_position;
				const size_t new_capacity = current_capacity * 2; //Doubling the capacity ensures that repeated insertion of a new element is done in amortised constant time.
				batch->next_position += new_capacity;

				//Make sure we have enough capacity in the element buffer itself. Grow by doubling there too.
				size_t buffer_capacity = batch->subelements.size();
				while(buffer_capacity < new_place + new_capacity) {
					buffer_capacity = buffer_capacity * 2;
				}
				batch->subelements.resize(buffer_capacity, Element());

				//Copy the leading elements to the start of the allocated space.
				for(size_t i = 0; i < index + count; ++i) {
					batch->subelements[new_place + i] = (*this)[i];
				}
				//Copy the trailing elements to the end of the allocated space.
				remaining_space = new_capacity - current_capacity;
				for(size_t i = index + count; i < current_capacity; ++i) {
					batch->subelements[new_place + i + remaining_space] = (*this)[i];
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
	 *
	 * If the subbatch is currently the last subbatch in the batch, it will not
	 * get moved unless capacity needs to be expanded. This prevents the need to
	 * move elements often, for the common case where data is initially entered
	 * in a linear fashion.
	 * \param new_capacity The amount of elements that can be stored without
	 * allocating new memory, after this operation has been completed.
	 */
	void reallocate(const size_t new_capacity) {
		size_t new_place = start_index;
		if(start_index + current_capacity != batch->next_position) { //If we're currently the last subbatch in the buffer, don't change the start index.
			new_place = batch->next_position;
		}
		batch->next_position = new_place + new_capacity;

		//Make sure we have enough capacity in the element buffer itself. Grow by doubling there too.
		size_t buffer_capacity = batch->subelements.size();
		while(buffer_capacity < new_place + new_capacity) {
			buffer_capacity *= 2;
		}
		batch->subelements.resize(buffer_capacity, Element());

		if(new_place != start_index) {
			//If we've moved, copy all of the data over.
			for(size_t index = 0; index < size(); ++index) {
				batch->subelements[new_place + index] = batch->subelements[start_index + index];
			}
			start_index = new_place;
		}

		current_capacity = new_capacity;
	}
};

}

namespace std {

/*!
 * Swap the contents of two batches.
 *
 * This swap is made by reference and can be executed in constant time, without
 * needing to copy or move the individual elements of the batches.
 * \param left The batch to swap with \ref right.
 * \param right The batch to swap with \ref left.
 * \tparam Element The type of elements contained in the batches.
 */
template<typename Element>
void swap(apex::BatchBase<Element>& left, apex::BatchBase<Element>& right) noexcept {
	left.swap(right);
}

}

#endif //APEX_BATCH_BASE