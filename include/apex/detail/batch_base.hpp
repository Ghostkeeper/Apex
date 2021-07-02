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

	/*!
	 * Construct a batch from the contents of an initialiser list.
	 * \param initialiser_list The list of elements to put in the new batch.
	 */
	BatchBase(std::initializer_list<Element> initialiser_list) : std::vector<Element>(initialiser_list) {}

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

}

namespace std {

/*!
 * Swap the contents of two batches.
 *
 * This swap is made by reference and can be executed in constant time, without
 * needing to copy or move the individual elements of the batches.
 * \tparam Element The type of elements contained in the batches.
 * \param left The batch to swap with \ref right.
 * \param right The batch to swap with \ref left.
 */
template<typename Element>
void swap(apex::BatchBase<Element>& left, apex::BatchBase<Element>& right) noexcept {
	left.swap(right);
}

}

#endif //APEX_BATCH_BASE

