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

	result = LineSegment::intersect(Point2(10, 10), Point2(110, 210), Point2(130, 250), Point2(170, 330));
	EXPECT_EQ(result, std::nullopt) << "Both line segments are parallel and even collinear, but still separated lengthwise and don't intersect or overlap.";
}

/*!
 * Test finding the intersection of two line segments that are collinear and
 * intersect at their endpoints, but don't overlap.
 */
TEST(LineSegment, IntersectionParallelVertex) {
	const std::optional<Point2> result = LineSegment::intersect(Point2(5, 5), Point2(50, 50), Point2(50, 50), Point2(100, 100)); //Both have a slope of 1, so they are collinear.
	ASSERT_NE(result, std::nullopt) << "The line segments share a vertex, so they intersect.";
	EXPECT_EQ(*result, Point2(50, 50)) << "The vertex they share is here.";
}

/*!
 * Test finding the intersection of two collinear line segments that partially
 * overlap.
 */
TEST(LineSegment, IntersectionParallelOverlap) {
	std::optional<Point2> result = LineSegment::intersect(Point2(100, 0), Point2(40, 60), Point2(70, 30), Point2(0, 100)); //Both have a slope of -1, so they are collinear. They overlap from X=40 to X=70.
	ASSERT_NE(result, std::nullopt) << "The line segments partially overlap, so they intersect.";
	EXPECT_GE(result->x, 30) << "The overlap is from X=40 to X=70, so a point in that range must be returned.";
	EXPECT_LE(result->x, 70) << "The overlap is from X=40 to X=70, so a point in that range must be returned.";
	EXPECT_EQ(result->y, 100 - result->x) << "The resulting point must be in the intersecting overlap.";

	result = LineSegment::intersect(Point2(70, 30), Point2(0, 100), Point2(100, 0), Point2(40, 60)); //Same as previous case, but swapped lines.
	ASSERT_NE(result, std::nullopt) << "The line segments partially overlap, so they intersect.";
	EXPECT_GE(result->x, 30) << "The overlap is from X=40 to X=70, so a point in that range must be returned.";
	EXPECT_LE(result->x, 70) << "The overlap is from X=40 to X=70, so a point in that range must be returned.";
	EXPECT_EQ(result->y, 100 - result->x) << "The resulting point must be in the intersecting overlap.";

	result = LineSegment::intersect(Point2(10, 10), Point2(10, 110), Point2(10, 30), Point2(10, 45)); //Both are vertical lines at X10, so they are collinear. The first line wholly overlaps the second.
	ASSERT_NE(result, std::nullopt) << "The line segments overlap, so they intersect.";
	EXPECT_EQ(result->x, 10) << "Both lines are vertical at X=10, so the intersection occurs there.";
	EXPECT_LE(result->y, 45) << "The second line is completely overlapped, so the intersection must be in that line.";
	EXPECT_GE(result->y, 30) << "The second line is completely overlapped, so the intersection must be in that line.";

	result = LineSegment::intersect(Point2(10, 30), Point2(10, 45), Point2(10, 110), Point2(10, 10)); //Same as previous case, but swapped lines and one line is reverted.
	ASSERT_NE(result, std::nullopt) << "The line segments overlap, so they intersect.";
	EXPECT_EQ(result->x, 10) << "Both lines are vertical at X=10, so the intersection occurs there.";
	EXPECT_LE(result->y, 45) << "The second line is completely overlapped, so the intersection must be in that line.";
	EXPECT_GE(result->y, 30) << "The second line is completely overlapped, so the intersection must be in that line.";
}

/*!
 * Test rounding the intersection coordinates to the nearest coordinate point.
 *
 * The actual intersection between lines is not always on an integer coordinate.
 * Often it will be an arbitrary rational coordinate. The result of the
 * intersection calculation must indicate the closest point that can be
 * represented with the coordinate system.
 */
TEST(LineSegment, IntersectionRounding) {
	//We'll use a diagonal line with a slope of 1:4, and then intersect that with vertical lines where the X coordinate is not divisible by 4 to get a rational intersection coordinate.
	std::optional<Point2> result = LineSegment::intersect(Point2(0, 0), Point2(400, 100), Point2(41, 0), Point2(41, 100)); //X=41, so the intersection is at Y=10.25.
	ASSERT_NE(result, std::nullopt) << "The line segments definitely intersect.";
	EXPECT_EQ(*result, Point2(41, 10)) << "They intersect at [41, 10.25], which is rounded to [41, 10].";

	//Same diagonal line, but now at X=43, so the intersection is at Y=10.75.
	result = LineSegment::intersect(Point2(0, 0), Point2(400, 100), Point2(43, 0), Point2(43, 100));
	ASSERT_NE(result, std::nullopt) << "The line segments definitely intersect.";
	EXPECT_EQ(*result, Point2(43, 11)) << "They intersect at [43, 10.75], which is rounded to [43, 11].";

	//At X=42, the intersection is at Y=10.5, which should get rounded away from zero.
	result = LineSegment::intersect(Point2(0, 0), Point2(400, 100), Point2(42, 0), Point2(42, 100));
	ASSERT_NE(result, std::nullopt) << "The line segments definitely intersect.";
	EXPECT_EQ(*result, Point2(42, 11)) << "They intersect at [42, 10.5], which is rounded to [42, 11].";
}

/*!
 * Test intersecting line segments when they narrowly miss, such that a rounding
 * error would cause a false intersection to be found.
 */
TEST(LineSegment, IntersectionShallowSlopeMiss) {
	//Make a line with a very shallow slope of 1:1000. Then when that line is almost up to 1, have another line come down to Y=1. They almost intersect, just 0.01 units short.
	std::optional<Point2> result = LineSegment::intersect(Point2(0, 0), Point2(1000, 1), Point2(990, 10), Point2(990, 1));
	EXPECT_EQ(result, std::nullopt) << "The line segments almost intersect, but not quite. There is 0.01 units of space between them.";

	//Make a line that crosses Y=1 at a non-integer X coordinate, then another line that reaches up to just left of where Y=1 is crossed.
	result = LineSegment::intersect(Point2(0, 0), Point2(101, 2), Point2(50, 10), Point2(50, 1));
	EXPECT_EQ(result, std::nullopt) << "The line segments almost intersect, but not quite. The second line ends just left of where the first line crosses Y=1.";
}

/*!
 * Test intersecting line segment when they only slightly hit, such that a
 * rounding error would cause them to be found to not be intersecting.
 */
TEST(LineSegment, IntersectionShallowSlopeHit) {
	//Make a line with a very shallow slope of 1:1000. Then create a line that dips just below that shallow slope.
	std::optional<Point2> result = LineSegment::intersect(Point2(0, 0), Point2(1000, 1), Point2(10, 10), Point2(10, 0));
	ASSERT_NE(result, std::nullopt) << "The line segments intersect, but only slightly.";
	EXPECT_EQ(*result, Point2(10, 0)) << "The line segments do intersect at [10, 0.01], which gets rounded to [10, 0].";

	//Make a line that crosses Y=1 at a non-integer X coordinate, then another line that reaches up to just right of where Y=1 is crossed.
	result = LineSegment::intersect(Point2(0, 0), Point2(99, 2), Point2(50, 10), Point2(50, 1));
	ASSERT_NE(result, std::nullopt) << "The line segments intersect, but only slightly.";
	EXPECT_EQ(*result, Point2(50, 1)) << "The line segments do intersect at [50, 1.02], which gets rounded to [50, 1].";
}

}