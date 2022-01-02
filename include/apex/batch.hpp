/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_BATCH
#define APEX_BATCH

#include <vector> //As data structure and to type-specialise batches of vectors.

#include "detail/batch_base.hpp" //Common implementation of many type-specialised Batch implementations.

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
 * The class behaves externally like \c std::vector in its interface. For most
 * types of elements, it behaves like a normal vector internally as well.
 * However the template argument can be specialised for some types. This is of
 * particular interest to complex data types which point to separately allocated
 * contents. By specialising the data structure of those elements, the batch can
 * be made more efficient with large data sets.
 *
 * The default implementation of batches works for any type of data, but does
 * not always call the destructors of the elements it contains. It may call
 * constructors internally when moving. As such, it is not suitable for data
 * types that produce side effects in its constructor or destructor. For some
 * types a class template specialisation exists that does make it work properly
 * (in particular the Batch class itself is made to work well as elements in
 * other batches). In general though, only plain old data types can be used in
 * these batches.
 * \tparam Element The type of element stored in this batch.
 */
template<typename Element>
class Batch : public BatchBase<Element> {
public:
	using BatchBase<Element>::BatchBase; //The constructors are the same.
};

/*!
 * The Batch class groups a list of elements together, both conceptually and in
 * memory.
 *
 * The purpose of this batching is to improve performance. If some algorithm
 * needs to be performed on many elements simultaneously, batching these all
 * together allows them to be transferred to different compute devices in one
 * go, which reduces the latency of the transfer. This is especially important
 * for batches of batches, since they would otherwise store a separate
 * allocation of memory for each subbatch.
 *
 * The class behaves externally like \ref std::vector in its interface. For the
 * case of a batch of batches, the internal behaviour is different though. It
 * will store all of the subelements in one long buffer. This buffer has only
 * one allocation. The aim of this batching is to improve performance, since
 * only one single piece of memory will have to be moved to the GPU if
 * algorithms are to be performed there.
 *
 * This implementation of batches does not always call the destructors of the
 * subelements it contains. It may call constructors internally when moving. As
 * such, it is not suitable for data types that produce side effects in its
 * constructor or destructor. Only plain old data types can be used in these
 * batches.
 */
template<typename Subelement>
class Batch<Batch<Subelement>> : public BatchBase<BatchBase<Subelement>> {};

}

#endif //APEX_BATCH