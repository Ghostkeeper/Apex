/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2020 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON
#define APEX_SIMPLE_POLYGON

#include <utility> //For std::forward.
#include <vector> //To store the vertex data.

#include "point2.hpp" //The vertices of the polygon are 2D points.

//Implementations separated out for readability using the Curiously Recurring Template Pattern.
#include "detail/simple_polygon_area.hpp"
#include "detail/simple_polygon_translate.hpp"

namespace apex {

/*!
 * A plane figure consisting of a single contour of straight line segments.
 *
 * This is a closed shape, represented by a list of vertices in 2D. Between
 * every two adjacent vertices, as well as between the first and last vertices,
 * is an edge. These edges together form a closed shape that is the contents of
 * the simple polygon.
 *
 * Technically this class represents a "closed polygonal chain". This polygon
 * can only have one continuous surface and cannot have holes, since it is
 * simple. However contrary to actual simple polygons, this polygon is allowed
 * to self-intersect.
 *
 * The polygon does not publicly have a start or end point. However its data
 * model must start somewhere and iterating over the vertices must choose a
 * vertex to start and end at.
 *
 * If the vertices of the polygon are winding counter-clockwise, the polygon is
 * positive. Otherwise it is negative.
 * \tparam VertexStorage The underlying data structure to use for this simple
 * polygon. By default this is a ``vector``, which allows the simple polygon to
 * be used separately. You can also use the view on a simple polygon batch to
 * use one of the elements in the batch separately this way. Any class that
 * implements the signature of a ``vector`` of ``Point2``s will do.
 */
template<typename VertexStorage = std::vector<Point2>>
class SimplePolygon :
		//Implementing the private functions in separate classes with Curiously Recurring Template Pattern.
		public SimplePolygonArea<SimplePolygon<VertexStorage>>,
		public SimplePolygonTranslate<SimplePolygon<VertexStorage>> {
public:
	/*!
	 * Iterates one loop around the polygon.
	 *
	 * This implementation hides the actual iterator used for the underlying
	 * data structures.
	 */
	typedef typename VertexStorage::iterator iterator;

	/*!
	 * Iterates one loop around the polygon.
	 *
	 * This implementation hides the actual iterator used for the underlying
	 * data structures.
	 */
	typedef typename VertexStorage::const_iterator const_iterator;

	/*!
	 * Iterates one loop around the polygon in reverse.
	 *
	 * This implementation hides the actual iterator used for the underlying
	 * data structures.
	 */
	typedef typename VertexStorage::reverse_iterator reverse_iterator;

	/*!
	 * Iterates one loop around the polygon in reverse.
	 *
	 * This implementation hides the actual iterator used for the underlying
	 * data structures.
	 */
	typedef typename VertexStorage::const_reverse_iterator const_reverse_iterator;

	/*!
	 * Copies a simple polygon.
	 *
	 * This also copies the vertex storage method, which may copy the actual
	 * vertex data (depending on the implementation).
	 * \param other The simple polygon to copy into this one.
	 * \tparam OtherVertexStorage The vertex storage type of the original simple
	 * polygon. This must match the vertex storage type of this simple polygon.
	 * If the other simple polygon doesn't have the same VertexStorage, the copy
	 * constructor will give a compilation error.
	 */
	template<typename OtherVertexStorage>
	SimplePolygon(SimplePolygon<OtherVertexStorage>& other) : vertices(other.vertices) {}

	/*!
	 * Copies a simple polygon.
	 *
	 * This also copies the vertex storage method, which may copy the actual
	 * vertex data (depending on the implementation).
	 * \param other The simple polygon to copy into this one.
	 * \tparam OtherVertexStorage The vertex storage type of the original simple
	 * polygon. This must match the vertex storage type of this simple polygon.
	 * If the other simple polygon doesn't have the same VertexStorage, the copy
	 * constructor will give a compilation error.
	 */
	template<typename OtherVertexStorage>
	SimplePolygon(const SimplePolygon<OtherVertexStorage>& other) : vertices(other.vertices) {}

	/*!
	 * Moves a simple polygon into this polygon.
	 *
	 * The vertex storage is moved directly. If the vertex storage supports it,
	 * this will also move the vertex storage which may in many cases prevent
	 * moving or copying the actual vertex data at all, making this a no-op and
	 * perform very well.
	 * \param other The simple polygon to move into this one.
	 * \tparam OtherVertexStorage The vertex storage type of the original simple
	 * polygon. This must match the vertex storage type of this simple polygon.
	 * If the other simple polygon doesn't have the same VertexStorage, the move
	 * operation will give a compilation error.
	 */
	template<typename OtherVertexStorage>
	SimplePolygon(SimplePolygon<OtherVertexStorage>&& other) : vertices(std::move(other.vertices)) {}

	/*!
	 * Creates a simple polygon filled with the same vertex multiple times.
	 *
	 * This by itself is not very useful as a shape, since all of the vertices
	 * will be on top of each other and the simple polygon will be degenerate.
	 * However you may use this in an algorithm where you first fill the polygon
	 * with arbitrary vertices and then access those vertices with random
	 * access.
	 */
	SimplePolygon(const size_t count, const Point2& value) : vertices(count, value) {}

	/*!
	 * Initialises the simple polygon with the constructor arguments of the
	 * vertex storage.
	 * \param storage_constructor_arguments The constructor arguments for the
	 * vertex storage type, to construct the vertex storage with.
	 * \tparam The constructor arguments of the vertex storage type.
	 */
	template<typename... Args>
	SimplePolygon(Args&&... storage_constructor_arguments) : vertices(std::forward<Args>(storage_constructor_arguments)...) {}

	/*!
	 * Copy assignment operator, to assign another simple polygon onto this
	 * simple polygon.
	 *
	 * This will copy the vertex storage data structure, which may copy the
	 * actual vertex data.
	 * \param other The simple polygon to assign to this simple polygon.
	 * \tparam OtherVertexStorage The vertex storage type of the other simple
	 * polygon. This must match the vertex storage type of this simple polygon.
	 * If the other simple polygon doesn't have the same VertexStorage, the
	 * assignment operation will give a compilation error.
	 */
	template<typename OtherVertexStorage>
	const SimplePolygon& operator =(const SimplePolygon<OtherVertexStorage>& other) {
		vertices = other.vertices;
	}

	/*!
	 * Move assignment operator, to move another simple polygon into this simple
	 * polygon.
	 *
	 * In some cases this will be a no-op, making it very fast to execute.
	 * \param other The simple polygon to assign to this simple polygon.
	 * \tparam OtherVertexStorage The vertex storage type of the other simple
	 * polygon. This must match the vertex storage type of this simple polygon.
	 * If the other simple polygon doesn't have the same VertexStorage, the
	 * assignment operation will give a compilation error.
	 */
	template<typename OtherVertexStorage>
	SimplePolygon& operator =(SimplePolygon<OtherVertexStorage>&& other) {
		vertices = std::move(other.vertices);
	}

	/*!
	 * Provides access to the individual vertices of the simple polygon.
	 *
	 * There is no real start or end to a simple polygon since it's a closed
	 * loop. Which specific vertex becomes the first vertex is not defined and
	 * cannot be relied upon after operations like clipping have been applied.
	 *
	 * If the simple polygon is modified within OpenCL, invoking this operator
	 * may trigger the application to wait until those operations have been
	 * completed and downloaded to the host.
	 * \param index The index of the vertex to get. Only when the simple polygon
	 * remains unmodified will this operator be consistent.
	 * \return The vertex at the specified index.
	 */
	const Point2& operator [](const size_t index) const {
		return vertices[index];
	}

	/*!
	 * Provides access to the individual vertices of the simple polygon.
	 *
	 * There is no real start or end to a simple polygon since it's a closed
	 * loop. Which specific vertex becomes the first vertex is not defined and
	 * cannot be relied upon after operations like clipping have been applied.
	 *
	 * If the simple polygon is modified within OpenCL, invoking this operator
	 * may trigger the application to wait until those operations have been
	 * completed and downloaded to the host.
	 * \param index The index of the vertex to get. Only when the simple polygon
	 * remains unmodified will this operator be consistent.
	 * \return The vertex at the specified index.
	 */
	Point2& operator [](const size_t index) {
		return vertices[index];
	}

	/*!
	 * Tests whether this simple polygon is equal to another.
	 *
	 * Two polygons are the same if they share the same set of vertices in the
	 * same order. However if they start at a different vertex around the
	 * contour, they may still be the same.
	 * \param other The polygon to test against.
	 * \return ``true`` if this polygon is the same as the other polygon, or
	 * ``false`` otherwise.
	 */
	bool operator ==(const SimplePolygon& other) const {
		//TODO: Put implementation in separate file and allow multiple implementations.
		if(size() != other.size()) {
			return false;
		}
		if(empty()) { //Both are empty.
			return true; //Don't go looking for the first vertex. There is none.
		}
		//Find first vertex.
		size_t vertex_offset = -1;
		for(size_t i = 0; i < other.size(); ++i) {
			if((*this)[0] == other[i]) {
				vertex_offset = i;
				break;
			}
		}
		if(vertex_offset == static_cast<size_t>(-1)) {
			return false; //First vertex is not in the other polygon.
		}
		//Now check if all vertices are the same, giving an offset for the check in the second polygon.
		for(size_t i = 0; i < size(); ++i) {
			if((*this)[i] != other[(i + vertex_offset) % other.size()]) {
				return false;
			}
		}
		return true;
	}

	/*!
	 * Tests whether this simple polygon is inequal to another.
	 *
	 * Two polygons are different if they have different sets of vertices or if
	 * the vertices appear in a different order. The starting point of the
	 * iterations is irrelevant; two vertices are not different if they have the
	 * same set of vertices in the same order but their iteration starts in a
	 * different location along the contour.
	 * \param other The polygon to test against.
	 * \return ``true`` if this polygon is different from the other polygon, or
	 * ``false`` otherwise.
	 */
	bool operator !=(const SimplePolygon& other) const {
		return !((*this) == other); //Implemented in terms of ==.
	}

	/*!
	 * Replaces the contents of the simple polygon with the specified vertex
	 * repeated.
	 *
	 * The resulting simple polygon will be a set of points on top of each
	 * other, which is probably useless. However you may still want to use this
	 * to create an appropriately-sized container and assign correct coordinates
	 * to the vertices afterwards in your algorithm.
	 * \param count The number of repetitions of the value.
	 * \param value The vertex to add multiple times.
	 */
	void assign(const size_t count, const Point2& value) {
		vertices.assign(count, value);
	}

	/*!
	 * Replaces the contents of the simple polygon with the contents of the
	 * range of elements between the specified two iterators.
	 * \param first The start of the iterator range.
	 * \param last The end of the iterator range.
	 */
	template<class InputIterator>
	void assign(InputIterator first, InputIterator last) {
		vertices.assign(first, last);
	}

	/*!
	 * Replaces the contents of the simple polygon with the initialiser list.
	 * \param initialiser_list The list to replace the polygon with.
	 */
	void assign(std::initializer_list<Point2> initialiser_list) {
		vertices.assign(initialiser_list);
	}

	/*!
	 * Returns a vertex at the specified position along the contour of the
	 * simple polygon, with bounds checking.
	 *
	 * If the position is not within the range of the container, an exception of
	 * type ``std::out_of_range`` is thrown.
	 */
	Point2& at(const size_t position) {
		return vertices.at(position);
	}

	/*!
	 * Returns a vertex at the specified position along the contour of the
	 * simple polygon, with bounds checking.
	 *
	 * If the position is not within the range of the container, an exception of
	 * type ``std::out_of_range`` is thrown.
	 */
	const Point2& at(const size_t position) const {
		return vertices.at(position);
	}

	/*!
	 * Returns the last vertex in the iteration of the polygon.
	 *
	 * No guarantees can be given if this vertex is consistent after
	 * transforming the polygon.
	 */
	Point2& back() {
		return vertices.back();
	}

	/*!
	 * Returns the last vertex in the iteration of the polygon.
	 *
	 * No guarantees can be given if this vertex is consistent after
	 * transforming the polygon.
	 */
	const Point2& back() const {
		return vertices.back();
	}

	/*!
	 * Gives an iterator to one of the vertices of the simple polygon.
	 *
	 * There is no beginning to the closed loop around the simple polygon. The
	 * vertex that this iterator will begin with is therefore arbitrary. The
	 * iterator will hit the iterator returned by ``end()`` after one complete
	 * iteration around the contour.
	 */
	iterator begin() noexcept {
		return vertices.begin();
	}

	/*!
	 * Gives an iterator to one of the vertices of the simple polygon.
	 *
	 * There is no beginning to the closed loop around the simple polygon. The
	 * vertex that this iterator will begin with is therefore arbitrary. The
	 * iterator will hit the iterator returned by ``end()`` after one complete
	 * iteration around the contour.
	 */
	const_iterator begin() const noexcept {
		return vertices.begin();
	}

	/*!
	 * Returns the number of vertices this simple polygon could contain without
	 * needing to resize its memory.
	 */
	size_t capacity() const noexcept {
		return vertices.capacity();
	}

	/*!
	 * Gives an iterator to one of the vertices of the simple polygon.
	 *
	 * There is no beginning to the closed loop around the simple polygon. The
	 * vertex that this iterator will begin with is therefore arbitrary. The
	 * iterator will hit the iterator returned by ``cend()`` after one complete
	 * iteration around the contour.
	 */
	const_iterator cbegin() const noexcept {
		return vertices.cbegin();
	}

	/*!
	 * Gives the end of the iteration as returned by ``cbegin()``.
	 *
	 * If this iterator is hit, one complete loop around the simple polygon has
	 * been made.
	 */
	const_iterator cend() const noexcept {
		return vertices.cend();
	}

	/*!
	 * Removes all vertices from the simple polygon.
	 */
	void clear() noexcept {
		vertices.clear();
	}

	/*!
	 * Gives an iterator to one of the vertices of the simple polygon that
	 * iterates in reverse order along the contour of the simple polygon.
	 *
	 * There is no beginning to the closed loop around the simple polygon. The
	 * vertex that this iterator will begin with is therefore arbitrary. The
	 * iterator will hit the iterator returned by ``crend()`` after one complete
	 * iteration around the contour.
	 */
	const_reverse_iterator crbegin() const noexcept {
		return vertices.crbegin();
	}

	/*!
	 * Gives the end of the iteration as returned by ``crbegin()``.
	 *
	 * If this iterator is hit, one complete loop around the simple polygon has
	 * been made.
	 */
	const_reverse_iterator crend() const noexcept {
		return vertices.crend();
	}

	/*!
	 * Returns a pointer to the underlying array serving as element storage.
	 *
	 * The pointer is such that the range ``[data(); data() + size()]`` is
	 * always a valid range, even if the container is empty (``data()`` is not
	 * dereferenceable in that case).
	 */
	Point2* data() noexcept {
		return vertices.data();
	}

	/*!
	 * Returns a pointer to the underlying array serving as element storage.
	 *
	 * The pointer is such that the range ``[data(); data() + size()]`` is
	 * always a valid range, even if the container is empty (``data()`` is not
	 * dereferenceable in that case).
	 */
	const Point2* data() const noexcept {
		return vertices.data();
	}

	/*!
	 * Adds a vertex to the simple polygon by providing the ``Point2``'s
	 * constructor arguments and a position to insert it at.
	 *
	 * The constructor arguments are passed on to the constructor of ``Point2``.
	 * It constructs a point with them, and adds it to this simple polygon.
	 * \param position The position in the polygon to insert the vertex at.
	 * \param arguments The arguments used to construct the vertices.
	 */
	template<class... Args>
	void emplace(const_iterator position, Args&&... arguments) {
		vertices.emplace(position, arguments...);
	}

	/*!
	 * Adds a vertex to the simple polygon by providing the ``Point2``'s
	 * constructor arguments.
	 *
	 * The constructor arguments are passed on to the constructor of ``Point2``.
	 * It constructs a point with them, and adds it to this simple polygon.
	 * \param arguments The arguments used to construct the vertices.
	 */
	template<class... Args>
	void emplace_back(Args&&... arguments) {
		vertices.emplace_back(arguments...);
	}

	/*!
	 * Checks whether the container has no elements.
	 */
	bool empty() const {
		return vertices.empty();
	}

	/*!
	 * Gives the end of the iteration as returned by ``begin()``.
	 *
	 * If this iterator is hit, one complete loop around the simple polygon has
	 * been made.
	 */
	iterator end() noexcept {
		return vertices.end();
	}

	/*!
	 * Gives the end of the iteration as returned by ``begin()``.
	 *
	 * If this iterator is hit, one complete loop around the simple polygon has
	 * been made.
	 */
	const_iterator end() const noexcept {
		return vertices.end();
	}

	/*!
	 * Removes one vertex from the simple polygon at the specified position.
	 * \param position An iterator referencing the position of the vertex to
	 * remove.
	 * \return An iterator of the vertex after the one removed.
	 */
	iterator erase(const_iterator position) {
		return vertices.erase(position);
	}

	/*!
	 * Removes a range of vertices from the simple polygon at the specified
	 * position.
	 * \param first The start of the range to remove.
	 * \param last The vertex past the end of the range to remove.
	 * \return The ``last`` iterator provided.
	 */
	iterator erase(const_iterator first, const_iterator last) {
		return vertices.erase(first, last);
	}

	/*!
	 * Returns the first vertex in the iteration of the polygon.
	 *
	 * No guarantees can be given if this vertex is consistent after
	 * transforming the polygon.
	 */
	Point2& front() {
		return vertices.front();
	}

	/*!
	 * Returns the first vertex in the iteration of the polygon.
	 *
	 * No guarantees can be given if this vertex is consistent after
	 * transforming the polygon.
	 */
	const Point2& front() const {
		return vertices.front();
	}

	/*!
	 * Inserts a new vertex at a specified position in the simple polygon.
	 * \param position The vertex is inserted before this position in the loop.
	 * \param vertex The coordinates of the new vertex.
	 * \return An iterator pointing to the newly inserted vertex.
	 */
	iterator insert(const_iterator position, const Point2& vertex) {
		return vertices.insert(position, vertex);
	}

	/*!
	 * Inserts a new vertex at a specified position in the simple polygon.
	 *
	 * The vertex is moved in the polygon without copying it.
	 * \param position The vertex is inserted before this position in the loop.
	 * \param vertex The coordinates of the new vertex.
	 * \return An iterator pointing to the newly inserted vertex.
	 */
	iterator insert(const_iterator position, Point2&& vertex) {
		return vertices.insert(position, vertex);
	}

	/*!
	 * Inserts a number of copies of a new vertex at a specified position in the
	 * simple polygon.
	 *
	 * This by itself is not very useful for polygons, but an algorithm that
	 * inserts a bunch of vertices may use this as a way to make space for
	 * actual vertex data that is filled in later. Inserting a bunch of vertices
	 * at once is more efficient than inserting them one by one.
	 * \param position The vertex is inserted before this position in the loop.
	 * \param count The number of copies to insert.
	 * \param vertex The coordinates of the new vertex.
	 * \return An iterator pointing to the newly inserted vertex.
	 */
	iterator insert(const const_iterator position, const size_t count, const Point2& vertex) {
		return vertices.insert(position, count, vertex);
	}

	/*!
	 * Inserts a range of vertices between ``first`` and ``last`` at a speciied
	 * position in the simple polygon.
	 * \param position The vertices are inserted before this position in the
	 * loop.
	 * \param first The first vertex to insert in the simple polygon.
	 * \param last The vertex after the last vertex to insert in the simple
	 * polygon.
	 */
	template<class InputIterator>
	void insert(const_iterator position, InputIterator first, InputIterator last) {
		vertices.insert(position, first, last);
	}

	/*!
	 * Inserts a range of vertices at a specified position in the simple
	 * polygon.
	 * \param position The vertices are inserted before this position in the
	 * loop.
	 * \param initialiser_list A list of vertices to insert.
	 * \return An iterator pointing to the first element inserted, or the
	 * ``position`` parameter if the list was empty.
	 */
	iterator insert(const_iterator position, std::initializer_list<Point2> initialiser_list) {
		return vertices.insert(position, initialiser_list);
	}

	/*!
	 * Gives the maximum number of vertices that this simple polygon is able to
	 * hold due to implementation limitations.
	 *
	 * This will depend on the OpenCL devices available.
	 * \return The maximum number of vertices that this simple polygon is able
	 * to hold.
	 */
	size_t max_size() const noexcept {
		return vertices.max_size();
	}

	/*!
	 * Removes the last vertex from the simple polygon.
	 */
	void pop_back() {
		vertices.pop_back();
	}

	/*!
	 * Adds a vertex at the end of the iteration of the vertices of this simple
	 * polygon.
	 * \param vertex The new vertex to add.
	 */
	void push_back(const Point2& vertex) {
		vertices.push_back(vertex);
	}

	/*!
	 * Adds a vertex at the end of the iteration of the vertices of this simple
	 * polygon.
	 * \param vertex The new vertex to move into the simple polygon.
	 */
	void push_back(const Point2&& vertex) {
		vertices.push_back(vertex);
	}

	/*!
	 * Gives an iterator to one of the vertices of the simple polygon that
	 * iterates in reverse order along the contour of the simple polygon.
	 *
	 * There is no beginning to the closed loop around the simple polygon. The
	 * vertex that this iterator will begin with is therefore arbitrary. The
	 * iterator will hit the iterator returned by ``rend()`` after one complete
	 * iteration around the contour.
	 */
	reverse_iterator rbegin() noexcept {
		return vertices.rbegin();
	}

	/*!
	 * Gives an iterator to one of the vertices of the simple polygon that
	 * iterates in reverse order along the contour of the simple polygon.
	 *
	 * There is no beginning to the closed loop around the simple polygon. The
	 * vertex that this iterator will begin with is therefore arbitrary. The
	 * iterator will hit the iterator returned by ``rend()`` after one complete
	 * iteration around the contour.
	 */
	const_reverse_iterator rbegin() const noexcept {
		return vertices.rbegin();
	}

	/*!
	 * Gives the end of the iteration as returned by ``rbegin()``.
	 *
	 * If this iterator is hit, one complete loop around the simple polygon has
	 * been made.
	 */
	reverse_iterator rend() noexcept {
		return vertices.rend();
	}

	/*!
	 * Gives the end of the iteration as returned by ``rbegin()``.
	 *
	 * If this iterator is hit, one complete loop around the simple polygon has
	 * been made.
	 */
	const_reverse_iterator rend() const noexcept {
		return vertices.rend();
	}

	/*!
	 * Increase the capacity of the simple polygon such that at least
	 * ``new_capacity`` number of vertices will vit in its memory without
	 * allocating more memory.
	 *
	 * More memory may still be reserved in OpenCL should the need arise.
	 * \param new_capacity The new minimum capacity for the simple polygon.
	 */
	void reserve(size_t new_capacity) {
		vertices.reserve(new_capacity);
	}

	/*!
	 * Makes the simple polygon use as little memory as possible by removing any
	 * unused capacity.
	 *
	 * It depends on the implementation whether this request is fulfilled. There
	 * are no guarantees. A reallocation might occur, in which case any
	 * iterators and pointers will be invalidated.
	 */
	void shrink_to_fit() {
		vertices.shrink_to_fit();
	}

	/*!
	 * Gives the number of vertices in the simple polygon.
	 * \return The number of vertices in the simple polygon.
	 */
	size_t size() const noexcept {
		return vertices.size();
	}

	/*!
	 * Swaps the contents of this simple polygon instance with that of another.
	 */
	void swap(SimplePolygon& other) {
		vertices.swap(other.vertices);
	}

protected:
	/*!
	 * The vertices contained in this simple polygon.
	 *
	 * This is a local copy. All local algorithms run on this data set. However,
	 * if the polygon was modified remotely via OpenCL, this data set may be
	 * outdated. Upon first accessing the data locally, this local data set will
	 * be updated.
	 */
	VertexStorage vertices;
};

}

namespace std {

/*!
 * Swaps the contents of two simple polygons.
 *
 * This specialises the std::swap algorithm for simple polygons. The contents of
 * the two simple polygons gets swapped in constant time.
 * \param lhs One of the containers to swap.
 * \param rhs The other container to swap.
 */
template<typename VertexStorage>
void swap(apex::SimplePolygon<VertexStorage>& lhs, apex::SimplePolygon<VertexStorage>& rhs) {
	lhs.swap(rhs);
}

}

#endif //APEX_SIMPLE_POLYGON