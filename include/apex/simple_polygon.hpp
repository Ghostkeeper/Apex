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

namespace apex {

/*
 * A plane figure consisting of a single contour of straight line segments.
 *
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
 */
class SimplePolygon {
public:
	/*
	 * The vertices that make up the corners of this polygon.
	 */
	std::vector<Point2> vertices;
}

}

#endif //APEX_SIMPLE_POLYGON