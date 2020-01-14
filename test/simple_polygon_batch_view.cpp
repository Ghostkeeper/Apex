/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2020 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "apex/simple_polygon_batch.hpp" //The code under test.

namespace apex {

/*!
 * Fixture for the purpose of testing views on the simple polygon batch.
 *
 * This fixture provides a simple batch to get views on.
 */
class SimplePolygonBatchViewFixture : public testing::Test {
public:
	/*!
	 * A triangle shaped simple polygon, with 3 vertices.
	 */
	SimplePolygon<> triangle;

	/*!
	 * A square shaped simple polygon, with 4 vertices.
	 */
	SimplePolygon<> square;

	/*!
	 * A batch that contains two simple polygons:
	 * - One triangle (3 vertices).
	 * - One square (4 vertices).
	 */
	SimplePolygonBatch triangle_and_square;

	/*!
	 * Constructs the fixture batches.
	 */
	void SetUp() {
		triangle.emplace_back(0, 0);
		triangle.emplace_back(1000, 0);
		triangle.emplace_back(500, 1000);

		square.emplace_back(0, 0);
		square.emplace_back(1000, 0);
		square.emplace_back(1000, 1000);
		square.emplace_back(0, 1000);

		triangle_and_square.push_back(triangle);
		triangle_and_square.push_back(square);
	}
};

/*!
 * Tests accessing individual vertices in the view.
 */
TEST_F(SimplePolygonBatchViewFixture, AccessorRead) {
	const SimplePolygon triangle_view = triangle_and_square[0];
	for(size_t vertex = 0; vertex < triangle.size(); ++vertex) {
		EXPECT_EQ(triangle_view[vertex], triangle[vertex]);
	}
	const SimplePolygon square_view = triangle_and_square[1];
	for(size_t vertex = 0; vertex < square.size(); ++vertex) {
		EXPECT_EQ(square_view[vertex], square[vertex]);
	}
}

/*!
 * Tests modifying individual vertices in the view.
 */
TEST_F(SimplePolygonBatchViewFixture, AccessorWrite) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view[1].x = 333;
	triangle_view[1].y = 555;
	EXPECT_EQ(triangle_view[1], Point2(333, 555)) << "The second vertex of the triangle was modified.";
}

/*!
 * Tests modifying vertices in the view and seeing if other views are edited by
 * reference.
 */
TEST_F(SimplePolygonBatchViewFixture, AccessorWriteByReference) {
	SimplePolygon first_view = triangle_and_square[0];
	const SimplePolygon second_view = triangle_and_square[0];
	first_view[1].x = 444;
	first_view[1].y = 666;
	EXPECT_EQ(second_view[1], Point2(444, 666)) << "The const SimplePolygon was modified because it's viewing the batch which was modified.";
}

/*!
 * Tests iterating over a view without modifying it.
 */
TEST_F(SimplePolygonBatchViewFixture, IteratorConst) {
	const SimplePolygon triangle = triangle_and_square[0];
	SimplePolygon<>::const_iterator it = triangle.begin();
	EXPECT_EQ(triangle[0], *it) << "The iteration must begin at the first vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle[2], *it) << "After iterating twice, it must represent the third vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle.end(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";

	const SimplePolygon square = triangle_and_square[1];
	it = square.begin();
	EXPECT_EQ(square[0], *it) << "The iteration must begin at the first vertex of the square.";

	it++;
	EXPECT_EQ(square[1], *it) << "After iterating once, it must represent the second vertex of the square.";

	it++;
	EXPECT_EQ(square[2], *it) << "After iterating twice, it must represent the third vertex of the square.";

	it++;
	EXPECT_EQ(square[3], *it) << "After iterating thrice, it must represent the fourth vertex of the square.";

	it++;
	EXPECT_EQ(square.end(), it) << "The square has only four vertices, so after iterating four times it must arrive at the end.";
}

/*!
 * Tests iterating over a view using the const begin version.
 */
TEST_F(SimplePolygonBatchViewFixture, IteratorCBegin) {
	const SimplePolygon triangle = triangle_and_square[0];
	SimplePolygon<>::const_iterator it = triangle.cbegin();
	EXPECT_EQ(triangle[0], *it) << "The iteration must begin at the first vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle[2], *it) << "After iterating twice, it must represent the third vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle.end(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";

	const SimplePolygon square = triangle_and_square[1];
	it = square.cbegin();
	EXPECT_EQ(square[0], *it) << "The iteration must begin at the first vertex of the square.";

	it++;
	EXPECT_EQ(square[1], *it) << "After iterating once, it must represent the second vertex of the square.";

	it++;
	EXPECT_EQ(square[2], *it) << "After iterating twice, it must represent the third vertex of the square.";

	it++;
	EXPECT_EQ(square[3], *it) << "After iterating thrice, it must represent the fourth vertex of the square.";

	it++;
	EXPECT_EQ(square.end(), it) << "The square has only four vertices, so after iterating four times it must arrive at the end.";
}

}