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
#include "../line_segment.hpp" //To intersect edges of the polygon.
#include "../self_intersection.hpp" //The return type of this operation.

namespace apex {

namespace detail {

//Forward declare the functions we'd like to use.
template<polygonal Polygon>
Batch<PolygonSelfIntersection> self_intersections_st_naive(const Polygon& polygon);

}

/*!
 * Finds all self-intersections in a polygon.
 *
 * Aside from edge cases, when a polygon self-intersects, it leads to some part
 * of the polygon being negative and another part being positive. This can be
 * problematic for some use cases. Finding self-intersections can be helpful to
 * identify regions that need special attention.
 *
 * This function finds all locations where an edge of the polygon hits or
 * crosses another edge. The vertices of the polygon are considered to be part
 * of both line segments they are part of, so if a vertex is positioned exactly
 * on a self-intersection, both line segments will be reported as intersecting.
 * Line segments are not considered to be self-intersecting with adjacent line
 * segments though, unless they overlap lengthwise (otherwise every vertex would
 * be the intersection of its two adjacent line segments).
 *
 * The intersection results this function returns contain the location of the
 * intersection, as well as the indices of the edges that intersect. These
 * indices refer to the edge after the vertex with that index. For instance, the
 * edge with index 0 is the edge that connects vertex 0 with vertex 1.
 *
 * A simple example is a proper intersection, where two edges cross each other
 * halfway:
 *
 * @image html res/self_intersection_polygon_proper.svg
 *
 * An edge case for intersections occurs when an edge of the polygon crosses
 * exactly through a vertex that it is not connected to. In the result, both
 * line segments adjacent to that vertex will be reported as intersecting.
 *
 * @image html res/self_intersection_polygon_vertex.svg
 *
 * Another edge case is when the polygon grazes itself without actually piercing
 * through to the other side. This is considered a self-intersection too, even
 * though it doesn't (directly) result in a mixed-orientation polygon. The
 * intersection result contains both adjacent edges again.
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
 *
 * @image html res/self_intersection_multiple.svg
 *
 * Zero-length edges are ignored in this result. It behaves as if they are not
 * there. Thus a self-intersection is never reported involving any zero-length
 * edge. What's more, the neighbours of a string of zero-length edges will not
 * be reported as intersecting, even though they touch at the endpoints. Only if
 * they touch at the endpoints but nonzero-length edges are in between will an
 * intersection be reported.
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

/*!
 * Naive implementation to find self-intersections in a polygon.
 *
 * This implementation simply compares all pairs of line segments to see if they
 * intersect. All found intersections are returned in a batch.
 *
 * The implementation is so simple that it may be very effective for low-
 * resolution polygons, but it scales badly for high-resolution polygons.
 * \param polygon The polygons to find self-intersections in.
 * \return A batch of self-intersections.
 */
template<polygonal Polygon>
Batch<PolygonSelfIntersection> self_intersections_st_naive(const Polygon& polygon) {
	Batch<PolygonSelfIntersection> result;
	if(polygon.size() == 2) [[unlikely]] {
		/* In the case of 2 vertices, we have a degenerate polygon with 2 line
		segments that loop back on each other, completely overlapping. The
		algorithm below would find two intersections because each line checks
		for overlap with the previous line (with clock arithmetic). But there is
		only one. So we can special-case that. */
		result.emplace_back(polygon[0], 0, 1); //The 0th segment always intersects with the 1st segment. Choose any point on the line as intersection point.
	} else if(polygon.size() > 2) [[likely]] {
		//Pre-compute the unique positions along the contour, to find and ignore zero-length edges.
		std::vector<size_t> position_index;
		position_index.reserve(polygon.size());
		Point2 last_position = polygon[0];
		position_index.push_back(0); //The first vertex is always a unique position.
		size_t unique_position = 0;
		for(size_t segment_index = 1; segment_index < polygon.size(); ++segment_index) {
			if(polygon[segment_index] != last_position) {
				unique_position++;
				last_position = polygon[segment_index];
			}
			position_index.push_back(unique_position);
		}
		for(size_t segment_index = 0; segment_index < polygon.size() && polygon[segment_index] == last_position; ++segment_index) { //Also loop around to eliminate the seam.
			position_index[segment_index] = position_index.back();
		}

		for(size_t segment_index = 0; segment_index < polygon.size(); ++segment_index) {
			const Point2 this_a = polygon[segment_index];
			const Point2 this_b = polygon[(segment_index + 1) % polygon.size()];
			for(size_t other_index = 0; other_index + 1 < segment_index; ++other_index) { //Stop 1 short of the neighbouring segment, because neighbours can be checked more easily.
				if(other_index == 0 && segment_index == polygon.size() - 1) {
					continue; //Don't check the last vs. the first segment, as they are also neighbours.
				}
				if(position_index[segment_index] == position_index[other_index]) { //Same position, so this is a zero-length segment.
					continue; //Skip. They may not intersect anything (and the segment intersection check doesn't deal with this well).
				}
				const Point2 other_a = polygon[other_index];
				const Point2 other_b = polygon[other_index + 1]; //No need to limit to polygon size, since this can never equal segment_index.
				const std::optional<Point2> intersection = LineSegment::intersect(this_a, this_b, other_a, other_b);
				if(intersection) { //They did intersect.
					if((position_index[segment_index] == position_index[other_index + 1] && *intersection == this_a) || (position_index[(segment_index + 1) % polygon.size()] == position_index[other_index] && *intersection == this_b)) { //But it's intersecting at the endpoints with only 0-length segments in between.
						continue; //Don't count those. They are essentially just along the same contour.
					}
					result.emplace_back(*intersection, segment_index, other_index);
				}
			}
			//We skipped the neighbour. Check now for self-intersection with the neighbour. This can only partially overlap, never properly intersect.
			const size_t previous_index = (segment_index + polygon.size() - 1) % polygon.size();
			const Point2 previous = polygon[previous_index];
			if(previous.orientation_with_line(this_a, this_b) == 0) { //Can only intersect if collinear.
				if((this_b > this_a && previous > this_a) || (this_b < this_a && previous < this_a)) { //Both line segments go in the same direction, so they partially overlap.
					result.emplace_back(this_a, previous_index, segment_index);
				}
			}
		}
	}
	return result;
}

template<polygonal Polygon>
Batch<PolygonSelfIntersection> self_intersections_mt_naive(const Polygon& polygon) {
	Batch<PolygonSelfIntersection> result;
	if(polygon.size() == 2) [[unlikely]] {
		/* In the case of 2 vertices, we have a degenerate polygon with 2 line
		segments that loop back on each other, completely overlapping. The
		algorithm below would find two intersections because each line checks
		for overlap with the previous line (with clock arithmetic). But there is
		only one. So we can special-case that. */
		result.emplace_back(polygon[0], 0, 1); //The 0th segment always intersects with the 1st segment. Choose any point on the line as intersection point.
	} else if(polygon.size() > 2) [[likely]] {
		//Pre-compute the unique positions along the contour, to find and ignore zero-length edges.
		//We divide the polygon into segments, one for each thread. These segments are then sewn together afterwards.
		std::vector<size_t> position_index;
		position_index.resize(polygon.size());
		const size_t thread_count = omp_get_num_threads();
		const size_t vertices_per_thread = polygon.size() / thread_count;
		const size_t leftover = polygon.size() % thread_count; //The first few threads get 1 extra, if it's not divisible by the thread count.
		#pragma omp parallel for
		for(size_t thread = 0; thread < thread_count; ++thread) {
			const size_t start_vertex = vertices_per_thread * thread + std::min(thread, leftover);
			const size_t end_vertex = start_vertex + vertices_per_thread + (thread < leftover);
			Point2 last_position = polygon[start_vertex];
			size_t unique_position = start_vertex; //Likely too high if there are any duplicates in previous segments, but for the algorithm this doesn't matter.
			for(size_t vertex = start_vertex + 1; vertex < end_vertex; ++vertex) {
				if(polygon[vertex] != last_position) {
					unique_positon++;
					last_position = polygon[vertex];
				}
				position_index[vertex] = unique_position;
			}
		}
		//If the vertices around the segment seams are equal, sow the seams to indicate that the positions are equal.
		for(size_t thread = 0; thread < thread_count; ++thread) {
			const size_t start_vertex = vertices_per_thread * thread + std::min(thread, leftover);
			const size_t end_vertex = start_vertex + vertices_per_thread + (thread < leftover);
			const size_t previous_vertex = (start_vertex + polygon.size() - 1) % polygon.size();
			if(polygon[previous_vertex] == polygon[start_vertex]) {
				const size_t old_position_index = position_index[start_vertex];
				for(size_t vertex = start_vertex; vertex < end_vertex && position_index[vertex] == old_position_index; ++vertex) {
					postion_index[vertex] = position_index[previous_index];
				}
			}
		}

		/* To parallellise this 2D triangular loop, we create a single index
		enumerating all pairs of edges, and parallelise over those indices. From
		that index we can calculate the segment indices A and B back.
		What we want is a triangular loop such as this (for 7 vertices):
		-   -   -   -   -   -   -
		-   -   -   -   -   -   -
		0x2 -   -   -   -   -   -
		0x3 1x3 -   -   -   -   -
		0x4 1x4 2x4 -   -   -   -
		0x5 1x5 2x5 3x5 -   -   -
		0x6 1x6 2x6 3x6 4x6 -   -
		To achieve such a loop, we can fold the bottom half of this loop onto
		the right side of the top half. To do that, we'll mirror the bottom half
		in both directions. First in the A dimension:
		-   -   -   -   -   -   -
		-   -   -   -   -   -   -
		0x2 -   -   -   -   -   -
		0x3 1x3 -   -   -   -   -
		0x4 1x4 2x4 -   -   -   -
		-   -   3x5 2x5 1x5 0x5 -
		-   4x6 3x6 2x6 1x6 0x6 -
		And then in the B dimension too:
		-   -   -   -   -   -   -
		-   -   -   -   -   -   -
		0x2 4x6 3x6 2x6 1x6 0x6 -
		0x3 1x3 3x5 2x5 1x5 0x5 -
		0x4 1x4 2x4 -   -   -   -
		-   -   -   -   -   -   -
		-   -   -   -   -   -   -
		By then iterating the correct amount of times ((N-1)*(N-2)/2, in this
		case 15) and selecting the correct rectangle, we can simply pretend this
		is a rectangular 2D loop and correct the coordinates for the part that
		needs to be mirrored. */
		const size_t num_pairs = (polygon.size() - 1) * (polygon.size() - 2) / 2;
		#pragma omp parallel for
		for(size_t pair_index = 0; pair_index < num_pairs; ++pair_index) {
			size_t segment_a = pair_index % (polygon.size() - 1);
			size_t segment_b = pair_index / (polygon.size() - 1);
			if(segment_a > segment_b) {
				segment_a = polygon.size() - 1 - segment_a; //Mirror A dimension.
				segment_b = polygon.size() - 2 - segment_b; //Mirror B dimension.
			}
			segment_b += 2;

			if(segment_a == 0 && segment_b == polygon.size() - 1) {
				continue; //Don't check the last vs. the first segment, as they are also neighbours.
			}
			if(position_index[segment_index] == position_index[other_index]) { //Same position, so this is a zero-length segment.
				continue; //Skip. They may not intersect anything (and the segment intersection check doesn't deal with this well).
			}
			const Point2 a_start = polygon[segment_a];
			const Point2 a_end = polygon[segment_a + 1]; //Since B > A, we don't need to check if this exceeds the polygon size.
			const Point2 b_start = polygon[segment_b];
			const Point2 b_end = polygon[(segment_b + 1) % polygon.size()];
			const std::optional<Point2> intersection = LineSegment::intersect(a_start, a_end, b_start, b_end);
			if(intersection) { //They did intersect.
				//TODO: Report intersection. We may have to make thread-local arrays that are later combined?
			}
		}

		//We skipped each vertex' neighbour. Check now for self-intersection with the neighbour. This can only partially overlap, never properly intersect.
		#pragma omp parallel for
		for(size_t vertex = 0; vertex < polygon.size(); ++vertex) { //Check the two adjacent edges around this vertex.
			const Point2 this_a = polygon[vertex];
			const Point2 this_b = polygon[(vertex + 1) % polygon.size()];
			const size_t previous_index = (vertex + polygon.size() - 1) % polygon.size();
			const Point2 previous = polygon[previous_index];
			if(previous.orientation_with_line(this_a, this_b) == 0) { //Can only intersect if collinear.
				if((this_b > this_a && previous > this_a) || (this_b < this_a && previous < this_a)) { //Both line segments go in the same direction, so they partially overlap.
					result.emplace_back(this_a, previous_index, vertex);
				}
			}
		}
	}
	return result;
}

}

}

#endif //APEX_SELF_INTERSECTIONS