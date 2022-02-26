/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_LINE_SEGMENT
#define APEX_LINE_SEGMENT

#include <optional>

#include "apex/point2.hpp"

namespace apex {

/*!
 * This class represents a straight line segment, a finite part of a line.
 */
class LineSegment {
public:
	/*!
	 * Check if two line segments intersect, without constructing those line
	 * segments.
	 *
	 * If the line segments intersect, the intersecting position, rounded to the
	 * nearest coordinate point, will be returned. If the line segments don't
	 * intersect, the optional result will not have a value. If the line
	 * segments overlap (wholly or partially), a point inside of the overlapping
	 * part will be returned, which is the point closest to \ref a_start . The
	 * intersection coordinates will be rounded to the nearest unit coordinate.
	 *
	 * While it may be possible to check for line intersection between instances
	 * of line segments, that requires actually creating a line segment, which
	 * probably involves writing them to memory. This function is static, and
	 * doesn't require copying the coordinates of the line segments.
	 *
	 * The line segments are considered to be intersecting if one of the line
	 * segments intersects with the body of the other. If only the endpoints of
	 * the line segments intersect, they are not considered to be intersecting.
	 *
	 * The line intersection check is implemented by creating a parametric
	 * representation of the lines through both line segments, and then finding
	 * where both of these equations are equal. There are many equations for a
	 * line, but the one used here is as follows:
	 * \f$L = \vec{p} + \vec{v}t\f$
	 * You can visualise this as the line starting from a position \f$\vec{p}\f$
	 * and extending in the directional vector \f$\vec{v}\f$. The parameter
	 * \f$t\f$ causes the position to slide along the line. We will choose the
	 * position \f$\vec{p}\f$ to be the starting vertex of a line segment, and
	 * the vector \f$\vec{v}\f$ to be the difference between the start and end
	 * positions. The resulting position is inside of the line segment then if
	 * \f$0 <= t <= 1\f$.
	 *
	 * To find the intersection between the two lines, we have to equate the two
	 * line representations to each other, and solve for the two \f$t\f$
	 * parameters. We can plug one of those into the formula for either of the
	 * lines to get the position of the intersection. For simplicity, we'll
	 * break the vectors into their X and Y components. This involves solving
	 * the following system of equations for \f$t_a\f$ and \f$t_b\f$:
	 * \f$\vec{p}_a.x + \vec{v}_a.x t_a = \vec{p}_b.x + \vec{v}_b.x t_b \\
	 * \vec{p}_a.y + \vec{v}_a.y * t_a = \vec{p}_b.y + \vec{v}_b.y t_b\f$
	 *
	 * This system of equations solves to:
	 * \f$t_a = \frac{\vec{v}_b.y(\vec{p}_a.x - \vec{p}_b.x) + \vec{v}_b.x(\vec{v}_b.y - \vec{v}_a.y)}{\vec{v}_b.x \vec{v}_a.y - \vec{v}_a.x \vec{v}_b.y} \\
	 * t_b = \frac{\vec{v}_a.y(\vec{p}_a.x - \vec{p}_b.x) + \vec{v}_a.x(\vec{v}_b.y - \vec{v}_a.y)}{\vec{v}_b.x \vec{v}_a.y - \vec{v}_a.x \vec{v}_b.y} \\
	 * \vec{v}_b.x \vec{v}_a.y - \vec{v}_a.x \vec{v}_b.y \neq 0\f$
	 *
	 * These formulas refer to \f$\vec{p}_a.x - \vec{p}_b.x\f$, and similarly
	 * for ``y``, which is simply the difference between the starting positions
	 * of the line segments and can be re-used. Similarly, the divisor of both
	 * equations are the same and can be re-used. Care should be taken with that
	 * divisor though, since dividing this with integer coordinates rounds the
	 * resulting \f$t\f$ parameters to integers, losing a lot of accuracy. This
	 * division can be delayed to the end, and instead of checking if the
	 * parameters are between 0 and 1, we'll check if they are between 0 and the
	 * divisor.
	 *
	 * If the divisor is 0, that means that the lines are parallel and there may
	 * not be a solution. This case needs to be handled separately.
	 * \param a_start One of the vertices of the first line segment.
	 * \param a_end The other vertex of the first line segment.
	 * \param b_start One of the vertices of the second line segment.
	 * \param b_end The other vertex of the second line segment.
	 * \return If they intersect, the point of intersection. If they don't
	 * intersect, an empty optional.
	 */
	static std::optional<Point2> intersect(const Point2& a_start, const Point2& a_end, const Point2& b_start, const Point2& b_end) {
		const Point2 a_delta = a_end - a_start;
		const Point2 b_delta = b_end - b_start;
		const Point2 starts_delta = a_start - b_start;

		//Find the parametric coordinates (named "t" in the documentation) where the intersection occurs.
		const area_t divisor = a_delta.x * b_delta.y - b_delta.x * a_delta.y; //TODO: If this is 0, the lines are parallel. You can check with a cross product whether they are colinear?
		const area_t a_parametric = b_delta.x * starts_delta.y - b_delta.y * starts_delta.x;
		const area_t b_parametric = a_delta.x * starts_delta.y - a_delta.y * starts_delta.x;

		//If both parameters are between 0 and the divisor, they intersect.
		const area_t lower_range = std::min(area_t(0), divisor);
		const area_t upper_range = std::max(area_t(0), divisor);
		if(a_parametric < lower_range || a_parametric > upper_range || b_parametric < lower_range || b_parametric > upper_range) //Intersection is not within the ranges of both segments.
		{
			return std::nullopt;
		}
		return a_start + Point2(a_parametric * a_delta.x / divisor, a_parametric * a_delta.y / divisor);
	}
};

}

#endif //APEX_LINE_SEGMENT