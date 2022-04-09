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
 * A struct representing a self-intersection result for polygons.
 *
 * This piece of data is the result of the algorithm that finds self-
 * intersections. It contains the information that was found about a self-
 * intersection that the consumer of the self-intersections might need.
 */
struct PolygonSelfIntersection {
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

	/*!
	 * Construct a new self-intersection.
	 * \param location The position of the self-intersection.
	 * \param segment_a The index of one of the segments that intersects.
	 * \param segment_b The index of the other of the segments that intersects.
	 */
	PolygonSelfIntersection(const Point2 location, const size_t segment_a, const size_t segment_b) : location(location), segment_a(segment_a), segment_b(segment_b) {};

	/*!
	 * Check if two self-intersections are the same.
	 *
	 * They are the same if they concern the same two segments. This assumes
	 * that the position where these two segments intersect remains unchanged.
	 */
	bool operator ==(const PolygonSelfIntersection& other) const {
		return segment_a == other.segment_a && segment_b == other.segment_b;
	}

	/*!
	 * Overloads streaming this self-intersection.
	 *
	 * This is useful for debugging, since it allows printing the data to a
	 * stream directly, giving you a reasonably readable output.
	 */
	friend std::ostream& operator <<(std::ostream& output_stream, const PolygonSelfIntersection& self_intersection) {
		return output_stream << self_intersection.segment_a << "x" << self_intersection.segment_b << "->" << self_intersection.location;
	}
};

}

#endif //APEX_SELF_INTERSECTION