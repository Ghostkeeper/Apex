/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_INPUT_ITERATOR_LIMITER
#define APEX_INPUT_ITERATOR_LIMITER

#include <iterator> //Used to get the type of value stored in a given iterator type.

namespace apex {

/*!
 * This is an iterator class that transforms any type of input iterator subclass
 * into the more limited input iterator.
 *
 * This limited iterator can then be passed to tests, to see if our
 * functionality works with the limited iterator as well.
 *
 * This input iterator only supplies the functionality that an input iterator
 * would: referencing the value, dereferencing, equality comparison and
 * incrementing. Even if the given input iterator originally provided more
 * functionality, the wrapped iterator purposefully does not.
 * \tparam InputIterator The type of input iterator this class must wrap.
 */
template<typename InputIterator>
class InputIteratorLimiter {
protected:
	/*!
	 * The type of value this iterator returns.
	 */
	using value_type = typename std::iterator_traits<InputIterator>::value_type;

public:
	/*!
	 * Wrap the limiter around an existing iterator.
	 * \param original_iterator The iterator that must be limited.
	 */
	InputIteratorLimiter(InputIterator original_iterator);

	/*!
	 * Compares two iterators for equality.
	 *
	 * The two iterators are equal if they reference the same memory address.
	 * \param other The iterator to compare with.
	 * \return ``true`` if the two iterators reference the same memory, or
	 * ``false`` if they don't.
	 */
	bool operator ==(const InputIteratorLimiter other) const;

	/*!
	 * Compares two iterators for inequality.
	 *
	 * The two iterators are inequal if they reference a different memory
	 * address.
	 * \param other The iterator to compare with.
	 * \return ``true`` if the two iterators reference different memory, or
	 * ``false`` if they don't.
	 */
	bool operator !=(const InputIteratorLimiter other) const;

	/*!
	 * Reference the value pointed to by the iterator.
	 * \return A reference to the value pointed to by the iterator.
	 */
	value_type& operator *() const;

	/*!
	 * Access a member of the value pointed to by the iterator.
	 * \return A pointer of an object of which the member access will be taken.
	 */
	value_type* operator ->() const;

	/*!
	 * Increments the iterator, causing it to point to the next element in a
	 * range.
	 *
	 * This is the pre-increment operator, causing it to first increment and
	 * then return the result.
	 */
	InputIteratorLimiter& operator ++();

	/*!
	 * Increments the iterator, causing it to point to the next element in a
	 * range.
	 *
	 * This is the post-increment operator, which needs to return a copy of the
	 * state of the iterator before incrementing it.
	 */
	InputIteratorLimiter operator ++(int);

protected:
	/*!
	 * The iterator this limiter is wrapping around.
	 *
	 * This keeps track of the value we're actually referencing.
	 */
	InputIterator original;
};

}

namespace std {

/*!
 * Specialises the type traits of the InputIteratorLimiter class.
 *
 * This makes it possible to derive the sub-types of the limited iterator, and
 * implements the input_iterator_tag so that algorithms can recognise this
 * iterator as a limited input iterator.
 */
template<typename InputIterator>
struct iterator_traits<apex::InputIteratorLimiter<InputIterator>> {
	using difference_type = typename iterator_traits<InputIterator>::difference_type;
	using value_type = typename iterator_traits<InputIterator>::value_type;
	using pointer = typename iterator_traits<InputIterator>::pointer;
	using reference = typename iterator_traits<InputIterator>::reference;

	using iterator_category = std::input_iterator_tag; //This defines the limiter as an input iterator!
};

}

#endif //APEX_INPUT_ITERATOR_LIMITER