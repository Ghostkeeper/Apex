/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the tests.
#include "apex/simple_polygon_batch.hpp" //The code under test.

namespace apex {

/*!
 * Tests that the default constructor creates an empty batch.
 */
TEST(SimplePolygonBatch, Empty) {
	SimplePolygonBatch batch;
	EXPECT_EQ(batch.size(), 0) << "The batch is constructed empty.";
}

/*!
 * Tests constructing a batch with 0 entries using the default constructor.
 */
TEST(SimplePolygonBatch, DefaultConstructorEmpty) {
	SimplePolygonBatch batch(0, 0);
	EXPECT_EQ(batch.size(), 0) << "The batch was constructed with 0 polygons.";
	batch = SimplePolygonBatch(0, 4);
	EXPECT_EQ(batch.size(), 0) << "Though each polygon had a certain size, the batch was still constructed with 0 polygons.";
}

/*!
 * Tests constructing a batch with some default entries.
 */
TEST(SimplePolygonBatch, DefaultConstructorSeveral) {
	SimplePolygonBatch batch(4, 0);
	EXPECT_EQ(batch.size(), 4) << "There should be 4 polygons pre-filled, even though they don't reserve any memory for it.";
	batch = SimplePolygonBatch(5, 6);
	EXPECT_EQ(batch.size(), 5) << "There should be 5 polygons pre-filled.";
}

}