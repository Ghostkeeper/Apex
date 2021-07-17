/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.
#include <list> //For linked lists, a data structure with inherently limited iterators, from which batches must be able to copy.

#include "../helpers/input_iterator_limiter.hpp" //To test with very limited iterator types.
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
	 * A batch without any subbatches.
	 */
	BatchBase<BatchBase<int>> empty_batch;

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
	 * A batch of batches with increasing sizes: 1, 2, 4, 8, 16, 32 elements.
	 */
	BatchBase<BatchBase<int>> power_increases;

	/*!
	 * Constructs the fixture batches.
	 */
	void SetUp() {
		one.assign({1});
		one_two.assign({1, 2});
		one_through_nine.assign({1, 2, 3, 4, 5, 6, 7, 8, 9});

		for(size_t subbatch = 0; subbatch < 6; ++subbatch) { //Up to 2^5 (32) elements in each subbatch.
			power_increases.emplace_back();
			for(size_t element = 0; element < (1 << subbatch); ++element) {
				power_increases.back().push_back(element + 1);
			}
		}
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

/*!
 * Tests constructing a batch of batches from a range defined by iterators.
 *
 * In this test, the iterators are random access, so it's possible to know
 * beforehand how many subbatches will be stored.
 */
TEST_F(BatchOfBatchesFixture, ConstructRandomAccessIterator) {
	const BatchBase<BatchBase<int>> batch_full(power_increases.begin(), power_increases.end()); //power_increases is also a batch, which has a random-access iterator.
	EXPECT_EQ(batch_full, power_increases) << "Using the random-access iterator of power_increases, a complete copy of that batch was made.";

	const BatchBase<BatchBase<int>> batch_partial(power_increases.begin(), power_increases.begin() + 3); //Construct from a part of the range of the original.
	EXPECT_EQ(batch_partial.size(), 3) << "The range this batch was constructed with contained only 3 elements. Not the entire power_increases batch.";
	EXPECT_EQ(batch_partial[0], power_increases[0]) << "The first subbatch was copied.";
	EXPECT_EQ(batch_partial[1], power_increases[1]) << "The second subbatch was copied.";
	EXPECT_EQ(batch_partial[2], power_increases[2]) << "The third subbatch was copied.";

	const BatchBase<BatchBase<int>> batch_last_third(power_increases.begin() + 4, power_increases.end()); //Construct from the last part of the range of the original.
	EXPECT_EQ(batch_last_third.size(), 2) << "The range this batch was constructed with contained only 2 elements. Not the entire power_increases batch.";
	EXPECT_EQ(batch_last_third[0], power_increases[4]) << "The fifth subbatch was copied.";
	EXPECT_EQ(batch_last_third[1], power_increases[5]) << "The last subbatch was copied.";
}

/*!
 * Tests constructing a batch of batches from a range defined by iterators.
 *
 * In this test, the iterators are forward, so it's possible to iterate over it
 * multiple times to find out how many subbatches will be stored.
 */
TEST_F(BatchOfBatchesFixture, ConstructForwardIterator) {
	//A linked list offers a bidirectional iterator, which is a forward iterator but not random access.
	const std::list<BatchBase<int>> batches = {empty, one, one_two, one_through_nine};

	const BatchBase<BatchBase<int>> batch_full(batches.begin(), batches.end());
	EXPECT_EQ(batch_full.size(), 4) << "We put 4 subbatches in the list.";
	EXPECT_EQ(batch_full[0], empty) << "The first element is the empty subbatch.";
	EXPECT_EQ(batch_full[1], one) << "The second element is the subbatch with one element.";
	EXPECT_EQ(batch_full[2], one_two) << "The third element is the subbatch with two elements.";
	EXPECT_EQ(batch_full[3], one_through_nine) << "The fourth element is the subbatch with 9 digits.";

	std::list<BatchBase<int>>::const_iterator third = batches.begin();
	std::advance(third, 2);
	const BatchBase<BatchBase<int>> batch_partial(batches.begin(), third);
	EXPECT_EQ(batch_partial.size(), 2) << "We iterated up until the third element. Everything before that should be included, not including the third element.";
	EXPECT_EQ(batch_partial[0], empty) << "The first element is the empty subbatch.";
	EXPECT_EQ(batch_partial[1], one) << "The second element is the subbatch with one element.";

	const BatchBase<BatchBase<int>> batch_second_half(third, batches.end());
	EXPECT_EQ(batch_second_half.size(), 2) << "Starting from the third subbatch, there's only the third and fourth elements left.";
	EXPECT_EQ(batch_second_half[0], one_two) << "The third item in the list is the first in this batch.";
	EXPECT_EQ(batch_second_half[1], one_through_nine) << "The fourth item in the list is the second in this batch.";
}

/*!
 * Tests constructing a batch of batches from a range defined by iterators.
 *
 * In this test, the iterators are limited to simple input iterators. We can't
 * iterate over it multiple times nor know beforehand how many items we'll
 * encounter.
 */
TEST_F(BatchOfBatchesFixture, ConstructInputIterator) {
	InputIteratorLimiter<BatchBase<BatchBase<int>>::const_iterator> begin(power_increases.begin());
	InputIteratorLimiter<BatchBase<BatchBase<int>>::const_iterator> third(power_increases.begin() + 2);
	InputIteratorLimiter<BatchBase<BatchBase<int>>::const_iterator> end(power_increases.end());

	const BatchBase<BatchBase<int>> batch_full(begin, end); //Construct with very limited type of iterator.
	EXPECT_EQ(batch_full.size(), power_increases.size()) << "We added the entire batch of batches to this batch.";

	const BatchBase<BatchBase<int>> batch_partial(begin, third);
	EXPECT_EQ(batch_partial.size(), 2) << "The first and second subbatches are in, but the third marked the end.";
	EXPECT_EQ(batch_partial[0], power_increases[0]) << "The first subbatch got placed in first place.";
	EXPECT_EQ(batch_partial[1], power_increases[1]) << "The second subbatch got placed in second place.";

	const BatchBase<BatchBase<int>> batch_second_half(third, end);
	EXPECT_EQ(batch_second_half.size(), 4) << "The third batch marks the start, and then the fourth, fifth and sixth are also in.";
	EXPECT_EQ(batch_second_half[0], power_increases[2]) << "The third subbatch got placed in first place.";
	EXPECT_EQ(batch_second_half[1], power_increases[3]) << "The fourth subbatch got placed in second place.";
	EXPECT_EQ(batch_second_half[2], power_increases[4]) << "The fifth subbatch got placed in third place.";
	EXPECT_EQ(batch_second_half[3], power_increases[5]) << "The last subbatch got placed in fourth place.";
}

/*!
 * Tests the copy constructor of batches of batches.
 */
TEST_F(BatchOfBatchesFixture, ConstructCopy) {
	const BatchBase<BatchBase<int>> copy_empty(empty_batch);
	EXPECT_EQ(copy_empty, empty_batch);

	const BatchBase<BatchBase<int>> copy_filled(power_increases);
	EXPECT_EQ(copy_filled, power_increases);
}

}