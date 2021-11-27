/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON
#define APEX_SIMPLE_POLYGON

#include <utility> //For std::forward.

#include "area.hpp" //To allow calculating the area of this shape.
#include "batch.hpp" //The vertex storage is by batch.
#include "point2.hpp" //The vertices of the polygon are 2D points.
#include "translate.hpp" //To allow moving this shape.

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
template<class VertexStorage = Batch<Point2>>
class SimplePolygon : public VertexStorage {
	template<class OtherVertexStorage>
	friend class SimplePolygon; //Allow touching the privates of other instances of this template.
public:
	/*!
	 * Construct an empty simple polygon.
	 *
	 * The empty simple polygon will be degenerate.
	 */
	SimplePolygon() noexcept : VertexStorage() {}

	/*!
	 * Construct a simple polygon consisting of a vertex repeated a number of
	 * times.
	 *
	 * The simple polygon will be degenerate, since it will have all vertices in
	 * the same point, leading to a shape without any area. However it might
	 * still be useful to construct simple polygons more easily with a fallback
	 * vertex.
	 * \param count The amount of vertices to construct the simple polygon with.
	 * \param vertex The vertex to repeatedly add to the simple polygon.
	 */
	SimplePolygon(const size_t count, const Point2& vertex = Point2()) : VertexStorage(count, vertex) {}

	/*!
	 * Construct a simple polygon consisting of a number of vertices in the
	 * coordinate origin.
	 *
	 * The simple polygon will be degenerate, since it will have all vertices in
	 * the same point, leading to a shape without any area. However it might
	 * still be useful to construct simple polygons more easily with a fallback
	 * vertex.
	 * \param count The amount of vertices to construct the simple polygon with.
	 */
	SimplePolygon(const size_t count) : VertexStorage(count) {}

	/*!
	 * Construct a simple polygon with the contents of the range
	 * ``[first, last)``.
	 * \tparam InputIterator This constructor accepts any type of input
	 * iterator.
	 * \param first The iterator to start copying from.
	 * \param last An iterator signalling that the range of vertices to copy has
	 * ended.
	 */
	template<class InputIterator>
	SimplePolygon(InputIterator first, InputIterator last) : VertexStorage(first, last) {}

	/*!
	 * Copy the specified simple polygon.
	 * \param other The simple polygon to copy.
	 */
	SimplePolygon(const SimplePolygon& other) : VertexStorage(static_cast<const VertexStorage&>(other)) {}

	/*!
	 * Move the specified simple polygon to a new memory location.
	 * \param other The simple polygon to move.
	 */
	SimplePolygon(SimplePolygon&& other) : VertexStorage(static_cast<VertexStorage&&>(other)) {}

	/*!
	 * Construct a simple polygon from the contents of an initialiser list.
	 * \param initialiser_list The list of vertices to put in the simple
	 * polygon.
	 */
	SimplePolygon(std::initializer_list<Point2> initializer_list) : VertexStorage(initializer_list) {}

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
	template<typename OtherVertexStorage>
	bool operator ==(const SimplePolygon<OtherVertexStorage>& other) const {
		//TODO: Put implementation in separate file and allow multiple implementations.
		if(VertexStorage::size() != other.size()) {
			return false;
		}
		if(VertexStorage::empty()) { //Both are empty.
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
		for(size_t i = 0; i < VertexStorage::size(); ++i) {
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
	 * Computes the surface area of the simple polygon.
	 *
	 * The sign of the area is linked to the polygon winding order. If the
	 * polygon is positive, the area will be positive too, and vice versa. If
	 * the polygon intersects itself, parts of the polygon will be subtracting
	 * from the area while other parts add up to the area.
	 *
	 * The area of the polygon is counted differently from the nonzero or
	 * even-odd fill rules. If a zone is looped around multiple times by the
	 * polygon, it will count to the total area multiple times as well.
	 * \return The surface area of the simple polygon.
	 */
	area_t area() const {
		return apex::area(*this);
	}

	/*!
	 * Moves this polygon with a certain offset.
	 *
	 * The polygon is moved in-place.
	 * \param delta The distance by which to move, representing both dimensions to
	 * move through as a single 2D vector.
	 */
	void translate(const Point2& delta) {
		apex::translate(*this, delta);
	}
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