/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.
#include <limits> //To test compatibility between the coordinate types.

#include "apex/coordinate.hpp" //The code under test.

namespace apex {

/*!
 * Test whether the range of coord_t is as expected.
 */
TEST(Coordinate, CoordTRange) {
	//Test increasing bit depths to see where it should give an overflow.
	coord_t x = 0;
	EXPECT_EQ(x, 0);

	x = 0x80;
	EXPECT_EQ(x, 0x80);

	x = 0x8000;
	EXPECT_EQ(x, 0x8000);

	x = 0x80000000;
	EXPECT_EQ(x, -0x80000000) << "Coordinates must be 32-bits, so integers must overflow here.";

#pragma GCC diagnostic ignored "-Woverflow"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconstant-conversion"
	x = 0x8000000000000000;
#pragma GCC diagnostic pop
	EXPECT_EQ(x, 0) << "Coordinates must be 32-bits, so only the lower bits must have been interpreted here.";

	x = -0x81;
	EXPECT_EQ(x, -0x81);

	x = -0x8001;
	EXPECT_EQ(x, -0x8001);

	x = -0x80000001;
	EXPECT_EQ(x, 0x7FFFFFFF) << "Coordinates must be 32-bits, so integers must underflow here.";

#pragma GCC diagnostic ignored "-Woverflow"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconstant-conversion"
	x = -0x8000000000000001;
#pragma GCC diagnostic pop
	EXPECT_EQ(x, -1) << "Coordinates must be 32-bits, so only the lower bits must have been interpreted here.";
};

/*!
 * Test whether the range of areas that can be expressed with area_t matches the
 * range of coordinates expressed by coord_t.
 *
 * Sadly, the limit of area_t is only up to half the coordinate space of coord_t
 * due to needing to represent the negative areas too.
 */
TEST(Coordinate, AreaTRange) {
	//Test whether the coordinates that can be represented with areas in coord_t are represented with area_t.
	uintmax_t max_distance = static_cast<uintmax_t>(std::numeric_limits<coord_t>::max()) * 2;
	area_t max_area = max_distance / 2 * max_distance;
	EXPECT_EQ(max_area, max_distance / 2 * max_distance) << "There was no integer overflow.";

	area_t min_area = -max_distance / 2 * max_distance;
	EXPECT_EQ(min_area, -max_distance / 2 * max_distance) << "There was no integer underflow.";
}

/*!
 * Test rounding a division when the answer is already integer.
 */
TEST(Coordinate, RoundDivideInteger) {
	EXPECT_EQ(round_divide(15, 3), 5) << "15 / 3 = 5, which is already integer and needs no further rounding.";
	EXPECT_EQ(round_divide(-40, 10), -4) << "-40 / 4 = -4, which is already integer and needs no further rounding.";
	EXPECT_EQ(round_divide(24, -8), -3) << "24 / -8 = -3, which is already integer and needs no further rounding.";
	EXPECT_EQ(round_divide(-42, -7), 6) << "-42 / -7 = 6, which is already integer and needs no further rounding.";
	EXPECT_EQ(round_divide(0, 10), 0) << "0 / 10 = 0, which is already integer and needs no further rounding.";
}

/*!
 * Test rounding a division of positive numbers.
 */
TEST(Coordinate, RoundDividePositive) {
	EXPECT_EQ(round_divide(7, 4), 2) << "7 / 4 = 1.75, which rounds up to 2.";
	EXPECT_EQ(round_divide(26, 8), 3) << "26 / 8 = 3.25, which rounds down to 3.";
	EXPECT_EQ(round_divide(27, 6), 5) << "27 / 6 = 4.5, which rounds away from zero to 5.";
}

/*!
 * Test rounding a division of two negative numbers.
 */
TEST(Coordinate, RoundDivideNegative) {
	EXPECT_EQ(round_divide(-14, -5), 3) << "-14 / -5 = 2.8, which rounds up to 3.";
	EXPECT_EQ(round_divide(-2, -12), 0) << "-2 / -12 = 0.16667, which rounds down to 0.";
	EXPECT_EQ(round_divide(-28, -8), 4) << "-28 / -8 = 3.5, which rounds away from zero to 4.";
}

/*!
 * Test rounding a division with a positive numerator but negative denominator.
 */
TEST(Coordinate, RoundDivideNegativeDenominator) {
	EXPECT_EQ(round_divide(16, -5), -3) << "16 / -5 = -3.2, which rounds up to -3.";
	EXPECT_EQ(round_divide(46, -8), -6) << "46 / -8 = -5.75, which rounds down to -6.";
	EXPECT_EQ(round_divide(3, -2), -2) << "3 / -2 = -1.5, which rounds away from zero to -2.";
}

/*!
 * Test rounding a division with a positive denominator but negative numerator.
 */
TEST(Coordinate, RoundDivideNegativeNumerator) {
	EXPECT_EQ(round_divide(-20, 6), -3) << "-20 / 6 = -3.33333, which rounds up to -3.";
	EXPECT_EQ(round_divide(-24, 5), -5) << "-24 / 5 = -4.8, which rounds down to -5.";
	EXPECT_EQ(round_divide(-25, 10), -3) << "-25 / 10 = -2.5, which rounds away from zero to -3.";
}

}