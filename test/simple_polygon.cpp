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
 * Fixture that contains a few pre-constructed simple polygons.
 */
class SimplePolygonFixture : public ::testing::Test {
public:
	/*
	 * A simple shape with three vertices.
	 */
	SimplePolygon triangle;

	/*
	 * Constructs the fixture simple polygons.
	 */
	void SetUp() {
		triangle.emplace_back(20, 20);
		triangle.emplace_back(100, 20);
		triangle.emplace_back(60, 60);
	}
};

/*
 * Tests constructing an empty simple polygon.
 */
TEST(SimplePolygon, ConstructEmpty) {
	SimplePolygon empty;
	EXPECT_EQ(empty.size(), 0);
}

/*
 * Tests copy-constructing a simple polygon.
 */
TEST_F(SimplePolygonFixture, ConstructCopy) {
	SimplePolygon copy(triangle); //Run the copy constructor.

	EXPECT_EQ(triangle, copy);
}

/*
 * Tests move-constructing a simple polygon.
 */
TEST_F(SimplePolygonFixture, ConstructMove) {
	SimplePolygon copy = triangle; //Make a copy to move so we keep the original fixture to compare against.
	SimplePolygon target = std::move(copy);

	EXPECT_EQ(triangle, target);
}

}