/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.
#include <limits> //To test compatibility between the coordinate types.

#include "apex/coordinate.hpp" //The code under test.

namespace apex {

/*!
 * Tests whether the range of coord_t is as expected.
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
	x = -0x8000000000000001;
#pragma GCC diagnostic pop
	EXPECT_EQ(x, -1) << "Coordinates must be 32-bits, so only the lower bits must have been interpreted here.";
};

/*!
 * Tests whether the range of areas that can be expressed with area_t matches
 * the range of coordinates expressed by coord_t.
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

}