/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_PAIRING_FUNCTION
#define APEX_PAIRING_FUNCTION

#include <utility>

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
	if(num_elements == 0) {
		return 0;
	}
	return (num_elements - 1) * (num_elements - !include_adjacent * 2) / 2;
}

/*!
 * Get a pair of indices to elements in some list.
 *
 * Increasing the index monotonically will eventually and efficiently enumerate
 * all pairs in a list with a certain given size. Optionally, this can also skip
 * some pairs that are adjacent in the list (where the difference between the
 * indices is 1).
 *
 * This function is an inverse pairing function. It unpacks a single integer
 * into a pair of integers, effectively enumerating those pairs. The pairing
 * function used here is a custom one optimised for enumerating a finite set of
 * pairs in a triangle. To visualise the order in which pairs are enumerated,
 * consider the following grid of possible pairs.
 *
 * <table>
 * <caption id="multi_row">Pairs of indices</caption>
 * <tr><td>0 x 1</td></tr>
 * <tr><td>0 x 2</td> <td>1 x 2</td></tr>
 * <tr><td>0 x 3</td> <td>1 x 3</td> <td>2 x 3</td></tr>
 * <tr><td>0 x 4</td> <td>1 x 4</td> <td>2 x 4</td> <td>3 x 4</td></tr>
 * <tr><td>0 x 5</td> <td>1 x 5</td> <td>2 x 5</td> <td>3 x 5</td> <td>4 x 5</td></tr>
 * <tr><td>0 x 6</td> <td>1 x 6</td> <td>2 x 6</td> <td>3 x 6</td> <td>4 x 6</td> <td>5 x 6</td> <td></td></tr>
 * </table>
 *
 * Enumerating this table is complex, but to simplify that we could transform it
 * into a grid by folding the table in on itself. First we'll mirror the bottom
 * half of the table.
 *
 * <table>
 * <caption id="multi_row">Mirrored bottom half</caption>
 * <tr><td>0 x 1</td></tr>
 * <tr><td>0 x 2</td> <td>1 x 2</td></tr>
 * <tr><td>0 x 3</td> <td>1 x 3</td> <td>2 x 3</td></tr>
 * <tr><td>     </td> <td>     </td> <td>     </td> <td>3 x 4</td> <td>2 x 4</td> <td>1 x 4</td> <td>0 x 4</td></tr>
 * <tr><td>     </td> <td>     </td> <td>4 x 5</td> <td>3 x 5</td> <td>2 x 5</td> <td>1 x 5</td> <td>0 x 5</td></tr>
 * <tr><td>     </td> <td>5 x 6</td> <td>4 x 6</td> <td>3 x 6</td> <td>2 x 6</td> <td>1 x 6</td> <td>0 x 6</td></tr>
 * </table>
 *
 * Then we can flip the bottom half of the table to fold it into the top half.
 * The two halves don't have any overlap then.
 *
 * <table>
 * <caption id="multi_row">Folded bottom half</caption>
 * <tr><td>0 x 1</td> <td>5 x 6</td> <td>4 x 6</td> <td>3 x 6</td> <td>2 x 6</td> <td>1 x 6</td> <td>0 x 6</td></tr>
 * <tr><td>0 x 2</td> <td>1 x 2</td> <td>4 x 5</td> <td>3 x 5</td> <td>2 x 5</td> <td>1 x 5</td> <td>0 x 5</td></tr>
 * <tr><td>0 x 3</td> <td>1 x 3</td> <td>2 x 3</td> <td>3 x 4</td> <td>2 x 4</td> <td>1 x 4</td> <td>0 x 4</td></tr>
 * </table>
 *
 * This table efficiently enumerates all possible pairs in a set of 7 elements,
 * has a known size and can be traversed simply with a modulus.
 *
 * Enumerating beyond the number of pairs in the set produces indeterminate
 * results. The indices will no longer be within the range of the set.
 * \param num_elements The number of elements in a set to enumerate the pairs
 * of.
 * \param index The index of the pair to return. Incrementing this between 0 and
 * the result of \ref num_pairings will eventually enumerate all pairs of
 * indices.
 * \param include_adjacent Whether to count adjacent elements as well.
 * \return A pair of indices in the set of elements.
 */
std::pair<size_t, size_t> enumerate_pairs(const size_t num_elements, const size_t index, const bool include_adjacent = true) {
	size_t a = index % (num_elements - 1);
	size_t b = index / (num_elements - 1);
	if(a > b) {
		a = num_elements - 1 - include_adjacent - a; //Mirror A dimension. -1 for iterating up to size-1, another -1 if we skip adjacent vertices here.
		b = num_elements - 1 - b; //Mirror B dimension. -1 for iterating up to size-1
	} else {
		b += 1 + include_adjacent; //Don't compare the same vertex or adjacent vertices.
	}
	return std::make_pair(a, b);
}

}

}

#endif //APEX_PAIRING_FUNCTION