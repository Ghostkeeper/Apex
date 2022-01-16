/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SELF_INTERSECTION
#define APEX_SELF_INTERSECTION

namespace apex {

/*!
 * A struct representing a self-intersection result for simple polygons.
 *
 * This piece of data is the result of the algorithm that finds self-
 * intersections. It contains the information that was found about a self-
 * intersection that the consumer of the self-intersections might need.
 */
struct SimplePolygonSelfIntersection {
	/*!
	 * The position of the self-intersection.
	 *
	 * This position is rounded to the closest unit coordinate.
	 */
	Point2 location;

	/*!
	 * The index of one of the segments that intersects.
	 *
	 * The segment index is equal to the lower of the two indices of its
	 * incident vertices. A segment index ``X`` is the segment between vertices
	 * with indices ``X`` and ``X+1``. In other words, you could see this as the
	 * index of the vertex after which the intersection occurs.
	 */
	size_t segment_a;

	/*!
	 * The index of one of the segments that intersects.
	 *
	 * The segment index is equal to the lower of the two indices of its
	 * incident vertices. A segment index ``X`` is the segment between vertices
	 * with indices ``X`` and ``X+1``. In other words, you could see this as the
	 * index of the vertex after which the intersection occurs.
	 */
	size_t segment_b;
};

}

#endif //APEX_SELF_INTERSECTION