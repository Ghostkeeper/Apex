/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.
#include <sstream> //To test printing to a stream.
#include <tuple> //To parametrise tests with multiple parameters.

#include "apex/coordinate.hpp" //To construct points.
#include "apex/point2.hpp" //The code under test.

namespace apex {

/*!
 * Fixture for parametrised tests with two points (or four coordinates).
 */
class Point2FourCoordinates : public testing::TestWithParam<std::tuple<coord_t, coord_t, coord_t, coord_t>> {};

/*!
 * Tests summing coordinates.
 *
 * The final coordinates must be element-wise summed.
 */
TEST_P(Point2FourCoordinates, Sum) {
	const std::tuple<coord_t, coord_t, coord_t, coord_t> parameters = GetParam();
	Point2 a(std::get<0>(parameters), std::get<1>(parameters));
	const Point2 b(std::get<2>(parameters), std::get<3>(parameters));
	const Point2 sum = a + b;
	EXPECT_EQ(sum.x, std::get<0>(parameters) + std::get<2>(parameters));
	EXPECT_EQ(sum.y, std::get<1>(parameters) + std::get<3>(parameters));

	a += b;
	EXPECT_EQ(a.x, sum.x);
	EXPECT_EQ(a.y, sum.y);
}

/*!
 * Tests subtracting coordinates.
 *
 * The final coordinates must be element-wise subtracted.
 */
TEST_P(Point2FourCoordinates, Subtract) {
	const std::tuple<coord_t, coord_t, coord_t, coord_t> parameters = GetParam();
	Point2 a(std::get<0>(parameters), std::get<1>(parameters));
	const Point2 b(std::get<2>(parameters), std::get<3>(parameters));
	const Point2 difference = a - b;
	EXPECT_EQ(difference.x, std::get<0>(parameters) - std::get<2>(parameters));
	EXPECT_EQ(difference.y, std::get<1>(parameters) - std::get<3>(parameters));

	a -= b;
	EXPECT_EQ(a.x, difference.x);
	EXPECT_EQ(a.y, difference.y);
}

/*!
 * Various combinations of points to test summing and subtracting with.
 */
INSTANTIATE_TEST_SUITE_P(SubtractInst, Point2FourCoordinates, testing::Values(
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

/*!
 * Tests equality between points.
 */
TEST(Point, Equality) {
	constexpr Point2 a(10, 20); //The point to compare to.

	constexpr Point2 b(10, 20); //Indeed equal.
	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a != b);

	constexpr Point2 diagonal(15, 25); //Not equal in either dimension.
	EXPECT_FALSE(a == diagonal);
	EXPECT_TRUE(a != diagonal);

	constexpr Point2 horizontal(15, 20); //Not equal in X dimension.
	EXPECT_FALSE(a == horizontal);
	EXPECT_TRUE(a != horizontal);

	constexpr Point2 vertical(10, 25); //Not equal in Y dimension.
	EXPECT_FALSE(a == vertical);
	EXPECT_TRUE(a != vertical);
}

/*!
 * Tests outputting this point to a stream.
 */
TEST(Point, Stream) {
	const Point2 point(42, 69);
	std::stringstream stream;
	stream << point;
	EXPECT_EQ(stream.str(), "[42,69]");
}

/*!
 * Tests whether the cross product adheres to the right-hand rule.
 *
 * The right-hand rule is a mnemonic to find the direction of the cross product.
 * When pointing the index finger in the direction of the first vector and the
 * middle finger in the direction of the second vector, and shaping the hand in
 * a sort of finger gun, the thumb will point in the direction of the cross
 * product of the first and second vectors.
 */
TEST(Point, CrossProductRightHandRule) {
	EXPECT_GT(Point2(1, 0).cross_product(Point2(0, 1)), 0) << "The cross product should be positive according to the right-hand rule.";
	EXPECT_LT(Point2(0, 1).cross_product(Point2(1, 0)), 0) << "The cross product should be negative according to the right-hand rule.";
}

}