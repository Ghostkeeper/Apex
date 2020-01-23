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
	list = {Point2(12, 21), Point2(34, 43), Point2(56, 65), Point2(78, 87)};
	square_view.assign(list);
	ASSERT_EQ(square_view.size(), list.size()) << "All of the vertices of the initialiser list must have been taken over.";
	i = 0;
	for(const Point2& vertex : list) {
		EXPECT_EQ(square_view[i++], vertex);
	}
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
	SimplePolygon<>::iterator middle = triangle_view.begin();
	middle++;
	triangle_view.emplace(middle, 20, 30);
	EXPECT_EQ(triangle_view[0], triangle[0]) << "The first vertex must be unchanged.";
	EXPECT_EQ(triangle_view[1], Point2(20, 30)) << "The new vertex must be in the second place.";
	EXPECT_EQ(triangle_view[2], triangle[1]) << "The second vertex must be shifted.";
	EXPECT_EQ(triangle_view[3], triangle[2]) << "The third vertex must be shifted.";

	SimplePolygon square_view = triangle_and_square[1];
	middle = square_view.begin();
	middle++; //Advance it to between the 2nd and 3rd vertices.
	middle++;
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
	SimplePolygon<>::iterator second_vert = square_view.begin();
	second_vert++; //Now it's the second vertex.

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

}