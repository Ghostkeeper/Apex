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
	EXPECT_EQ(result.size(), 1) << "The polygon is closed, so it has two line segments. They exactly overlap, so that's an intersection.";
	//For simplicity we'll just check equality with the vertices, since in practice we'll always return one of the endpoints, not somewhere halfway the line.
	EXPECT_TRUE(result[0].location == polygon[0] || result[0].location == polygon[1]) << "The intersecting point must be somewhere on the line.";
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
	EXPECT_EQ(self_intersections(PolygonTestCases::touching_edge()), ground_truth) << "A vertex touches an edge, so both incident edges will be reported as intersecting.";
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
	EXPECT_EQ(self_intersections(PolygonTestCases::touching_vertex()), ground_truth) << "Two vertices touch each other, and it's not just zero-length segments. Every non-adjacent edge must be reported as intersecting.";
}

}