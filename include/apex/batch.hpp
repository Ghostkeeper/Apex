/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_BATCH
#define APEX_BATCH

#include <vector> //As data structure and to type-specialise batches of vectors.

namespace apex {

/*!
 * The Batch class groups a list of elements together, both conceptually and in
 * memory.
 *
 * The purpose of this batching is to improve performance. If some algorithm
 * needs to be performed on many elements simultaneously, batching these all
 * together allows them to be transferred to different compute devices in one
 * go, which reduces the latency of the transfer.
 *
 * The class behaves externally like \ref std::vector in its interface. For most
 * types of elements, it behaves like a normal vector internally as well.
 * However if the element is itself a vector, this class specialises the data
 * structure as a vector-of-vectors. Instead of each vector having its own
 * allocation of memory, this batch will coalesce all data to a single array. It
 * then keeps track of where each vector starts and ends in an array of indices.
 * The purpose of that is that this single array can be transferred in one go,
 * and allocated on other devices in one go, improving performance.
 *
 * There are also disadvantages to this approach. It essentially operates as a
 * monotonic allocator, so if any of the elements have to grow in size, they
 * will need to be moved in their entirety to the end of the buffer. There is
 * also no tracking of where there might be gaps halfway in the buffer to move
 * them to, so the buffer will only grow unless it is optimised with a manual
 * call.
 *
 * If the batch is of a simple type, accessing elements of the batch behaves as
 * they would normally. However accessing elements of a batch of vectors will
 * result in a "view" on the batch. These view objects are not actual vectors,
 * but have a level of indirection so that they can keep working even if the
 * buffers are reallocated. They behave the same way as a vector, but they are
 * not actual vectors. If the elements of the batch have the correct
 * constructor, they can be made to behave identical in code (but memory
 * allocation may be different). Individually changing elements in a batch this
 * way is less efficient than group operations, and even marginally less
 * efficient than if the element would not be inside a batch.
 *
 * The data and views together form the complete state of the batch. They are
 * intended to be sent together as two buffers to the compute devices. This
 * allows all of the batch operations to be executed on the compute devices.
 * \tparam Element The type of element stored in this batch.
 */
template<typename Element>
class Batch : public std::vector<Element> {}; //For normal elements, batches are simply vectors.

template<typename Element>
class Batch<std::vector<Element>> { //Specialise batches of vectors.
	
};

}

#endif //APEX_BATCH