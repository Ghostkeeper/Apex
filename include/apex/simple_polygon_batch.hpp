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
 * They are intended to be sent together as two buffers to the compute devices
 * of OpenCL. This allows all of the batch operations to be executed on the
 * compute devices.
 * \tparam SimplePolygon An implementation of simple polygons to use. Used to
 * swap out dependencies in automated tests.
 */
class SimplePolygonBatch {
protected:
	class Reference;

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
		friend class SimplePolygonBatch; //This enclosing class knows about the implementation of the nested class.
		friend class SimplePolygonBatch::Reference;
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
		View(SimplePolygonBatch& batch, const size_t start_index, const size_t size, const size_t capacity) :
				batch(batch),
				start_index(start_index),
				num_vertices(size),
				current_capacity(capacity) {};

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
			return batch.vertex_buffer[start_index + index];
		}

		/*!
		 * Gives the vertex at the specified index in this simple polygon.
		 * \param index The index of the vertex to get.
		 * \return The vertex at the specified index.
		 */
		Point2& operator [](const size_t index) {
			return batch.vertex_buffer[start_index + index];
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
				batch.vertex_buffer[start_index + i] = value;
			}
			num_vertices = count; //Update the size, discarding any vertices that we didn't override if the original was bigger.
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
					num_vertices = current_size; //Reallocation needs to know the size.
					reallocate((current_size + 1) * 2); //With generic iterators, we won't know the size beforehand since we can't always iterate over the input twice to see how long it goes on.
				}
				batch.vertex_buffer[start_index + current_size] = *begin;
				begin++;
			}
			num_vertices = current_size; //Update the size, clearing any vertices that we didn't override if the original was bigger.
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
				batch.vertex_buffer[start_index + position++] = vertex;
			}
			num_vertices = initialiser_list.size(); //Update the size, clearing any vertices that we didn't override if the original was bigger.
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
			return batch.vertex_buffer[start_index + position];
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
			return batch.vertex_buffer[start_index + position];
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
			return batch.vertex_buffer[start_index + size() - 1];
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
			return batch.vertex_buffer[start_index + size() - 1];
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
			return batch.vertex_buffer.begin() + start_index;
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
			return batch.vertex_buffer.begin() + start_index;
		}

		/*!
		 * Return the number of vertices that this simple polygon could contain
		 * without needing to allocate more memory.
		 * \return The capacity of this simple polygon.
		 */
		size_t capacity() const {
			return current_capacity;
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
			num_vertices = 0; //Set the size to 0, which automatically pretends like the remaining vertices don't exist.
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
			return &batch.vertex_buffer[start_index];
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
			return &batch.vertex_buffer[start_index];
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

			for(size_t i = size(); i > index; i--) { //Move all vertices beyond the position by one place to make room.
				batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i - 1];
			}
			//Construct the vertex in-place.
			batch.vertex_buffer[start_index + index] = Point2(arguments...);

			++num_vertices; //Increment the size.
			return begin() + index;
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

			batch.vertex_buffer[start_index + size()] = Point2(arguments...); //Construct the vertex in-place.
			++num_vertices; //Increment the size.
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
			return batch.vertex_buffer.begin() + (start_index + size());
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
			return batch.vertex_buffer.begin() + (start_index + size());
		}

		/*!
		 * Erases one vertex from the simple polygon.
		 *
		 * The iterators pointing to positions at or after this vertex will be
		 * invalidated. The iterators pointing to positions before this vertex
		 * will not be invalidated.
		 * \param position The position of the vertex to erase.
		 * \return An iterator to the vertex after the removed vertex.
		 */
		iterator erase(const const_iterator position) {
			const size_t index = position - begin();
			for(size_t i = index; i < size() - 1; ++i) { //Shift other vertices over this one.
				batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i + 1];
			}
			--num_vertices; //Reduce the size by one.
			return batch.vertex_buffer.begin() + (start_index + index); //Convert iterator to non-const version by constructing a new one.
		}

		/*!
		 * Erases a range of vertices from the simple polygon.
		 *
		 * The iterators pointing to positions within or after the range will be
		 * invalidated. The iterators pointing to positions before the range
		 * will not be invalidated.
		 * \param first The beginning of the range of vertices to remove.
		 * \param end The position \e after the last vertex to remove.
		 * \return A new iterator to the vertex indicated by `end`.
		 */
		iterator erase(const_iterator first, const const_iterator end) {
			const size_t index = first - begin();
			const size_t num_removed = end - first;
			for(size_t i = index; i < size() - num_removed; ++i) { //Shift other vertices over the range.
				batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i + num_removed];
			}
			num_vertices -= num_removed; //Reduce the size.
			return batch.vertex_buffer.begin() + (start_index + index); //Convert iterator to non-const version by constructing a new one.
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
			return batch.vertex_buffer[start_index];
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
			return batch.vertex_buffer[start_index];
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

			for(size_t i = size(); i > index; --i) { //Move all vertices beyond the position by one place to make room.
				batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i - 1];
			}
			//Insert the new vertex.
			batch.vertex_buffer[start_index + index] = value;

			++num_vertices; //Increment the size.
			return begin() + index;
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

			for(size_t i = size(); i > index; --i) { //Move all vertices beyond the position by one place to make room.
				batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i - 1];
			}
			//Insert the new vertex.
			batch.vertex_buffer[start_index + index] = value;

			++num_vertices; //Increment the size.
			return begin() + index;
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

			for(size_t i = size() - 1 + count; i > index; --i) { //Move all vertices beyond the position by multiple places to make room.
				batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i - count];
			}
			//Insert the new vertices.
			for(size_t i = 0; i < count; ++i) {
				batch.vertex_buffer[start_index + index + i] = value;
			}

			num_vertices += count; //Increase the size.
			return begin() + index;
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

			for(size_t i = size() - 1 + count; i > index; --i) { //Move all vertices beyond the position by multiple places to make room.
				batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i - count];
			}
			//Insert the new vertices.
			size_t i = 0;
			for(const Point2& vertex : initialiser_list) {
				batch.vertex_buffer[start_index + index + i++] = vertex;
			}

			num_vertices += count; //Increase the size.
			return begin() + index;
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
		 * Removes the last vertex of the loop.
		 *
		 * Calling ``pop_back`` on an empty container is undefined. In fact,
		 * this might just cause an underflow on the size.
		 */
		void pop_back() {
			--num_vertices; //Just reduce the size. This automatically forgets one vertex from the end.
		}

		/*!
		 * Appends a vertex at the end of the simple polygon.
		 *
		 * The vertex is copied in this case. This may cause a reallocation,
		 * which will invalidate all iterators to vertices in this batch.
		 * \param vertex The vertex to add to the simple polygon.
		 */
		void push_back(const Point2& vertex) {
			if(size() + 1 > capacity()) {
				reallocate(capacity() * 2 + 1);
			}
			batch.vertex_buffer[start_index + size()] = vertex;
			++num_vertices;
		}

		/*!
		 * Appends a vertex at the end of the simple polygon.
		 *
		 * The vertex is moved in this case. This may cause a reallocation,
		 * which will invalidate all iterators to vertices in this batch.
		 * \param vertex The vertex to add to the simple polygon.
		 */
		void push_back(Point2&& vertex) {
			if(size() + 1 > capacity()) {
				reallocate(capacity() * 2 + 1);
			}
			batch.vertex_buffer[start_index + size()] = vertex;
			++num_vertices;
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
			std::advance(beginning, batch.vertex_buffer.size() - start_index - size());
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
			std::advance(beginning, batch.vertex_buffer.size() - start_index - size());
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
			std::advance(ending, batch.vertex_buffer.size() - start_index);
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
			std::advance(ending, batch.vertex_buffer.size() - start_index);
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
			if(new_capacity <= capacity()) {
				return; //Already got the required capacity.
			}
			reallocate(new_capacity);
		}

		/*!
		 * Resize the simple polygon to a specific size.
		 *
		 * If the new size is smaller, vertices are removed.
		 *
		 * If the new size is bigger, the default value is added until it has
		 * the desired size.
		 * \param new_size The new size for a container.
		 * \param default_value A vertex to insert if the simple polygon needs
		 * to become bigger.
		 */
		void resize(const size_t new_size, const Point2& default_value = Point2(0, 0)) {
			if(new_size > size()) {
				reserve(new_size);
				const size_t start = start_index;
				for(size_t i = size(); i < new_size; ++i) {
					batch.vertex_buffer[start + i] = default_value;
				}
			}
			num_vertices = new_size; //Set the new size. This may drop some vertices.
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
			return num_vertices;
		}

		/*!
		 * Exchanges the contents of this simple polygon with those of another.
		 *
		 * The membership of a batch or the position in it is not swapped. If
		 * there are any other views on the same polygon, they will also change
		 * along.
		 *
		 * If the two views are part of the same batch, this will only swap
		 * indices around and it will complete in constant time. If the two
		 * views are not part of the same batch, the data itself needs to be
		 * swapped, which will take linear time and may cause reallocations in
		 * the batch of the smaller simple polygon.
		 * \param other The view with which to swap the contents.
		 */
		void swap(View& other) {
			if(&batch == &other.batch) {
				std::swap(start_index, other.start_index);
				std::swap(num_vertices, other.num_vertices);
				std::swap(current_capacity, other.current_capacity);
			} else { //Different batches, so we need to swap the actual contents.
				//TODO: The reservation will move the vertices to the new storage. Afterwards we'll move them. We could prevent one move here, shaving off up to 50% of the work.
				//Make sure both have enough storage space.
				const size_t my_size = size();
				const size_t other_size = other.size();
				reserve(other_size);
				other.reserve(my_size);

				size_t i = 0;
				for(; i < std::min(size(), other.size()); ++i) {
					std::swap(batch.vertex_buffer[start_index + i], other.batch.vertex_buffer[other.start_index + i]);
				}
				if(my_size > other_size) { //Move over any remaining items from the biggest to the smallest one.
					for(; i < my_size; ++i) {
						other.batch.vertex_buffer[other.start_index + i] = batch.vertex_buffer[start_index + i];
					}
				} else if(other_size > my_size) {
					for(; i < other_size; ++i) {
						batch.vertex_buffer[start_index + i] = other.batch.vertex_buffer[other.start_index + i];
					}
				}

				std::swap(num_vertices, other.num_vertices); //Swap the sizes too.
				//But the start index and the capacity remains their own.
			}
		}

		/*!
		 * Exchange the contents of this view on a simple polygon batch with the
		 * contents of another type of vertex storage for simple polygons: a
		 * vector.
		 *
		 * This allows swapping simple polygon data between two simple polygons
		 * with different storage types.
		 * \param other The vector of vertices to swap the data with.
		 */
		void swap(std::vector<Point2>& other) {
			const size_t my_size = size();
			const size_t other_size = other.size();
			reserve(other_size);
			other.reserve(my_size);

			size_t i = 0;
			for(; i < std::min(my_size, other_size); ++i) {
				std::swap(batch.vertex_buffer[start_index + i], other[i]);
			}
			if(my_size > other_size) {
				for(; i < my_size; ++i) {
					other.push_back(batch.vertex_buffer[i]);
				}
			} else if(other_size > my_size) {
				for(; i < other_size; ++i) {
					batch.vertex_buffer[start_index + i] = other[i];
				}
				other.resize(my_size, Point2(0, 0)); //Shrink the other one to discard any remaining vertices.
			}
			num_vertices = other_size;
		}

		void swap(SimplePolygonBatch::Reference& other) {
			swap(other.batch.simple_polygons[other.index].storage());
		}

	protected:
		/*!
		 * The batch of simple polygons that this view is referring to.
		 */
		SimplePolygonBatch& batch;

		/*!
		 * The position within the vertex buffer of the batch where this simple
		 * polygon starts.
		 */
		size_t start_index;

		/*!
		 * The number of vertices currently in this simple polygon.
		 */
		size_t num_vertices;

		/*!
		 * How many vertices this simple polygon could maximally contain without
		 * needing to resize its allocation within the vertex buffer.
		 *
		 * This would be named ``capacity`` if it weren't for the specification
		 * of ``vector`` already using that name for a method.
		 */
		size_t current_capacity;

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

			for(size_t i = size() - 1 + count; i > index; --i) { //Move all vertices beyond the position by multiple places to make room.
				batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i - count];
			}
			//Insert the new vertices.
			for(size_t i = 0; range_start != range_end; range_start++, ++i) {
				batch.vertex_buffer[start_index + index + i] = *range_start;
			}

			num_vertices += count;
			return begin() + index;
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
				for(size_t i = original_size - 1 + count; i > index; --i) { //Move all vertices beyond the position by multiple places to make room.
					batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i - count];
				}
				//Insert the new vertices.
				for(size_t i = 0; range_start != range_end; range_start++, ++i) {
					batch.vertex_buffer[start_index + index + i] = *range_start;
				}
			} else { //Don't bother counting and shifting if we're inserting at the end.
				for(size_t i = 0; range_start != range_end; range_start++, ++i) {
					if(original_size + i >= capacity()) {
						num_vertices = original_size + count; //Update the size before reallocating.
						reallocate(capacity() * 2 + 1);
					}
					batch.vertex_buffer[start_index + index + i] = *range_start;
					count++;
				}
			}

			num_vertices = original_size + count; //Final size.
			return begin() + index;
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
					num_vertices = original_size + count; //Update the size before reallocating.
					reallocate(capacity() * 2 + 1);
				}
				if(remaining_space == 0) { //Need to make sure we've got room to insert without overwriting vertices afterwards.
					//Move towards the end of the capacity so that we can insert more often.
					const size_t move_distance = capacity() - original_size - count; //How far can we move these vertices.
					for(size_t i = capacity() - 1; i >= index + count + move_distance; --i) {
						batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i - move_distance];
					}
					remaining_space = move_distance;
				}
				batch.vertex_buffer[start_index + index + count] = *range_start;
				--remaining_space;
			}

			//Move vertices after the insertion back if there was any remaining space.
			if(remaining_space > 0) {
				for(size_t i = index + count; i < capacity() - remaining_space; ++i) {
					batch.vertex_buffer[start_index + i] = batch.vertex_buffer[start_index + i + remaining_space];
				}
			}

			num_vertices = original_size + count; //Final size.
			return begin() + index;
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
			const size_t new_place = batch.next_position;
			batch.next_position += new_capacity; //TODO: Not thread-safe. To make this thread-safe, read-and-update atomically and do something about the vertex buffer data structure.

			//Make sure we have enough capacity in the vertex buffer itself. Grow by doubling there too.
			size_t buffer_capacity = batch.vertex_buffer.size();
			while(buffer_capacity < new_place + new_capacity) {
				buffer_capacity = buffer_capacity * 2 + 1;
			}
			batch.vertex_buffer.resize(buffer_capacity, Point2(0, 0));

			//Copy all of the data over.
			for(size_t vertex_index = 0; vertex_index < size(); vertex_index++) {
				batch.vertex_buffer[new_place + vertex_index] = batch.vertex_buffer[start_index + vertex_index];
			}

			start_index = new_place;
			current_capacity = new_capacity;
		}
	};

	/*!
	 * This is a helper class that refers to one of the views on the batch of
	 * simple polygons.
	 *
	 * This class is intended to behave similar to a `View&`, a reference to a
	 * view, or perhaps as a `View*`, but with a few differences. The reference
	 * refers to the batch and an index within that batch, so that the reference
	 * doesn't get invalidated if the contents of the batch gets reallocated
	 * such as for an increase in capacity.
	 *
	 * The purpose of this class is to allow creating multiple instances of a
	 * `SimplePolygon` within a batch by accessing the polygons within the batch
	 * from their index, but to have each of those refer to the same actual view
	 * on the batch. This way, if one of the instances gets modified e.g. by
	 * adding vertices to the simple polygon, the other instances also get
	 * updated. If this weren't the case, the data structure could get corrupt.
	 *
	 * The reference gets invalidated once the index doesn't point to the same
	 * element any more, for example by inserting a simple polygon in the batch
	 * before this reference's index, or by deleting the simple polygon it
	 * refers to. It also gets invalidated if the whole batch gets moved or
	 * destroyed, since the pointer kept in this reference gets invalidated
	 * then.
	 *
	 * This reference is pretty much a transparent wrapper around the `View`
	 * that it refers to.
	 */
	class Reference {
		friend class SimplePolygonBatch::View;
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
         * Construct a new reference to a view in a batch.
         * \param batch The batch of simple polygons to refer to.
         * \param index The view within that batch to refer to.
         */
		Reference(SimplePolygonBatch& batch, const size_t index) : batch(batch), index(index) {};

		bool operator ==(const Reference& other) const {
			return batch.simple_polygons[index] == other.batch.simple_polygons[other.index];
		}

		bool operator !=(const Reference& other) const {
			return batch.simple_polygons[index] != other.batch.simple_polygons[other.index];
		}

		const Point2& operator [](const size_t index) const {
			return batch.simple_polygons[this->index][index];
		}

		Point2& operator [](const size_t index) {
			return batch.simple_polygons[this->index][index];
		}

		void assign(const size_t count, const Point2& value) {
			batch.simple_polygons[index].assign(count, value);
		}

		template<class InputIterator>
		void assign(InputIterator begin, InputIterator end) {
			batch.simple_polygons[index].assign(begin, end);
		}

		void assign(const std::initializer_list<Point2> initialiser_list) {
			batch.simple_polygons[index].assign(initialiser_list);
		}

		const Point2& at(const size_t position) const {
			return batch.simple_polygons[index].at(position);
		}

		Point2& at(const size_t position) {
			return batch.simple_polygons[index].at(position);
		}

		const Point2& back() const {
			return batch.simple_polygons[index].back();
		}

		Point2& back() {
			return batch.simple_polygons[index].back();
		}

		iterator begin() {
			return batch.simple_polygons[index].begin();
		}

		const_iterator begin() const {
			return batch.simple_polygons[index].begin();
		}

		size_t capacity() const {
			return batch.simple_polygons[index].capacity();
		}

		const_iterator cbegin() const {
			return batch.simple_polygons[index].cbegin();
		}

		const_iterator cend() const {
			return batch.simple_polygons[index].cend();
		}

		void clear() noexcept {
			batch.simple_polygons[index].clear();
		}

		const_reverse_iterator crbegin() const {
			return batch.simple_polygons[index].crbegin();
		}

		const_reverse_iterator crend() const {
			return batch.simple_polygons[index].crend();
		}

		const Point2* data() const noexcept {
			return batch.simple_polygons[index].data();
		}

		Point2* data() noexcept {
			return batch.simple_polygons[index].data();
		}

		template<class... Args>
		iterator emplace(const const_iterator position, Args&&... arguments) {
			return batch.simple_polygons[index].emplace(position, arguments...);
		}

		template<class... Args>
		void emplace_back(Args&&... arguments) {
			return batch.simple_polygons[index].emplace_back(arguments...);
		}

		bool empty() const {
			return batch.simple_polygons[index].empty();
		}

		iterator end() {
			return batch.simple_polygons[index].end();
		}

		const_iterator end() const {
			return batch.simple_polygons[index].end();
		}

		iterator erase(const const_iterator position) {
			return batch.simple_polygons[index].erase(position);
		}

		iterator erase(const_iterator first, const const_iterator end) {
			return batch.simple_polygons[index].erase(first, end);
		}

		const Point2& front() const {
			return batch.simple_polygons[index].front();
		}

		Point2& front() {
			return batch.simple_polygons[index].front();
		}

		iterator insert(const const_iterator position, const Point2& value) {
			return batch.simple_polygons[index].insert(position, value);
		}

		iterator insert(const const_iterator position, Point2&& value) {
			return batch.simple_polygons[index].insert(position, value);
		}

		iterator insert(const const_iterator position, const size_t count, const Point2& value) {
			return batch.simple_polygons[index].insert(position, count, value);
		}

		template<class InputIterator>
		iterator insert(const const_iterator position, InputIterator begin, const InputIterator end) {
			return batch.simple_polygons[index].insert(position, begin, end);
		}

		iterator insert(const const_iterator position, const std::initializer_list<Point2> initialiser_list) {
			return batch.simple_polygons[index].insert(position, initialiser_list);
		}

		size_t max_size() const noexcept {
			return batch.simple_polygons[index].max_size();
		}

		void pop_back() {
			batch.simple_polygons[index].pop_back();
		}

		void push_back(const Point2& vertex) {
			batch.simple_polygons[index].push_back(vertex);
		}

		void push_back(Point2&& vertex) {
			batch.simple_polygons[index].push_back(vertex);
		}

		const_reverse_iterator rbegin() const {
			return batch.simple_polygons[index].rbegin();
		}

		reverse_iterator rbegin() {
			return batch.simple_polygons[index].rbegin();
		}

		const_reverse_iterator rend() const {
			return batch.simple_polygons[index].rend();
		}

		reverse_iterator rend() {
			return batch.simple_polygons[index].rend();
		}

		void reserve(const size_t new_capacity) {
			batch.simple_polygons[index].reserve(new_capacity);
		}

		void resize(const size_t new_size, const Point2& default_value = Point2(0, 0)) {
			batch.simple_polygons[index].resize(new_size, default_value);
		}

		void shrink_to_fit() noexcept {
			batch.simple_polygons[index].shrink_to_fit();
		}

		size_t size() const {
			return batch.simple_polygons[index].size();
		}
		
		void swap(SimplePolygonBatch::View& other) {
			batch.simple_polygons[index].storage().swap(other);
		}

		/*!
		 * Exchange the contents of this view on a simple polygon batch with the
		 * contents of another type of vertex storage for simple polygons: a
		 * vector.
		 *
		 * This allows swapping simple polygon data between two simple polygons
		 * with different storage types.
		 * \param other The vector of vertices to swap the data with.
		 */
		void swap(std::vector<Point2>& other) {
			batch.simple_polygons[index].storage().swap(other);
		}

		void swap(SimplePolygonBatch::Reference& other) {
			batch.simple_polygons[index].storage().swap(other.batch.simple_polygons[other.index].storage()); //Swap the views that each of these refer to.
		}

	protected:
		SimplePolygonBatch& batch;
		size_t index;
	};

	/*!
	 * This is an iterator that allows iterating over the simple polygons inside
	 * this batch.
	 *
	 * The iterator is of the random access type, mimicking that of a vector.
	 * You can freely iterate back and forth, jump ahead with multiple indices
	 * at once, repeatedly iterate, etc.
	 *
	 * Getting the underlying vector within the batch requires constructing the
	 * view on the batch and a simple polygon around it. This involves some
	 * performance cost. As always, try not to use the individual simple
	 * polygons within a batch; always use batch operations for maximum
	 * performance.
	 *
	 * Since the iterator points at a hypothetical simple polygon inside the
	 * batch and the view on the batch needs to have knowledge of which batch
	 * it's viewing on, this iterator also needs to store which batch it's
	 * viewing on. This has the downside that the iterator takes twice as much
	 * memory, in order to store a reference to the batch as well as which index
	 * in the batch. However it also has the upside that these iterators don't
	 * ever get invalidated as long as the same polygon is in that index.
	 * \tparam The type of the batch that this iterator is pointing to. This
	 * must be either a normal ``SimplePolygonBatch`` or a ``const
	 * SimplePolygonBatch``. Depending on whether this is const or not, the
	 * entire iterator will be const.
	 */
	template<typename BatchType>
	struct Iterator {
		/*!
		 * The batch that this iterator is iterating over.
		 */
		BatchType& batch;

		/*!
		 * The index of a simple polygon in the batch.
		 */
		size_t index;

		/*!
		 * Constructs a fresh ``Iterator`` pointing to a batch and an index
		 * inside that batch.
		 * \param batch The batch to which the iterator points.
		 * \param index The index of the simple polygon within that batch to
		 * which this iterator is currently pointing.
		 */
		Iterator(BatchType& batch, const size_t index) : batch(batch), index(index) {};

		/*!
		 * Copies an ``Iterator``.
		 * \param original The iterator to copy.
		 */
		Iterator(const Iterator<BatchType>& original) = default;

		/*!
		 * Assigns an ``Iterator`` to a different version, copying it.
		 * \param original The iterator to copy.
		 * \return A reference to this iterator.
		 */
		Iterator<BatchType>& operator =(const Iterator<BatchType>& original) = default;

		/*!
		 * Returns the simple polygon that the iterator is currently pointing
		 * at.
		 *
		 * This actually constructs a simple polygon based on a view on the
		 * batch. This is not very efficient. It's better to use batch
		 * operations on the batch, rather than working with individual
		 * polygons.
		 */
		SimplePolygon<const SimplePolygonBatch::View> operator *() const {
			return batch.simple_polygons[index];
		}

		/*!
		 * Check whether two iterators point to the same simple polygon.
		 * \param other The iterator to compare with.
		 * \return ``true`` if the other iterator points to the same simple
		 * polygon, or ``false`` if it doesn't.
		 */
		bool operator ==(const Iterator<BatchType>& other) const {
			return index == other.index && &batch == &other.batch;
		}

		/*!
		 * Check whether two iterators don't point to the same simple polygon.
		 * \param other The iterator to compare with.
		 * \return ``false`` if the other iterator points to the same simple
		 * polygon, or ``true`` if it doesn't.
		 */
		bool operator !=(const Iterator<BatchType>& other) const {
			return !(*this == other);
		}

		/*!
		 * Increments the iterator to point to the next simple polygon within
		 * the batch.
		 *
		 * This is the prefix increment operator. It's used as ``++it;``.
		 * \return A reference to the incremented iterator.
		 */
		Iterator<BatchType>& operator++() {
			++index;
			return *this;
		}

		/*!
		 * Increments the iterator to point to the next simple polygon within
		 * the batch.
		 *
		 * This is the postfix increment operator. It's used as ``it++;``.
		 * \return The position of the iterator before incrementing it.
		 */
		Iterator<BatchType> operator++(int) {
			Iterator<BatchType> result(*this); //A copy before incrementing to return.
			++index;
			return result;
		}
	};

public:
	/*!
	 * The iterator for iterating over the simple polygons in this batch.
	 *
	 * This iterator is a random access iterator, meaning you can iterate back
	 * and forth, jump around with multiple positions at once and everything.
	 * You can also make modifications to the batch using this iterator.
	 */
	using iterator = Iterator<SimplePolygonBatch>;

	/*!
	 * The iterator for iterating over the simple polygons in this batch, read-
	 * only mode.
	 *
	 * This iterator is a random access iterator, meaning you can iterate back
	 * and forth, jump around with multiple positions at once and everything.
	 */
	using const_iterator = Iterator<const SimplePolygonBatch>;

	/*!
	 * Construct a new vector of vectors, completely empty.
	 */
	SimplePolygonBatch() :
			next_position(0) {};

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
	SimplePolygonBatch(const size_t num_simple_polygons, const size_t vertices_per_polygon) :
			simple_polygons(num_simple_polygons, SimplePolygon<View>(*this, 0, 0, 0)), //Start at 0, 0 size and 0 capacity. Size and capacity will grow as necessary.
			next_position(0) {
		vertex_buffer.reserve(num_simple_polygons * vertices_per_polygon);
	}

	/*!
	 * Constructs a new batch of simple polygons, repeating a given simple
	 * polygon a number of times.
	 *
	 * The data of the simple polygon is copied a number of times. The memory
	 * reserved is exactly the amount necessary for all the copies of this
	 * simple polygon.
	 * \param num_simple_polygons How many copies of the given simple polygon to
	 * create.
	 * \param repeated_simple_polygon The simple polygon to copy.
	 */
	SimplePolygonBatch(const size_t num_simple_polygons, const SimplePolygon<>& repeated_simple_polygon) {
		const size_t vertices_per_polygon = repeated_simple_polygon.size();

		//Fill the entire index buffer first (for cache locality).
		simple_polygons.reserve(num_simple_polygons);
		for(size_t i = 0; i < num_simple_polygons; ++i) {
			simple_polygons.emplace_back(*this, i * vertices_per_polygon, vertices_per_polygon, vertices_per_polygon);
		}
		next_position = num_simple_polygons * vertices_per_polygon;

		//Fill in the vertex buffer then, repeating the same polygon over and over again.
		vertex_buffer.reserve(next_position);
		for(size_t i = 0; i < num_simple_polygons; ++i) {
			for(const Point2& vertex : repeated_simple_polygon) {
				vertex_buffer.push_back(vertex);
			}
		}
	}

	/*!
	 * Copies this batch.
	 *
	 * All of the data is actually copied. This is linear in the total amount of
	 * vertices in the batch.
	 * \param other The batch to copy into this one.
	 */
	SimplePolygonBatch(const SimplePolygonBatch& other) :
			vertex_buffer(other.vertex_buffer), //Copies all the vertex data at once.
			next_position(other.next_position) {
		//Copy the simple polygons one by one since we need to adjust their references to the batch.
		simple_polygons.reserve(other.simple_polygons.size());
		for(const SimplePolygon<View>& other_polygon : other.simple_polygons) {
			simple_polygons.emplace_back(*this, other_polygon.storage().start_index, other_polygon.storage().num_vertices, other_polygon.storage().current_capacity);
		}
	}

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
			next_position(std::move(other.next_position)) {
		//Copy the simple polygons one by one since we need to adjust their references to the batch.
		simple_polygons.reserve(other.simple_polygons.size());
		for(const SimplePolygon<View>& other_polygon : other.simple_polygons) {
			simple_polygons.emplace_back(*this, other_polygon.storage().start_index, other_polygon.storage().num_vertices, other_polygon.storage().current_capacity);
		}
	}

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
		//Copy the simple polygons one by one since we need to adjust their references to the batch.
		simple_polygons.reserve(other.simple_polygons.size());
		for(const SimplePolygon<View>& other_polygon : other.simple_polygons) {
			simple_polygons.emplace_back(*this, other_polygon.storage().start_index, other_polygon.storage().num_vertices, other_polygon.storage().current_capacity);
		}
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
		//Copy the simple polygons one by one since we need to adjust their references to the batch.
		simple_polygons.reserve(other.simple_polygons.size());
		for(const SimplePolygon<View>& other_polygon : other.simple_polygons) {
			simple_polygons.emplace_back(*this, other_polygon.storage().start_index, other_polygon.storage().num_vertices, other_polygon.storage().current_capacity);
		}
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
	const SimplePolygon<const Reference> operator [](const size_t position) const {
		/*This code uses a const_cast to remove the constness of the batch.
		This would be unsafe, but since we're building a SimplePolygon based on
		a const view, the const view will guard the constness of the batch. The
		operation is only unsafe if a copy is made of the actual view into a
		non-const variable. But since the VertexStorage itself is not exposed by
		SimplePolygon, you can't make such a copy.*/
		return SimplePolygon<const Reference>(const_cast<SimplePolygonBatch&>(*this), position);
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
	SimplePolygon<Reference> operator [](const size_t position) {
		return SimplePolygon<Reference>(*this, position);
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
	 * Gives an iterator to the first simple polygon in the batch.
	 *
	 * If the batch is empty, the returned iterator will be equal to ``end()``.
	 * \return An iterator to the first simple polygon in the batch.
	 */
	const_iterator begin() const {
		return const_iterator(*this, 0);
	}

	/*!
	 * Returns the number of simple polygons this batch can contain without
	 * needing to allocate more memory for the index buffer.
	 *
	 * This only concerns the index buffer which tracks the size, capacity and
	 * position of the vertex data for each simple polygon in the batch. This
	 * does not concern the vertex data. How many simple polygons the batch can
	 * actually hold without reallocating the vertex buffer depends on the size
	 * of these simple polygons and how often the have been reallocated before.
	 * \return The number of simple polygons this batch can contain without
	 * needing to allocate more memory for the index buffer.
	 */
	size_t capacity() const {
		return simple_polygons.capacity();
	}

	/*!
	 * Gives an iterator to the element beyond the last simple polygon in the
	 * batch.
	 * \return An iterator to beyond the last simple polygon in the batch.
	 */
	const_iterator end() const {
		return const_iterator(*this, size());
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
		size_t buffer_capacity = vertex_buffer.size();
		while(buffer_capacity < next_position + simple_polygon.size()) {
			buffer_capacity = buffer_capacity * 2 + 1;
		}
		simple_polygons.emplace_back(*this, next_position, simple_polygon.size(), simple_polygon.size());
		vertex_buffer.resize(buffer_capacity, Point2(0, 0));
		for(size_t i = 0; i < simple_polygon.size(); ++i) { //Copy the actual data into the batch.
			vertex_buffer[next_position + i] = simple_polygon[i];
		}
		next_position += simple_polygon.size();
	}

	/*!
	 * Returns the number of simple polygons in this batch.
	 * \return The number of simple polygons in this batch.
	 */
	size_t size() const {
		return simple_polygons.size();
	}
protected:
	/*!
	 * The list of simple polygons within this batch.
	 *
	 * The list of simple polygons is ordered, so the order matters. Changing
	 * the order means changing the batch.
	 *
	 * The simple polygons in this vector are based of the \ref View data
	 * source. This ``View`` refers to positions within the vertex buffer of the
	 * batch.
	 */
	std::vector<SimplePolygon<View>> simple_polygons;

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

	/*!
	 * Position within the \ref vertex_buffer where the next simple polygon
	 * would start from if a new range would need to be allocated.
	 *
	 * Since the vertex buffer is not completely contiguous (after some
	 * reallocations have occurred) it is not known where there could be room
	 * somewhere halfway the vertex buffer. Instead, we're going to allocate
	 * monotonically. The allocation will always occur at the end of the vertex
	 * buffer. Since the polygons can be out of order in the vertex buffer,
	 * finding the end of the vertex buffer would also be linear. Instead, we'll
	 * store the next position here so that we can get it in constant time.
	 */
	size_t next_position;
};

}

#endif //SIMPLE_POLYGON_BATCH_HPP