/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "apex/detail/batch_base.hpp" //The code under test.

namespace apex {

/*
You might expect there to be some tests for batches of simple elements here.
However the BatchBase<E> class simply inherits all of its functions from
std::vector<E>. Writing tests for this then is effectively like writing tests
for the vector implementation of your compiler. Since std::vector can be
considered stable, writing tests for it is not effective. The tests below only
apply to class template specialisations with more interesting behaviour.
*/

/*!
 * A fixture with a few pre-constructed batches for easy writing of tests.
 */
class BatchOfBatchesFixture : public ::testing::Test {
public:
	/*!
	 * A batch without any elements.
	 */
	BatchBase<int> empty;

	/*!
	 * A batch with a single element, the number 1.
	 */
	BatchBase<int> one;

	/*!
	 * A batch with numbers 1 and 2.
	 */
	BatchBase<int> one_two;

	/*!
	 * A batch with numbers 1, 2, 3, 4, 5, 6, 7, 8 and 9.
	 */
	BatchBase<int> one_through_nine;

	/*!
	 * Constructs the fixture batches.
	 */
	void SetUp() {
		one.assign({1});
		one_two.assign({1, 2});
		one_through_nine.assign({1, 2, 3, 4, 5, 6, 7, 8, 9});
	}
};

/*!
 * Tests constructing a batch of batches without any batches.
 *
 * This also serves as a basic test for whether the class can be used at all.
 */
TEST(BatchOfBatches, ConstructEmpty) {
	const BatchBase<BatchBase<int>> empty;
	EXPECT_EQ(empty.size(), 0) << "The batch is empty after its creation.";
	EXPECT_TRUE(empty.empty()) << "The batch is empty after its creation.";
}

/*!
 * Tests constructing a batch of batches with the default constructor for
 * subbatches.
 */
TEST(BatchOfBatches, ConstructDefault) {
	const BatchBase<BatchBase<int>> batch1(1);
	EXPECT_EQ(batch1.size(), 1) << "We added one empty subbatch.";
	EXPECT_TRUE(batch1[0].empty()) << "The subbatch was created with its default constructor, which should leave it empty.";

	const BatchBase<BatchBase<int>> batch4(4);
	EXPECT_EQ(batch4.size(), 4) << "We added 4 default-constructed subbatches.";
	EXPECT_TRUE(batch4[0].empty()) << "The subbatch was created with its default constructor, which should leave it empty.";
	EXPECT_TRUE(batch4[1].empty()) << "The subbatch was created with its default constructor, which should leave it empty.";
	EXPECT_TRUE(batch4[2].empty()) << "The subbatch was created with its default constructor, which should leave it empty.";
	EXPECT_TRUE(batch4[3].empty()) << "The subbatch was created with its default constructor, which should leave it empty.";
}

/*!
 * Tests constructing a batch of batches by making copies of one subbatch.
 */
TEST_F(BatchOfBatchesFixture, ConstructCopies) {
	const BatchBase<BatchBase<int>> batch1_1(1, one);
	EXPECT_EQ(batch1_1.size(), 1) << "There is one subbatch in this batch.";
	EXPECT_EQ(batch1_1[0], one) << "The subbatch is the one with one element.";

	const BatchBase<BatchBase<int>> batch4_1(4, one);
	EXPECT_EQ(batch4_1.size(), 4) << "There are four copies of the subbatch in this batch.";
	EXPECT_EQ(batch4_1[0], one) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_1[1], one) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_1[2], one) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_1[3], one) << "The subbatches are copies of the original.";

	const BatchBase<BatchBase<int>> batch4_12(4, one_two);
	EXPECT_EQ(batch4_12.size(), 4) << "There are four copies of the subbatch in this batch.";
	EXPECT_EQ(batch4_12[0], one_two) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_12[1], one_two) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_12[2], one_two) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_12[3], one_two) << "The subbatches are copies of the original.";

	const BatchBase<BatchBase<int>> batch4_19(4, one_through_nine);
	EXPECT_EQ(batch4_19.size(), 4) << "There are four copies of the subbatch in this batch.";
	EXPECT_EQ(batch4_19[0], one_through_nine) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_19[1], one_through_nine) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_19[2], one_through_nine) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_19[3], one_through_nine) << "The subbatches are copies of the original.";
}

}