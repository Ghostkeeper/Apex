/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the tests.

#include "apex/simple_polygon_batch.hpp" //The code under test.

#include "helpers/mock_simple_polygon.hpp" //To mock out the simple polygon dependency.

namespace apex {

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
TEST(SimplePolygonBatch, DefaultConstructorEmpty) {
	const SimplePolygonBatch batch1(0, 0);
	EXPECT_EQ(batch1.size(), 0) << "The batch was constructed with 0 polygons.";
	const SimplePolygonBatch batch2(0, 4);
	EXPECT_EQ(batch2.size(), 0) << "Though each polygon had a certain size, the batch was still constructed with 0 polygons.";
}

/*!
 * Tests constructing a batch with some default entries.
 */
TEST(SimplePolygonBatch, DefaultConstructorSeveral) {
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
 * Tests the copy assignment operator.
 */
TEST(SimplePolygonBatch, AssignmentOperatorCopy) {
	SimplePolygonBatch<MockSimplePolygon> original(2, 4);
	original.push_back(MockSimplePolygon(MockSimplePolygon::Shape::SQUARE_1000));
	original.push_back(MockSimplePolygon(MockSimplePolygon::Shape::TRIANGLE_1000));

	SimplePolygonBatch<MockSimplePolygon> copy;
	copy = original; //Should copy all of the data.

	EXPECT_EQ(copy, original);
}

/*!
 * Tests the move assignment operator.
 */
TEST(SimplePolygonBatch, AssignmentOperatorMove) {
	SimplePolygonBatch<MockSimplePolygon> original(2, 4);
	original.push_back(MockSimplePolygon(MockSimplePolygon::Shape::SQUARE_1000));
	original.push_back(MockSimplePolygon(MockSimplePolygon::Shape::TRIANGLE_1000));
	SimplePolygonBatch<MockSimplePolygon> copy(original); //A copy that we can move into the new batch while still being able to compare it to the original.

	SimplePolygonBatch<MockSimplePolygon> target; //The polygon we're going to move it into.
	target = std::move(copy);

	EXPECT_EQ(target, original);
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
	const SimplePolygonBatch<MockSimplePolygon> batch(4, 2);
	SimplePolygonBatch<MockSimplePolygon>::ConstView first = batch[0];
	EXPECT_EQ(first.size(), 0) << "While space for 2 vertices was reserved, no vertices were added.";
}

/*!
 * Tests accessing an idividual simple polygon within the batch.
 *
 * This also tests the back link to the original batch by looking at the number
 * of vertices in the simple polygon.
 *
 * This version tests accessing members of a non-const batch.
 */
TEST(SimplePolygonBatch, AccessViewNonConst) {
	SimplePolygonBatch<MockSimplePolygon> batch(5, 3);
	SimplePolygonBatch<MockSimplePolygon>::View second = batch[1];
	EXPECT_EQ(second.size(), 0) << "While space for 3 vertices was reserved, no vertices were added.";
}

/*!
 * Tests the push_back operation that copies a simple polygon into the batch.
 */
TEST(SimplePolygonBatch, PushBackCopy) {
	const MockSimplePolygon square(MockSimplePolygon::Shape::SQUARE_1000);
	SimplePolygonBatch<MockSimplePolygon> batch;
	batch.push_back(square);
	ASSERT_EQ(batch.size(), 1) << "We now put 1 simple polygon in the batch.";
	EXPECT_EQ(batch[0].size(), square.size()) << "The polygon in the batch must be the same as the original.";
	for(size_t vertex = 0; vertex < square.size(); ++vertex) {
		EXPECT_EQ(batch[0][vertex], square[vertex]) << "The vertices in the batch must be the same as the original.";
	}

	const MockSimplePolygon triangle(MockSimplePolygon::Shape::TRIANGLE_1000);
	batch.push_back(triangle); //Add another one to the batch.
	ASSERT_EQ(batch.size(), 2) << "There is now a second polygon in the batch.";
	EXPECT_EQ(batch[1].size(), triangle.size()) << "The polygon in the batch must be the same as the original.";
	for(size_t vertex = 0; vertex < triangle.size(); ++vertex) {
		EXPECT_EQ(batch[1][vertex], triangle[vertex]) << "The vertices in the batch must be the same as the original.";
	}
}

}