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

struct Point2;

/*
 * Some fixtures for the SimplePolygonTranslate tests.
 */
class SimplePolygonTranslate : public testing::Test {
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
 * Tests whether moving by 0,0 yields the original simple polygon.
 */
TEST_F(SimplePolygonTranslate, MoveZero) {
	SimplePolygon square_1000_original(square_1000); //Make a copy of the original to compare against.
	square_1000.translate(0, 0);
	EXPECT_EQ(square_1000, square_1000_original) << "The polygon may not have changed by moving 0,0.";
}

/*
 * Tests moving a polygon along the X direction.
 */
TEST_F(SimplePolygonTranslate, MoveX) {
	SimplePolygon square_1000_original(square_1000); //Make a copy of the original to compare against.
	square_1000.translate(250, 0);

	ASSERT_EQ(square_1000.size(), square_1000_original.size()) << "The polygon may not gain or lose any vertices by translating it.";

	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], square_1000_original[i] + Point2(250, 0));
	}
}

/*
 * Tests moving a polygon along the Y direction.
 */
TEST_F(SimplePolygonTranslate, MoveY) {
	SimplePolygon square_1000_original(square_1000); //Make a copy of the original to compare against.
	square_1000.translate(0, -300);

	ASSERT_EQ(square_1000.size(), square_1000_original.size()) << "The polygon may not gain or lose any vertices by translating it.";

	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], square_1000_original[i] + Point2(0, -300));
	}
}

/*
 * Tests moving a polygon in both dimensions at the same time.
 */
TEST_F(SimplePolygonTranslate, MoveXY) {
	SimplePolygon square_1000_original(square_1000); //Make a copy of the original to compare against.

	const Point2 move_vector(-40, 70);
	square_1000.translate(move_vector);

	ASSERT_EQ(square_1000.size(), square_1000_original.size()) << "The polygon may not gain or lose any vertices by translating it.";

	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], square_1000_original[i] + move_vector);
	}
}

}