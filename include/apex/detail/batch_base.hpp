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

namespace apex {

/*!
 * This class forms the basis of all batch implementations.
 *
 * Batches are supposed to behave transparently like vectors. Inheriting from
 * vectors is unsafe if used polymorphously. Since we have no controls over how
 * the (public) batch classes are used, and we want to impose no restrictions,
 * we'll let the Batch implementations inherit from the private BatchBase class.
 * That class behaves like a vector, but uses composition rather than
 * inheritance to mimic its behaviour. That way, inheriting from this batch base
 * is safe.
 *
 * This batch base doesn't support customising the allocator. If a specific
 * allocator is necessary, it should be customised in the template
 * specialisation. It also doesn't support ``shrink_to_fit``, which is not
 * available to all specialisations of the batch.
 * \tparam Element The type of data stored in this batch.
 */
template<typename Element>
class BatchBase {
public:
	/*!
	 * Random-access iterator type to traverse elements in this batch.
	 */
	using iterator = std::vector<Element>::iterator;

	/*!
	 * Random-access iterator type to traverse elements in this batch without
	 * allowing modifications to those elements.
	 */
	using const_iterator = std::vector<Element>::const_iterator;

	/*!
	 * Random-access iterator type to traverse elements in this batch in reverse
	 * order.
	 */
	using reverse_iterator = std::vector<Element>::reverse_iterator;

	/*!
	 * Random-access iterator type to traverse elements in this batch in reverse
	 * order without allowing modifications to those elements.
	 */
	using const_reverse_iterator = std::vector<Element>::const_reverse_iterator;

	/*!
	 * Constructs an empty batch.
	 */
	BatchBase() noexcept {}

	/*!
	 * Construct a batch containing the specified element repeated a number of
	 * times.
	 * \param count The amount of elements to fill the batch with.
	 * \param value The element to fill the batch with. This element gets
	 * repeated a number of times.
	 */
	BatchBase(const size_t count, const Element& value = Element()) : elements(count, value) {}

	/*!
	 * Construct a batch containing default-inserted instances of the element.
	 * \param count The amount of elements to fill the batch with.
	 */
	BatchBase(const size_t count) : elements(count) {}

	/*!
	 * Construct a batch with the contents of the range ``[first, last)``.
	 * \param first The iterator to start copying from.
	 * \param last An iterator signalling that the range of elements to copy has
	 * ended.
	 */
	template<class InputIterator>
	BatchBase(InputIterator first, InputIterator last) : elements(first, last) {}

	/*!
	 * Construct a copy of the specified batch.
	 *
	 * All elements inside the batch will be copied as well.
	 * \param other The batch to copy.
	 */
	BatchBase(const BatchBase& other) : elements(other.elements) {}

	/*!
	 * Move constructor, moving one batch to another location using move
	 * semantics.
	 *
	 * After the move, ``other`` is guaranteed to be empty.
	 * \param other The batch to move into the new batch.
	 */
	BatchBase(BatchBase&& other) noexcept : elements(std::move(other.elements)) {}

	/*!
	 * Constructs a batch from the contents of an initialiser list.
	 * \param initialiser_list The list of elements to put in the new batch.
	 */
	BatchBase(std::initializer_list<Element> initialiser_list) : elements(initialiser_list) {}

	/*!
	 * Copies the contents of a different batch to the assigned batch.
	 * \param other The batch to copy into this batch.
	 * \return A reference to this batch itself.
	 */
	BatchBase<Element>& operator =(const BatchBase<Element>& other) {
		elements = other.elements;
		return *this;
	}

	/*!
	 * Moves the contents of a different batch into the assigned batch.
	 *
	 * The elements of the batch will not be copied.
	 * \param other The batch to move into this batch.
	 * \return A reference to this batch itself.
	 */
	BatchBase<Element>& operator =(BatchBase<Element>&& other) noexcept {
		elements = std::move(other.elements);
		return *this;
	}

	/*!
	 * Copies the contents of an intialiser list into this batch.
	 * \param initialiser_list The list to copy into this batch.
	 * \return A reference to this batch itself.
	 */
	BatchBase<Element>& operator =(std::initializer_list<Element> initialiser_list) {
		elements = initialiser_list;
		return *this;
	}

	/*!
	 * Return a reference to the element at the specified position.
	 *
	 * Individual access of elements in a batch defeats its purpose for
	 * improving performance of parallel algorithms. In some cases, it may be
	 * considerably slower than accessing elements of a normal vector. Consider
	 * using the batch's methods as much as possible for processing its
	 * contents.
	 */
	Element& operator [](const size_t position) {
		return elements[position];
	}

	/*!
	 * Return a reference to the element at the specified position.
	 *
	 * Individual access of elements in a batch defeats its purpose for
	 * improving performance of parallel algorithms. In some cases, it may be
	 * considerably slower than accessing elements of a normal vector. Consider
	 * using the batch's methods as much as possible for processing its
	 * contents.
	 */
	const Element& operator [](const size_t position) const {
		return elements[position];
	}

	/*!
	 * Replace the contents of the batch with a number of copies of the
	 * specified value.
	 *
	 * Any old elements in the batch are erased.
	 * \param count The number of copies of the value to put into this batch.
	 * \param value The element to copy repeatedly.
	 */
	void assign(const size_t count, const Element& value) {
		elements.assign(count, value);
	}

	/*!
	 * Replace the contents of the batch with the elements in the specified
	 * range ``[first, last)``.
	 * \param first The first element in the range of elements to copy into the
	 * batch.
	 * \param last The iterator signalling the end of the range of elements to
	 * copy into the batch.
	 */
	template<class InputIterator>
	void assign(InputIterator first, InputIterator last) {
		elements.assign(first, last);
	}

	/*!
	 * Replace the contents of the batch with the elements in an initialiser
	 * list.
	 * \param initialiser_list A list of elements to copy into this batch.
	 */
	void assign(std::initializer_list<Element> initialiser_list) {
		elements.assign(initialiser_list);
	}

	/*!
	 * Return a reference to the element in the batch at the specified position.
	 *
	 * If the position is out of bounds for the size of the batch,
	 * ``std::out_of_range`` will be raised.
	 *
	 * Individual access of elements in a batch defeats its purpose for
	 * improving performance of parallel algorithms. In some cases, it may be
	 * considerably slower than accessing elements of a normal vector. Consider
	 * using the batch's methods as much as possible for processing its
	 * contents.
	 * \return A reference to the element in the batch at the specified
	 * position.
	 */
	Element& at(const size_t position) {
		return elements.at(position);
	}

	/*!
	 * Return a reference to the element in the batch at the specified position.
	 *
	 * If the position is out of bounds for the size of the batch,
	 * ``std::out_of_range`` will be raised.
	 *
	 * Individual access of elements in a batch defeats its purpose for
	 * improving performance of parallel algorithms. In some cases, it may be
	 * considerably slower than accessing elements of a normal vector. Consider
	 * using the batch's methods as much as possible for processing its
	 * contents.
	 * \return A reference to the element in the batch at the specified
	 * position.
	 */
	const Element& at(const size_t position) const {
		return elements.at(position);
	}

	/*!
	 * Return a reference to the last element in the batch.
	 *
	 * Calling back on an empty batch is undefined.
	 * \return A reference to the last element in the batch.
	 */
	Element& back() {
		return elements.back();
	}

	/*!
	 * Return a reference to the last element in the batch.
	 *
	 * Calling back on an empty batch is undefined.
	 * \return A reference to the last element in the batch.
	 */
	const Element& back() const {
		return elements.back();
	}

	/*!
	 * Return an iterator to the first element of the batch.
	 * \return An iterator to the first element of the batch.
	 */
	iterator begin() noexcept {
		return elements.begin();
	}

	/*!
	 * Return an iterator to the first element of the batch.
	 * \return An iterator to the first element of the batch.
	 */
	const_iterator begin() const noexcept {
		return elements.begin();
	}

	/*!
	 * Return the number of elements that the batch has currently allocated
	 * space for.
	 * \return The capacity of the currently allocated storage.
	 */
	size_t capacity() const noexcept {
		return elements.capacity();
	}

	/*!
	 * Return an iterator to the first element of the batch.
	 * \return An iterator to the first element of the batch.
	 */
	const_iterator cbegin() const noexcept {
		return elements.cbegin();
	}

	/*!
	 * Return an iterator marking the end of the elements in the batch.
	 *
	 * This iterator itself is beyond the range of the batch and not accessible.
	 * \return An iterator marking the end of the elements in the batch.
	 */
	const_iterator cend() const noexcept {
		return elements.cend();
	}

	/*!
	 * Erases all contents of the batch.
	 *
	 * After this, the batch will be empty. The capacity will be unchanged.
	 *
	 * All iterators into the batch will be invalidated.
	 */
	void clear() noexcept {
		elements.clear();
	}

	/*!
	 * Return a reverse iterator to the first element of the reversed range.
	 *
	 * This points to the element referenced by \ref back, the last element of
	 * the batch.
	 * \return A reverse iterator to the first element of the reversed range.
	 */
	const_reverse_iterator crbegin() const noexcept {
		return elements.crbegin();
	}

	/*!
	 * Return a reverse iterator marking the end of the reversed range.
	 *
	 * This iterator itself is beyond the range of the batch and not accessible.
	 * \return A reverse iterator marking the end of the reversed range.
	 */
	const_reverse_iterator crend() const noexcept {
		return elements.crend();
	}

	/*!
	 * Returns a pointer to the underlying array serving as element storage.
	 *
	 * This can be used for high-performance algorithms to work on the direct
	 * elements of the batch, particularly when working with libraries that use
	 * more of a C-style interface.
	 * \return A pointer to the start of the data in the batch.
	 */
	Element* data() noexcept {
		return elements.data();
	}

	/*!
	 * Returns a pointer to the underlying array serving as element storage.
	 *
	 * This can be used for high-performance algorithms to work on the direct
	 * elements of the batch, particularly when working with libraries that use
	 * more of a C-style interface.
	 * \return A pointer to the start of the data in the batch.
	 */
	const Element* data() const noexcept {
		return elements.data();
	}

	/*!
	 * Tests whether the batch contains any elements or not.
	 * \return ``true`` if the batch contains no elements, or ``false`` if there
	 * are some elements in the batch.
	 */
	bool empty() const noexcept {
		return elements.empty();
	}

	/*!
	 * Return an iterator marking the end of the elements in the batch.
	 *
	 * This iterator itself is beyond the range of the batch and not accessible.
	 * \return An iterator marking the end of the elements in the batch.
	 */
	iterator end() noexcept {
		return elements.end();
	}

	/*!
	 * Return an iterator marking the end of the elements in the batch.
	 *
	 * This iterator itself is beyond the range of the batch and not accessible.
	 * \return An iterator marking the end of the elements in the batch.
	 */
	const_iterator end() const noexcept {
		return elements.end();
	}

	/*!
	 * Return a reference to the first element in the batch.
	 *
	 * Calling front on an empty batch is undefined.
	 * \return A reference to the first element in the batch.
	 */
	Element& front() {
		return elements.front();
	}

	/*!
	 * Return a reference to the first element in the batch.
	 *
	 * Calling front on an empty batch is undefined.
	 * \return A reference to the first element in the batch.
	 */
	const Element& front() const {
		return elements.front();
	}

	/*!
	 * Return the theoretical maximum number of elements that this batch could
	 * hold.
	 *
	 * This is the maximum that the library safely supports without hitting
	 * integer overflows or other such limitations. The actual maximum may also
	 * be limited by the available RAM or other resource limitations, but that
	 * is not reflected by this value.
	 */
	size_t max_size() const noexcept {
		return elements.max_size();
	}

	/*!
	 * Return a reverse iterator to the first element of the reversed range.
	 *
	 * This points to the element referenced by \ref back, the last element of
	 * the batch.
	 * \return A reverse iterator to the first element of the reversed range.
	 */
	reverse_iterator rbegin() noexcept {
		return elements.rbegin();
	}

	/*!
	 * Return a reverse iterator to the first element of the reversed range.
	 *
	 * This points to the element referenced by \ref back, the last element of
	 * the batch.
	 * \return A reverse iterator to the first element of the reversed range.
	 */
	const_reverse_iterator rbegin() const noexcept {
		return elements.rbegin();
	}

	/*!
	 * Return a reverse iterator marking the end of the reversed range.
	 *
	 * This iterator itself is beyond the range of the batch and not accessible.
	 * \return A reverse iterator marking the end of the reversed range.
	 */
	reverse_iterator rend() noexcept {
		return elements.rend();
	}

	/*!
	 * Return a reverse iterator marking the end of the reversed range.
	 *
	 * This iterator itself is beyond the range of the batch and not accessible.
	 * \return A reverse iterator marking the end of the reversed range.
	 */
	const_reverse_iterator rend() const noexcept {
		return elements.rend();
	}

	/*!
	 * Increase the capacity of the batch to accommodate at least a certain
	 * number of elements without having to re-allocate more memory for the
	 * data.
	 *
	 * The actual contents of the batch are unchanged. This does not add any
	 * elements to the batch. Reducing the capacity to anything lower than what
	 * it currently is has no effect.
	 *
	 * If the capacity is increased, all iterators to elements in this batch are
	 * invalidated. References are also invalidated for some ``Batch``
	 * specialisations, but may preserve validation for others.
	 * \param new_capacity The minimum amount of elements that the batch needs
	 * to be able to contain without having to allocate more memory after this.
	 */
	void reserve(const size_t new_capacity) {
		elements.reserve(new_capacity);
	}

	/*!
	 * Return the number of elements in the batch.
	 * \return The number of elements in the batch.
	 */
	size_t size() const noexcept {
		return elements.size();
	}

protected:
	/*!
	 * The main data contained in the batch.
	 *
	 * The batch behaves almost exactly like this vector.
	 */
	std::vector<Element> elements;
};

}

#endif //APEX_BATCH_BASE

