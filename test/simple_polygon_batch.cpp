/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2020 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the tests.

#include "apex/simple_polygon.hpp" //To construct simple polygons to put in the batches.
#include "apex/simple_polygon_batch.hpp" //The code under test.

namespace apex {

/*!
 * Fixture that pre-constructs a few simple polygon batches beforehand.
 */
class SimplePolygonBatchFixture : public testing::Test {
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
 * Tests that the default constructor creates an empty batch.
 */
TEST(SimplePolygonBatch, Empty) {
	const SimplePolygonBatch batch;
	EXPECT_EQ(batch.size(), 0) << "The batch is constructed empty.";
}

/*!
 * Tests constructing a batch with 0 entries using the default constructor.
 */
TEST(SimplePolygonBatch, ConstructorReservingEmpty) {
	const SimplePolygonBatch batch1(0, 0);
	EXPECT_EQ(batch1.size(), 0) << "The batch was constructed with 0 polygons.";
	const SimplePolygonBatch batch2(0, 4);
	EXPECT_EQ(batch2.size(), 0) << "Though each polygon had a certain size, the batch was still constructed with 0 polygons.";
}

/*!
 * Tests constructing a batch with some default entries.
 */
TEST(SimplePolygonBatch, ConstructorReservingSeveral) {
	const SimplePolygonBatch batch1(4, 0);
	EXPECT_EQ(batch1.size(), 4) << "There should be 4 polygons pre-filled, even though they don't reserve any memory for it.";
	for(size_t polygon = 0; polygon < batch1.size(); ++polygon) {
		EXPECT_EQ(batch1[polygon].size(), 0) << "The individual simple polygons must be empty.";
	}
	const SimplePolygonBatch batch2(5, 6);
	EXPECT_EQ(batch2.size(), 5) << "There should be 5 polygons pre-filled.";
	for(size_t polygon = 0; polygon < batch2.size(); ++polygon) {
		EXPECT_EQ(batch2[polygon].size(), 0) << "While there was memory reserved for each simple polygon, their actual data is still empty.";
	}
}

/*!
 * Tests the copy constructor.
 */
TEST_F(SimplePolygonBatchFixture, ConstructorCopy) {
	SimplePolygonBatch original(2, 4);

	const SimplePolygonBatch copy_empty(original); //Copy before there is any actual vertex data.
	EXPECT_EQ(copy_empty, original);

	original.push_back(square);
	original.push_back(triangle);

	const SimplePolygonBatch copy_filled(original); //Try another copy after filling in some data.
	EXPECT_EQ(copy_filled, original);
}

/*!
 * Tests the move constructor.
 */
TEST_F(SimplePolygonBatchFixture, ConstructorMove) {
	SimplePolygonBatch original(2, 4);
	SimplePolygonBatch copy_empty(original); //Make a copy that we can move while still being able to compare to the original.

	const SimplePolygonBatch target_empty(copy_empty);
	EXPECT_EQ(target_empty, original);

	original.push_back(square);
	original.push_back(triangle);
	SimplePolygonBatch copy_filled(original); //Another copy to move into the target.

	const SimplePolygonBatch target_filled(copy_filled); //Try another move after filling in some data.
	EXPECT_EQ(target_filled, original);
}

/*!
 * Tests the copy assignment operator.
 */
TEST_F(SimplePolygonBatchFixture, AssignmentOperatorCopy) {
	SimplePolygonBatch copy;
	copy = triangle_and_square; //Should copy all of the data.

	EXPECT_EQ(copy, triangle_and_square);
}

/*!
 * Tests the move assignment operator.
 */
TEST_F(SimplePolygonBatchFixture, AssignmentOperatorMove) {
	SimplePolygonBatch copy(triangle_and_square); //A copy that we can move into the new batch while still being able to compare it to the original.

	SimplePolygonBatch target; //The polygon we're going to move it into.
	target = std::move(copy);

	EXPECT_EQ(target, triangle_and_square);
}

/*!
 * Tests accessing an individual simple polygon within the batch.
 *
 * This also tests the back link to the original batch by looking at the number
 * of vertices in the simple polygon.
 *
 * This version tests accessing members of a const batch.
 */
TEST(SimplePolygonBatch, AccessViewConst) {
	const SimplePolygonBatch batch(4, 2);
	SimplePolygon first = batch[0];
	EXPECT_EQ(first.size(), 0) << "While space for 2 vertices was reserved, no vertices were added.";
}

/*!
 * Tests accessing an individual simple polygon within the batch.
 *
 * This also tests the back link to the original batch by looking at the number
 * of vertices in the simple polygon.
 *
 * This version tests accessing members of a non-const batch.
 */
TEST(SimplePolygonBatch, AccessViewNonConst) {
	SimplePolygonBatch batch(5, 3);
	SimplePolygon second = batch[1];
	EXPECT_EQ(second.size(), 0) << "While space for 3 vertices was reserved, no vertices were added.";
}

/*!
 * Tests whether non-const access of a simple polygon within the batch actually
 * allows modification by reference.
 */
TEST(SimplePolygonBatch, AccessViewNonConstReferenceModification) {
	SimplePolygonBatch batch(5, 3);
	SimplePolygon second = batch[1];
	SimplePolygon other_second = batch[1];
	second.emplace_back(Point2(10, 10));
	EXPECT_EQ(other_second.size(), 1) << "The polygon was resized in another instance derived from the same batch.";
}

/*!
 * Tests the push_back operation that copies a simple polygon into the batch.
 */
TEST_F(SimplePolygonBatchFixture, PushBackCopy) {
	SimplePolygonBatch batch;
	batch.push_back(square);
	ASSERT_EQ(batch.size(), 1) << "We now put 1 simple polygon in the batch.";
	EXPECT_EQ(batch[0].size(), square.size()) << "The polygon in the batch must be the same as the original.";
	for(size_t vertex = 0; vertex < square.size(); ++vertex) {
		EXPECT_EQ(batch[0][vertex], square[vertex]) << "The vertices in the batch must be the same as the original.";
	}

	batch.push_back(triangle); //Add another one to the batch.
	ASSERT_EQ(batch.size(), 2) << "There is now a second polygon in the batch.";
	EXPECT_EQ(batch[1].size(), triangle.size()) << "The polygon in the batch must be the same as the original.";
	for(size_t vertex = 0; vertex < triangle.size(); ++vertex) {
		EXPECT_EQ(batch[1][vertex], triangle[vertex]) << "The vertices in the batch must be the same as the original.";
	}
}

}