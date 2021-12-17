/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "apex/point2.hpp" //To provide the delta vector to translate by.
#include "apex/translate.hpp" //The function under test.
#include "helpers/simple_polygon_test_cases.hpp" //To load testing polygons to translate.

namespace apex {

static SimplePolygonTestCases test_cases;

/*!
 * Tests whether moving by 0,0 yields the original simple polygon.
 */
TEST(SimplePolygonTranslate, MoveZero) {
	SimplePolygon square_1000 = test_cases.square_1000();
	translate(square_1000, Point2(0, 0));
	EXPECT_EQ(square_1000, test_cases.square_1000()) << "The polygon may not have changed by moving 0,0.";
}

/*!
 * Tests moving a polygon along the X direction.
 */
TEST(SimplePolygonTranslate, MoveX) {
	SimplePolygon square_1000 = test_cases.square_1000();
	const SimplePolygon original = square_1000; //Keep a copy to compare to the original.
	translate(square_1000, Point2(250, 0));

	ASSERT_EQ(square_1000.size(), original.size()) << "The polygon may not gain or lose any vertices by translating it.";
	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], original[i] + Point2(250, 0));
	}
}

/*!
 * Tests moving a polygon along the Y direction.
 */
TEST(SimplePolygonTranslate, MoveY) {
	SimplePolygon square_1000 = test_cases.square_1000();
	const SimplePolygon original = square_1000; //Keep a copy to compare to the original.
	translate(square_1000, Point2(0, -300));

	ASSERT_EQ(square_1000.size(), original.size()) << "The polygon may not gain or lose any vertices by translating it.";
	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], original[i] + Point2(0, -300));
	}
}

/*!
 * Tests moving a polygon in both dimensions at the same time.
 */
TEST(SimplePolygonTranslate, MoveXY) {
	SimplePolygon square_1000 = test_cases.square_1000();
	const SimplePolygon original = square_1000; //Keep a copy to compare to the original.

	const Point2 move_vector(-40, 70);
	translate(square_1000, move_vector);

	ASSERT_EQ(square_1000.size(), original.size()) << "The polygon may not gain or lose any vertices by translating it.";
	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], original[i] + move_vector);
	}
}

}