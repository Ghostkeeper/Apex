/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include "input_iterator_limiter.hpp" //The class of which we'll implement its members.

namespace apex {

template<typename InputIterator>
InputIteratorLimiter<InputIterator>::InputIteratorLimiter(InputIterator original_iterator) : original(original_iterator) {}

template<typename InputIterator>
bool InputIteratorLimiter<InputIterator>::operator ==(InputIteratorLimiter<InputIterator> other) const {
	return original == other.original;
}

template<typename InputIterator>
bool InputIteratorLimiter<InputIterator>::operator !=(InputIteratorLimiter<InputIterator> other) const {
	return !((*this) == other); //Implement in terms of the equality operator.
}

template<typename InputIterator>
typename InputIteratorLimiter<InputIterator>::value_type& InputIteratorLimiter<InputIterator>::operator *() const {
	return *original;
}

template<typename InputIterator>
typename InputIteratorLimiter<InputIterator>::value_type* InputIteratorLimiter<InputIterator>::operator ->() const {
	return &(*original);
}

template<typename InputIterator>
InputIteratorLimiter<InputIterator>& InputIteratorLimiter<InputIterator>::operator ++() {
	++original;
	return *this;
}

template<typename InputIterator>
InputIteratorLimiter<InputIterator> InputIteratorLimiter<InputIterator>::operator ++(int) {
	InputIteratorLimiter<InputIterator> previous(original); //Make a copy, wrapping again around the iterator we currently have.
	original++;
	return previous;
}

}