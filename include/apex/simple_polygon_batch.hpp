/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2020 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef SIMPLE_POLYGON_BATCH_HPP
#define SIMPLE_POLYGON_BATCH_HPP

#include <vector> //A dynamic data structure to model the two buffers that this batch maintains.

#include "point2.hpp" //To store the vertex data.
#include "simple_polygon.hpp" //The default for the simple polygons in the batch.

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
 * The index buffer contains the total number of simple polygons, the location
 * of the next polygon in the buffer, and the start, size and end of each of
 * them. This way, the two buffers contain the complete state of the batch, and
 * copying them both to a compute device allows all of the same computations
 * there.
 * \tparam SimplePolygon An implementation of simple polygons to use. Used to
 * swap out dependencies in automated tests.
 */
class SimplePolygonBatch {
protected:
	/*!
	 * Provides a view on the data of one simple polygon inside a
	 * `SimplePolygonBatch`.
	 *
	 * This view behaves as a vector. It contains all of the same methods. As
	 * such, it can be used as duck type of the vector, allowing a simple
	 * polygon to be constructed using either a real vector or this view on a
	 * part of the data of a `SimplePolygonBatch`.
	 *
	 * Internally, this view refers to a certain batch by reference. The
	 * reference is invalidated if the batch is ever moved. This class is
	 * intended to be used as a short-lived view on the batch, and moving a
	 * batch is bad practice anyway since the batch data is typically very
	 * heavy. Moving a simple polygon inside the batch (e.g. when reserving more
	 * memory for it) does not invalidate the view.
	 *
	 * This version of the view is const, meaning that it cannot edit the data
	 * in the batch. For a version that allows editing the simple polygons,
	 * refer to \ref SimplePolygonBatch.View .
	 */
	class View {
	public:
		/*!
		 * Iterates one loop around the polygon.
		 *
		 * This actually uses the iterator type of the vertex buffer in the
		 * batch of simple polygons, since the view will iterate over those
		 * vertices.
		 */
		typedef typename std::vector<Point2>::iterator iterator;

		/*!
		 * Iterates one loop around the polygon.
		 *
		 * This actually uses the iterator type of the vertex buffer in the
		 * batch of simple polygons, since the view will iterate over those
		 * vertices.
		 */
		typedef typename std::vector<Point2>::const_iterator const_iterator;

		/*!
		 * Iterates one loop around the polygon in reverse.
		 *
		 * This actually uses the iterator type of the vertex buffer in the
		 * batch of simple polygons, since the view will iterate over those
		 * vertices.
		 */
		typedef typename std::vector<Point2>::reverse_iterator reverse_iterator;

		/*!
		 * Iterates one loop around the polygon in reverse.
		 *
		 * This actually uses the iterator type of the vertex buffer in the
		 * batch of simple polygons, since the view will iterate over those
		 * vertices.
		 */
		typedef typename std::vector<Point2>::const_reverse_iterator const_reverse_iterator;

		/*!
		 * Constructs a new view on a simple polygon batch.
		 * \param batch The batch to view on.
		 * \param polygon_index The simple polygon within that batch that this
		 * view is viewing on.
		 */
		View(SimplePolygonBatch& batch, const size_t polygon_index) :
			batch(batch),
			polygon_index(polygon_index) {};

		/*!
		 * Compares two simple polygons for equality.
		 * \param other The simple polygon to compare this with.
		 * \return ``true`` if the two simple polygons are equal, or ``false``
		 * if they are different.
		 */
		bool operator ==(const View& other) const {
			//TODO: Implement this via CRTP using the implementation of SimplePolygonArea.
			if(size() != other.size()) {
				return false;
			}
			for(size_t vertex = 0; vertex < size(); ++vertex) {
				if((*this)[vertex] != other[vertex]) {
					return false;
				}
			}
			return true;
		}

		/*!
		 * Compares two simple polygons for inequality.
		 * \param other The simple polygon to compare this with.
		 * \return ``true`` if the two simple polygons are different, or
		 * ``false`` if they are equal.
		 */
		bool operator !=(const View& other) const {
			return !((*this) == other);
		}

		/*!
		 * Gives the vertex at the specified index in this simple polygon.
		 * \param index The index of the vertex to get.
		 * \return The vertex at the specified index.
		 */
		const Point2& operator [](const size_t index) const {
			return batch.vertex_buffer[start_index() + index];
		}

		/*!
		 * Gives the vertex at the specified index in this simple polygon.
		 * \param index The index of the vertex to get.
		 * \return The vertex at the specified index.
		 */
		Point2& operator [](const size_t index) {
			return batch.vertex_buffer[start_index() + index];
		}

		/*!
		 * Replace the contents of the simple polygon with a repeated instance
		 * of a given vertex.
		 * \param count How many times to repeat the vertex.
		 * \param value The vertex coordinate to repeat.
		 */
		void assign(const size_t count, const Point2& value) {
			reserve(count);
			for(size_t i = 0; i < count; ++i) {
				batch.vertex_buffer[start_index() + i] = value;
			}
			batch.index_buffer[2 + polygon_index * 3 + 1] = count; //Update the size, clearing any vertices that we didn't override if the original was bigger.
		}

		/*!
		 * Replace the contents of the simple polygon with the contents of a
		 * range between the two given iterators.
		 * \param begin The beginning of the range to iterate over.
		 * \param end The vertex past the last vertex to iterate over.
		 * \tparam InputIterator The type of iterator to iterate with. The
		 * ``begin`` iterator needs to have the same type as the ``end``
		 * iterator.
		 */
		template<class InputIterator>
		void assign(InputIterator begin, InputIterator end) {
			size_t current_size = 0;
			for(; begin != end; ++current_size) {
				if(current_size >= capacity()) {
					batch.index_buffer[2 + polygon_index * 3 + 1] = current_size; //Reallocation needs to know the size.
					reallocate((current_size + 1) * 2); //With generic iterators, we won't know the size beforehand since we can't always iterate over the input twice to see how long it goes on.
				}
				batch.vertex_buffer[start_index() + current_size] = *begin;
				begin++;
			}
			batch.index_buffer[2 + polygon_index * 3 + 1] = current_size; //Update the size, clearing any vertices that we didn't override if the original was bigger.
		}

		/*!
		 * Replace the contents of the simple polygon with the contents of an
		 * initialiser list.
		 * \param initialiser_list The initialiser list containing vertices.
		 */
		void assign(const std::initializer_list<Point2> initialiser_list) {
			reserve(initialiser_list.size());
			size_t position = 0;
			for(const Point2& vertex : initialiser_list) {
				batch.vertex_buffer[start_index() + position++] = vertex;
			}
			batch.index_buffer[2 + polygon_index * 3 + 1] = initialiser_list.size(); //Update the size, clearing any vertices that we didn't override if the original was bigger.
		}

		/*!
		 * Returns a reference to a vertex in a certain position in the simple
		 * polygon.
		 *
		 * If the given position is out of range, an ``std::out_of_range``
		 * exception is thrown.
		 * \param position The index of the vertex to return.
		 * \return A reference to the vertex in the specified position.
		 */
		const Point2& at(const size_t position) const {
			if(position >= size()) {
				throw std::out_of_range("Out of range for this view on a simple polygon.");
			}
			return batch.vertex_buffer[start_index() + position];
		}

		/*!
		 * Returns a reference to a vertex in a certain position in the simple
		 * polygon.
		 *
		 * If the given position is out of range, an ``std::out_of_range``
		 * exception is thrown.
		 * \param position The index of the vertex to return.
		 * \return A reference to the vertex in the specified position.
		 */
		Point2& at(const size_t position) {
			if(position >= size()) {
				throw std::out_of_range("Out of range for this view on a simple polygon.");
			}
			return batch.vertex_buffer[start_index() + position];
		}

		/*!
		 * Returns a reference to the last vertex of the view on the simple
		 * polygon.
		 *
		 * Getting the back of an empty simple polygon is undefined. It may
		 * return a vertex of a different simple polygon, or run out of range of
		 * the internal memory which may cause a segfault.
		 * \return A reference to the last vertex of the view on the simple
		 * polygon.
		 */
		const Point2& back() const {
			return batch.vertex_buffer[start_index() + size() - 1];
		}

		/*!
		 * Returns a reference to the last vertex of the view on the simple
		 * polygon.
		 *
		 * Getting the back of an empty simple polygon is undefined. It may
		 * return a vertex of a different simple polygon, or run out of range of
		 * the internal memory which may cause a segfault.
		 * \return A reference to the last vertex of the view on the simple
		 * polygon.
		 */
		Point2& back() {
			return batch.vertex_buffer[start_index() + size() - 1];
		}

		/*!
		 * Get an iterator to the first vertex in the view on the simple
		 * polygon.
		 *
		 * This actually returns an iterator to the vertex in the batch. You
		 * could theoretically keep iterating further, but this is not supported
		 * since you could iterate beyond the vertex buffer itself and into
		 * unallocated memory in between the simple polygons. You should never
		 * iterate beyond the ``end()`` iterator.
		 * \return An iterator pointing at the first vertex of the simple
		 * polygon inside the batch.
		 */
		iterator begin() {
			iterator beginning = batch.vertex_buffer.begin();
			std::advance(beginning, start_index());
			return beginning;
		}

		/*!
		 * Get an iterator to the first vertex in the view on the simple
		 * polygon.
		 *
		 * This actually returns an iterator to the vertex in the batch. You
		 * could theoretically keep iterating further, but this is not supported
		 * since you could iterate beyond the vertex buffer itself and into
		 * unallocated memory in between the simple polygons. You should never
		 * iterate beyond the ``end()`` iterator.
		 * \return An iterator pointing at the first vertex of the simple
		 * polygon inside the batch.
		 */
		const_iterator begin() const {
			const_iterator beginning = batch.vertex_buffer.begin();
			std::advance(beginning, start_index());
			return beginning;
		}

		/*!
		 * Return the number of vertices that this simple polygon could contain
		 * without needing to allocate more memory.
		 * \return The capacity of this simple polygon.
		 */
		size_t capacity() const {
			return end_index() - start_index();
		}

		/*!
		 * Get a const iterator to the first vertex in the view on the simple
		 * polygon.
		 *
		 * This actually returns an iterator to the vertex in the batch. You
		 * could theoretically keep iterating further, but this is not supported
		 * since you could iterate beyond the vertex buffer itself and into
		 * unallocated memory in between the simple polygons. You should never
		 * iterate beyond the ``end()`` iterator.
		 * \return A const iterator pointing at the first vertex of the simple
		 * polygon inside the batch.
		 */
		const_iterator cbegin() const {
			return begin(); //Calls the const version, which returns a const_iterator anyway.
		}

		/*!
		 * Get a const iterator to beyond the last vertex in the view on the
		 * simple polygon.
		 *
		 * This actually returns an iterator to the end of the view in the
		 * vertex list in the batch. You could theoretically keep iterating
		 * further, but this is not supported since you could iterate beyond the
		 * vertex buffer itself and into unallocated memory in between the
		 * simple polygons.
		 * \return An iterator pointing beyond the last vertex of the simple
		 * polygon inside the batch.
		 */
		const_iterator cend() const {
			return end(); //Calls the const version, which returns a const_iterator anyway.
		}

		/*!
		 * Erases all vertices from the simple polygon in the view.
		 *
		 * The capacity of the simple polygon view is unchanged. Nothing is
		 * re-allocated.
		 */
		void clear() noexcept {
			batch.index_buffer[2 + polygon_index * 3 + 1] = 0; //Set the size to 0. 2+ for the 2 starting vertices, +1 to set the size.
		}

		/*!
		 * Get a const iterator to the first vertex in the view on the simple
		 * polygon when iterating in reverse (which would normally be the last
		 * vertex).
		 *
		 * This actually returns an iterator to the last vertex in the view in
		 * the vertex list in the batch. You could theoretically keep iterating
		 * further, but this is not supported since you could iterate beyond the
		 * vertex buffer itself and into unallocated memory in between the
		 * simple polygons.
		 * \return A reverse iterator pointing to the last vertex of the simple
		 * polygon inside the batch.
		 */
		const_reverse_iterator crbegin() const {
			return rbegin(); //Calls the const version, which returns a const_iterator anyway.
		}

		/*!
		 * Get a const iterator to beyond the last vertex in the view on the
		 * simple polygon when iterating in reverse (which would normally be
		 * before the first vertex).
		 *
		 * This actually returns an iterator to before the beginning of the view
		 * in the vertex list in the batch. You could theoretically keep
		 * iterating further, but this is not supported since you could iterate
		 * beyond the vertex buffer itself and into unallocated memory in
		 * between the simple polygons.
		 * \return A reverse iterator pointing to beyond the first vertex of the
		 * simple polygon inside the batch.
		 */
		const_reverse_iterator crend() const {
			return rend(); //Calls the const version, which returns a const_iterator anyway.
		}

		/*!
		 * Returns a pointer to the underlying batch buffer serving as element
		 * storage.
		 *
		 * This returns a pointer to the first vertex that is part of this view.
		 * If the size of the underlying vertex buffer is 0, this may return a
		 * null pointer.
		 * \return A pointer to the first vertex that's part of this view in the
		 * underlying data structure of the view.
		 */
		const Point2* data() const noexcept {
			if(batch.vertex_buffer.empty()) {
				return batch.vertex_buffer.data();
			}
			return &batch.vertex_buffer[start_index()];
		}

		/*!
		 * Returns a pointer to the underlying batch buffer serving as element
		 * storage.
		 *
		 * This returns a pointer to the first vertex that is part of this view.
		 * If the size of the underlying vertex buffer is 0, this may return a
		 * null pointer.
		 * \return A pointer to the first vertex that's part of this view in the
		 * underlying data structure of the view.
		 */
		Point2* data() noexcept {
			if(batch.vertex_buffer.empty()) {
				return batch.vertex_buffer.data();
			}
			return &batch.vertex_buffer[start_index()];
		}

		/*!
		 * Constructs a new vertex in-place in the simple polygon in the batch.
		 * \param position The position within this simple polygon.
		 * \param arguments The constructor arguments of the vertex to add (the
		 * X and Y coordinates).
		 * \tparam Args The types of the constructor arguments.
		 */
		template<class... Args>
		iterator emplace(const const_iterator position, Args&&... arguments) {
			const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the input iterator).
			if(size() >= capacity()) {
				reallocate(capacity() * 2 + 1);
			}

			const size_t start = start_index();
			for(size_t i = size(); i > index; i--) { //Move all vertices beyond the position by one place to make room.
				batch.vertex_buffer[start + i] = batch.vertex_buffer[start + i - 1];
			}
			//Construct the vertex in-place.
			batch.vertex_buffer[start + index] = Point2(arguments...);

			batch.index_buffer[2 + polygon_index * 3 + 1]++; //Increment the size.

			iterator result = begin();
			std::advance(result, index);
			return result;
		}

		/*!
		 * Constructs a new vertex in-place at the end of the view in the batch.
		 * \param arguments The constructor arguments of the vertex to add (the
		 * X and Y coordinates).
		 * \tparam Args The types of the constructor arguments.
		 */
		template<class... Args>
		void emplace_back(Args&&... arguments) {
			if(size() >= capacity()) {
				reallocate(capacity() * 2 + 1);
			}

			batch.vertex_buffer[start_index() + size()] = Point2(arguments...); //Construct the vertex in-place.
			batch.index_buffer[2 + polygon_index * 3 + 1]++; //Increment the size.
		}

		/*!
		 * Returns whether this view on the batch refers to an empty simple
		 * polygon.
		 * \return ``true`` if this view is empty, or ``false`` otherwise.
		 */
		bool empty() const {
			return size() == 0;
		}

		/*!
		 * Get an iterator to beyond the last vertex in the view on the simple
		 * polygon.
		 *
		 * This actually returns an iterator to the end of the view in the
		 * vertex list in the batch. You could theoretically keep iterating
		 * further, but this is not supported since you could iterate beyond the
		 * vertex buffer itself and into unallocated memory in between the
		 * simple polygons.
		 * \return An iterator pointing beyond the last vertex of the simple
		 * polygon inside the batch.
		 */
		iterator end() {
			iterator ending = batch.vertex_buffer.begin();
			std::advance(ending, start_index() + size());
			return ending;
		}

		/*!
		 * Get an iterator to beyond the last vertex in the view on the simple
		 * polygon.
		 *
		 * This actually returns an iterator to the end of the view in the
		 * vertex list in the batch. You could theoretically keep iterating
		 * further, but this is not supported since you could iterate beyond the
		 * vertex buffer itself and into unallocated memory in between the
		 * simple polygons.
		 * \return An iterator pointing beyond the last vertex of the simple
		 * polygon inside the batch.
		 */
		const_iterator end() const {
			const_iterator ending = batch.vertex_buffer.begin();
			std::advance(ending, start_index() + size());
			return ending;
		}

		/*!
		 * Returns a reference to the first element of the view on the simple
		 * polygon.
		 *
		 * Getting the front of an empty simple polygon is undefined. It may
		 * return a vertex of a different simple polygon, or run out of range of
		 * the internal memory which may cause a segfault.
		 * \return A reference to the first element of the view on the simple
		 * polygon.
		 */
		const Point2& front() const {
			return batch.vertex_buffer[start_index()];
		}

		/*!
		 * Returns a reference to the first element of the view on the simple
		 * polygon.
		 *
		 * Getting the front of an empty simple polygon is undefined. It may
		 * return a vertex of a different simple polygon, or run out of range of
		 * the internal memory which may cause a segfault.
		 * \return A reference to the first element of the view on the simple
		 * polygon.
		 */
		Point2& front() {
			return batch.vertex_buffer[start_index()];
		}

		/*!
		 * Insert a new vertex at the specified position in the simple polygon.
		 *
		 * The new vertex is inserted \e before the specified position.
		 *
		 * All iterators pointing to the specified position or after it will be
		 * invalidated. The iterators pointing to positions before the insertion
		 * point will not be invalidated, unless this insertion causes the size
		 * to exceed the current capacity, in which case it'll need to
		 * reallocate for more memory, which invalidates all iterators.
		 * \param position The position before which the new vertex will be
		 * inserted. To insert it at the end, the \ref end iterator may be
		 * supplied.
		 * \param value The vertex to insert.
		 * \return An iterator pointing to the new vertex.
		 */
		iterator insert(const const_iterator position, const Point2& value) {
			const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the input iterator).
			if(size() >= capacity()) {
				reallocate(capacity() * 2 + 1);
			}

			const size_t start = start_index();
			for(size_t i = size(); i > index; --i) { //Move all vertices beyond the position by one place to make room.
				batch.vertex_buffer[start + i] = batch.vertex_buffer[start + i - 1];
			}
			//Insert the new vertex.
			batch.vertex_buffer[start + index] = value;

			batch.index_buffer[2 + polygon_index * 3 + 1]++; //Increment the size.

			iterator result = begin();
			std::advance(result, index);
			return result;
		}

		/*!
		 * Insert a new vertex at the specified position in the simple polygon.
		 *
		 * This version tries to move the vertex rather than reallocating it. A
		 * compiler may be able to omit the move then.
		 *
		 * The new vertex is inserted \e before the specified position.
		 *
		 * All iterators pointing to the specified position or after it will be
		 * invalidated. The iterators pointing to positions before the insertion
		 * point will not be invalidated, unless this insertion causes the size
		 * to exceed the current capacity, in which case it'll need to
		 * reallocate for more memory, which invalidates all iterators.
		 * \param position The position before which the new vertex will be
		 * inserted. To insert it at the end, the \ref end iterator may be
		 * supplied.
		 * \param value The vertex to insert.
		 * \return An iterator pointing to the new vertex.
		 */
		iterator insert(const const_iterator position, Point2&& value) {
			const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the input iterator).
			if(size() >= capacity()) {
				reallocate(capacity() * 2 + 1);
			}

			const size_t start = start_index();
			for(size_t i = size(); i > index; --i) { //Move all vertices beyond the position by one place to make room.
				batch.vertex_buffer[start + i] = batch.vertex_buffer[start + i - 1];
			}
			//Insert the new vertex.
			batch.vertex_buffer[start + index] = value;

			batch.index_buffer[2 + polygon_index * 3 + 1]++; //Increment the size.

			iterator result = begin();
			std::advance(result, index);
			return result;
		}

		/*!
		 * Inserts a number of copies of a vertex at the specified position in
		 * the simple polygon.
		 *
		 * This by itself is not useful for polygons, but it could be part of an
		 * algorithm that needs to insert a bunch of vertices. Moving the
		 * vertices behind it multiple spaces at the same time can be more
		 * efficient than inserting all vertices one by one.
		 *
		 * The new vertices are inserted \e before the specified position.
		 *
		 * All iterators pointing to the specified position or after it will be
		 * invalidated. The iterators pointing to positions before the insertion
		 * point will not be invalidated, unless this insertion causes the size
		 * to exceed the current capacity, in which case it'll need to
		 * reallocate for more memory, which invalidates all iterators.
		 * \param position The position before which the new vertices will be
		 * inserted. To insert them at the end, the \ref end iterator may be
		 * supplied.
		 * \param count How many copies of the vertex to insert.
		 * \param value The vertex to insert there.
		 * \return An iterator pointing to the first of the new vertices that
		 * was inserted.
		 */
		iterator insert(const const_iterator position, const size_t count, const Point2& value) {
			const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the input iterator).
			if(size() + count > capacity()) {
				reallocate(capacity() * 2 + count);
			}

			const size_t start = start_index();
			for(size_t i = size() - 1 + count; i > index; --i) { //Move all vertices beyond the position by multiple places to make room.
				batch.vertex_buffer[start + i] = batch.vertex_buffer[start + i - count];
			}
			//Insert the new vertices.
			for(size_t i = 0; i < count; ++i) {
				batch.vertex_buffer[start + index + i] = value;
			}

			batch.index_buffer[2 + polygon_index * 3 + 1] += count; //Increase the size.

			iterator result = begin();
			std::advance(result, index);
			return result;
		}

		/*!
		 * Inserts a range of vertices at the specified position in the simple
		 * polygon.
		 *
		 * The new vertices are inserted \e before the specified position.
		 *
		 * All iterators pointing to the specified position or after it will be
		 * invalidated. The iterators pointing to positions before the insertion
		 * point will not be invalidated, unless this insertion causes the size
		 * to exceed the current capacity, in which case it'll need to
		 * reallocate for more memory, which invalidates all iterators.
		 * \param position The position before which the new vertices will be
		 * inserted. To insert them at the end, the \ref end iterator may be
		 * supplied.
		 * \param begin The start of the range of vertices to insert.
		 * \param end The end of the range of vertices to insert. This is the
		 * element \e after the last vertex.
		 * \return An iterator pointing to the first of the new vertices that
		 * was inserted.
		 */
		template<class InputIterator>
		iterator insert(const const_iterator position, InputIterator begin, const InputIterator end) {
			//Dispatch to the most efficient implementation for the current iterator type.
			return insert_iterator_dispatch<InputIterator>(position, begin, end, typename std::iterator_traits<InputIterator>::iterator_category());
		}

		/*!
		 * Inserts a list of vertices at the specified position in the simple
		 * polygon.
		 *
		 * The new vertices are inserted \e before the specified position.
		 *
		 * All iterators pointing to the specified position or after it will be
		 * invalidated. The iterators pointing to positions before the insertion
		 * point will not be invalidated, unless this insertion causes the size
		 * to exceed the current capacity, in which case it'll need to
		 * reallocate for more memory, which invalidates all iterators.
		 * \param position The position before which the new vertices will be
		 * inserted. To insert them at the end, the \ref end iterator may be
		 * supplied.
		 * \param initialiser_list The list of vertices to insert.
		 * \return An iterator pointing to the first of the new vertices that
		 * was inserted.
		 */
		iterator insert(const const_iterator position, const std::initializer_list<Point2> initialiser_list) {
			const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the inpute iterator).
			const size_t count = initialiser_list.size();
			if(size() + count > capacity()) {
				reallocate(capacity() * 2 + count);
			}

			const size_t start = start_index();
			for(size_t i = size() - 1 + count; i > index; --i) { //Move all vertices beyond the position by multiple places to make room.
				batch.vertex_buffer[start + i] = batch.vertex_buffer[start + i - count];
			}
			//Insert the new vertices.
			size_t i = 0;
			for(const Point2& vertex : initialiser_list) {
				batch.vertex_buffer[start + index + i++] = vertex;
			}

			batch.index_buffer[2 + polygon_index * 3 + 1] += count; //Increase the size.

			iterator result = begin();
			std::advance(result, index);
			return result;
		}

		/*!
		 * Returns the maximum number of vertices that this simple polygon is
		 * theoretically able to hold due to the implementation.
		 *
		 * This maximum may be reduced if the batch also contains other simple
		 * polygons with vertices.
		 * \return The maximum number of vertices that this simple polygon is
		 * able to hold.
		 */
		size_t max_size() const noexcept {
			return batch.vertex_buffer.max_size(); //Should really subtract the total size of all other polygons in the batch, but that would be linear.
		}

		/*!
		 * Get an iterator to the first vertex in the view on the simple polygon
		 * when iterating in reverse (which would normally be the last vertex).
		 *
		 * This actually returns an iterator to the last vertex in the view in
		 * the vertex list in the batch. You could theoretically keep iterating
		 * further, but this is not supported since you could iterate beyond the
		 * vertex buffer itself and into unallocated memory in between the
		 * simple polygons.
		 * \return A reverse iterator pointing to the last vertex of the simple
		 * polygon inside the batch.
		 */
		const_reverse_iterator rbegin() const {
			const_reverse_iterator beginning = batch.vertex_buffer.rbegin(); //So pointing to the last vertex in the buffer.
			std::advance(beginning, batch.vertex_buffer.size() - start_index() - size());
			return beginning;
		}

		/*!
		 * Get an iterator to the first vertex in the view on the simple polygon
		 * when iterating in reverse (which would normally be the last vertex).
		 *
		 * This actually returns an iterator to the last vertex in the view in
		 * the vertex list in the batch. You could theoretically keep iterating
		 * further, but this is not supported since you could iterate beyond the
		 * vertex buffer itself and into unallocated memory in between the
		 * simple polygons.
		 * \return A reverse iterator pointing to the last vertex of the simple
		 * polygon inside the batch.
		 */
		reverse_iterator rbegin() {
			reverse_iterator beginning = batch.vertex_buffer.rbegin(); //So pointing to the last vertex in the buffer.
			std::advance(beginning, batch.vertex_buffer.size() - start_index() - size());
			return beginning;
		}

		/*!
		 * Get an iterator to beyond the last vertex in the view on the simple
		 * polygon when iterating in reverse (which would normally be before the
		 * first vertex).
		 *
		 * This actually returns an iterator to before the beginning of the view
		 * in the vertex list in the batch. You could theoretically keep
		 * iterating further, but this is not supported since you could iterate
		 * beyond the vertex buffer itself and into unallocated memory in
		 * between the simple polygons.
		 * \return A reverse iterator pointing to beyond the first vertex of the
		 * simple polygon inside the batch.
		 */
		const_reverse_iterator rend() const {
			const_reverse_iterator ending = batch.vertex_buffer.rbegin(); //So pointing to the last vertex in the buffer.
			std::advance(ending, batch.vertex_buffer.size() - start_index());
			return ending;
		}

		/*!
		 * Get an iterator to beyond the last vertex in the view on the simple
		 * polygon when iterating in reverse (which would normally be before the
		 * first vertex).
		 *
		 * This actually returns an iterator to before the beginning of the view
		 * in the vertex list in the batch. You could theoretically keep
		 * iterating further, but this is not supported since you could iterate
		 * beyond the vertex buffer itself and into unallocated memory in
		 * between the simple polygons.
		 * \return A reverse iterator pointing to beyond the first vertex of the
		 * simple polygon inside the batch.
		 */
		reverse_iterator rend() {
			reverse_iterator ending = batch.vertex_buffer.rbegin(); //So pointing to the last vertex in the buffer.
			std::advance(ending, batch.vertex_buffer.size() - start_index());
			return ending;
		}

		/*!
		 * Increase the capacity of the view on a polygon to ensure that it can
		 * contain the specified number of vertices without reallocating to a
		 * new strip of memory.
		 *
		 * The actual size or vertex data is not changed. However if the
		 * capacity of the view is increased, all current iterators to positions
		 * in the simple polygon are invalidated. They cannot be used any more
		 * since the place they refer to is no longer where the data is held.
		 */
		void reserve(const size_t new_capacity) {
			if(new_capacity < capacity()) {
				return; //Already got the required capacity.
			}
			reallocate(new_capacity);
		}

		/*!
		 * This function is a no-op for this implementation.
		 *
		 * It would normally request the memory usage to be reduced to fit
		 * exactly the amount of vertices used by this simple polygon. However
		 * since this simple polygon is part of a batch and the batch uses a
		 * monotonic allocation algorithm to reserve memory for its members, the
		 * memory usage cannot shrink for just one individual simple polygon.
		 *
		 * To actually reduce memory usage here, you need to use the
		 * \ref SimplePolygonBatch::shrink_to_fit function of the batch that
		 * contains this simple polygon.
		 */
		inline void shrink_to_fit() noexcept {}

		/*!
		 * Get the number of vertices in this simple polygon.
		 * \return The number of vertices in this simple polygon.
		 */
		size_t size() const {
			return batch.index_buffer[2 + polygon_index * 3 + 1]; //2+ due to the two starting indices, then +1 because we're getting the size.
		}

	protected:
		/*!
		 * The batch of simple polygons that this view is referring to.
		 */
		SimplePolygonBatch& batch;

		/*!
		 * The simple polygon within the batch that this view is viewing on.
		 */
		const size_t polygon_index;

		/*!
		 * Get the index in the vertex buffer where this view starts.
		 * \return The index in the vertex buffer where this view starts.
		 */
		inline size_t start_index() const {
			return batch.index_buffer[2 + polygon_index * 3]; //2+ due to the two starting indices.
		}

		/*!
		 * Get the index in the vertex buffer where this view ends.
		 *
		 * This is the index \e after the last vertex. The actual index is not
		 * part of this view, and may be unallocated, belong to a different
		 * simple polygon or may even be outside of the vertex buffer.
		 * \return The index in the vertex buffer where this view ends.
		 */
		inline size_t end_index() const {
			return batch.index_buffer[2 + polygon_index * 3 + 2]; //2+ due to the two starting indices, +2 since we need the end.
		}

		/*!
		 * This implements the \ref insert function if the input decorator is a
		 * random access iterator.
		 *
		 * This is the most preferred implementation, because if the iterator
		 * allows random access, the number of elements to insert can be
		 * calculated in constant time by taking the difference of the begin and
		 * end pointers. This way we'll know how much to shift the rest of the
		 * vertices and how much memory to reserve beforehand.
		 * \param position The position in the simple polygon to insert the
		 * vertices before.
		 * \param range_start The beginning of the range to insert.
		 * \param range_end The ending of the range to insert (after the last
		 * vertex).
		 * \return An iterator to the beginning of the inserted range.
		 */
		template<class InputIterator>
		iterator insert_iterator_dispatch(const const_iterator position, InputIterator range_start, const InputIterator range_end, const std::random_access_iterator_tag) {
			const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the input iterator).
			const size_t count = range_end - range_start;
			if(size() + count >= capacity()) {
				reallocate(capacity() * 2 + count);
			}

			const size_t buffer_start = start_index();
			for(size_t i = size() - 1 + count; i > index; --i) { //Move all vertices beyond the position by multiple places to make room.
				batch.vertex_buffer[buffer_start + i] = batch.vertex_buffer[buffer_start + i - count];
			}
			//Insert the new vertices.
			for(size_t i = 0; range_start != range_end; range_start++, ++i) {
				batch.vertex_buffer[buffer_start + index + i] = *range_start;
			}

			batch.index_buffer[2 + polygon_index * 3 + 1] += count; //Increase the size.

			iterator result = begin();
			std::advance(result, index);
			return result;
		}

		/*!
		 * This implements the \ref insert function if the input decorator is a
		 * forward iterator.
		 *
		 * This is less ideal than random access, since it can't be determined
		 * how big the range is without iterating over it. However since this is
		 * a forward iterator, we are able to rewind and iterate over it once
		 * first to determine its size in linear time, still preventing the need
		 * to shift the vertices after the position multiple times.
		 * \param position The position in the simple polygon to insert the
		 * vertices before.
		 * \param range_start The beginning of the range to insert.
		 * \param range_end The ending of the range to insert (after the last
		 * vertex).
		 * \return An iterator to the beginning of the inserted range.
		 */
		template<class InputIterator>
		iterator insert_iterator_dispatch(const const_iterator position, InputIterator range_start, const InputIterator range_end, const std::forward_iterator_tag) {
			const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the input iterator).
			const size_t original_size = size(); //Get the size at the start since we need to update the size before reallocating.
			size_t count = 0;
			if(position != end()) {
				for(InputIterator counter = range_start; counter != range_end; counter++) { //Count how many inputs we have.
					++count;
				}
				if(original_size + count >= capacity()) {
					reallocate(capacity() * 2 + count);
				}
				const size_t buffer_start = start_index();
				for(size_t i = original_size - 1 + count; i > index; --i) { //Move all vertices beyond the position by multiple places to make room.
					batch.vertex_buffer[buffer_start + i] = batch.vertex_buffer[buffer_start + i - count];
				}
				//Insert the new vertices.
				for(size_t i = 0; range_start != range_end; range_start++, ++i) {
					batch.vertex_buffer[buffer_start + index + i] = *range_start;
				}
			} else { //Don't bother counting and shifting if we're inserting at the end.
				for(size_t i = 0; range_start != range_end; range_start++, ++i) {
					if(original_size + i >= capacity()) {
						batch.index_buffer[2 + polygon_index * 3 + 1] = original_size + count; //Update the size before reallocating.
						reallocate(capacity() * 2 + 1);
					}
					batch.vertex_buffer[start_index() + index + i] = *range_start;
					count++;
				}
			}

			batch.index_buffer[2 + polygon_index * 3 + 1] = original_size + count; //Increase the size.

			iterator result = begin();
			std::advance(result, index);
			return result;
		}

		/*!
		 * This implements the \ref insert function if the input decorator is
		 * not a forward iterator.
		 *
		 * This is the worst case, where we're not allowed to iterate over the
		 * range more than once and we can't determine the size beforehand. So
		 * the only thing we can really do is to shift the other vertices,
		 * insert a new vertex, and repeat until the iterator range is done.
		 * However one trick we can apply is to shift by multiple spaces at once
		 * and shift back after the iteration is done.
		 * \param position The position in the simple polygon to insert the
		 * vertices before.
		 * \param range_start The beginning of the range to insert.
		 * \param range_end The ending of the range to insert (after the last
		 * vertex).
		 * \return An iterator to the beginning of the inserted range.
		 */
		template<class InputIterator>
		iterator insert_iterator_dispatch(const const_iterator position, InputIterator range_start, const InputIterator range_end, const std::input_iterator_tag) {
			const size_t index = position - begin(); //Get the index before possibly reallocating (which would invalidate the input iterator).
			const size_t original_size = size(); //Get the size as well since we need to update the size before reallocating.
			size_t remaining_space = 0; //How many spots we still have left before we need to shift vertices again.
			size_t count = 0;
			for(;range_start != range_end; range_start++, ++count) {
				if(original_size + count >= capacity()) {
					batch.index_buffer[2 + polygon_index * 3 + 1] = original_size + count; //Update the size before reallocating.
					reallocate(capacity() * 2 + 1);
				}
				const size_t buffer_start = start_index();
				if(remaining_space == 0) { //Need to make sure we've got room to insert without overwriting vertices afterwards.
					//Move towards the end of the capacity so that we can insert more often.
					const size_t move_distance = capacity() - original_size - count; //How far can we move these vertices.
					for(size_t i = capacity() - 1; i >= index + count + move_distance; --i) {
						batch.vertex_buffer[buffer_start + i] = batch.vertex_buffer[buffer_start + i - move_distance];
					}
					remaining_space = move_distance;
				}
				batch.vertex_buffer[buffer_start + index + count] = *range_start;
				--remaining_space;
			}

			//Move vertices after the insertion back if there was any remaining space.
			if(remaining_space > 0) {
				const size_t buffer_start = start_index();
				for(size_t i = index + count; i < capacity() - remaining_space; ++i) {
					batch.vertex_buffer[buffer_start + i] = batch.vertex_buffer[buffer_start + i + remaining_space];
				}
			}

			batch.index_buffer[2 + polygon_index * 3 + 1] = original_size + count; //Increase the size.

			iterator result = begin();
			std::advance(result, index);
			return result;
		}

		/*!
		 * Moves this batch to a new location inside the vertex buffer to make
		 * more space for new vertices.
		 *
		 * This copies all of the vertex data to a new location, making it a
		 * linear operation. Much like the ArrayList variant though, adding new
		 * vertices won't have to call this operation very often any more as the
		 * simple polygon grows bigger, resulting in an amortised constant time
		 * complexity for adding vertices.
		 * \param new_capacity The amount of vertices that can be stored without
		 * allocating new memory, after this operation has been completed.
		 */
		void reallocate(const size_t new_capacity) {
			const size_t new_place = batch.index_buffer[1];
			batch.index_buffer[1] += new_capacity; //TODO: Not thread-safe. To make this thread-safe, read-and-update atomically and do something about the vertex buffer data structure.

			//Make sure we have enough capacity in the vertex buffer itself. Grow by doubling there too.
			size_t buffer_capacity = batch.vertex_buffer.size();
			while(buffer_capacity < new_place + new_capacity) {
				buffer_capacity = buffer_capacity * 2 + 1;
			}
			batch.vertex_buffer.resize(buffer_capacity, Point2(0, 0));

			//Copy all of the data over.
			const size_t old_place = start_index();
			for(size_t vertex_index = 0; vertex_index < size(); vertex_index++) {
				batch.vertex_buffer[new_place + vertex_index] = batch.vertex_buffer[old_place + vertex_index];
			}

			//Update the index buffer with the new place.
			batch.index_buffer[2 + polygon_index * 3] = new_place; //2+ due to the two starting indices.
			batch.index_buffer[2 + polygon_index * 3 + 2] = new_place + new_capacity; //2+ due to the two starting indices, +2 due to setting the end index.
		}
	};

public:
	/*!
	 * Construct a new vector of vectors, completely empty.
	 */
	SimplePolygonBatch() {
		index_buffer.push_back(0); //Start off with 0 simple polygons.
		index_buffer.push_back(0); //And the first polygon would get allocated to the 0th position.
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
	 * The memory for each individual simple polygon does not get assigned yet.
	 * If one simple polygon ends up using less memory, others can use more
	 * without the need to reserve new memory.
	 * \param num_simple_polygons The amount of polygons to add to the batch.
	 * \param vertices_per_polygon How much memory to reserve for each polygon
	 * on average.
	 */
	SimplePolygonBatch(const size_t num_simple_polygons, const size_t vertices_per_polygon) {
		vertex_buffer.reserve(num_simple_polygons * vertices_per_polygon);
		index_buffer.reserve(num_simple_polygons * 3 + 2);
		index_buffer.push_back(num_simple_polygons); //Number of polygons.
		index_buffer.push_back(0); //Position of next polygon.

		//Store ranges for each simple polygon in the index buffer.
		index_buffer.insert(index_buffer.end(), num_simple_polygons * 3, 0); //Insert a 0 for the start, the size and the reserved memory, for each polygon.
	}

	/*!
	 * Copies this batch.
	 *
	 * All of the data is actually copied. This is linear in the total amount of
	 * vertices in the batch.
	 * \param other The batch to copy into this one.
	 */
	SimplePolygonBatch(const SimplePolygonBatch& other) :
		vertex_buffer(other.vertex_buffer),
		index_buffer(other.index_buffer) {}

	/*!
	 * Moves a batch into this batch.
	 *
	 * Depending on the situation and the compiler, the data might actually get
	 * moved to a new location or it might not. If it is moved, this constructor
	 * is linear in the size of the vertex data. If it's not, it's a constant.
	 * After the constructor has completed, the batch provided in the parameter
	 * is in an indeterminate state and should no longer be used.
	 * \param other The batch to move into this one.
	 */
	SimplePolygonBatch(SimplePolygonBatch&& other) noexcept :
		vertex_buffer(std::move(other.vertex_buffer)),
		index_buffer(std::move(other.index_buffer)) {}

	/*!
	 * Copy assignment operator to copy one batch into another variable.
	 *
	 * All of the data is actually copied. This is linear in the total amount of
	 * vertices in the batch.
	 * \param other The batch to copy into this one.
	 * \return A reference to this batch for chaining.
	 */
	SimplePolygonBatch& operator =(const SimplePolygonBatch& other) {
		vertex_buffer = other.vertex_buffer;
		index_buffer = other.index_buffer;
		return *this;
	}

	/*!
	 * Move assignment operator to move one batch into another variable.
	 *
	 * Sometimes all of the data will be copied, sometimes not. This is up to
	 * the compiler and the specific situation. The batch supplied here as
	 * parameter will be in an indeterminate state and should no longer be used
	 * after this method completes.
	 * \param other The batch to move into this one.
	 * \return A reference to this batch for chaining.
	 */
	SimplePolygonBatch& operator =(SimplePolygonBatch&& other) noexcept {
		vertex_buffer = std::move(other.vertex_buffer);
		index_buffer = std::move(other.index_buffer);
		return *this;
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
	SimplePolygon<const View> operator [](const size_t position) const {
		/*This code uses a const_cast to remove the constness of the batch.
		This would be unsafe, but since we're building a SimplePolygon based on
		a const view, the const view will guard the constness of the batch. The
		operation is only unsafe if a copy is made of the actual view into a
		non-const variable. But since the VertexStorage itself is not exposed by
		SimplePolygon, you can't make such a copy.*/
		return SimplePolygon<const View>(const_cast<SimplePolygonBatch&>(*this), position);
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
	SimplePolygon<View> operator [](const size_t position) {
		return SimplePolygon<View>(*this, position);
	}

	/*!
	 * Compares two batches for equality.
	 *
	 * Batches of simple polygons are equal if the simple polygons inside them
	 * are in the same order and pairwise equal to each other.
	 * \param other The batch to compare this batch to.
	 * \return ``true`` if both batches are equal, or ``false`` if they are not.
	 */
	bool operator ==(const SimplePolygonBatch& other) const {
		//TODO: Implement efficient batch operation for this.
		if(size() != other.size()) {
			return false;
		}
		for(size_t simple_polygon = 0; simple_polygon < size(); ++simple_polygon) {
			if((*this)[simple_polygon] != other[simple_polygon]) { //Must be in the same order.
				return false;
			}
		}
		return true;
	}

	/*!
	 * Copies a simple polygon into the batch.
	 *
	 * The data of the simple polygon is copied completely, making this
	 * operation linear in the size of the provided simple polygon.
	 * \param simple_polygon The polygon to add to the batch.
	 */
	template<typename VertexStorage>
	void push_back(const SimplePolygon<VertexStorage>& simple_polygon) {
		const size_t next_position = index_buffer[1];
		size_t buffer_capacity = vertex_buffer.size();
		while(buffer_capacity < next_position + simple_polygon.size()) {
			buffer_capacity = buffer_capacity * 2 + 1;
		}
		vertex_buffer.resize(buffer_capacity, Point2(0, 0));
		index_buffer.push_back(next_position); //Position of this polygon.
		index_buffer.push_back(simple_polygon.size()); //Size of the polygon.
		index_buffer.push_back(next_position + simple_polygon.size()); //End of the reserved memory.
		for(size_t i = 0; i < simple_polygon.size(); ++i) { //Copy the actual data into the batch.
			vertex_buffer[next_position + i] = simple_polygon[i];
		}
		index_buffer[0]++; //There is now one more simple polygon.
		index_buffer[1] += simple_polygon.size();
	}

	/*!
	 * Returns the number of simple polygons in this batch.
	 * \return The number of simple polygons in this batch.
	 */
	size_t size() const {
		return index_buffer[0];
	}

	/*!
	 * The main buffer that contains the vertex data.
	 *
	 * This \e only concerns the actual bytes of data to store the actual
	 * coordinates of the simple polygons. The ranges where each simple polygon
	 * starts and ends are stored in the \ref SimplePolygonBatch.index_buffer.
	 * \todo This buffer is currently always resized in order to provide enough
	 * space for direct access of the vertices. However this initialises some
	 * vertices twice (once at zero, the second time with actual vertex data)
	 * and sometimes unnecessarily. We should reserve instead of resize, but
	 * then push_back instead of directly accessing the elements of the vertex
	 * buffer. Measure whether this actually improves performance.
	 */
	std::vector<Point2> vertex_buffer;
protected:
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