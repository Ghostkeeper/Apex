/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON
#define APEX_SIMPLE_POLYGON

#include <vector> //To store the vertex data.

#include "point2.hpp" //The vertices of the polygon are 2D points.

//Implementations separated out for readability using the Curiously Recurring Template Pattern.
#include "detail/simple_polygon_area.hpp"
#include "detail/simple_polygon_translate.hpp"

namespace apex {

/*
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
 */
class SimplePolygon : private std::vector<Point2>,
		//Implementing the private functions in separate classes with Curiously Recurring Template Pattern.
		public SimplePolygonArea<SimplePolygon>,
		public SimplePolygonTranslate<SimplePolygon> {
public:
	/*
	 * Constructs an empty simple polygon.
	 *
	 * The polygon will have no vertices or edges and no area.
	 */
	SimplePolygon() {}

	/*
	 * Copies a simple polygon.
	 * \param original The polygon to create a copy of.
	 */
	SimplePolygon(const SimplePolygon& original) : std::vector<Point2>(original) {}

	/*
	 * Moves a simple polygon.
	 * \param original The polygon to move to a different instance.
	 */
	SimplePolygon(SimplePolygon&& original) : std::vector<Point2>(original) {}

	//Operations inheriting from std::vector.
	using std::vector<Point2>::operator=;
	using std::vector<Point2>::operator[];
	using std::vector<Point2>::assign;
	using std::vector<Point2>::at;
	using std::vector<Point2>::back;
	using std::vector<Point2>::begin;
	using std::vector<Point2>::capacity;
	using std::vector<Point2>::cbegin;
	using std::vector<Point2>::cend;
	using std::vector<Point2>::crbegin;
	using std::vector<Point2>::crend;
	using std::vector<Point2>::clear;
	using std::vector<Point2>::data;
	using std::vector<Point2>::emplace;
	using std::vector<Point2>::emplace_back;
	using std::vector<Point2>::empty;
	using std::vector<Point2>::end;
	using std::vector<Point2>::front;
	using std::vector<Point2>::get_allocator;
	using std::vector<Point2>::insert;
	using std::vector<Point2>::max_size;
	using std::vector<Point2>::pop_back;
	using std::vector<Point2>::push_back;
	using std::vector<Point2>::rbegin;
	using std::vector<Point2>::rend;
	using std::vector<Point2>::reserve;
	using std::vector<Point2>::shrink_to_fit;
	using std::vector<Point2>::size;
	using std::vector<Point2>::swap;

	/*
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

	/*
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
		return area_st();
	}

	/*
	 * Moves this polygon with a certain offset.
	 *
	 * The polygon is moved in-place.
	 * \param dx How far to move the polygon in the X direction. This could be
	 * negative.
	 * \param dy How far to move the polygon in the Y direction. This could be
	 * negative.
	 */
	void translate(const coord_t dx, const coord_t dy) {
		return translate(Point2(dx, dy));
	}

	/*
	 * Moves this polygon with a certain offset.
	 *
	 * The polygon is moved in-place.
	 * \param delta The distance by which to move, representing both dimensions
	 * to move through as a single 2D vector.
	 */
	void translate(const Point2& delta) {
		return translate_st(delta);
	}
};

}

#endif //APEX_SIMPLE_POLYGON