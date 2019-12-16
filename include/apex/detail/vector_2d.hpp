/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef VECTOR_2D_HPP
#define VECTOR_2D_HPP

#include <memory> //To use the default allocator by default, but provide the option to use your own.
#include <vector> //A dynamic data structure to model the two buffers that this vector contains.

namespace apex {

/*
 * This class is an implementation of a vector of vectors, which stores the data
 * as a single contiguous data structure rather than allocating each subvector
 * separately.
 *
 * The purpose and main advantage of this is that the allocation and access are
 * faster. For the purpose of Apex it also provides a way to transmit the data
 * to a different compute device in a single copy, rather than having to
 * spraggle the data from many different allocations in the heap. The main
 * disadvantage is that increasing the size of the inner vectors is considerably
 * more expensive, since it likely means that the inner vector needs to be moved
 * to a location where enough memory is available (the end of the buffer).
 *
 * Apart from the actual data, the vector also contains a separate buffer that
 * contains both the number of inner vectors, and the indices in the main buffer
 * where each inner vector starts and ends. This information is needed in order
 * to use the main vector properly. This datastructure itself uses the same two
 * buffers, so that there is still a single source of truth on each compute
 * device.
 * \tparam Element The type of objects stored in the vector of vectors.
 * \tparam Allocator The allocator to use to reserve memory for this data.
 */
template<typename Element, typename Allocator = std::allocator<Element>>
class Vector2D {
public:
	/*
	 * Construct a new vector of vectors, completely empty.
	 */
	Vector2D() {
		index_buffer.push_back(0); //Start off with 0 subvectors.
	}

	/*
	 * Returns the number of subvectors in this vector of vectors.
	 */
	size_t size() const {
		return index_buffer[0];
	}

private:
	/*
	 * The main buffer that contains the element data.
	 *
	 * This \e only concerns the actual bytes of data to store the actual
	 * `Element` data, which would otherwise get allocated on the stack. The
	 * ranges of the subvectors are stored in the \ref Vector2D.index_buffer
	 * buffer.
	 */
	std::vector<Element> buffer;

	/*
	 * A separate buffer that contains the ranges where each subvector starts
	 * and ends, as well as the total number of subvectors.
	 *
	 * The total number of subvectors is stored in the first element.
	 */
	std::vector<size_t> index_buffer;
};

}

#endif //VECTOR_2D_HPP