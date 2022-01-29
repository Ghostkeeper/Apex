/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_CONVEXITY
#define APEX_CONVEXITY

namespace apex {

/*!
 * This geometric property conveys whether a shape is convex or not.
 *
 * A shape is convex when any straight line segment starting and ending inside
 * the shape stays completely inside the shape. If a straight line segment
 * exists that starts and ends inside the shape, but halfway along exits the
 * shape, the shape is not convex but concave.
 */
enum class Convexity {
	/*!
	 * No convexity information has been calculated yet about this shape.
	 */
	UNKNOWN = 0,

	/*!
	 * The shape is convex. No straight line between any points inside the shape
	 * will intersect the border of the shape.
	 */
	CONVEX = 1,

	/*!
	 * The shape is concave. There are straight lines between points inside the
	 * shape that intersect with its edges.
	 */
	CONCAVE = 2,

	/*!
	 * The shape is degenerate. Convexity has no meaning with this shape.
	 *
	 * For instance, the shape could be a point or a line, which has no positive
	 * area to draw lines in, or it could be self-intersecting.
	 */
	DEGENERATE = 3
};

}

#endif //APEX_CONVEXITY