/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "../helpers/polygon_test_cases.hpp" //To load testing polygons to compute the area of.
#include "apex/operations/self_intersections.hpp" //The unit we're testing here.

namespace apex {

/*!
 * Test finding self-intersections on an empty polygon.
 */
TEST(PolygonSelfIntersections, Empty) {
	const Batch<PolygonSelfIntersection> ground_truth; //No self-intersections, empty batch.
	EXPECT_EQ(self_intersections(PolygonTestCases::empty()), ground_truth) << "There should be no self-intersections in the empty polygon.";
}

/*!
 * Test finding self-intersections on a polygon with a single vertex.
 */
TEST(PolygonSelfIntersections, Point) {
	const Batch<PolygonSelfIntersection> ground_truth; //No self-intersections, empty batch.
	EXPECT_EQ(self_intersections(PolygonTestCases::point()), ground_truth) << "With only 1 vertex, there are no edges that can intersect.";
}

/*!
 * Test finding self-intersections on a polygon with two vertices.
 *
 * This polygon technically self-intersects, because the polygon closes back on
 * itself over the same line segment.
 */
TEST(PolygonSelfIntersections, Line) {
	const Polygon polygon = PolygonTestCases::line();
	const Batch<PolygonSelfIntersection> result = self_intersections(polygon);
	ASSERT_EQ(result.size(), 1) << "The polygon is closed, so it has two line segments. They exactly overlap, so that's an intersection.";
	EXPECT_TRUE(LineSegment(polygon[0], polygon[1]).intersects(result[0].location)) << "The intersecting point must be somewhere on the line segment.";
}

/*!
 * Test finding self-intersections on a simple square. It does not
 * self-intersect.
 */
TEST(PolygonSelfIntersections, Square) {
	const Batch<PolygonSelfIntersection> ground_truth; //No self-intersections, empty batch.
	EXPECT_EQ(self_intersections(PolygonTestCases::square_1000()), ground_truth) << "This square has no self-intersections.";
}

/*!
 * Test finding self-intersections in a concave shape. It does not
 * self-intersect.
 */
TEST(PolygonSelfIntersections, Concave) {
	const Batch<PolygonSelfIntersection> ground_truth; //No self-intersections, empty batch.
	EXPECT_EQ(self_intersections(PolygonTestCases::arrowhead()), ground_truth) << "This shape has no self-intersections.";
}

/*!
 * Test finding self-intersections in the clear-cut case where the polygon
 * clearly self-intersects.
 */
TEST(PolygonSelfIntersections, SimpleSelfIntersection) {
	const Batch<PolygonSelfIntersection> ground_truth = {PolygonSelfIntersection(Point2(500, 500), 0, 2)};
	EXPECT_EQ(self_intersections(PolygonTestCases::hourglass()), ground_truth) << "The 0th segment intersects with the 2nd segment, in the middle at position 500,500.";
}

/*!
 * Test finding self-intersections when there are zero-length segments in the
 * polygon.
 *
 * The zero-length segments may not be causing self-intersections. And the line
 * segments around them may also not be thought of as intersecting even though
 * their endpoints touch.
 */
TEST(PolygonSelfIntersections, ZeroLengthSegments) {
	const Batch<PolygonSelfIntersection> ground_truth;
	EXPECT_EQ(self_intersections(PolygonTestCases::zero_length_segments()), ground_truth) << "Zero-length line segments are not counted in the self-intersection.";
}

/*!
 * Test finding grazing self-intersections, where a vertex touches an edge of
 * the same polygon.
 */
TEST(PolygonSelfIntersections, GrazingVertex) {
	const Batch<PolygonSelfIntersection> ground_truth = {
		PolygonSelfIntersection(Point2(500, 0), 0, 2),
		PolygonSelfIntersection(Point2(500, 0), 0, 3)
	};
	const Batch<PolygonSelfIntersection> result = self_intersections(PolygonTestCases::touching_edge());
	ASSERT_EQ(ground_truth.size(), result.size()) << "A vertex touches an edge, so both edges incident to that vertex will be reported as intersecting.";
	for(const PolygonSelfIntersection& intersection : result) {
		EXPECT_EQ(std::count(ground_truth.begin(), ground_truth.end(), intersection), std::count(result.begin(), result.end(), intersection)) << "The intersection must be reported the correct number of times.";
	}
}

/*!
 * Test finding self-intersections where two vertices are on the same position.
 *
 * This is similar to the case where there are zero-length segments, in the
 * sense that the endpoints of line segments touch. But this time there are non-
 * zero length segments in between. It should find those then.
 */
TEST(PolygonSelfIntersections, TouchingVertex) {
	const Batch<PolygonSelfIntersection> ground_truth = {
		PolygonSelfIntersection(Point2(1000, 500), 0, 3),
		PolygonSelfIntersection(Point2(1000, 500), 0, 4),
		PolygonSelfIntersection(Point2(1000, 500), 1, 3),
		PolygonSelfIntersection(Point2(1000, 500), 1, 4)
	};
	const Batch<PolygonSelfIntersection> result = self_intersections(PolygonTestCases::touching_vertex());
	ASSERT_EQ(ground_truth.size(), result.size()) << "Two vertices touch each other, and it's not just zero-length segments. Every non-adjacent pair of edges must be reported as intersecting.";
	for(const PolygonSelfIntersection& intersection : result) {
		EXPECT_EQ(std::count(ground_truth.begin(), ground_truth.end(), intersection), std::count(result.begin(), result.end(), intersection)) << "The intersection must be reported the correct number of times.";
	}
}

/*!
 * Test finding self-intersections when multiple subsequent edges are
 * overlapping.
 */
TEST(PolygonSelfIntersections, OverlappingSegments) {
	//The ground truth in this test features some intersecting points, as well as overlapping segments.
	//These are the intersecting points. The segments are tested manually.
	const Batch<PolygonSelfIntersection> ground_truth_points = {
		PolygonSelfIntersection(Point2(100, 500), 0, 13), //Tip of left triangle.
		PolygonSelfIntersection(Point2(100, 500), 0, 14),
		PolygonSelfIntersection(Point2(100, 500), 1, 14),
		PolygonSelfIntersection(Point2(900, 500), 5, 8), //Tip of right triangle.
		PolygonSelfIntersection(Point2(900, 500), 6, 8),
		PolygonSelfIntersection(Point2(900, 500), 6, 9),
		PolygonSelfIntersection(Point2(300, 1000), 1, 12), //Vertices along the overlapping parts.
		PolygonSelfIntersection(Point2(300, 1000), 2, 13),
		PolygonSelfIntersection(Point2(400, 1000), 2, 11),
		PolygonSelfIntersection(Point2(400, 1000), 3, 12),
		PolygonSelfIntersection(Point2(700, 1000), 4, 9),
		PolygonSelfIntersection(Point2(700, 1000), 5, 10)
	};
	/* The overlapping segments are:
	 - 1 with 13
	 - 2 with 12
	 - 3 with part of 11
	 - part of 4 with part of 11
	 - part of 4 with 10
	 - 5 with 9
	*/
	const Polygon polygon = PolygonTestCases::zero_width_connection();
	const Batch<PolygonSelfIntersection> result = self_intersections(polygon);
	ASSERT_EQ(ground_truth_points.size() + 6, result.size()) << "The result must include all of the intersecting points, plus 6 overlapping segments.";
	for(const PolygonSelfIntersection& intersection : ground_truth_points) {
		EXPECT_EQ(std::count(ground_truth_points.begin(), ground_truth_points.end(), intersection), std::count(result.begin(), result.end(), intersection)) << "The intersection must be reported the correct number of times.";
	}
	//Check for the overlapping segments being reported.
	for(const PolygonSelfIntersection& intersection : result) {
		if((intersection.segment_a == 1 && intersection.segment_b == 13) || (intersection.segment_a == 13 && intersection.segment_b == 1)) {
			EXPECT_TRUE(LineSegment(polygon[1], polygon[2]).intersects(intersection.location)) << "Segment 1 overlaps with segment 13.";
		}
		if((intersection.segment_a == 2 && intersection.segment_b == 12) || (intersection.segment_a == 12 && intersection.segment_b == 2)) {
			EXPECT_TRUE(LineSegment(polygon[2], polygon[3]).intersects(intersection.location)) << "Segment 2 overlaps with segment 12.";
		}
		if((intersection.segment_a == 3 && intersection.segment_b == 11) || (intersection.segment_a == 11 && intersection.segment_b == 3)) {
			EXPECT_TRUE(LineSegment(polygon[3], polygon[4]).intersects(intersection.location)) << "Segment 3 overlaps with part of segment 11.";
		}
		if((intersection.segment_a == 4 && intersection.segment_b == 11) || (intersection.segment_a == 11 && intersection.segment_b == 4)) {
			EXPECT_TRUE(LineSegment(polygon[4], polygon[11]).intersects(intersection.location)) << "Part of segment 4 overlaps with part of segment 11.";
		}
		if((intersection.segment_a == 4 && intersection.segment_b == 10) || (intersection.segment_a == 10 && intersection.segment_b == 4)) {
			EXPECT_TRUE(LineSegment(polygon[11], polygon[10]).intersects(intersection.location)) << "Part of segment 4 intersects with segment 10.";
		}
		if((intersection.segment_a == 5 && intersection.segment_b == 9) || (intersection.segment_a == 9 && intersection.segment_b == 5)) {
			EXPECT_TRUE(LineSegment(polygon[5], polygon[9]).intersects(intersection.location)) << "Segment 5 overlaps with segment 9.";
		}
	}
}

}