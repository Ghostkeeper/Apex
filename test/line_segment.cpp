/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "apex/line_segment.hpp" //The code under test.

namespace apex {

/*!
 * Test finding the intersection of two line segments that are simply crossing.
 */
TEST(LineSegment, IntersectionCross) {
	const std::optional<Point2> result = LineSegment::intersect(Point2(10, 100), Point2(210, 20), Point2(130, 10), Point2(180, 60));
	ASSERT_NE(result, std::nullopt) << "These line segments intersect fully.";
	EXPECT_EQ(*result, Point2(160, 40)) << "This is where the two line segments intersect.";
}

/*!
 * Test finding the intersection of two line segments when they don't cross.
 */
TEST(LineSegment, IntersectionDontCross) {
	std::optional<Point2> result = LineSegment::intersect(Point2(10, 10), Point2(140, 130), Point2(200, 80), Point2(230, 180));
	EXPECT_EQ(result, std::nullopt) << "These two line segments don't intersect.";

	result = LineSegment::intersect(Point2(-100, 100), Point2(-10, 10), Point2(20, 10), Point2(20, -500));
	EXPECT_EQ(result, std::nullopt) << "These two line segments don't intersect.";
}

/*!
 * Test finding the intersection of two line segments when an endpoint of the
 * line intersects with the body of the other line.
 */
TEST(LineSegment, IntersectionEndpoint) {
	std::optional<Point2> result = LineSegment::intersect(Point2(10, 10), Point2(110, 60), Point2(60, 35), Point2(70, 50)); //[60,35] is exactly halfway the first line.
	ASSERT_NE(result, std::nullopt) << "One of the vertices of the second line segment is exactly on the first line segment.";
	EXPECT_EQ(*result, Point2(60, 35)) << "One of the vertices of the second line segment is exactly on the first line segment.";

	result = LineSegment::intersect(Point2(10, 10), Point2(110, 60), Point2(70, 50), Point2(60, 35)); //Second line flipped around, to test the second vertex.
	ASSERT_NE(result, std::nullopt) << "Even if the segment is flipped around, they still intersect.";
	EXPECT_EQ(*result, Point2(60, 35)) << "Even if the segment is flipped around, the intersection is the same.";

	result = LineSegment::intersect(Point2(60, 35), Point2(70, 50), Point2(10, 10), Point2(110, 60)); //The two line segments swapped around.
	ASSERT_NE(result, std::nullopt) << "Swapping the two line segments around should make no difference. They still intersect.";
	EXPECT_EQ(*result, Point2(60, 35)) << "Swapping the two line segments around should make no difference. The intersection point is the same.";

	result = LineSegment::intersect(Point2(70, 50), Point2(60, 35), Point2(10, 10), Point2(110, 60)); //The first line flipped around, to test the second vertex.
	ASSERT_NE(result, std::nullopt) << "Even if the segment is flipped around, they still intersect.";
	EXPECT_EQ(*result, Point2(60, 35)) << "Even if the segment is flipped around, the intersection is the same.";
}

/*!
 * Test finding the intersection of two line segments when they are joined at
 * one of their endpoints.
 */
TEST(LineSegment, IntersectionEndpoints) {
	std::optional<Point2> result = LineSegment::intersect(Point2(0, 0), Point2(50, 50), Point2(60, 0), Point2(50, 50)); //At their last endpoints.
	ASSERT_NE(result, std::nullopt) << "The line segments share a vertex, so they intersect.";
	EXPECT_EQ(*result, Point2(50, 50)) << "The vertex they share is here.";

	result = LineSegment::intersect(Point2(0, 0), Point2(50, 50), Point2(50, 50), Point2(60, 0)); //The second line flipped around, causing that one to intersect at its first vertex but the first at its second vertex.
	ASSERT_NE(result, std::nullopt) << "The line segments share a vertex, so they intersect, even after flipping one of the lines.";
	EXPECT_EQ(*result, Point2(50, 50)) << "The vertex they share is here.";

	result = LineSegment::intersect(Point2(50, 50), Point2(0, 0), Point2(50, 50), Point2(60, 0)); //Both lines flipped around, causing them to intersect at their first vertices.
	ASSERT_NE(result, std::nullopt) << "The line segments share their first vertices, so they intersect.";
	EXPECT_EQ(*result, Point2(50, 50)) << "The vertex they share is here.";

	result = LineSegment::intersect(Point2(50, 50), Point2(0, 0), Point2(60, 0), Point2(50, 50)); //Just the first line flipped around, causing that one to intersect at its first vertex but the second at its second vertex.
	ASSERT_NE(result, std::nullopt) << "The line segments share a vertex, so they intersect, even after flipping one of the lines.";
	EXPECT_EQ(*result, Point2(50, 50)) << "The vertex they share is here.";
}

/*!
 * Test finding the intersection of two line segments when they don't intersect,
 * but are parallel anyway.
 */
TEST(LineSegment, IntersectionParallelSeparate) {
	std::optional<Point2> result = LineSegment::intersect(Point2(10, 10), Point2(110, 210), Point2(50, 50), Point2(90, 130)); //Both have a slope of 2 over 1, so they are parallel, but don't intersect.
	EXPECT_EQ(result, std::nullopt) << "Both line segments have the same slope, but they start from different places and don't intersect or overlap.";

	result = LineSegment::intersect(Point2(10, 10), Point2(110, 210), Point2(130, 250), Point2(170, 320));
	EXPECT_EQ(result, std::nullopt) << "Both line segments are parallel and even colinear, but still separated lengthwise and don't intersect or overlap.";
}

}