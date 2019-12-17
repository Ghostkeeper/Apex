/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "../helpers/mock_simple_polygon.hpp" //Mock away the base SimplePolygon class. Also provides the translate function under test.

namespace apex {

struct Point2;

/*!
 * Tests whether moving by 0,0 yields the original simple polygon.
 */
TEST(SimplePolygonTranslate, MoveZero) {
	MockSimplePolygon square_1000(MockSimplePolygon::Shape::SQUARE_1000);
	square_1000.translate(0, 0);
	EXPECT_EQ(square_1000, MockSimplePolygon(MockSimplePolygon::Shape::SQUARE_1000)) << "The polygon may not have changed by moving 0,0.";
}

/*!
 * Tests moving a polygon along the X direction.
 */
TEST(SimplePolygonTranslate, MoveX) {
	MockSimplePolygon square_1000(MockSimplePolygon::Shape::SQUARE_1000);
	square_1000.translate(250, 0);

	const MockSimplePolygon original(MockSimplePolygon::Shape::SQUARE_1000); //Compare to the polygon before it's moved.
	ASSERT_EQ(square_1000.size(), original.size()) << "The polygon may not gain or lose any vertices by translating it.";

	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], original[i] + Point2(250, 0));
	}
}

/*!
 * Tests moving a polygon along the Y direction.
 */
TEST(SimplePolygonTranslate, MoveY) {
	MockSimplePolygon square_1000(MockSimplePolygon::Shape::SQUARE_1000);
	square_1000.translate(0, -300);

	const MockSimplePolygon original(MockSimplePolygon::Shape::SQUARE_1000); //Compare to the polygon before it's moved.
	ASSERT_EQ(square_1000.size(), original.size()) << "The polygon may not gain or lose any vertices by translating it.";

	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], original[i] + Point2(0, -300));
	}
}

/*!
 * Tests moving a polygon in both dimensions at the same time.
 */
TEST(SimplePolygonTranslate, MoveXY) {
	MockSimplePolygon square_1000(MockSimplePolygon::Shape::SQUARE_1000);

	const Point2 move_vector(-40, 70);
	square_1000.translate(move_vector);

	const MockSimplePolygon original(MockSimplePolygon::Shape::SQUARE_1000); //Compare to the polygon before it's moved.
	ASSERT_EQ(square_1000.size(), original.size()) << "The polygon may not gain or lose any vertices by translating it.";

	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], original[i] + move_vector);
	}
}

}