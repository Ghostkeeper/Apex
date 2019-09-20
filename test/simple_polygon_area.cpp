/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "helpers/MockSimplePolygon.hpp" //Mock away the base SimplePolygon class. Also provides the area function under test.

namespace apex {

/*
 * Tests whether the area of an empty polygon starts off at 0.
 */
TEST(SimplePolygonAreaTest, InitialAreaIsZero) {
	MockSimplePolygon empty_polygon;
	EXPECT_EQ(empty_polygon.area(), 0);
}

/*
 * Tests the area of a basic 1000 by 1000 square.
 */
TEST(SimplePolygonAreaTest, Square1000) {
	MockSimplePolygon square_1000(MockSimplePolygon::Shape::SQUARE_1000);
	EXPECT_EQ(square_1000.area(), 1000 * 1000);
}

}