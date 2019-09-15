/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "apex/simple_polygon.hpp" //The code under test.

namespace apex {

/*
 * Some fixtures for the SimplePolygonArea tests.
 */
class SimplePolygonArea : public testing::Test {
protected:
	/*
	 * A square of 1000 by 1000.
	 */
	SimplePolygon square_1000;

	/*
	 * Prepares for running a test by constructing the fixtures we need.
	 */
	virtual void SetUp() {
		square_1000.emplace_back(0, 0);
		square_1000.emplace_back(1000, 0);
		square_1000.emplace_back(1000, 1000);
		square_1000.emplace_back(0, 1000);
	}
};

/*
 * Tests whether the area of an empty polygon starts off at 0.
 */
TEST_F(SimplePolygonArea, InitialAreaIsZero) {
	SimplePolygon empty_polygon;
	EXPECT_EQ(empty_polygon.area(), 0);
}

/*
 * Tests the area of a basic 1000 by 1000 square.
 */
TEST_F(SimplePolygonArea, Square1000) {
	EXPECT_EQ(square_1000.area(), 1000 * 1000);
}

}