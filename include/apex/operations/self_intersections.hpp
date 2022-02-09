/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SELF_INTERSECTIONS
#define APEX_SELF_INTERSECTIONS

#include "../detail/geometry_concepts.hpp" //To disambiguate overloads.
#include "../batch.hpp" //To perform batch operations and to return batches of self-intersections.
#include "../self_intersection.hpp" //The return type of this operation.

namespace apex {

/*!
 * Finds all self-intersections in a polygon.
 *
 * Aside from edge cases, when a polygon self-intersects, it leads to some part
 * of the polygon being negative and another part being positive. This can be
 * problematic for some use cases. Finding self-intersections can be helpful to
 * identify regions that need special attention.
 *
 * This function finds all locations where an edge of the polygon hits or
 * crosses another edge, except the exact vertex at the last endpoint of the
 * edge. The first vertex of the edge is included with the edge, but the last is
 * not (otherwise it would mark all vertices where two edges come together to be
 * self-intersections). The intersection results this function returns contain
 * the location of the intersection, as well as the indices of the edges that
 * intersect. These indices refer to the edge after the vertex with that index,
 * and include the start endpoint of that edge. So given such an index, you can
 * derive that either the vertex with that index was hit, or the edge that
 * follows it.
 *
 * A simple example is a proper intersection, where two edges cross each other
 * halfway:
 *
 * @image html res/self_intersection_polygon_proper.svg
 *
 * An edge case for intersections occurs when an edge of the polygon crosses
 * exactly through a vertex that it is not connected to. In the result, such an
 * intersection is shown as the edge crossing the edge following the vertex that
 * was crossed, as explained above.
 *
 * @image html res/self_intersection_polygon_vertex.svg
 *
 * Another edge case is when the polygon grazes itself without actually piercing
 * through to the other side. This is considered a self-intersection too, even
 * though it doesn't (directly) result in a mixed-orientation polygon. The
 * intersection result contains the edge that follows the vertex that was hit.
 *
 * @image html res/self_intersection_polygon_grazing.svg
 *
 * A more complex edge case is when two or more edges of the polygon overlap
 * partially. While the intersection is technically not a point any more then,
 * this function doesn't return line segments. It will return the start and end
 * points of the segment that intersects separately, and also every vertex along
 * the way during the overlap of these edges.
 *
 * @image html res/self_intersection_polygon_edges.svg
 *
 * When more than two edges of the polygon intersect in one point, the same
 * point will be reported multiple times, one for each pairwise intersection.
 * \tparam Polygon A class that behaves like a polygon.
 * \param polygon A polygon to test for self-intersections.
 * \return A batch of self-intersection results. Each self-intersection contains
 * the position of the intersection and the indices of the two edges that are
 * involved.
 */
template<polygonal Polygon>
Batch<PolygonSelfIntersection> self_intersections(const Polygon& polygon) {
	return detail::self_intersections_st_naive(polygon);
}

namespace detail {

Batch<PolygonSelfIntersection> self_intersections_st_naive(const Polygon& polygon) {
	Batch<PolygonSelfIntersection> result;
	for(size_t segment_index = 0; segment_index < polygon.size(); ++segment_index) {
		const LineSegment segment(polygon[segment_index], polygon[(segment_index + 1) % polygon.size()])
		for(size_t other_index = 0; other_index < segment_index; ++other_index) {
			const LineSegment other(polygon[other_index], polygon[other_index + 1]); //No need to limit to polygon size, since this can never equal segment_index.
			std::optional<Point2> intersection = intersection_line_segments(segment, other);
			if(intersection) { //They did intersect.
				result.emplace_back(segment_index, other_index, *intersection);
			}
		}
	}
}

}

}

#endif //APEX_SELF_INTERSECTIONS