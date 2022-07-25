/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_PAIRING_FUNCTION
#define APEX_PAIRING_FUNCTION

namespace apex {

namespace detail {

/*!
 * Gets the number of unique pairs that can be made with a certain number of
 * elements.
 *
 * For example, with the 4 elements A, B, C and D you could make 6 different
 * pairs:
 * * A and B
 * * A and C
 * * A and D
 * * B and C
 * * B and D
 * * C and D
 *
 * This does not count two different orderings as a different pair. AB is the
 * same pair as BA, and will not be counted twice.
 *
 * Optionally, this function can also skip adjacent elements in the ordering.
 * Skipping adjacent pairs is useful for certain algorithms where the adjacent
 * pairs need to be handled separately. In the above example, you will then only
 * have three unique pairs left:
 * * A and C
 * * A and D
 * * B and D
 * \param num_elements The number of elements from which to select pairs.
 * \param include_adjacent Whether to count adjacent elements as well.
 */
size_t num_pairings(const size_t num_elements, const bool include_adjacent = true) {
	return (num_elements - 1) * (num_elements - (!include_adjacent * 2)) / 2;
}

}

}

#endif //APEX_PAIRING_FUNCTION