/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_POLYGON
#define APEX_POLYGON

#include <utility> //For std::forward.

#include "batch.hpp" //The vertex storage is by batch.
#include "detail/polygon_properties.hpp" //Properties about polygons to cache.
#include "operations/area.hpp" //To allow calculating the area of this shape.
#include "operations/translate.hpp" //To allow moving this shape.
#include "point2.hpp" //The vertices of the polygon are 2D points.

namespace apex {

/*!
 * A plane figure consisting of a single contour of straight line segments.
 *
 * This is a closed shape, represented by a list of vertices in 2D. Between
 * every two adjacent vertices, as well as between the first and last vertices,
 * is an edge. These edges together form a closed shape that is the contents of
 * the polygon.
 *
 * Since the shape only has a single closed polygonal chain, it cannot have
 * multiple boundaries, like a doughnut-shape with a hole inside. This would be
 * a multi-polygon. However the polygon may be self-intersecting. It does not
 * need to be a simple polygon. Operations on the polygon are expected to deal
 * with all such kinds of polygons correctly.
 *
 * The polygon does not publicly have a start or end point. However its data
 * model must start somewhere and iterating over the vertices must choose a
 * vertex to start and end at. Operations on the polygons should behave the same
 * regardless of where the polygon starts its iteration.
 *
 * If the vertices of the polygon are winding counter-clockwise, the polygon is
 * positive. Otherwise it is negative.
 */
class Polygon : public Batch<Point2> {
public:
	/*!
	 * Constructs an empty polygon, without any vertices.
	 *
	 * The polygon will be degenerate, since it has no vertices.
	 */
	Polygon() noexcept : Batch<Point2>(),
		properties(
			static_cast<unsigned int>(PolygonProperties::Convexity::DEGENERATE)
			| static_cast<unsigned int>(PolygonProperties::SelfIntersecting::NO)
			| static_cast<unsigned int>(PolygonProperties::Orientation::POSITIVE)
		) {}

	/*!
	 * Constructs a polygon containing a single point repeated numerous times.
	 *
	 * The resulting polygon will be degenerate and is probably not so useful in
	 * practical applications. However it could be useful to have a certain
	 * number of vertices in before editing the polygon.
	 * \param count The number of copies of the vertex to add.
	 * \param vertex The vertex to add copies of. If no vertex is given, the
	 * vertex at position [0, 0] will be stored.
	 */
	Polygon(const size_t count, const Point2& vertex = Point2(0, 0)) : Batch<Point2>(count, vertex),
		properties(
			static_cast<unsigned int>(PolygonProperties::Convexity::DEGENERATE)
			| static_cast<unsigned int>(PolygonProperties::SelfIntersecting::UNKNOWN) //Would be an edge case if count >= 2.
			| static_cast<unsigned int>(PolygonProperties::Orientation::POSITIVE)
		) {}

	/*!
	 * Constructs a polygon, filling it with the vertices in the given range.
	 * \param begin The first vertex to insert.
	 * \param end The position beyond the last vertex to insert.
	 */
	template<class InputIterator>
	Polygon(const InputIterator begin, const InputIterator end) : Batch<Point2>(begin, end),
		properties(0) {} //Properties are completely unknown.

	/*!
	 * Constructs a polygon, filled with the vertices from this initialiser
	 * list.
	 * \param vertices The vertices to add to the polygon.
	 */
	Polygon(const std::initializer_list<Point2>& vertices) : Batch<Point2>(vertices),
		properties(0) {} //Properties are completely unknown.

	/*!
	 * Copies the vertices of the subbatch into a polygon.
	 * \param subbatch The batch to construct a polygon with.
	 */
	Polygon(const Subbatch<Point2>& subbatch) : Batch<Point2>(subbatch),
		properties(0) {} //Properties are completely unknown. It doesn't necessarily come from a polygon.

	/*!
	 * Converts a batch into a polygon.
	 * \param batch The batch to convert.
	 */
	Polygon(const Batch<Point2>& batch) : Batch<Point2>(batch),
		properties(0) {} //Properties are completely unknown.

	/*!
	 * Copies a polygon.
	 * \param original The polygon to copy.
	 */
	Polygon(const Polygon& original) : Batch<Point2>(original),
		properties(original.properties) {} //The same properties as the original.

	/*!
	 * Move constructor to move a polygon into a different polygon.
	 *
	 * This move constructor doesn't require a copy of the polygon data, and it
	 * can execute in constant time.
	 * \param original The polygon to move.
	 */
	Polygon(Polygon&& original) : Batch<Point2>(original),
		properties(original.properties) {} //The same properties as the original.

	/*!
	 * Assigns a different polygon to this polygon.
	 *
	 * This makes this polygon copy the contents from the other polygon.
	 * \param other The polygon to assign to this one.
	 * \return A reference to this polygon.
	 */
	Polygon& operator =(const Polygon& other) {
		Batch<Point2>::operator =(other);
		properties = other.properties; //Also copy its properties
		return *this;
	}

	/*!
	 * Assigns a different polygon to this polygon.
	 *
	 * This moves the contents of the other polygon into this one. The vertex
	 * data can have just its reference moved, saving work.
	 * \param other The polygon to assign to this one.
	 * \return A reference to this polygon.
	 */
	Polygon& operator =(Polygon&& other) noexcept {
		Batch<Point2>::operator =(other);
		properties = other.properties;
		return *this;
	}

	/*!
	 * Tests whether this polygon is equal to another.
	 *
	 * Two polygons are the same if they share the same set of vertices in the
	 * same order. However if they start at a different vertex around the
	 * contour, they may still be the same.
	 * \param other The polygon to test against.
	 * \return ``true`` if this polygon is the same as the other polygon, or
	 * ``false`` otherwise.
	 */
	bool operator ==(const Polygon& other) const {
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
	 * Tests whether this polygon is unequal to another.
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
	bool operator !=(const Polygon& other) const {
		return !((*this) == other); //Implemented in terms of ==.
	}

	/*!
	 * Computes the surface area of the polygon.
	 *
	 * The sign of the area is linked to the polygon winding order. If the
	 * polygon is positive, the area will be positive too, and vice versa. If
	 * the polygon intersects itself, parts of the polygon will be subtracting
	 * from the area while other parts add up to the area.
	 *
	 * The area of the polygon is counted differently from the nonzero or
	 * even-odd fill rules. If a zone is looped around multiple times by the
	 * polygon, it will count to the total area multiple times as well.
	 * \return The surface area of the polygon.
	 */
	area_t area() const {
		return apex::area(*this);
	}

	/*!
	 * Replaces the content of the polygon with a vertex, repeated a number of
	 * times.
	 *
	 * For most applications this will not be useful by itself. The polygon will
	 * be degenerate. However it may serve as a starting point for modifying the
	 * polygon afterwards.
	 * \param count The number of vertices to add.
	 * \param vertex The vertex to add multiple times.
	 */
	void assign(const size_t count, const Point2& vertex) {
		properties = static_cast<unsigned int>(PolygonProperties::Convexity::DEGENERATE)
			| static_cast<unsigned int>(PolygonProperties::SelfIntersecting::UNKNOWN) //Would be an edge case if count >= 2.
			| static_cast<unsigned int>(PolygonProperties::Orientation::POSITIVE);
		Batch<Point2>::assign(count, vertex);
	}

	/*!
	 * Replaces the content of the polygon with the vertices in the given range.
	 * \param first An iterator pointing to the first vertex to insert.
	 * \param last An iterator indicating to beyond the last vertex to insert.
	 */
	template<class InputIterator>
	void assign(InputIterator first, InputIterator last) {
		properties.reset();
		Batch<Point2>::assign(first, last);
	}

	/*!
	 * Replaces the content of the polygon with the vertices in the given list.
	 * \param vertices An initialiser list containing vertices to assign to this
	 * polygon.
	 */
	void assign(const std::initializer_list<Point2>& vertices) {
		properties.reset();
		Batch<Point2>::assign(vertices);
	}

	/*!
	 * Empties out the polygon, resulting in an empty polygon.
	 */
	void clear() noexcept {
		properties = static_cast<unsigned int>(PolygonProperties::Convexity::DEGENERATE)
			| static_cast<unsigned int>(PolygonProperties::SelfIntersecting::NO)
			| static_cast<unsigned int>(PolygonProperties::Orientation::POSITIVE);
		Batch<Point2>::clear();
	}

	/*!
	 * Constructs a vertex in-place at the specified position in the polygon.
	 *
	 * The vertex will be constructed directly inside of the memory allocated
	 * for the polygon, possibly saving a copy.
	 * \param position The position where to insert the new vertex.
	 * \param arguments The constructor arguments for \ref Point2.
	 * \return The position of the newly inserted vertex.
	 */
	template<typename... Arguments>
	iterator emplace(const_iterator position, Arguments... arguments) {
		properties.reset();
		return Batch<Point2>::emplace(position, arguments...);
	}

	/*!
	 * Constructs a vertex in-place at the seam of the polygon.
	 *
	 * The vertex will be constructed directly inside of the memory allocated
	 * for the polygon, possibly saving a copy.
	 * \param arguments The constructor arguments for \ref Point2.
	 */
	template<typename... Arguments>
	void emplace_back(Arguments... arguments) {
		properties.reset();
		Batch<Point2>::emplace_back(arguments...);
	}

	/*!
	 * Erase the vertex in the specified position.
	 *
	 * The vertices around that position will be joined with a new edge.
	 * \param position An iterator pointing to the edge to remove.
	 * \return An iterator referring to the vertex after the removed vertex.
	 */
	iterator erase(const_iterator position) {
		properties.reset();
		return Batch<Point2>::erase(position);
	}

	/*!
	 * Erase the vertices in the specified range.
	 *
	 * All vertices in that range are removed, and the vertices around it are
	 * joined together.
	 * \param first The first element of the range to erase.
	 * \param last An iterator referring to the vertex after the last removed
	 * vertex.
	 * \return An iterator referring to the vertex after the last removed
	 * vertex.
	 */
	iterator erase(const_iterator first, const_iterator last) {
		properties.reset();
		return Batch<Point2>::erase(first, last);
	}

	/*!
	 * Insert a vertex in a certain place in the polygonal chain.
	 * \param position The vertex will be inserted before this position.
	 * \param vertex The new coordinate to insert.
	 * \return An iterator referring to the newly inserted vertex.
	 */
	iterator insert(const_iterator position, const Point2& vertex) {
		properties.reset();
		return Batch<Point2>::insert(position, vertex);
	}

	/*!
	 * Insert a vertex in a certain place in the polygonal chain.
	 *
	 * The vertex would be moved if possible, although the vertex itself doesn't
	 * lend itself to it.
	 * \param position The vertex will be inserted before this position.
	 * \param vertex The new coordinate to insert.
	 * \return An iterator referring to the newly inserted vertex.
	 */
	iterator insert(const_iterator position, const Point2&& vertex) {
		properties.reset();
		return Batch<Point2>::insert(position, vertex);
	}

	/*!
	 * Insert a number of copies of a vertex into the polygonal chain.
	 *
	 * This causes the polygon to become degenerate. However it may still be
	 * useful if these vertices are later modified.
	 * \param position The vertices will be inserted before this position.
	 * \param count The number of copies of the vertex to insert.
	 * \param vertex The vertex to insert a number of times.
	 * \return An iterator referring to the first of the newly inserted
	 * vertices.
	 */
	iterator insert(const_iterator position, const size_t count, const Point2& vertex) {
		properties.reset();
		return Batch<Point2>::insert(position, count, vertex);
	}

	/*!
	 * Insert a range of vertices into the polygonal chain.
	 *
	 * The vertices in the given range will be copied.
	 * \param position The vertices will be inserted before this position.
	 * \param first An iterator referring to the first of the vertices to
	 * insert.
	 * \param last An iterator referring to the vertex after the last of the
	 * vertices to insert.
	 * \return An iterator referring to the first of the newly inserted
	 * vertices.
	 */
	template<class InputIterator>
	iterator insert(const_iterator position, InputIterator first, InputIterator last) {
		properties.reset();
		return Batch<Point2>::insert(position, first, last);
	}

	/*!
	 * Insert a list of vertices into the polygonal chain.
	 * \param position The vertices will be inserted before this position.
	 * \param vertices A list of vertices to insert.
	 * \return An iterator referring to the first of the newly inserted
	 * vertices.
	 */
	iterator insert(const_iterator position, const std::initializer_list<Point2>& vertices) {
		properties.reset();
		return Batch<Point2>::insert(position, vertices);
	}

	/*!
	 * Remove the last vertex in the polygonal chain, just before the seam.
	 */
	void pop_back() {
		properties.reset();
		Batch<Point2>::pop_back();
	}

	/*!
	 * Add a new vertex to the polygonal chain, just before the seam.
	 * \param vertex The position of the new vertex to insert.
	 */
	void push_back(const Point2& vertex) {
		properties.reset();
		Batch<Point2>::push_back(vertex);
	}

	/*!
	 * Add a new vertex to the polygonal chain, just before the seam.
	 *
	 * The new vertex would be moved into the polygon, if possible.
	 * \param vertex The position of the new vertex to insert.
	 */
	void push_back(Point2&& vertex) {
		properties.reset();
		Batch<Point2>::push_back(vertex);
	}

	/*!
	 * Resize the list of vertices of the polygon such that it reaches the given
	 * size.
	 *
	 * If the new size is larger than the current size, the \ref fill_vertex
	 * will be added in the seam numerous times until the given size is reached.
	 * This could make the polygon degenerate, but may still be useful if the
	 * fill vertices are properly filled in later with real data.
	 *
	 * If the new size is smaller than the current size, vertices will be
	 * dropped just before the seam until the desired size is reached.
	 * \param new_size The new number of vertices to resize this polygon to.
	 * \param fill_vertex A vertex to insert in case the polygon needs to have
	 * more vertices.
	 */
	void resize(const size_t new_size, const Point2& fill_vertex = Point2(0, 0)) {
		properties.reset();
		Batch<Point2>::resize(new_size, fill_vertex);
	}

	/*!
	 * Moves this polygon with a certain offset.
	 *
	 * The polygon is moved in-place.
	 * \param delta The distance by which to move, representing both dimensions
	 * to move through as a single 2D vector.
	 */
	void translate(const Point2& delta) {
		apex::translate(*this, delta);
	}

protected:
	/*!
	 * Geometric properties that we know (or don't know) about this polygon.
	 *
	 * This is used to cache important information and use that speed up future
	 * calculations with the polygon if we already know some properties about
	 * the polygon.
	 */
	PolygonProperties properties;
};

/*!
 * This specialisation of batches of polygons allows polygon operations on
 * batches, possibly increasing parallelism.
 */
template<>
class Batch<Polygon> : public Batch<Batch<Point2>> {
public:
	using Batch<Batch<Point2>>::Batch; //The constructors are the same.

	/*!
	 * Computes the surface area of the polygons in this batch.
	 *
	 * The sign of the area is linked to the polygon winding order. If the
	 * polygon is positive, the area will be positive too, and vice versa. If
	 * the polygon intersects itself, parts of the polygon will be subtracting
	 * from the area while other parts add up to the area.
	 *
	 * The area of the polygon is counted differently from the nonzero or
	 * even-odd fill rules. If a zone is looped around multiple times by the
	 * polygon, it will count to the total area multiple times as well.
	 * \return A list, equally long to the number of polygons in this batch,
	 * that lists the areas of each polygon in the same order.
	 */
	Batch<area_t> area() const {
		return apex::area(*this);
	}

	/*!
	 * Moves all polygons in this batch with the same offset.
	 *
	 * The polygons are moved in-place.
	 * \param delta The distance by which to move, representing both dimensions
	 * to move through as a single 2D vector.
	 */
	void translate(const Point2& delta) {
		apex::translate(*this, delta);
	}

protected:
	/*!
	 * Geometric properties that we know (or don't know) about the polygons in
	 * this batch.
	 *
	 * This is used to cache important information and use that speed up future
	 * calculations with the polygons if we already know some properties
	 * about the polygons.
	 */
	std::vector<PolygonProperties> properties;
};

}

namespace std {

/*!
 * Swaps the contents of two polygons.
 *
 * This specialises the std::swap algorithm for polygons. The contents of the
 * two polygons gets swapped in constant time.
 * \param lhs One of the containers to swap.
 * \param rhs The other container to swap.
 */
inline void swap(apex::Polygon& lhs, apex::Polygon& rhs) {
	lhs.swap(rhs);
}

}

#endif //APEX_POLYGON