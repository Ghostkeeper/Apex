/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON_BATCH
#define APEX_SIMPLE_POLYGON_BATCH

#include <vector> //A dynamic data structure to model the two buffers that this batch maintains.

#include "batch.hpp" //The batch is implemented from the generic batch.

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
 * This class is essentially a vector-of-vectors implementation. The vertex data
 * is stored in a single buffer, which allows that buffer to be transferred to
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
 * The \ref simple_polygons vector is a set of simple polygons based on views on
 * the batch. These simple polygons use the internal \ref View class as their
 * data source, which allows them to access and modify the batch safely. These
 * polygons then behave in the same way as if they are not part of a batch at
 * all. However individually changing polygons within a batch this way is less
 * efficient than group operations (and even marginally less efficient than if
 * the simple polygon would not be inside a batch).
 *
 * The vertex data and the views together form the complete state of the batch.
 * They are intended to be sent together as two buffers to the compute devices.
 * This allows all of the batch operations to be executed on the compute
 * devices.
 * \tparam SimplePolygon An implementation of simple polygons to use. Used to
 * swap out dependencies in automated tests.
 */
class SimplePolygonBatch :
		public Batch<std::vector<Point2>>,
		//Implementing the private functions in separate classes with Curiously Recurring Template Pattern.
		public SimplePolygonBatchArea<SimplePolygonBatch> {
};

}

#endif //APEX_SIMPLE_POLYGON_BATCH