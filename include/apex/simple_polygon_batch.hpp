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
public:
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
		 * Constructs a new vertex in-place in the simple polygon in the batch.
		 * \param position The position within this simple polygon.
		 * \param arguments The constructor arguments of the vertex to add (the
		 * X and Y coordinates).
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
		 * Get the number of vertices in this simple polygon.
		 * \return The number of vertices in this simple polygon.
		 */
		size_t size() const {
			return batch.index_buffer[polygon_index * 3 + 3]; //+2 due to the two starting indices, then +1 because we're getting the size.
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
			size_t buffer_capacity = batch.vertex_buffer.capacity();
			while(buffer_capacity < new_place + new_capacity) {
				buffer_capacity = buffer_capacity * 2 + 1;
			}
			if(buffer_capacity > batch.vertex_buffer.capacity()) {
				batch.vertex_buffer.reserve(buffer_capacity);
			}

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
		for(size_t i = 0; i < num_simple_polygons; i++) {
			index_buffer.push_back(0); //Start of the range.
			index_buffer.push_back(0); //Size of this simple polygon.
			index_buffer.push_back(0); //Reserved memory of this simple polygon.
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
		if(vertex_buffer.capacity() < next_position + simple_polygon.size()) {
			vertex_buffer.reserve(vertex_buffer.capacity() * 2);
		}
		index_buffer.push_back(next_position); //Position of this polygon.
		index_buffer.push_back(simple_polygon.size()); //Size of the polygon.
		index_buffer.push_back(simple_polygon.size()); //Reserved memory.
		for(size_t i = 0; i < simple_polygon.size(); ++i) { //Copy the actual data into the batch.
			vertex_buffer.push_back(simple_polygon[i]);
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

protected:
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