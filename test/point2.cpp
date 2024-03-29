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
 * Test summing coordinates.
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
 * Test subtracting coordinates.
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
 * Various combinations of points to test with.
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
 * Test equality between points.
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
 * Test comparing the ordering of two points lexicographically.
 *
 * The X coordinate should be the leading comparison. In case of ties, the Y
 * coordinate is considered.
 */
TEST(Point, Comparison) {
	constexpr Point2 point(10, 20); //The point to compare to.
	constexpr Point2 lower(9, 21); //Lower X coordinate, which wins over the higher Y coordinate.
	EXPECT_TRUE(lower < point);
	EXPECT_TRUE(lower <= point);
	EXPECT_FALSE(point < lower);
	EXPECT_FALSE(point <= lower);
	EXPECT_TRUE(point > lower);
	EXPECT_TRUE(point >= lower);
	EXPECT_FALSE(lower > point);
	EXPECT_FALSE(lower >= point);

	const Point2 higher(11, -200); //Higher X coordinate, which wins over the lower Y coordinate.
	EXPECT_TRUE(point < higher);
	EXPECT_TRUE(point <= higher);
	EXPECT_FALSE(higher < point);
	EXPECT_FALSE(higher <= point);
	EXPECT_TRUE(higher > point);
	EXPECT_TRUE(higher >= point);
	EXPECT_FALSE(point > higher);
	EXPECT_FALSE(point >= higher);

	const Point2 above(10, 50); //Same X coordinate, but higher Y coordinate which breaks the tie.
	EXPECT_TRUE(point < above);
	EXPECT_TRUE(point <= above);
	EXPECT_FALSE(above < point);
	EXPECT_FALSE(above <= point);
	EXPECT_TRUE(above > point);
	EXPECT_TRUE(above >= point);
	EXPECT_FALSE(point > above);
	EXPECT_FALSE(point >= above);

	const Point2 below(10, -100); //Same X coordinate, but lower Y coordinate which breaks the tie.
	EXPECT_TRUE(below < point);
	EXPECT_TRUE(below <= point);
	EXPECT_FALSE(point < below);
	EXPECT_FALSE(point <= below);
	EXPECT_TRUE(point > below);
	EXPECT_TRUE(point >= below);
	EXPECT_FALSE(below > point);
	EXPECT_FALSE(below >= point);
}

/*!
 * Test outputting this point to a stream.
 */
TEST(Point, Stream) {
	const Point2 point(42, 69);
	std::stringstream stream;
	stream << point;
	EXPECT_EQ(stream.str(), "[42,69]");
}

/*!
 * Test if the memory alignment of the point is what you'd expect.
 */
TEST(Point, Alignment) {
	EXPECT_EQ(std::alignment_of<Point2>::value, 8);
}

/*!
 * Test whether the cross product adheres to the right-hand rule.
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

/*!
 * The magnitude of the cross product should be the positive area of the
 * parallelogram created with the two vectors as two of the sides.
 */
TEST(Point, CrossProductMagnitude) {
	//Use Pythagorean triples, so that the magnitude of these vectors is integer.
	const Point2 a(12, 16); //Magnitude 20.
	const Point2 b = a + Point2(-4, 3); //Delta is perpendicular to a, magnitude 5. Perpendicularity makes this delta the height of the parallelogram.
	EXPECT_EQ(std::abs(a.cross_product(b)), 20 * 5) << "The magnitude of the cross product should equal the area of the parallelogram.";
}

/*!
 * The cross product is anti-commutative, meaning that reversing the operands
 * causes the resulting magnitude to be inverted too.
 */
TEST_P(Point2FourCoordinates, CrossProductAntiCommutative) {
	const std::tuple<coord_t, coord_t, coord_t, coord_t> parameters = GetParam();
	const Point2 a(std::get<0>(parameters), std::get<1>(parameters));
	const Point2 b(std::get<2>(parameters), std::get<3>(parameters));

	EXPECT_EQ(a.cross_product(b), -b.cross_product(a)) << "When the operands of the cross product are switched, the result should be inverted.";
}

/*!
 * Test the orientation of a point with a line if the line is horizontal.
 */
TEST(Point, OrientationWithLineHorizontal) {
	const Point2 line_start(10, 10);
	const Point2 line_end(110, 10);
	EXPECT_LT(Point2(40, 40).orientation_with_line(line_start, line_end), 0) << "The point is above the line, which is left of the line, so the result should be negative.";
	EXPECT_GT(Point2(40, 0).orientation_with_line(line_start, line_end), 0) << "The point is below the line, which is right of the line, so the result should be positive.";
	EXPECT_EQ(Point2(50, 10).orientation_with_line(line_start, line_end), 0) << "The point is exactly on the line, so the result should be 0.";

	//If the line switches direction, the orientation should be reversed.
	EXPECT_GT(Point2(40, 40).orientation_with_line(line_end, line_start), 0) << "Since the line is flipped (going right to left), the point above the line is to the right, so the result should be positive.";
	EXPECT_LT(Point2(40, 0).orientation_with_line(line_end, line_start), 0) << "Since the line is flipped (going right to left), the point below the line is to the left, so the result should be negative.";
	EXPECT_EQ(Point2(50, 10).orientation_with_line(line_end, line_start), 0) << "The point is exactly on the line, so the result should be 0, even if the line is flipped.";
}

/*!
 * Test the orientation of a point with a line if the line is vertical.
 */
TEST(Point, OrientationWithLineVertical) {
	const Point2 line_start(10, 10);
	const Point2 line_end(10, 110);
	EXPECT_LT(Point2(-10, 40).orientation_with_line(line_start, line_end), 0) << "The point is left of the line, so the result should be negative.";
	EXPECT_GT(Point2(40, 40).orientation_with_line(line_start, line_end), 0) << "The point is right of the line, so the result should be negative.";
	EXPECT_EQ(Point2(10, 40).orientation_with_line(line_start, line_end), 0) << "The point is exactly on the line, so the result should be 0.";

	//If the line switches direction, the orientation should be reversed.
	EXPECT_GT(Point2(-10, 40).orientation_with_line(line_end, line_start), 0) << "Since the line is flipped (going up to down), the point is now to its right, so the result should be positive.";
	EXPECT_LT(Point2(40, 40).orientation_with_line(line_end, line_start), 0) << "Since the line is flipped (going up to down), the point is now to its left, so the result should be negative.";
	EXPECT_EQ(Point2(10, 40).orientation_with_line(line_end, line_start), 0) << "The point is exactly on the line, so the result should be 0, even if the line is flipped.";
}

/*!
 * Test the orientation of a point with a line if the line is diagonal.
 */
TEST(Point, OrientationWithLineDiagonal) {
	const Point2 line_start(10, 110); //Going from top-left to bottom-right.
	const Point2 line_end(110, 10);
	EXPECT_LT(Point2(100, 100).orientation_with_line(line_start, line_end), 0) << "The point in the top-right is to the left of the line going from top-left to bottom-right, so the result should be negative.";
	EXPECT_GT(Point2(0, 0).orientation_with_line(line_start, line_end), 0) << "The point in the bottom-left is to the right of the line going from top-left to bottom-right, so the result should be positive.";
	EXPECT_EQ(Point2(40, 80).orientation_with_line(line_start, line_end), 0) << "The point is exactly on the diagonal line, so the result should be 0.";
}

/*!
 * Test the orientation of a point with a line if the point is extremely close
 * to the line, but not exactly on it.
 *
 * With a diagonal line, a lot of the points in the line cannot be represented
 * with the integer unit coordinate system. Those points are either to the left
 * or to the right of the infinitely thin line. If the point is very close,
 * there is a danger that the point appears as if it's exactly on the line due
 * to rounding errors, but in reality it's just really close. This test ensures
 * that the points are indeed recognised as not being exactly on the line.
 *
 * The implementation should be able to handle that by virtue of the greater
 * coordinate range of the output of \ref Point2::orientation_with_line.
 */
TEST(Point, OrientationWithLineRoundingErrors) {
	//Create a line with a fairly shallow slope.
	const Point2 line_start(0, 0);
	const Point2 line_end(100, 5);

	ASSERT_EQ(Point2(20, 1).orientation_with_line(line_start, line_end), 0) << "This point is exactly on the diagonal line."; //Assert this one since the rest of the tests depend on it.
	EXPECT_LT(Point2(19, 1).orientation_with_line(line_start, line_end), 0) << "The point is slightly left of the [20,1] integer coordinate point which is on the line, so the result should be negative.";
	EXPECT_GT(Point2(21, 1).orientation_with_line(line_start, line_end), 0) << "The point is slightly right of the [20,1] integer coordinate point which is on the line, so the result should be positive.";
	EXPECT_LT(Point2(21, 2).orientation_with_line(line_start, line_end), 0) << "The point is slightly above the [20,1] integer coordinate point which is on the line, so the result should be negative.";
	EXPECT_GT(Point2(19, 0).orientation_with_line(line_start, line_end), 0) << "The point is slightly below the [20,1] integer coordinate point which is on the line, so the result should be positive.";
}

}