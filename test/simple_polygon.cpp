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

	copy[0].x += 1; //Modify the copy so that we're sure this is not held by reference or anything.
	EXPECT_NE(triangle, copy);
}

/*
 * Tests move-constructing a simple polygon.
 */
TEST_F(SimplePolygonFixture, ConstructMove) {
	SimplePolygon copy = triangle; //Make a copy to move so we keep the original fixture to compare against.
	SimplePolygon target = std::move(copy);

	EXPECT_EQ(triangle, target);
}

/*
 * Tests making a copy via assignment.
 */
TEST_F(SimplePolygonFixture, Assignment) {
	SimplePolygon copy = triangle; //Make a copy by assignment.

	EXPECT_EQ(triangle, copy);

	copy[0].x += 1; //Modify the copy so that we're sure this is not held by reference or anything.
	EXPECT_NE(triangle, copy);
}

/*
 * Tests accessing the vertices of the simple polygon by reference.
 */
TEST_F(SimplePolygonFixture, AccessReference) {
	Point2& vertex = triangle[0];
	vertex.x = 42; //Modify it.
	EXPECT_EQ(triangle[0].x, 42) << "The modified coordinates should be modified by reference, so it should be stored in the simple polygon as well.";
}

/*
 * Tests accessing individual vertices of the simple polygon by copy.
 */
TEST_F(SimplePolygonFixture, AccessCopy) {
	const Point2 vertex = triangle[1];
	EXPECT_EQ(vertex.x, 100);
	EXPECT_EQ(vertex.y, 20);
}

}