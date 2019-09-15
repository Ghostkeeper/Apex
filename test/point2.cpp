/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.
#include <tuple> //To parametrise tests with multiple parameters.

#include "apex/coordinate.hpp" //To construct points.
#include "apex/point2.hpp" //The code under test.

namespace apex {

class Point2FourCoordinates : public testing::TestWithParam<std::tuple<coord_t, coord_t, coord_t, coord_t>> {};

TEST_P(Point2FourCoordinates, SumSimple) {
	const std::tuple<coord_t, coord_t, coord_t, coord_t> parameters = GetParam();
	const Point2 a(std::get<0>(parameters), std::get<1>(parameters));
	const Point2 b(std::get<2>(parameters), std::get<3>(parameters));
	const Point2 sum = a + b;
	EXPECT_EQ(sum.x, std::get<0>(parameters) + std::get<2>(parameters));
	EXPECT_EQ(sum.y, std::get<1>(parameters) + std::get<3>(parameters));
}

INSTANTIATE_TEST_SUITE_P(SumSimpleInst, Point2FourCoordinates, testing::Values(
	std::make_tuple(0, 0, 0, 0),
	std::make_tuple(5, 10, 8, 21),
	std::make_tuple(1000, 1333, -1500, -6000),
	std::make_tuple(-100, -200, 400, 750),
	std::make_tuple(-2, -3, -10, -30)
));

TEST_P(Point2FourCoordinates, SubtractSimple) {
	const std::tuple<coord_t, coord_t, coord_t, coord_t> parameters = GetParam();
	const Point2 a(std::get<0>(parameters), std::get<1>(parameters));
	const Point2 b(std::get<2>(parameters), std::get<3>(parameters));
	const Point2 difference = a - b;
	EXPECT_EQ(difference.x, std::get<0>(parameters) - std::get<2>(parameters));
	EXPECT_EQ(difference.y, std::get<1>(parameters) - std::get<3>(parameters));
}

INSTANTIATE_TEST_SUITE_P(SubtractSimpleInst, Point2FourCoordinates, testing::Values(
	std::make_tuple(0, 0, 0, 0),
	std::make_tuple(8, 21, 5, 10),
	std::make_tuple(5, 10, 8, 21),
	std::make_tuple(1000, 1333, -1500, -6000),
	std::make_tuple(-1500, -6000, 1000, 1333),
	std::make_tuple(-100, -200, 400, 750),
	std::make_tuple(400, 750, -100, -200),
	std::make_tuple(-2, -3, -10, -30),
	std::make_tuple(-10, -30, -2, -3)
));

}