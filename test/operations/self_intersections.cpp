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

}