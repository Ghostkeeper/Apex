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

/*!
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
 * start, size and end of each of them. This way, the two buffers contain the
 * complete state of the batch, and copying them both to a compute device allows
 * all of the same computations there.
 */
class SimplePolygonBatch {
public:
	/*!
	 * Provides a view on the data of one simple polygon inside a
	 * `SimplePolygonBatch`.
	 *
	 * This view behaves as a vector. It contains all of the same methods. As such,
	 * it can be used as duck type of the vector, allowing a simple polygon to be
	 * constructed using either a real vector or this view on a part of the data of
	 * a `SimplePolygonBatch`.
	 *
	 * Internally, this view refers to a certain batch by reference. The reference
	 * is invalidated if the batch is ever moved. This class is intended to be used
	 * as a short-lived view on the batch, and moving a batch is bad practice anyway
	 * since the batch data is typically very heavy. Moving a simple polygon inside
	 * the batch (e.g. when reserving more memory for it) does not invalidate the
	 * view.
	 */
	class View {
	public:
		/*!
		 * Constructs a new view on a simple polygon batch.
		 * \param batch The batch to view on.
		 * \param polygon_index The simple polygon within that batch that this view
		 * is viewing on.
		 */
		View(SimplePolygonBatch& batch, size_t polygon_index) : batch(batch), polygon_index(polygon_index) { };

		/*!
		 * Get the size of this simple polygon.
		 * \return The number of vertices in this simple polygon.
		 */
		size_t size() const {
			return batch.index_buffer[2 + polygon_index * 3];
		}

	private:
		/*!
		 * The batch of simple polygons that this view is referring to.
		 */
		SimplePolygonBatch& batch;

		/*!
		 * The simple polygon within the batch that this view is viewing on.
		 */
		size_t polygon_index;
	};

	/*!
	 * Construct a new vector of vectors, completely empty.
	 */
	SimplePolygonBatch() {
		index_buffer.push_back(0); //Start off with 0 simple polygons.
	}

	/*!
	 * Constructs a new batch of simple polygons, reserving space for a certain
	 * amount of vertices for each polygon.
	 *
	 * The simple polygons are still actually left unassigned, but space will be
	 * reserved in memory to allow growing each polygon up to
	 * ``vertices_per_polygon`` without needing to reserve more memory or move
	 * the vertex data around within the buffer.
	 *
	 * The range of memory for each simple polygon is fixed. The number of
	 * vertices per polygon is a maximum for each polygon. If one polygon ends
	 * up using less memory, that does \e not mean that another polygon can use
	 * more without reserving more memory.
	 * \param num_simple_polygons The amount of polygons to add to the batch.
	 * \param vertices_per_polygon How much memory to reserve for each polygon.
	 * The memory reserved will be such that each polygon can grow to this
	 * number of vertices.
	 */
	SimplePolygonBatch(const size_t num_simple_polygons, const size_t vertices_per_polygon) {
		vertex_buffer.reserve(num_simple_polygons * vertices_per_polygon);
		index_buffer.reserve(num_simple_polygons * 3 + 1);
		index_buffer.push_back(num_simple_polygons);

		//Store ranges for each simple polygon in the index buffer.
		for(size_t i = 0; i < num_simple_polygons; i++) {
			index_buffer.push_back(i * 3); //Start of the range.
			index_buffer.push_back(0); //Size of this simple polygon.
			index_buffer.push_back(vertices_per_polygon); //Reserved memory of this simple polygon.
		}
	}

	/*!
	 * Access a single simple polygon within this batch.
	 *
	 * Accessing this creates a view on the batch. This is not the most
	 * efficient way of accessing the vertex data, since it involves two extra
	 * referrals to get at the actual vertex data. However this way the data in
	 * the batch can be used directly and transparently as if this view is the
	 * actual data of a simple polygon.
	 *
	 * Rather than using this accessor, try to use batch processing operations
	 * as much as possible.
	 */
	View operator [](size_t position) {
		return View(*this, position);
	}

	/*!
	 * Returns the number of simple polygons in this batch..
	 */
	size_t size() const {
		return index_buffer[0];
	}

private:
	/*!
	 * The main buffer that contains the vertex data.
	 *
	 * This \e only concerns the actual bytes of data to store the actual
	 * coordinates of the simple polygons. The ranges where each simple polygon
	 * starts and ends are stored in the \ref SimplePolygonBatch.index_buffer.
	 */
	std::vector<Point2> vertex_buffer;

	/*!
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