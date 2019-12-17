/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef SIMPLE_POLYGON_BATCH_HPP
#define SIMPLE_POLYGON_BATCH_HPP

#include <vector> //A dynamic data structure to model the two buffers that this batch maintains.

#include <apex/point2.hpp> //To store the vertex data.

namespace apex {

/*
 * This class represents a set of simple polygons that are grouped together.
 *
 * The purpose of this batching is to improve performance. If some algorithm
 * needs to be performed on many polygons simultaneously, batching these all
 * together allows them to be transferred to different compute devices in one
 * go, which reduces the latency of transfer. If the polygons are small, it also
 * allows those compute devices to parallelise better, dividing the work and
 * ultimately coming up with an answer sooner.
 *
 * This class is essentially a vector-of-vectors implementation. The data is
 * stored in a single buffer, which allows that buffer to be transferred to
 * compute devices in one single copy, rather than having to spraggle the data
 * from many different places on heap memory (at the mercy of where each
 * separate vector left the data). However a second buffer is required to keep
 * track of the positions of each vector in that buffer.
 *
 * While the main advantage is that transfer to compute devices will be quicker
 * and batch operations too, there are also disadvantages to this approach. It
 * essentially operates as a monotonic allocator, so if any of the simple
 * polygons have to grow in size, they will need to be moved in their entirety
 * to the end of the buffer. There is also no tracking of where there might be
 * gaps halfway in the buffer to move them to, so the buffer will only grow
 * unless it is optimised.
 *
 * The index buffer contains both the total number of simple polygons and the
 * start and end of each of them. This way, the two buffers contain the complete
 * state of the batch, and copying them both to a compute device allows all of
 * the same computations there.
 */
class SimplePolygonBatch {
public:
	/*
	 * Construct a new vector of vectors, completely empty.
	 */
	SimplePolygonBatch() {
		index_buffer.push_back(0); //Start off with 0 simple polygons.
	}

	/*
	 * Returns the number of simple polygons in this batch..
	 */
	size_t size() const {
		return index_buffer[0];
	}

private:
	/*
	 * The main buffer that contains the vertex data.
	 *
	 * This \e only concerns the actual bytes of data to store the actual
	 * coordinates of the simple polygons. The ranges where each simple polygon
	 * starts and ends are stored in the \ref SimplePolygonBatch.index_buffer.
	 */
	std::vector<Point2> vertex_buffer;

	/*
	 * A separate buffer that contains the positions in the
	 * \ref SimplePolygonBatch.vertex_buffer where each simple polygon starts
	 * and ends, as well as the total number of simple polygons.
	 *
	 * The total number of simple polygons is stored in the first element. Every
	 * pair of numbers afterwards is a position in the vertex buffer where each
	 * simple polygon starts and ends.
	 */
	std::vector<size_t> index_buffer;
};

}

#endif //SIMPLE_POLYGON_BATCH_HPP