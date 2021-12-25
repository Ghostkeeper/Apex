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
 * The class behaves externally like \ref std::vector in its interface. For most
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
	/*!
	 * Construct an empty batch.
	 */
	Batch() : BatchBase<Element>() {}

	/*!
	 * Construct a batch containing the specified element repeated a number of
	 * times.
	 * \param count The amount of elements to fill the batch with.
	 * \param value The element to fill the batch with. This element gets
	 * repeated a number of times.
	 */
	Batch(const size_t count, const Element& value = Element()) : BatchBase<Element>(count, value) {}

	/*!
	 * Construct a batch containing default-inserted instances of the element.
	 * \param count The amount of elements to fill the batch with.
	 */
	Batch(const size_t count) : BatchBase<Element>(count) {}

	/*!
	 * Construct a batch with the contents of the range ``[first, last)``.
	 * \tparam InputIterator This constructor accepts any type of input
	 * iterator.
	 * \param first The iterator to start copying from.
	 * \param last An iterator signalling that the range of elements to copy has
	 * ended.
	 */
	template<class InputIterator>
	Batch(InputIterator first, InputIterator last) : BatchBase<Element>(first, last) {}

	/*!
	 * Construct a copy of the specified batch.
	 *
	 * All elements inside the batch will be copied as well.
	 * \param other The batch to copy.
	 */
	Batch(const Batch<Element>& other) : BatchBase<Element>(static_cast<const BatchBase<Element>&>(other)) {}

	/*!
	 * Move constructor, moving one batch to another location using move
	 * semantics.
	 *
	 * After the move, ``other`` is guaranteed to be empty.
	 * \param other The batch to move into the new batch.
	 */
	Batch(Batch<Element>&& other) : BatchBase<Element>(static_cast<BatchBase<Element>&&>(other)) {}

	/*!
	 * Construct a batch from the contents of an initialiser list.
	 * \param initialiser_list The list of elements to put in the new batch.
	 */
	Batch(std::initializer_list<Element> initialiser_list) : BatchBase<Element>(initialiser_list) {}
};

}

#endif //APEX_BATCH