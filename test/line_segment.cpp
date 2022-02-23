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
 * Tests finding the intersection of two line segments simply crossing.
 */
TEST(LineSegment, IntersectionCross) {
	std::optional<Point2> result = LineSegment::intersect(Point2(10, 100), Point2(210, 20), Point2(130, 10), Point2(180, 60));
	EXPECT_EQ(*result, Point2(160, 40));
}

}