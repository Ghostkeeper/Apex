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
 * \tparam Element The type of data stored in this batch.
 */
template<typename Element>
class BatchBase {
public:
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
	 */
	BatchBase<Element>& operator =(const BatchBase<Element>& other) {
		elements = other.elements;
	}

	/*!
	 * Moves the contents of a different batch into the assigned batch.
	 *
	 * The elements of the batch will not be copied.
	 * \param other The batch to move into this batch.
	 */
	BatchBase<Element>& operator =(BatchBase<Element>&& other) noexcept {
		elements = std::move(other.elements);
	}

	/*!
	 * Copies the contents of an intialiser list into this batch.
	 * \param initialiser_list The list to copy into this batch.
	 */
	BatchBase<Element>& operator =(std::initializer_list<Element> initialiser_list) {
		elements = initialiser_list;
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

