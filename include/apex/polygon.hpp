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
	using Batch<Point2>::Batch; //The constructors are the same.

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