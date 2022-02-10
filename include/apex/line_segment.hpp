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

namespace apex {

/*!
 * This class represents a straight line segment, a finite part of a line.
 */
class LineSegment {
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
	 * \param a_start One of the vertices of the first line segment.
	 * \param a_end The other vertex of the first line segment.
	 * \param b_start One of the vertices of the second line segment.
	 * \param b_end The other vertex of the second line segment.
	 * \return Whether the two line segments are considered to be intersecting.
	 */
	static std::optional<Point2> intersects(const Point2& a_start, const Point2& a_end, const Point2& b_start, const Point2& b_end) {
		return std::nullopt; //TODO: Implement.
	}
};

}

#endif //APEX_LINE_SEGMENT