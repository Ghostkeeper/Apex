/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2020 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.
#include <initializer_list> //To create an initialiser_list input for assign() and compare the result with them.

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
 * Tests assigning one repeated vertex to the views on simple polygons in the
 * batch.
 */
TEST_F(SimplePolygonBatchViewFixture, AssignRepeated) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.assign(10, Point2(123, 456));

	ASSERT_EQ(triangle_view.size(), 10) << "There should now be 10 vertices in what used to be a triangle.";
	for(size_t i = 0; i < triangle_view.size(); ++i) {
		EXPECT_EQ(triangle_view[i], Point2(123, 456));
	}

	SimplePolygon square_view = triangle_and_square[1];
	square_view.assign(20, Point2(789, 123));

	ASSERT_EQ(square_view.size(), 20) << "There should now be 20 vertices in what used to be a square.";
	for(size_t i = 0; i < square_view.size(); ++i) {
		EXPECT_EQ(square_view[i], Point2(789, 123));
	}
}

/*!
 * Tests assigning a range of vertices to a view on the batch through beginning
 * and ending iterators.
 */
TEST_F(SimplePolygonBatchViewFixture, AssignIterators) {
	std::vector<Point2> new_poly_first; //To be assigned to the triangle.
	for(size_t i = 0; i < 10; ++i) {
		new_poly_first.emplace_back(i * 10 + i, i * 10 + i);
	}
	std::vector<Point2> new_poly_second; //To be assigned to the square.
	for(size_t i = 0; i < 24; ++i) {
		new_poly_second.emplace_back(i * 10 + i + 1, i * 10 + i + 1);
	}

	//Try assigning to the triangle, and verify whether that went well.
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.assign(new_poly_first.begin(), new_poly_first.end());
	ASSERT_EQ(triangle_view.size(), new_poly_first.size());
	for(size_t i = 0; i < new_poly_first.size(); ++i) {
		EXPECT_EQ(triangle_view[i], new_poly_first[i]);
	}

	//Now try the same for the square.
	SimplePolygon square_view = triangle_and_square[1];
	square_view.assign(new_poly_second.begin(), new_poly_second.end());
	ASSERT_EQ(square_view.size(), new_poly_second.size());
	for(size_t i = 0; i < new_poly_second.size(); ++i) {
		EXPECT_EQ(square_view[i], new_poly_second[i]);
	}
}

/*!
 * Tests replacing the contents of a simple polygon with the contents of an
 * initialiser list.
 */
TEST_F(SimplePolygonBatchViewFixture, AssignInitialiserList) {
	SimplePolygon triangle_view = triangle_and_square[0];
	std::initializer_list<Point2> list = {Point2(11, 22), Point2(33, 44), Point2(55, 66), Point2(77, 88)};
	triangle_view.assign(list);
	ASSERT_EQ(triangle_view.size(), list.size()) << "All of the vertices of the initialiser list must have been taken over.";
	size_t i = 0;
	for(const Point2& vertex : list) {
		EXPECT_EQ(triangle_view[i++], vertex);
	}

	//Repeat for the second polygon in the batch.
	SimplePolygon square_view = triangle_and_square[1];
	std::initializer_list<Point2> list2 = {Point2(12, 21), Point2(34, 43), Point2(56, 65), Point2(78, 87)};
	square_view.assign(list2);
	ASSERT_EQ(square_view.size(), list2.size()) << "All of the vertices of the initialiser list must have been taken over.";
	i = 0;
	for(const Point2& vertex : list2) {
		EXPECT_EQ(square_view[i++], vertex);
	}
}

/*!
 * Tests returning a value using the at() function.
 */
TEST_F(SimplePolygonBatchViewFixture, AtConstInRange) {
	const SimplePolygon triangle_view = triangle_and_square[0];
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view.at(i), triangle[i]);
	}

	const SimplePolygon square_view = triangle_and_square[1];
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view.at(i), square[i]);
	}
}

/*!
 * Tests raising an error if you use at() out side of the view's range.
 */
TEST_F(SimplePolygonBatchViewFixture, AtConstOutOfRange) {
	const SimplePolygon triangle_view = triangle_and_square[0];
	EXPECT_THROW(triangle_view.at(3), std::out_of_range) << "The triangle has 3 vertices, so number 3 is out of range (due to zero-index counting).";
	EXPECT_THROW(triangle_view.at(7), std::out_of_range) << "This is out of the actual bounds of the vertex buffer.";
	EXPECT_THROW(triangle_view.at(-1), std::out_of_range) << "This should underflow to maxint, which is also out of range.";

	const SimplePolygon square_view = triangle_and_square[1];
	EXPECT_THROW(square_view.at(4), std::out_of_range) << "The square has 4 vertices, so number 4 is out of range (due to zero-index counting).";
	EXPECT_THROW(square_view.at(-1), std::out_of_range) << "This should underflow to maxint, which is also out of range.";
}

/*!
 * Tests changing a value via the at() function.
 */
TEST_F(SimplePolygonBatchViewFixture, AtModify) {
	SimplePolygon triangle_view = triangle_and_square[0];
	const SimplePolygon triangle_view2 = triangle_and_square[0];
	triangle_view.at(1).x = 666;
	EXPECT_EQ(triangle_view2[1].x, 666) << "The triangle must be modified by reference through the at() function.";

	SimplePolygon square_view = triangle_and_square[1];
	const SimplePolygon square_view2 = triangle_and_square[1];
	square_view.at(3).y = 42;
	EXPECT_EQ(square_view2[3].y, 42) << "The square must be modified by reference through the at() function.";
}

/*!
 * Test getting the back of a view.
 */
TEST_F(SimplePolygonBatchViewFixture, BackConst) {
	const SimplePolygon triangle_view = triangle_and_square[0];
	EXPECT_EQ(triangle_view.back(), triangle[2]);

	const SimplePolygon square_view = triangle_and_square[1];
	EXPECT_EQ(square_view.back(), square[3]);
}

/*!
 * Tests modifying the back of a view.
 */
TEST_F(SimplePolygonBatchViewFixture, BackModify) {
	const SimplePolygon triangle_view = triangle_and_square[0];
	triangle_and_square[0].back().x = 256;
	EXPECT_EQ(triangle_view[2], Point2(256, triangle[2].y)) << "The X coordinate was modified from the reference returned by back().";

	const SimplePolygon square_view = triangle_and_square[1];
	triangle_and_square[1].back().y = 512;
	EXPECT_EQ(square_view[3], Point2(square[3].x, 512)) << "The Y coordinate was modified from the reference returned by back().";
}

/*!
 * Tests getting the capacity of a view on a batch where nothing is in the batch
 * yet.
 */
TEST(SimplePolygonBatchView, CapacityEmpty) {
	SimplePolygonBatch batch(3, 0);
	for(size_t i = 0; i < batch.size(); ++i) {
		EXPECT_EQ(batch[0].capacity(), 0) << "No memory has been reserved yet for the individual simple polygon.";
	}
}

/*!
 * Tests getting the capacity of a view on a batch when the batch is filled to
 * capacity and beyond.
 */
TEST(SimplePolygonBatchView, CapacityFull) {
	SimplePolygonBatch batch(3, 3);
	batch[1].emplace_back(20, 30);
	EXPECT_GE(batch[1].capacity(), 1) << "There needs to be capacity for at least one vertex now.";
	batch[0].emplace_back(0, 0);
	batch[0].emplace_back(0, 0);
	EXPECT_GE(batch[0].capacity(), 2) << "There needs to be capacity for at least two vertices now.";
	for(size_t i = 0; i < 100; ++i) {
		batch[2].emplace_back(0, 0);
	}
	EXPECT_GE(batch[2].capacity(), 100) << "There needs to be capacity for at least a hundred vertices now.";
}

/*!
 * Tests clearing the data of views in the batch.
 */
TEST_F(SimplePolygonBatchViewFixture, Clear) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.clear();
	EXPECT_EQ(triangle_view.size(), 0) << "After clearing, the size must be 0.";

	SimplePolygon square_view = triangle_and_square[1];
	square_view.clear();
	EXPECT_EQ(square_view.size(), 0) << "After clearing, the size must be 0.";
}

/*!
 * Tests getting the data pointer of a view in a batch.
 */
TEST_F(SimplePolygonBatchViewFixture, DataConst) {
	const SimplePolygon triangle_view = triangle_and_square[0];
	const Point2* triangle_data = triangle_view.data();
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(*(triangle_data + i), triangle[i]) << "The pointer should point at the beginning of the entire range of the triangle.";
	}

	const SimplePolygon square_view = triangle_and_square[1];
	const Point2* square_data = square_view.data();
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(*(square_data + i), square[i]) << "The pointer should point at the beginning of the entire range of the square.";
	}
}

/*!
 * Tests getting the data pointer to an empty view in an empty batch.
 *
 * This is tested because it's an exceptional case since there is no starting
 * vertex then, not even for the entire buffer.
 */
TEST(SimplePolygonBatchView, DataEmpty) {
	const SimplePolygonBatch batch(3, 0);
	const SimplePolygon view0 = batch[0];
	EXPECT_NO_THROW(view0.data());
	const SimplePolygon view1 = batch[1];
	EXPECT_NO_THROW(view1.data());
	const SimplePolygon view2 = batch[2];
	EXPECT_NO_THROW(view2.data());
}

/*!
 * Tests modifying a batch view through the data() pointer.
 */
TEST_F(SimplePolygonBatchViewFixture, DataModify) {
	triangle_and_square[0].data()->x = 654;
	EXPECT_EQ(triangle_and_square[0][0].x, 654) << "The X coordinate of this vertex was modified through the data() pointer.";

	(triangle_and_square[1].data() + 1)->y = 321;
	EXPECT_EQ(triangle_and_square[1][1].y, 321) << "The Y coordinate of this vertex was modified through the data() pointer.";
}

/*!
 * Place a new vertex at the start of the simple polygon.
 */
TEST_F(SimplePolygonBatchViewFixture, EmplaceStart) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.emplace(triangle_view.begin(), 20, 30);
	EXPECT_EQ(triangle_view[0], Point2(20, 30)) << "The new vertex must be the first vertex.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i + 1], triangle[i]) << "The original vertices must be shifted.";
	}

	SimplePolygon square_view = triangle_and_square[1];
	square_view.emplace(square_view.begin(), 40, 50);
	EXPECT_EQ(square_view[0], Point2(40, 50)) << "The new vertex must be the first vertex.";
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i + 1], square[i]) << "The original vertices must be shifted.";
	}
}

/*!
 * Place a new vertex in the middle of the simple polygon.
 */
TEST_F(SimplePolygonBatchViewFixture, EmplaceMiddle) {
	SimplePolygon triangle_view = triangle_and_square[0];
	SimplePolygon<>::iterator middle = triangle_view.begin() + 1;
	triangle_view.emplace(middle, 20, 30);
	EXPECT_EQ(triangle_view[0], triangle[0]) << "The first vertex must be unchanged.";
	EXPECT_EQ(triangle_view[1], Point2(20, 30)) << "The new vertex must be in the second place.";
	EXPECT_EQ(triangle_view[2], triangle[1]) << "The second vertex must be shifted.";
	EXPECT_EQ(triangle_view[3], triangle[2]) << "The third vertex must be shifted.";

	SimplePolygon square_view = triangle_and_square[1];
	middle = square_view.begin() + 2;
	square_view.emplace(middle, 40, 50);
	EXPECT_EQ(square_view[0], square[0]) << "The first vertex must be unchanged.";
	EXPECT_EQ(square_view[1], square[1]) << "The second vertex must be unchanged.";
	EXPECT_EQ(square_view[2], Point2(40, 50)) << "The new vertex must be in the third place.";
	EXPECT_EQ(square_view[3], square[2]) << "The third vertex must be shifted.";
	EXPECT_EQ(square_view[4], square[3]) << "The fourth vertex must be shifted.";
}

/*!
 * Place a new vertex at the end of the simple polygon.
 */
TEST_F(SimplePolygonBatchViewFixture, EmplaceEnd) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.emplace(triangle_view.end(), 20, 30);
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "All of the original vertices are still in their places.";
	}
	EXPECT_EQ(triangle_view[3], Point2(20, 30)) << "The new vertex must be at the end.";

	SimplePolygon square_view = triangle_and_square[1];
	square_view.emplace(square_view.end(), 40, 50);
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "All of the original vertices are still in their places.";
	}
	EXPECT_EQ(square_view[4], Point2(40, 50)) << "The new vertex must be at the end.";
}

/*!
 * Tests emplacing a new vertex at the end using ``emplace_back()``.
 */
TEST_F(SimplePolygonBatchViewFixture, EmplaceBack) {
	//This test is explicitly performed in the other order compared to EmplaceEnd, to hit a bit different cases for what would otherwise be practically the same test.
	SimplePolygon square_view = triangle_and_square[1];
	square_view.emplace_back(40, 50);
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "All of the original vertices are still in their places.";
	}
	EXPECT_EQ(square_view[4], Point2(40, 50)) << "The new vertex must be at the end.";

	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.emplace_back(20, 30);
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "All of the original vertices are still in their places.";
	}
	EXPECT_EQ(triangle_view[3], Point2(20, 30)) << "The new vertex must be at the end.";
}

/*!
 * Tests whether empty views are indeed empty according to ``empty()``, and
 * filled views are not.
 */
TEST(SimplePolygonBatchView, Empty) {
	SimplePolygonBatch batch(3, 0);
	EXPECT_TRUE(batch[0].empty()) << "The first batch was initialised empty.";
	EXPECT_TRUE(batch[1].empty()) << "The second batch was initialised empty.";
	EXPECT_TRUE(batch[2].empty()) << "The third batch was initialised empty.";

	batch[1].emplace_back(40, 50);
	EXPECT_TRUE(batch[0].empty()) << "The first batch was not modified.";
	EXPECT_FALSE(batch[1].empty()) << "The second batch is no longer empty because a vertex was added.";
	EXPECT_TRUE(batch[2].empty()) << "The third batch was not modified.";
}

/*!
 * Tests erasing a vertex from the start of the view.
 */
TEST_F(SimplePolygonBatchViewFixture, EraseStart) {
	SimplePolygon triangle_view = triangle_and_square[0];
	SimplePolygon<>::iterator result = triangle_view.erase(triangle_view.begin());

	ASSERT_EQ(triangle_view.size(), triangle.size() - 1) << "The size must be reduced by 1.";
	for(size_t i = 0; i < triangle_view.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i + 1]) << "All of the vertices must have been shifted by 1.";
	}
	EXPECT_EQ(*result, triangle_view[0]) << "Return the vertex after the removed one.";

	SimplePolygon square_view = triangle_and_square[1];
	result = square_view.erase(square_view.begin());

	ASSERT_EQ(square_view.size(), square.size() - 1) << "The size must be reduced by 1.";
	for(size_t i = 0; i < square_view.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i + 1]) << "All of the vertices must have been shifted by 1.";
	}
	EXPECT_EQ(*result, square_view[0]) << "Return the vertex after the removed one.";
}

/*!
 * Tests erasing a vertex from the middle of the view.
 */
TEST_F(SimplePolygonBatchViewFixture, EraseMiddle) {
	SimplePolygon triangle_view = triangle_and_square[0];
	const SimplePolygon<>::const_iterator second_vertex = triangle_view.begin() + 1;
	SimplePolygon<>::iterator result = triangle_view.erase(second_vertex);

	ASSERT_EQ(triangle_view.size(), triangle.size() - 1) << "The size must be reduced by 1.";
	EXPECT_EQ(triangle_view[0], triangle[0]) << "The first vertex must still be in its place.";
	for(size_t i = 1; i < triangle_view.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i + 1]) << "The rest of the vertices must have been shifted by 1.";
	}
	EXPECT_EQ(*result, triangle_view[1]) << "Return the vertex after the removed one.";

	SimplePolygon square_view = triangle_and_square[1];
	const SimplePolygon<>::const_iterator third_vertex = square_view.begin() + 2;
	result = square_view.erase(third_vertex);

	ASSERT_EQ(square_view.size(), square.size() - 1) << "The size must be reduced by 1.";
	for(size_t i = 2; i < square_view.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i + 1]) << "The rest of the vertices must have been shifted by 1.";
	}
	EXPECT_EQ(*result, square_view[2]) << "Return the vertex after the removed one.";
}

/*!
 * Tests removing a range of vertices from the beginning of the view.
 */
TEST_F(SimplePolygonBatchViewFixture, EraseRangeStart) {
	SimplePolygon triangle_view = triangle_and_square[0];
	const SimplePolygon<>::const_iterator third_vertex = triangle_view.begin() + 2;
	SimplePolygon<>::iterator result = triangle_view.erase(triangle_view.begin(), third_vertex); //Erase the first and second vertices.

	ASSERT_EQ(triangle_view.size(), triangle.size() - 2) << "That must have erased two vertices.";
	for(size_t i = 0; i < triangle_view.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i + 2]) << "The rest of the vertices must have been shifted by 2.";
	}
	EXPECT_EQ(*result, triangle_view[0]) << "Return the vertex after the last removed one.";

	SimplePolygon square_view = triangle_and_square[1];
	const SimplePolygon<>::const_iterator fourth_vertex = square_view.begin() + 3;
	result = square_view.erase(square_view.begin(), fourth_vertex); //Erase the first 3 vertices.

	ASSERT_EQ(square_view.size(), square.size() - 3) << "That must have erased three vertices.";
	for(size_t i = 0; i < square_view.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i + 3]) << "The rest of the vertices must have been shifted by 3.";
	}
	EXPECT_EQ(*result, square_view[0]) << "Return the vertex after the last removed one.";
}

/*!
 * Tests removing a range of vertices from the middle and end of a view.
 */
TEST_F(SimplePolygonBatchViewFixture, EraseRangeMiddleEnd) {
	SimplePolygon triangle_view = triangle_and_square[0];
	SimplePolygon<>::const_iterator second_vertex = triangle_view.begin() + 1;
	SimplePolygon<>::iterator result = triangle_view.erase(second_vertex, triangle_view.end()); //Erase the second and third vertices.

	ASSERT_EQ(triangle_view.size(), triangle.size() - 2) << "That must have erased two vertices.";
	EXPECT_EQ(triangle_view[0], triangle[0]) << "The first vertex must still be left intact.";
	//The rest is removed.
	EXPECT_EQ(result, triangle_view.end()) << "Return the vertex after the last removed one.";

	SimplePolygon square_view = triangle_and_square[1];
	second_vertex = square_view.begin() + 1;
	SimplePolygon<>::const_iterator fourth_vertex = square_view.begin() + 3;
	result = square_view.erase(second_vertex, fourth_vertex); //Erase the second and third vertices.
	ASSERT_EQ(square_view.size(), square.size() - 2) << "That must have erased two vertices.";
	EXPECT_EQ(square_view[0], square[0]) << "The first vertex must still be left intact.";
	for(size_t i = 1; i < square_view.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i + 2]) << "The last vertices must be shifted by 2.";
	}
	EXPECT_EQ(*result, square_view[1]) << "Return the vertex after the last removed one.";
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
	EXPECT_EQ(triangle.cend(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";

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
	EXPECT_EQ(square.cend(), it) << "The square has only four vertices, so after iterating four times it must arrive at the end.";
}

/*!
 * Tests modifying the polygon by modifying the data in the iterator.
 */
TEST_F(SimplePolygonBatchViewFixture, IteratorModification) {
	SimplePolygon triangle = triangle_and_square[0];
	SimplePolygon<>::iterator it = triangle.begin();
	EXPECT_EQ(triangle[0], *it) << "The iteration must begin at the first vertex of the triangle.";
	it->x = 42;
	EXPECT_EQ(triangle[0].x, 42) << "After the iterator has been changed by reference, the data must be stored in the triangle too.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex of the triangle.";
	it->y = 69;
	EXPECT_EQ(triangle[1].y, 69) << "After the iterator has been changed by reference, the data must be stored in the triangle too.";

	it++;
	EXPECT_EQ(triangle[2], *it) << "After iterating twice, it must represent the third vertex of the triangle.";
	it->x = 666;
	EXPECT_EQ(triangle[2].x, 666) << "After the iterator has been changed by reference, the data must be stored in the triangle too.";

	SimplePolygon square = triangle_and_square[1];
	it = square.begin();
	EXPECT_EQ(square[0], *it) << "The iteration must begin at the first vertex of the square.";
	it->x = 42;
	EXPECT_EQ(square[0].x, 42) << "After the iterator has been changed by reference, the data must be stored in the square too.";

	it++;
	EXPECT_EQ(square[1], *it) << "After iterating once, it must represent the second vertex of the square.";
	it->y = 69;
	EXPECT_EQ(square[1].y, 69) << "After the iterator has been changed by reference, the data must be stored in the square too.";

	it++;
	EXPECT_EQ(square[2], *it) << "After iterating twice, it must represent the third vertex of the square.";
	it->x = 666;
	EXPECT_EQ(square[2].x, 666) << "After the iterator has been changed by reference, the data must be stored in the square too.";

	it++;
	EXPECT_EQ(square[3], *it) << "After iterating thrice, it must represent the fourth vertex of the square.";
	it->x = 1337;
	EXPECT_EQ(square[3].x, 1337) << "After the iterator has been changed by reference, the data must be stored in the square too.";
}

/*!
 * Tests iterating over a view in reverse without modifying it.
 */
TEST_F(SimplePolygonBatchViewFixture, IteratorReverseConst) {
	const SimplePolygon triangle = triangle_and_square[0];
	SimplePolygon<>::const_reverse_iterator it = triangle.rbegin();
	EXPECT_EQ(triangle[2], *it) << "The iteration must begin at the last vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle[0], *it) << "After iterating twice, it must represent the first vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle.rend(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the (reverse) end.";

	const SimplePolygon square = triangle_and_square[1];
	it = square.rbegin();
	EXPECT_EQ(square[3], *it) << "The iteration must begin at the last vertex of the square.";

	it++;
	EXPECT_EQ(square[2], *it) << "After iterating once, it must represent the third vertex of the square.";

	it++;
	EXPECT_EQ(square[1], *it) << "After iterating twice, it must represent the second vertex of the square.";

	it++;
	EXPECT_EQ(square[0], *it) << "After iterating thrice, it must represent the first vertex of the square.";

	it++;
	EXPECT_EQ(square.rend(), it) << "The square has only four vertices, so after iterating four times it must arrive at the (reverse) end.";
}

/*!
 * Tests modifying the polygon by modifying the data in the reverse iterator.
 */
TEST_F(SimplePolygonBatchViewFixture, IteratorReverseModification) {
	SimplePolygon triangle = triangle_and_square[0];
	SimplePolygon<>::reverse_iterator it = triangle.rbegin();
	EXPECT_EQ(triangle[2], *it) << "The iteration must begin at the last vertex of the triangle.";
	it->x = 42;
	EXPECT_EQ(triangle[2].x, 42) << "After the iterator has been changed by reference, the data must be stored in the triangle too.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex of the triangle.";
	it->y = 69;
	EXPECT_EQ(triangle[1].y, 69) << "After the iterator has been changed by reference, the data must be stored in the triangle too.";

	it++;
	EXPECT_EQ(triangle[0], *it) << "After iterating twice, it must represent the first vertex of the triangle.";
	it->x = 666;
	EXPECT_EQ(triangle[0].x, 666) << "After the iterator has been changed by reference, the data must be stored in the triangle too.";

	SimplePolygon square = triangle_and_square[1];
	it = square.rbegin();
	EXPECT_EQ(square[3], *it) << "The iteration must begin at the last vertex of the square.";
	it->x = 42;
	EXPECT_EQ(square[3].x, 42) << "After the iterator has been changed by reference, the data must be stored in the square too.";

	it++;
	EXPECT_EQ(square[2], *it) << "After iterating once, it must represent the third vertex of the square.";
	it->y = 69;
	EXPECT_EQ(square[2].y, 69) << "After the iterator has been changed by reference, the data must be stored in the square too.";

	it++;
	EXPECT_EQ(square[1], *it) << "After iterating twice, it must represent the second vertex of the square.";
	it->x = 666;
	EXPECT_EQ(square[1].x, 666) << "After the iterator has been changed by reference, the data must be stored in the square too.";

	it++;
	EXPECT_EQ(square[0], *it) << "After iterating thrice, it must represent the first vertex of the square.";
	it->x = 1337;
	EXPECT_EQ(square[0].x, 1337) << "After the iterator has been changed by reference, the data must be stored in the square too.";
}

/*!
 * Tests iterating over a view in reverse without modifying it with the explicit
 * const iterator.
 */
TEST_F(SimplePolygonBatchViewFixture, IteratorReverseCBegin) {
	const SimplePolygon triangle = triangle_and_square[0];
	SimplePolygon<>::const_reverse_iterator it = triangle.crbegin();
	EXPECT_EQ(triangle[2], *it) << "The iteration must begin at the last vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle[0], *it) << "After iterating twice, it must represent the first vertex of the triangle.";

	it++;
	EXPECT_EQ(triangle.crend(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the (reverse) end.";

	const SimplePolygon square = triangle_and_square[1];
	it = square.crbegin();
	EXPECT_EQ(square[3], *it) << "The iteration must begin at the last vertex of the square.";

	it++;
	EXPECT_EQ(square[2], *it) << "After iterating once, it must represent the third vertex of the square.";

	it++;
	EXPECT_EQ(square[1], *it) << "After iterating twice, it must represent the second vertex of the square.";

	it++;
	EXPECT_EQ(square[0], *it) << "After iterating thrice, it must represent the first vertex of the square.";

	it++;
	EXPECT_EQ(square.crend(), it) << "The square has only four vertices, so after iterating four times it must arrive at the (reverse) end.";
}

/*!
 * Test getting the front of a view.
 */
TEST_F(SimplePolygonBatchViewFixture, FrontConst) {
	const SimplePolygon triangle_view = triangle_and_square[0];
	EXPECT_EQ(triangle_view.front(), triangle[0]);

	const SimplePolygon square_view = triangle_and_square[1];
	EXPECT_EQ(square_view.front(), square[0]);
}

/*!
 * Tests modifying the front of a view.
 */
TEST_F(SimplePolygonBatchViewFixture, FrontModify) {
	const SimplePolygon triangle_view = triangle_and_square[0];
	triangle_and_square[0].front().x = 256;
	EXPECT_EQ(triangle_view[0], Point2(256, triangle[0].y)) << "The X coordinate was modified from the reference returned by front().";

	const SimplePolygon square_view = triangle_and_square[1];
	triangle_and_square[1].front().y = 512;
	EXPECT_EQ(square_view[0], Point2(square[0].x, 512)) << "The Y coordinate was modified from the reference returned by front().";
}

/*!
 * Tests inserting a vertex at the front of the view by copying it in.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertCopyFront) {
	SimplePolygon triangle_view = triangle_and_square[0];
	SimplePolygon<>::const_iterator result = triangle_view.insert(triangle_view.begin(), Point2(42, 1337));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 1) << "The number of vertices has risen by 1.";
	EXPECT_EQ(triangle_view[0], Point2(42, 1337)) << "The new vertex is now the first vertex.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i + 1], triangle[i]) << "The rest of the vertices have shifted by 1.";
	}
	EXPECT_EQ(*result, Point2(42, 1337)) << "The resulting iterator must point to the new vertex.";

	SimplePolygon square_view = triangle_and_square[1];
	result = square_view.insert(square_view.begin(), Point2(66, 777));
	ASSERT_EQ(square_view.size(), square.size() + 1) << "The number of vertices has risen by 1.";
	EXPECT_EQ(square_view[0], Point2(66, 777)) << "The new vertex is now the first vertex.";
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i + 1], square[i]) << "The rest of the vertices have shifted by 1.";
	}
	EXPECT_EQ(*result, Point2(66, 777)) << "The resulting iterator must point to the new vertex.";
}

/*!
 * Tests inserting a vertex in the middle of the view by copying it in.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertCopyMiddle) {
	SimplePolygon triangle_view = triangle_and_square[0];
	const SimplePolygon<>::const_iterator second_vertex = triangle_view.begin() + 1;
	SimplePolygon<>::const_iterator result = triangle_view.insert(second_vertex, Point2(53, 23));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 1) << "The number of vertices has risen by 1.";
	EXPECT_EQ(triangle_view[0], triangle[0]) << "The first vertex is not moved.";
	EXPECT_EQ(triangle_view[1], Point2(53, 23)) << "The new vertex is in the second place.";
	EXPECT_EQ(triangle_view[2], triangle[1]) << "The second vertex has shifted by 1.";
	EXPECT_EQ(triangle_view[3], triangle[2]) << "The third vertex has shifted by 1.";
	EXPECT_EQ(*result, Point2(53, 23)) << "The resulting iterator must point to the new vertex.";

	SimplePolygon square_view = triangle_and_square[1];
	const SimplePolygon<>::const_iterator third_vertex = square_view.begin() + 2;
	result = square_view.insert(third_vertex, Point2(13, 37));
	ASSERT_EQ(square_view.size(), square.size() + 1) << "The number of vertices has risen by 1.";
	EXPECT_EQ(square_view[0], square[0]) << "The first vertex is not moved.";
	EXPECT_EQ(square_view[1], square[1]) << "The second vertex is not moved.";
	EXPECT_EQ(square_view[2], Point2(13, 37)) << "The new vertex is in the third place.";
	EXPECT_EQ(square_view[3], square[2]) << "The third vertex has shifted by 1.";
	EXPECT_EQ(square_view[4], square[3]) << "The fourth vertex has shifted by 1.";
	EXPECT_EQ(*result, Point2(13, 37)) << "The resulting iterator must point to the new vertex.";
}

/*!
 * Tests inserting a vertex at the end of the view by copying it in.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertCopyEnd) {
	SimplePolygon triangle_view = triangle_and_square[0];
	SimplePolygon<>::const_iterator result = triangle_view.insert(triangle_view.end(), Point2(666, 555));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 1) << "The number of vertices has risen by 1.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "All of the original vertices are still in their original places.";
	}
	EXPECT_EQ(triangle_view[3], Point2(666, 555)) << "The new vertex is now the last vertex.";
	EXPECT_EQ(*result, Point2(666, 555)) << "The resulting iterator must point to the new vertex.";

	SimplePolygon square_view = triangle_and_square[1];
	result = square_view.insert(square_view.end(), Point2(444, 333));
	ASSERT_EQ(square_view.size(), square.size() + 1) << "The number of vertices has risen by 1.";
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "All of the original vertices are still in their original places.";
	}
	EXPECT_EQ(square_view[4], Point2(444, 333)) << "The new vertex is now the last vertex.";
	EXPECT_EQ(*result, Point2(444, 333)) << "The resulting iterator must point to the new vertex.";
}

/*!
 * Tests inserting a vertex at the front of the view by moving it in.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertMoveFront) {
	SimplePolygon triangle_view = triangle_and_square[0];
	Point2 vertex(42, 1337);
	SimplePolygon<>::const_iterator result = triangle_view.insert(triangle_view.begin(), std::move(vertex));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 1) << "The number of vertices has risen by 1.";
	EXPECT_EQ(triangle_view[0], Point2(42, 1337)) << "The new vertex is now the first vertex.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i + 1], triangle[i]) << "The rest of the vertices have shifted by 1.";
	}
	EXPECT_EQ(*result, Point2(42, 1337)) << "The resulting iterator must point to the new vertex.";

	SimplePolygon square_view = triangle_and_square[1];
	Point2 vertex2(66, 777); //Use a new one since the old one has been moved and shouldn't be used again, though in theory overwriting it is fine.
	result = square_view.insert(square_view.begin(), std::move(vertex2));
	ASSERT_EQ(square_view.size(), square.size() + 1) << "The number of vertices has risen by 1.";
	EXPECT_EQ(square_view[0], Point2(66, 777)) << "The new vertex is now the first vertex.";
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i + 1], square[i]) << "The rest of the vertices have shifted by 1.";
	}
	EXPECT_EQ(*result, Point2(66, 777)) << "The resulting iterator must point to the new vertex.";
}

/*!
 * Tests inserting a vertex in the middle of the view by copying it in.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertMoveMiddle) {
	SimplePolygon triangle_view = triangle_and_square[0];
	const SimplePolygon<>::const_iterator second_vertex = triangle_view.begin() + 1;
	Point2 vertex(53, 23);
	SimplePolygon<>::const_iterator result = triangle_view.insert(second_vertex, std::move(vertex));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 1) << "The number of vertices has risen by 1.";
	EXPECT_EQ(triangle_view[0], triangle[0]) << "The first vertex is not moved.";
	EXPECT_EQ(triangle_view[1], Point2(53, 23)) << "The new vertex is in the second place.";
	EXPECT_EQ(triangle_view[2], triangle[1]) << "The second vertex has shifted by 1.";
	EXPECT_EQ(triangle_view[3], triangle[2]) << "The third vertex has shifted by 1.";
	EXPECT_EQ(*result, Point2(53, 23)) << "The resulting iterator must point to the new vertex.";

	SimplePolygon square_view = triangle_and_square[1];
	const SimplePolygon<>::const_iterator third_vertex = square_view.begin() + 2;
	Point2 vertex2(13, 37);
	result = square_view.insert(third_vertex, std::move(vertex2));
	ASSERT_EQ(square_view.size(), square.size() + 1) << "The number of vertices has risen by 1.";
	EXPECT_EQ(square_view[0], square[0]) << "The first vertex is not moved.";
	EXPECT_EQ(square_view[1], square[1]) << "The second vertex is not moved.";
	EXPECT_EQ(square_view[2], Point2(13, 37)) << "The new vertex is in the third place.";
	EXPECT_EQ(square_view[3], square[2]) << "The third vertex has shifted by 1.";
	EXPECT_EQ(square_view[4], square[3]) << "The fourth vertex has shifted by 1.";
	EXPECT_EQ(*result, Point2(13, 37)) << "The resulting iterator must point to the new vertex.";
}

/*!
 * Tests inserting a vertex at the end of the view by copying it in.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertMoveEnd) {
	SimplePolygon triangle_view = triangle_and_square[0];
	Point2 vertex(666, 555);
	SimplePolygon<>::const_iterator result = triangle_view.insert(triangle_view.end(), std::move(vertex));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 1) << "The number of vertices has risen by 1.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "All of the original vertices are still in their original places.";
	}
	EXPECT_EQ(triangle_view[3], Point2(666, 555)) << "The new vertex is now the last vertex.";
	EXPECT_EQ(*result, Point2(666, 555)) << "The resulting iterator must point to the new vertex.";

	SimplePolygon square_view = triangle_and_square[1];
	Point2 vertex2(444, 333);
	result = square_view.insert(square_view.end(), std::move(vertex2));
	ASSERT_EQ(square_view.size(), square.size() + 1) << "The number of vertices has risen by 1.";
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "All of the original vertices are still in their original places.";
	}
	EXPECT_EQ(square_view[4], Point2(444, 333)) << "The new vertex is now the last vertex.";
	EXPECT_EQ(*result, Point2(444, 333)) << "The resulting iterator must point to the new vertex.";
}

/*!
 * Tests inserting multiple copies of a vertex at a time at the front of the
 * simple polygon.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertMultipleFront) {
	SimplePolygon triangle_view = triangle_and_square[0];
	SimplePolygon<>::const_iterator result = triangle_view.insert(triangle_view.begin(), 42, Point2(13, 37));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 42) << "The number of vertices has risen by 42.";
	for(size_t i = 0; i < 42; ++i) {
		EXPECT_EQ(triangle_view[i], Point2(13, 37)) << "The first 42 vertices must all be equal to the inserted vertex.";
	}
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i + 42], triangle[i]) << "All of the original vertices must have shifted by 42 to make space.";
	}
	EXPECT_EQ(result, triangle_view.begin()) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";

	SimplePolygon square_view = triangle_and_square[1];
	result = square_view.insert(square_view.begin(), 69, Point2(313, 37));
	ASSERT_EQ(square_view.size(), square.size() + 69) << "The number of vertices has risen by 69.";
	for(size_t i = 0; i < 69; ++i) {
		EXPECT_EQ(square_view[i], Point2(313, 37)) << "The first 69 vertices must all be equal to the inserted vertex.";
	}
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i + 69], square[i]) << "All of the original vertices must have shifted by 69 to make space.";
	}
	EXPECT_EQ(result, square_view.begin()) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";
}

/*!
 * Tests inserting multiple copies of a vertex at a time in the middle of the
 * simple polygon.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertMultipleMiddle) {
	SimplePolygon triangle_view = triangle_and_square[0];
	const SimplePolygon<>::const_iterator second_vertex = triangle_view.begin() + 1;
	SimplePolygon<>::const_iterator result = triangle_view.insert(second_vertex, 42, Point2(37, 13));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 42) << "The number of vertices has risen by 42.";
	EXPECT_EQ(triangle_view[0], triangle[0]) << "The new vertices were inserted between the first and second vertices, so the first one must still be there.";
	for(size_t i = 0; i < 42; ++i) {
		EXPECT_EQ(triangle_view[i + 1], Point2(37, 13)) << "The next 42 vertices must all be copies of the one inserted.";
	}
	for(size_t i = 1; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i + 42], triangle[i]) << "The rest of the vertices must have shifted by 42 places.";
	}
	EXPECT_EQ(*result, triangle_view[1]) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";

	SimplePolygon square_view = triangle_and_square[1];
	const SimplePolygon<>::const_iterator third_vertex = square_view.begin() + 2;
	result = square_view.insert(third_vertex, 42, Point2(37, 13));
	ASSERT_EQ(square_view.size(), square.size() + 42) << "The number of vertices has risen by 42.";
	EXPECT_EQ(square_view[0], square[0]) << "The new vertices were inserted between the second and third vertices, so the first one must still be there.";
	EXPECT_EQ(square_view[1], square[1]) << "The new vertices were inserted between the second and third vertices, so the second one must still be there.";
	for(size_t i = 0; i < 42; ++i) {
		EXPECT_EQ(square_view[i + 2], Point2(37, 13)) << "The next 42 vertices must all be copies of the one inserted.";
	}
	for(size_t i = 2; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i + 42], square[i]) << "The rest of the vertices must have shifted by 42 places.";
	}
	EXPECT_EQ(*result, square_view[2]) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";
}

/*!
 * Tests inserting multiple copies of a vertex at a time at the end of the
 * simple polygon.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertMultipleEnd) {
	SimplePolygon triangle_view = triangle_and_square[0];
	SimplePolygon<>::const_iterator result = triangle_view.insert(triangle_view.end(), 42, Point2(13, 37));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 42) << "The number of vertices has risen by 42.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "All of the original vertices are still in their original places.";
	}
	for(size_t i = 0; i < 42; ++i) {
		EXPECT_EQ(triangle_view[i + triangle.size()], Point2(13, 37)) << "The new vertices are now at the end.";
	}
	EXPECT_EQ(*result, triangle_view[triangle.size()]) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";

	SimplePolygon square_view = triangle_and_square[1];
	result = square_view.insert(square_view.end(), 69, Point2(313, 37));
	ASSERT_EQ(square_view.size(), square.size() + 69) << "The number of vertices has risen by 69.";
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "All of the original vertices are still in their original places.";
	}
	for(size_t i = 0; i < 69; ++i) {
		EXPECT_EQ(square_view[i + square.size()], Point2(313, 37)) << "The new vertices are now at the end.";
	}
	EXPECT_EQ(*result, square_view[square.size()]) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";
}

/*!
 * Iterator type to wrap ``vector``'s iterator to restrict its usage more for
 * the purpose of a test.
 *
 * The insertion function needs to work for strictly forward iterators as well.
 */
struct MyForwardIterator : public std::vector<Point2>::iterator {
	using iterator_category = std::forward_iterator_tag;
};

/*!
 * Iterator type to wrap ``vector``'s iterator to restrict its usage more for
 * the purpose of a test.
 *
 * The insertion function needs to work for input iterators as well.
 */
struct MyInputIterator : public std::vector<Point2>::iterator {
	using iterator_category = std::input_iterator_tag;
};

/*!
 * To type-parametrise the insert-by-iterators tests, make a copy of the fixture
 * that we'll type-parametrise.
 * \tparam T The type that this test suite is parametrised with.
 */
template<typename T>
class InsertIteratorsParametrised : public SimplePolygonBatchViewFixture {
public:
	/*!
	 * The range that is being inserted.
	 */
	std::vector<Point2> inserted_range;

	/*!
	 * The start of the range to insert.
	 */
	T* range_start;

	/*!
	 * The end of the inserted range.
	 */
	T* range_end;

	/*!
	 * The start of the range to insert, in its original type.
	 */
	std::vector<Point2>::iterator vector_begin;

	/*!
	 * The end of the range to insert, in its original type.
	 */
	std::vector<Point2>::iterator vector_end;

	/*!
	 * Sets up the fixtures of re-used objects in these tests.
	 */
	void SetUp() {
		SimplePolygonBatchViewFixture::SetUp();

		for(size_t i = 0; i < 42; ++i) {
			inserted_range.emplace_back(69 + i, 96 + i);
		}
		vector_begin = inserted_range.begin();
		vector_end = inserted_range.end();
		range_start = static_cast<T*>(&vector_begin);
		range_end = static_cast<T*>(&vector_end);
	}
};
using MyIteratorTypes = testing::Types<std::vector<Point2>::iterator, MyForwardIterator, MyInputIterator>;
TYPED_TEST_SUITE(InsertIteratorsParametrised, MyIteratorTypes);

/*!
 * Tests inserting a range between iterators at the beginning of a simple
 * polygon.
 */
TYPED_TEST(InsertIteratorsParametrised, InsertIteratorsFront) {
	SimplePolygon triangle_view = this->triangle_and_square[0];
	SimplePolygon<>::const_iterator result = triangle_view.insert(triangle_view.begin(), *this->range_start, *this->range_end);
	ASSERT_EQ(triangle_view.size(), this->triangle.size() + this->inserted_range.size()) << "The number of vertices has risen by the contents of the inserted range.";
	for(size_t i = 0; i < this->inserted_range.size(); ++i) {
		EXPECT_EQ(triangle_view[i], this->inserted_range[i]) << "The inserted range is now at the beginning of the triangle.";
	}
	for(size_t i = 0; i < this->triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[this->inserted_range.size() + i], this->triangle[i]) << "The original triangle vertices are now shifted to the end.";
	}
	EXPECT_EQ(*result, triangle_view[0]) << "The returned iterator must point to the beginning of the inserted range.";

	SimplePolygon square_view = this->triangle_and_square[1];
	result = square_view.insert(square_view.begin(), *this->range_start, *this->range_end);
	ASSERT_EQ(square_view.size(), this->square.size() + this->inserted_range.size()) << "The number of vertices has risen by the contents of the inserted range.";
	for(size_t i = 0; i < this->inserted_range.size(); ++i) {
		EXPECT_EQ(square_view[i], this->inserted_range[i]) << "The inserted range is now at the beginning of the square.";
	}
	for(size_t i = 0; i < this->square.size(); ++i) {
		EXPECT_EQ(square_view[this->inserted_range.size() + i], this->square[i]) << "The original square vertices are now shifted to the end.";
	}
	EXPECT_EQ(*result, square_view[0]) << "The returned iterator must point to the beginning of the inserted range.";
}

/*!
 * Tests inserting a range between iterators in the middle of a simple polygon.
 */
TYPED_TEST(InsertIteratorsParametrised, InsertIteratorsMiddle) {
	SimplePolygon triangle_view = this->triangle_and_square[0];
	const SimplePolygon<>::const_iterator second_vertex = triangle_view.begin() + 1;
	SimplePolygon<>::const_iterator result = triangle_view.insert(second_vertex, *this->range_start, *this->range_end);
	ASSERT_EQ(triangle_view.size(), this->triangle.size() + this->inserted_range.size()) << "The number of vertices has risen by the contents of the inserted range.";
	EXPECT_EQ(triangle_view[0], this->triangle[0]) << "The first vertex is still in its original position.";
	for(size_t i = 0; i < this->inserted_range.size(); ++i) {
		EXPECT_EQ(triangle_view[i + 1], this->inserted_range[i]) << "The inserted range is now in the middle of the triangle.";
	}
	for(size_t i = 1; i < this->triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[this->inserted_range.size() + i], this->triangle[i]) << "The rest of the triangle vertices are now shifted to the end.";
	}
	EXPECT_EQ(*result, triangle_view[1]) << "The returned iterator must point to the beginning of the inserted range.";

	SimplePolygon square_view = this->triangle_and_square[1];
	const SimplePolygon<>::const_iterator third_vertex = square_view.begin() + 2;
	result = square_view.insert(third_vertex, *this->range_start, *this->range_end);
	ASSERT_EQ(square_view.size(), this->square.size() + this->inserted_range.size()) << "The number of vertices has risen by the contents of the inserted range.";
	EXPECT_EQ(square_view[0], this->square[0]) << "The first vertex is still in its original position.";
	EXPECT_EQ(square_view[1], this->square[1]) << "The second vertex is still in its original position.";
	for(size_t i = 0; i < this->inserted_range.size(); ++i) {
		EXPECT_EQ(square_view[i + 2], this->inserted_range[i]) << "The inserted range is now in the middle of the square.";
	}
	for(size_t i = 2; i < this->square.size(); ++i) {
		EXPECT_EQ(square_view[this->inserted_range.size() + i], this->square[i]) << "The rest of the square vertices are now shifted to the end.";
	}
	EXPECT_EQ(*result, square_view[2]) << "The returned iterator must point to the beginning of the inserted range.";
}

/*!
 * Tests inserting a range between iterators at the end of a simple polygon.
 */
TYPED_TEST(InsertIteratorsParametrised, InsertIteratorsEnd) {
	SimplePolygon triangle_view = this->triangle_and_square[0];
	SimplePolygon<>::const_iterator result = triangle_view.insert(triangle_view.end(), *this->range_start, *this->range_end);
	ASSERT_EQ(triangle_view.size(), this->triangle.size() + this->inserted_range.size()) << "The number of vertices has risen by the contents of the inserted range.";
	for(size_t i = 0; i < this->triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], this->triangle[i]) << "All of the triangle vertices are still in their original places.";
	}
	for(size_t i = 0; i < this->inserted_range.size(); ++i) {
		EXPECT_EQ(triangle_view[this->triangle.size() + i], this->inserted_range[i]) << "The inserted range is now at the end of the triangle.";
	}
	EXPECT_EQ(*result, triangle_view[this->triangle.size()]) << "The returned iterator must point to the beginning of the inserted range.";

	SimplePolygon square_view = this->triangle_and_square[1];
	result = square_view.insert(square_view.end(), *this->range_start, *this->range_end);
	ASSERT_EQ(square_view.size(), this->square.size() + this->inserted_range.size()) << "The number of vertices has risen by the contents of the inserted range.";
	for(size_t i = 0; i < this->square.size(); ++i) {
		EXPECT_EQ(square_view[i], this->square[i]) << "All of the square vertices are still in their original places.";
	}
	for(size_t i = 0; i < this->inserted_range.size(); ++i) {
		EXPECT_EQ(square_view[this->square.size() + i], this->inserted_range[i]) << "The inserted range is now at the end of the square.";
	}
	EXPECT_EQ(*result, square_view[this->square.size()]) << "The returned iterator must point to the beginning of the inserted range.";
}

/*!
 * Tests inserting a list of vertices at the front of the simple polygon.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertInitialiserListFront) {
	const std::initializer_list<Point2> inserted_list({Point2(10, 20), Point2(20, 30), Point2(30, 40), Point2(40, 50)});

	SimplePolygon triangle_view = triangle_and_square[0];
	SimplePolygon<>::const_iterator result = triangle_view.insert(triangle_view.begin(), inserted_list);
	ASSERT_EQ(triangle_view.size(), triangle.size() + inserted_list.size()) << "The number of vertices has increased by the size of the list.";
	size_t i = 0;
	for(const Point2& vertex : inserted_list) {
		EXPECT_EQ(triangle_view[i++], vertex) << "The inserted list is at the front.";
	}
	for(; i < inserted_list.size() + triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i - inserted_list.size()]) << "The original triangle vertices have been shifted to the end.";
	}
	EXPECT_EQ(*result, triangle_view[0]) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";

	SimplePolygon square_view = triangle_and_square[1];
	result = square_view.insert(square_view.begin(), inserted_list);
	ASSERT_EQ(square_view.size(), square.size() + inserted_list.size()) << "The number of vertices has increased by the size of the list.";
	i = 0;
	for(const Point2& vertex : inserted_list) {
		EXPECT_EQ(square_view[i++], vertex) << "The inserted list is at the front.";
	}
	for(; i < inserted_list.size() + square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i - inserted_list.size()]) << "The original square vertices have been shifted to the end.";
	}
	EXPECT_EQ(*result, square_view[0]) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";
}

/*!
 * Tests inserting a list of vertices in the middle of the simple polygon.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertInitialiserListMiddle) {
	const std::initializer_list<Point2> inserted_list({Point2(10, 20), Point2(20, 30), Point2(30, 40), Point2(40, 50)});

	SimplePolygon triangle_view = triangle_and_square[0];
	const SimplePolygon<>::const_iterator second_vertex = triangle_view.begin() + 1;
	SimplePolygon<>::const_iterator result = triangle_view.insert(second_vertex, inserted_list);
	ASSERT_EQ(triangle_view.size(), triangle.size() + inserted_list.size()) << "The number of vertices has increased by the size of the list.";
	EXPECT_EQ(triangle_view[0], triangle[0]) << "The first vertex is still in its original place.";
	size_t i = 1;
	for(const Point2& vertex : inserted_list) {
		EXPECT_EQ(triangle_view[i++], vertex) << "The inserted list is in the middle.";
	}
	for(; i < inserted_list.size() + triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i - inserted_list.size()]) << "The rest of the triangle vertices have been shifted to the end.";
	}
	EXPECT_EQ(*result, triangle_view[1]) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";

	SimplePolygon square_view = triangle_and_square[1];
	const SimplePolygon<>::const_iterator third_vertex = square_view.begin() + 2;
	result = square_view.insert(third_vertex, inserted_list);
	ASSERT_EQ(square_view.size(), square.size() + inserted_list.size()) << "The number of vertices has increased by the size of the list.";
	EXPECT_EQ(square_view[0], square[0]) << "The first vertex is still in its original place.";
	EXPECT_EQ(square_view[1], square[1]) << "The second vertex is still in its original place.";
	i = 2;
	for(const Point2& vertex : inserted_list) {
		EXPECT_EQ(square_view[i++], vertex) << "The inserted list is in the middle.";
	}
	for(; i < inserted_list.size() + square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i - inserted_list.size()]) << "The rest of the square vertices have been shifted to the end.";
	}
	EXPECT_EQ(*result, square_view[2]) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";
}

/*!
 * Tests inserting a list of vertices at the end of the simple polygon.
 */
TEST_F(SimplePolygonBatchViewFixture, InsertInitialiserListEnd) {
	const std::initializer_list<Point2> inserted_list({Point2(10, 20), Point2(20, 30), Point2(30, 40), Point2(40, 50)});

	SimplePolygon triangle_view = triangle_and_square[0];
	SimplePolygon<>::const_iterator result = triangle_view.insert(triangle_view.end(), inserted_list);
	ASSERT_EQ(triangle_view.size(), triangle.size() + inserted_list.size()) << "The number of vertices has increased by the size of the list.";
	size_t i = 0;
	for(; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "The original triangle vertices are still in their original places.";
	}
	for(const Point2& vertex : inserted_list) {
		EXPECT_EQ(triangle_view[i++], vertex) << "The inserted list is at the end.";
	}
	EXPECT_EQ(*result, triangle_view[triangle.size()]) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";

	SimplePolygon square_view = triangle_and_square[1];
	result = square_view.insert(square_view.end(), inserted_list);
	ASSERT_EQ(square_view.size(), square.size() + inserted_list.size()) << "The number of vertices has increased by the size of the list.";
	i = 0;
	for(; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "The original square vertices are still in their original places.";
	}
	for(const Point2& vertex : inserted_list) {
		EXPECT_EQ(square_view[i++], vertex) << "The inserted list is at the end.";
	}
	EXPECT_EQ(*result, square_view[square.size()]) << "The returned iterator must point to the beginning where the vertices were inserted (after reallocation).";
}

/*!
 * Tests the maximum size of the simple polygon.
 *
 * The maximum size may not be the limiting factor for the implementation.
 */
TEST_F(SimplePolygonBatchViewFixture, MaxSize) {
	EXPECT_GE(triangle_and_square[0].max_size(), 32 * 1024 / sizeof(Point2)) << "According to OpenCL specs, the global memory must be at least 32kB. The library implementation may not be the limiting factor.";
	EXPECT_GE(triangle_and_square[0].max_size(), 32 * 1024 / sizeof(Point2)) << "According to OpenCL specs, the global memory must be at least 32kB. The library implementation may not be the limiting factor.";
}

/*!
 * Tests removing the last vertex.
 */
TEST_F(SimplePolygonBatchViewFixture, PopBack) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.pop_back();
	ASSERT_EQ(triangle_view.size(), triangle.size() - 1) << "The pop_back must have removed one vertex.";
	for(size_t i = 0; i < triangle_view.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "The original vertices must still be present.";
	}

	SimplePolygon square_view = triangle_and_square[1];
	square_view.pop_back();
	ASSERT_EQ(square_view.size(), square.size() - 1) << "The pop_back must have removed one vertex.";
	for(size_t i = 0; i < square_view.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "The original vertices must still be present.";
	}
}

/*!
 * Tests pushing a vertex to the back by copying it in.
 */
TEST_F(SimplePolygonBatchViewFixture, PushBackCopy) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.push_back(Point2(42, 69));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 1) << "The size must be increased by 1.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "The original vertices must still be in their places.";
	}
	EXPECT_EQ(triangle_view[triangle.size()], Point2(42, 69)) << "The new vertex must be at the end.";

	SimplePolygon square_view = triangle_and_square[1];
	square_view.push_back(Point2(69, 42));
	ASSERT_EQ(square_view.size(), square.size() + 1) << "The size must be increased by 1.";
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "The original vertices must still be in their places.";
	}
	EXPECT_EQ(square_view[square.size()], Point2(69, 42)) << "The new vertex must be at the end.";
}

/*!
 * Tests pushing a vertex to the back by moving it in.
 */
TEST_F(SimplePolygonBatchViewFixture, PushBackMove) {
	SimplePolygon triangle_view = triangle_and_square[0];
	Point2 new_vertex(42, 69);
	triangle_view.push_back(std::move(new_vertex));
	ASSERT_EQ(triangle_view.size(), triangle.size() + 1) << "The size must be increased by 1.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "The original vertices must still be in their places.";
	}
	EXPECT_EQ(triangle_view[triangle.size()], Point2(42, 69)) << "The new vertex must be at the end.";

	SimplePolygon square_view = triangle_and_square[1];
	Point2 new_vertex2(69, 42);
	square_view.push_back(std::move(new_vertex2));
	ASSERT_EQ(square_view.size(), square.size() + 1) << "The size must be increased by 1.";
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "The original vertices must still be in their places.";
	}
	EXPECT_EQ(square_view[square.size()], Point2(69, 42)) << "The new vertex must be at the end.";
}

/*!
 * Tests reserving memory for an empty simple polygon in a batch.
 */
TEST(SimplePolygonBatchView, ReserveEmpty) {
	SimplePolygonBatch batch(3, 0);
	SimplePolygon view = batch[1];
	const size_t original_capacity = view.capacity();
	view.reserve(0);
	EXPECT_EQ(view.capacity(), original_capacity) << "No memory was reserved, so it should remain at its original capacity.";

	view.reserve(16);
	EXPECT_GE(view.capacity(), 16) << "After reserving for 16 vertices, the capacity should be at least 16.";
}

/*!
 * Tests reserving less memory than the original capacity.
 */
TEST_F(SimplePolygonBatchViewFixture, ReserveLower) {
	SimplePolygon square_view = triangle_and_square[1];

	//To test that iterators don't get invalidated, see if modifying it through the iterator actually modifies the simple polygon.
	SimplePolygon<>::iterator second_vert = square_view.begin() + 1;

	square_view.reserve(2); //Less than the 4 vertices it already contains.
	EXPECT_GE(square_view.capacity(), 4) << "Should still have enough capacity for the 4 vertices in there.";

	second_vert->x = 456; //Modify it using the iterator that shouldn't have gotten invalidated.
	EXPECT_EQ(square_view[1].x, 456) << "The iterator shouldn't get invalidated if the capacity was not increased.";
}

/*!
 * Tests reserving more memory than the current capacity.
 */
TEST_F(SimplePolygonBatchViewFixture, ReserveHigher) {
	SimplePolygon square_view = triangle_and_square[1];

	square_view.reserve(10);
	EXPECT_GE(square_view.capacity(), 10) << "Should now have capacity for at least 10 vertices.";
}

/*!
 * Tests resizing a view to make it smaller.
 */
TEST_F(SimplePolygonBatchViewFixture, ResizeSmaller) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.resize(1);
	ASSERT_EQ(triangle_view.size(), 1) << "The size was set to 1.";
	EXPECT_EQ(triangle_view[0], triangle[0]) << "The last remaining vertex must remain the same.";

	SimplePolygon square_view = triangle_and_square[1];
	square_view.resize(2);
	ASSERT_EQ(square_view.size(), 2) << "The size was set to 2.";
	EXPECT_EQ(square_view[0], square[0]) << "The original vertices must remain in place.";
	EXPECT_EQ(square_view[1], square[1]) << "The original vertices must remain in place, also the second one.";
}

/*!
 * Tests resizing a view to make it larger without providing a fill vertex.
 */
TEST_F(SimplePolygonBatchViewFixture, ResizeLargerDefault) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.resize(5);
	ASSERT_EQ(triangle_view.size(), 5) << "The size was set to 5.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "The original vertices are still there.";
	}
	for(size_t i = triangle.size(); i < triangle_view.size(); ++i) {
		EXPECT_EQ(triangle_view[i], Point2(0, 0)) << "The rest of the vertices were zero-initialised.";
	}

	SimplePolygon square_view = triangle_and_square[1];
	square_view.resize(9);
	ASSERT_EQ(square_view.size(), 9) << "The size was set to 9.";
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "The original vertices are still there.";
	}
	for(size_t i = square.size(); i < square_view.size(); ++i) {
		EXPECT_EQ(square_view[i], Point2(0, 0)) << "The rest of the vertices were zero-initialised.";
	}
}

/*!
 * Tests resizing a view to make it larger with a custom fill vertex.
 */
TEST_F(SimplePolygonBatchViewFixture, ResizeLargerCustom) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.resize(5, Point2(52, 15));
	ASSERT_EQ(triangle_view.size(), 5) << "The size was set to 5.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_view[i], triangle[i]) << "The original vertices are still there.";
	}
	for(size_t i = triangle.size(); i < triangle_view.size(); ++i) {
		EXPECT_EQ(triangle_view[i], Point2(52, 15)) << "The rest of the vertices were filled with our custom fill vertex.";
	}

	SimplePolygon square_view = triangle_and_square[1];
	square_view.resize(9, Point2(69, 96));
	ASSERT_EQ(square_view.size(), 9) << "The size was set to 9.";
	for(size_t i = 0; i < square.size(); ++i) {
		EXPECT_EQ(square_view[i], square[i]) << "The original vertices are still there.";
	}
	for(size_t i = square.size(); i < square_view.size(); ++i) {
		EXPECT_EQ(square_view[i], Point2(69, 96)) << "The rest of the vertices were filled with our custom fill vertex.";
	}
}

/*!
 * Tests that shrinking to fit doesn't lose data.
 */
TEST_F(SimplePolygonBatchViewFixture, ShrinkToFit) {
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.reserve(100);
	triangle_view.shrink_to_fit();
	EXPECT_GE(triangle_view.capacity(), triangle_view.size());
	EXPECT_EQ(triangle_view.size(), triangle.size());
}

/*!
 * Tests getting the size of a view when it's empty.
 */
TEST(SimplePolygonBatchView, SizeEmpty) {
	SimplePolygonBatch batch(3, 0);
	for(size_t i = 0; i < batch.size(); ++i) {
		EXPECT_EQ(batch[i].size(), 0);
	}
}

/*!
 * Tests getting the size of views on the batch.
 */
TEST_F(SimplePolygonBatchViewFixture, Size) {
	EXPECT_EQ(triangle_and_square[0].size(), 3) << "The triangle has 3 vertices.";
	EXPECT_EQ(triangle_and_square[1].size(), 4) << "The square has 4 vertices.";
}

/*!
 * Tests swapping simple polygons within the same batch.
 *
 * This implementation won't need to actually swap the data, just the indices
 * pointing to the data.
 */
TEST_F(SimplePolygonBatchViewFixture, SwapWithinBatch) {
	SimplePolygon square_view = triangle_and_square[1];
	triangle_and_square[0].swap(square_view);

	const SimplePolygon triangle_view = triangle_and_square[0];
	ASSERT_EQ(triangle_view.size(), square.size()) << "The triangle and square were swapped, so the size must now be the square's size.";
	for(size_t i = 0; i < triangle_view.size(); ++i) {
		EXPECT_EQ(triangle_view[i], square[i]) << "The triangle now contains the square's vertices.";
	}

	ASSERT_EQ(square_view.size(), triangle.size()) << "The triangle and square were swapped, so the size must now be the square's size.";
	for(size_t i = 0; i < square_view.size(); ++i) {
		EXPECT_EQ(square_view[i], triangle[i]) << "The square now contains the triangle's vertices.";
	}
}

/*!
 * Tests swapping simple polygons between batches.
 *
 * This will need to swap the actual vertex data.
 */
TEST_F(SimplePolygonBatchViewFixture, SwapDifferentBatch) {
	//Create another batch to swap from.
	SimplePolygonBatch other_batch(0, 4);
	other_batch.push_back(square);
	other_batch.push_back(square);

	SimplePolygon other_view = other_batch[1];
	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.swap(other_view);

	ASSERT_EQ(triangle_view.size(), square.size()) << "The triangle was swapped with a square, so the size must swap.";
	for(size_t i = 0; i < triangle_view.size(); ++i) {
		EXPECT_EQ(triangle_view[i], square[i]) << "The triangle was swapped with a square, so the data must swap.";
	}

	ASSERT_EQ(other_view.size(), triangle.size()) << "The square was swapped with a triangle.";
	for(size_t i = 0; i < other_view.size(); ++i) {
		EXPECT_EQ(other_view[i], triangle[i]) << "The square was swapped with a triangle, so the data must swap.";
	}
}

/*!
 * Tests swapping the contents of a view with that of a vector.
 */
TEST_F(SimplePolygonBatchViewFixture, SwapVector) {
	SimplePolygon target; //Vector-based simple polygon.
	for(size_t i = 0; i < 20; ++i) {
		target.emplace_back(i * 121, i * 133);
	}
	SimplePolygon original = target; //Make a copy so that we can compare with the original state.

	SimplePolygon triangle_view = triangle_and_square[0];
	triangle_view.swap(target);

	ASSERT_EQ(triangle_view.size(), original.size()) << "The triangle was swapped with the target, so it must now have the target's original size.";
	for(size_t i = 0; i < triangle_view.size(); ++i) {
		EXPECT_EQ(triangle_view[i], original[i]) << "The triangle was swapped with the target, so it must now have the target's original vertices.";
	}
	ASSERT_EQ(target.size(), triangle.size()) << "The target was swapped with the triangle, so it must now have the triangle's original size.";
	for(size_t i = 0; i < target.size(); ++i) {
		EXPECT_EQ(target[i], triangle[i]) << "The target was swapped with the triangle, so it must now have the triangle's original vertices.";
	}
}

}