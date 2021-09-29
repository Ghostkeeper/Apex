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
	 * A batch of batches with linearly increasing sizes: 1, 2, 3, 4, 5
	 * elements.
	 */
	BatchBase<BatchBase<int>> linear_increases;

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

		for(size_t subbatch = 0; subbatch < 5; ++subbatch) {
			linear_increases.emplace_back();
			for(size_t element = 0; element <= subbatch; ++element) {
				linear_increases.back().push_back(element + 1);
			}
		}
		for(size_t subbatch = 0; subbatch < 6; ++subbatch) { //Up to 2^5 (32) elements in each subbatch.
			power_increases.emplace_back();
			for(size_t element = 0; element < (1 << subbatch); ++element) {
				power_increases.back().push_back(element + 1);
			}
		}
	}
};

/*!
 * Test constructing a batch of batches without any batches.
 *
 * This also serves as a basic test for whether the class can be used at all.
 */
TEST(BatchOfBatches, ConstructEmpty) {
	const BatchBase<BatchBase<int>> empty;
	EXPECT_EQ(empty.size(), 0) << "The batch is empty after its creation.";
	EXPECT_TRUE(empty.empty()) << "The batch is empty after its creation.";
}

/*!
 * Test constructing a batch of batches with the default constructor for
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
 * Test constructing a batch of batches by making copies of one subbatch.
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
 * Test constructing a batch of batches from a range defined by iterators.
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
 * Test constructing a batch of batches from a range defined by iterators.
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
 * Test constructing a batch of batches from a range defined by iterators.
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
 * Test the copy constructor of batches of batches.
 */
TEST_F(BatchOfBatchesFixture, ConstructCopy) {
	const BatchBase<BatchBase<int>> copy_empty(empty_batch);
	EXPECT_EQ(copy_empty, empty_batch);

	const BatchBase<BatchBase<int>> copy_filled(power_increases);
	EXPECT_EQ(copy_filled, power_increases);
}

/*!
 * Test the move constructor of batches of batches.
 *
 * This test includes the constraint that the actual element data didn't change
 * position, to give more certainty that the data wasn't actually copied.
 */
TEST_F(BatchOfBatchesFixture, ConstructMove) {
	const BatchBase<BatchBase<int>> original_batch(power_increases); //Make a copy so that we can compare the data in the batch without using the decommissioned moved batch.

	const int* original_position = &power_increases[5][5]; //Grab an arbitrary element in the array, noting its position in memory.
	const BatchBase<BatchBase<int>> moved_batch(std::move(power_increases));
	const int* new_position = &moved_batch[5][5];

	EXPECT_EQ(moved_batch, original_batch) << "After the move, all element data and subelement data is still unchanged.";
	EXPECT_EQ(original_position, new_position) << "The actual subelement data has not moved in the memory, eliding a copy for better performance.";
}

/*!
 * Test constructing batches of batches from initialiser lists.
 */
TEST(BatchOfBatches, ConstructInitialiserList) {
	const BatchBase<BatchBase<int>> empty({});
	EXPECT_EQ(empty.size(), 0) << "We constructed this from an empty initialiser list, so the batch must be empty too.";
	
	const BatchBase<BatchBase<int>> batch({{1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11, 12}});
	EXPECT_EQ(batch.size(), 3) << "The initialiser list had 3 members.";
	EXPECT_EQ(batch[0], BatchBase<int>({1, 2, 3})) << "The first subbatch.";
	EXPECT_EQ(batch[1], BatchBase<int>({4, 5, 6, 7})) << "The second subbatch.";
	EXPECT_EQ(batch[2], BatchBase<int>({8, 9, 10, 11, 12})) << "The third subbatch.";
}

/*!
 * Test assigning empty batches to other batches with copy-assignment.
 */
TEST_F(BatchOfBatchesFixture, AssignCopyEmpty) {
	BatchBase<BatchBase<int>> assign_empty_to_empty(empty_batch);
	assign_empty_to_empty = empty_batch; //Assign the empty batch again.
	EXPECT_EQ(assign_empty_to_empty, empty_batch) << "After assigning the empty batch, this batch is still empty.";

	BatchBase<BatchBase<int>> assign_empty_to_singular({one});
	assign_empty_to_singular = empty_batch;
	EXPECT_EQ(assign_empty_to_singular, empty_batch) << "After assigning the empty batch, this batch is empty.";

	BatchBase<BatchBase<int>> assign_empty_to_filled(power_increases);
	assign_empty_to_filled = empty_batch;
	EXPECT_EQ(assign_empty_to_filled, empty_batch) << "After assigning the empty batch, this batch is empty.";
}

/*!
 * Test assigning filled batches to other batches with copy-assignment.
 */
TEST_F(BatchOfBatchesFixture, AssignCopyFilled) {
	BatchBase<BatchBase<int>> assign_empty(empty_batch);
	assign_empty = power_increases; //Assign a filled batch to this empty batch.
	EXPECT_EQ(assign_empty, power_increases) << "After assigning this batch to the batch, it must be equal to the new batch.";

	BatchBase<BatchBase<int>> assign_singular({one});
	assign_singular = power_increases;
	EXPECT_EQ(assign_singular, power_increases) << "After assigning this batch, it must be equal to this batch.";

	BatchBase<BatchBase<int>> assign_filled(linear_increases);
	assign_filled = power_increases;
	EXPECT_EQ(assign_filled, power_increases) << "After assigning this batch, it must be equal to this batch.";
}

/*!
 * Test assigning empty batches to other batches with move-assignment.
 */
TEST_F(BatchOfBatchesFixture, AssignMoveEmpty) {
	BatchBase<BatchBase<int>> empty_copy_1(empty_batch); //Make a copy of the empty batch we're about to move, so we can still compare with the original.
	BatchBase<BatchBase<int>> assign_empty_to_empty(empty_batch);
	assign_empty_to_empty = std::move(empty_copy_1); //Assign the empty batch again.
	EXPECT_EQ(assign_empty_to_empty, empty_batch) << "After assigning the empty batch, this batch is still empty.";

	BatchBase<BatchBase<int>> empty_copy_2(empty_batch); //Don't re-use the moved batch ever again!
	BatchBase<BatchBase<int>> assign_empty_to_singular({one});
	assign_empty_to_singular = std::move(empty_copy_2);
	EXPECT_EQ(assign_empty_to_singular, empty_batch) << "After assigning the empty batch, this batch is empty.";

	BatchBase<BatchBase<int>> empty_copy_3(empty_batch); //Don't re-use the moved batch ever again!
	BatchBase<BatchBase<int>> assign_empty_to_filled(power_increases);
	assign_empty_to_filled = std::move(empty_copy_3);
	EXPECT_EQ(assign_empty_to_filled, empty_batch) << "After assigning the empty batch, this batch is empty.";
}

/*!
 * Test assigning filled batches to other batches with move-assignment.
 */
TEST_F(BatchOfBatchesFixture, AssignMoveFilled) {
	BatchBase<BatchBase<int>> powers_copy_1(power_increases); //Make a copy of the batch we're about to move, so we can still compare with the original.
	BatchBase<BatchBase<int>> assign_empty(empty_batch);
	assign_empty = std::move(powers_copy_1); //Assign a filled batch to this empty batch.
	EXPECT_EQ(assign_empty, power_increases) << "After assigning this batch to the batch, it must be equal to the new batch.";

	BatchBase<BatchBase<int>> powers_copy_2(power_increases); //Don't re-use the moved batch ever again!
	BatchBase<BatchBase<int>> assign_singular({one});
	assign_singular = std::move(powers_copy_2);
	EXPECT_EQ(assign_singular, power_increases) << "After assigning this batch, it must be equal to this batch.";

	BatchBase<BatchBase<int>> powers_copy_3(power_increases); //Don't re-use the moved batch ever again!
	BatchBase<BatchBase<int>> assign_filled(linear_increases);
	assign_filled = std::move(powers_copy_3);
	EXPECT_EQ(assign_filled, power_increases) << "After assigning this batch, it must be equal to this batch.";
}

/*!
 * Test equality of various types of batches of batches that are in fact equal.
 */
TEST_F(BatchOfBatchesFixture, CompareEqualityEqual) {
	//Empty containers are always equal.
	const BatchBase<BatchBase<int>> empty;
	EXPECT_TRUE(empty == empty_batch) << "Both batches are empty, so they are equal.";
	EXPECT_FALSE(empty != empty_batch) << "Both batches are empty, so they are equal.";

	//Comparing against itself is always equal.
	EXPECT_TRUE(empty_batch == empty_batch) << "A batch is always equal to itself.";
	EXPECT_FALSE(empty_batch != empty_batch) << "A batch is always equal to itself.";
	EXPECT_TRUE(power_increases == power_increases) << "A batch is always equal to itself.";
	EXPECT_FALSE(power_increases != power_increases) << "A batch is always equal to itself.";
	EXPECT_TRUE(linear_increases == linear_increases) << "A batch is always equal to itself.";
	EXPECT_FALSE(linear_increases != linear_increases) << "A batch is always equal to itself.";

	//Now test with different instances that happen to be the same.
	const BatchBase<BatchBase<int>> left({{3, 2, 1}, {7, 6, 5, 4}, {}, {9, 8}});
	const BatchBase<BatchBase<int>> right({{3, 2, 1}, {7, 6, 5, 4}, {}, {9, 8}});
	EXPECT_TRUE(left == right) << "These batches are constructed from the same initialiser lists, so they must be equal.";
	EXPECT_FALSE(left != right) << "These batches are constructed from the same initialiser lists, so they must be equal.";
}

/*!
 * Test equality of batches that have different numbers of subbatches.
 */
TEST_F(BatchOfBatchesFixture, CompareEqualityDifferentSize) {
	const BatchBase<BatchBase<int>> empty_subbatch({{}}); //Has one subbatch, which is empty.
	EXPECT_FALSE(empty_batch == empty_subbatch) << "empty_subbatch contains one subbatch, while the other does not.";
	EXPECT_FALSE(empty_subbatch == empty_batch) << "empty_subbatch contains one subbatch, while the other does not.";
	EXPECT_TRUE(empty_batch != empty_subbatch) << "empty_subbatch contains one subbatch, while the other does not.";
	EXPECT_TRUE(empty_subbatch != empty_batch) << "empty_subbatch contains one subbatch, while the other does not.";

	const BatchBase<BatchBase<int>> two_empty_subbatches({{}, {}});
	EXPECT_FALSE(empty_subbatch == two_empty_subbatches) << "One batch has one empty subbatch, while the other has two empty subbatches.";
	EXPECT_FALSE(two_empty_subbatches == empty_subbatch) << "One batch has one empty subbatch, while the other has two empty subbatches.";
	EXPECT_TRUE(empty_subbatch != two_empty_subbatches) << "One batch has one empty subbatch, while the other has two empty subbatches.";
	EXPECT_TRUE(two_empty_subbatches != empty_subbatch) << "One batch has one empty subbatch, while the other has two empty subbatches.";

	const BatchBase<BatchBase<int>> just_one({one});
	const BatchBase<BatchBase<int>> just_one_twice({one, one});
	EXPECT_FALSE(just_one == just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch. Their size is different.";
	EXPECT_FALSE(just_one_twice == just_one) << "The just_one batch is a prefix of the just_one_twice batch. Their size is different.";
	EXPECT_TRUE(just_one != just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch. Their size is different.";
	EXPECT_TRUE(just_one_twice != just_one) << "The just_one batch is a prefix of the just_one_twice batch. Their size is different.";

	const BatchBase<BatchBase<int>> just_one_many({one, one_through_nine});
	EXPECT_FALSE(just_one == just_one_many) << "The just_one batch is a prefix of the just_one_many batch. Their size is different.";
	EXPECT_FALSE(just_one_many == just_one) << "The just_one batch is a prefix of the just_one_many batch. Their size is different.";
	EXPECT_TRUE(just_one != just_one_many) << "The just_one batch is a prefix of the just_one_many batch. Their size is different.";
	EXPECT_TRUE(just_one_many != just_one) << "The just_one batch is a prefix of the just_one_many batch. Their size is different.";
}

/*!
 * Test equality of batches that have the same number of subbatches, but those
 * subbatches have different sizes.
 */
TEST_F(BatchOfBatchesFixture, CompareEqualityDifferentSubsize) {
	const BatchBase<BatchBase<int>> just_one({one});
	const BatchBase<BatchBase<int>> just_one_two({one_two});
	EXPECT_FALSE(just_one == just_one_two) << "One batch has a subbatch with 1 element, while the other has a subbatch with 2 elements.";
	EXPECT_FALSE(just_one_two == just_one) << "One batch has a subbatch with 1 element, while the other has a subbatch with 2 elements.";
	EXPECT_TRUE(just_one != just_one_two) << "One batch has a subbatch with 1 element, while the other has a subbatch with 2 elements.";
	EXPECT_TRUE(just_one_two != just_one) << "One batch has a subbatch with 1 element, while the other has a subbatch with 2 elements.";

	const BatchBase<BatchBase<int>> one_onetwo({one, one_two});
	const BatchBase<BatchBase<int>> one_many({one, one_through_nine});
	EXPECT_FALSE(one_onetwo == one_many) << "The first subbatch is the same for each, but the second subbatch is longer in one batch.";
	EXPECT_FALSE(one_many == one_onetwo) << "The first subbatch is the same for each, but the second subbatch is longer in one batch.";
	EXPECT_TRUE(one_onetwo != one_many) << "The first subbatch is the same for each, but the second subbatch is longer in one batch.";
	EXPECT_TRUE(one_many != one_onetwo) << "The first subbatch is the same for each, but the second subbatch is longer in one batch.";
}

/*!
 * Test equality of batches where only the subvalues are different.
 */
TEST(BatchOfBatches, CompareEqualityDifferentValues) {
	const BatchBase<BatchBase<int>> just_one({{1}});
	const BatchBase<BatchBase<int>> just_two({{2}});
	EXPECT_FALSE(just_one == just_two) << "One subbatch has the number 1, the other has the number 2.";
	EXPECT_FALSE(just_two == just_one) << "One subbatch has the number 1, the other has the number 2.";
	EXPECT_TRUE(just_one != just_two) << "One subbatch has the number 1, the other has the number 2.";
	EXPECT_TRUE(just_two != just_one) << "One subbatch has the number 1, the other has the number 2.";

	const BatchBase<BatchBase<int>> onetwo_threefour({{1, 2}, {3, 4}});
	const BatchBase<BatchBase<int>> onetwo_fourthree({{1, 2}, {4, 3}});
	EXPECT_FALSE(onetwo_threefour == onetwo_fourthree) << "The second subbatch is in a different order.";
	EXPECT_FALSE(onetwo_fourthree == onetwo_threefour) << "The second subbatch is in a different order.";
	EXPECT_TRUE(onetwo_threefour != onetwo_fourthree) << "The second subbatch is in a different order.";
	EXPECT_TRUE(onetwo_fourthree != onetwo_threefour) << "The second subbatch is in a different order.";
}

/*!
 * Test ordering of various types of batches of batches when they are equal.
 *
 * If they are equal, <= and >= operators should always return ``true`` and
 * < and > operators should always return ``false``.
 */
TEST_F(BatchOfBatchesFixture, CompareOrderEqual) {
	const BatchBase<BatchBase<int>> empty;
	EXPECT_TRUE(empty <= empty_batch) << "Both batches are empty, so they are equal.";
	EXPECT_TRUE(empty >= empty_batch) << "Both batches are empty, so they are equal.";
	EXPECT_FALSE(empty < empty_batch) << "Both batches are empty, one is not greater or less than the other.";
	EXPECT_FALSE(empty > empty_batch) << "Both batches are empty, one is not greater or less than the other.";

	//Comparisons against itself.
	EXPECT_TRUE(empty_batch <= empty_batch) << "The batch is equal to itself.";
	EXPECT_TRUE(empty_batch >= empty_batch) << "The batch is equal to itself.";
	EXPECT_FALSE(empty_batch < empty_batch) << "The batch is not greater or less than itself.";
	EXPECT_FALSE(empty_batch > empty_batch) << "The batch is not greater or less than itself.";
	EXPECT_TRUE(power_increases <= power_increases) << "The batch is equal to itself.";
	EXPECT_TRUE(power_increases >= power_increases) << "The batch is equal to itself.";
	EXPECT_FALSE(power_increases < power_increases) << "The batch is not greater or less than itself.";
	EXPECT_FALSE(power_increases > power_increases) << "The batch is not greater or less than itself.";
	EXPECT_TRUE(linear_increases <= linear_increases) << "The batch is equal to itself.";
	EXPECT_TRUE(linear_increases >= linear_increases) << "The batch is equal to itself.";
	EXPECT_FALSE(linear_increases < linear_increases) << "The batch is not greater or less than itself.";
	EXPECT_FALSE(linear_increases > linear_increases) << "The batch is not greater or less than itself.";

	//Now test with different instances that happen to be the same.
	const BatchBase<BatchBase<int>> left({{3, 2, 1}, {7, 6, 5, 4}, {}, {9, 8}});
	const BatchBase<BatchBase<int>> right({{3, 2, 1}, {7, 6, 5, 4}, {}, {9, 8}});
	EXPECT_TRUE(left <= right) << "Both batches have the same contents, so they are equal.";
	EXPECT_TRUE(left >= right) << "Both batches have the same contents, so they are equal.";
	EXPECT_FALSE(left < right) << "Both batches have the same contents, so one is not greater or less than the other.";
	EXPECT_FALSE(left > right) << "Both batches have the same contents, so one is not greater or less than the other.";
}

/*!
 * Test ordering of batches when one batch is a prefix of the other.
 *
 * This has various tests of prefixes of the main batch as well as prefixes in
 * subbatches.
 */
TEST_F(BatchOfBatchesFixture, CompareOrderPrefix) {
	const BatchBase<BatchBase<int>> empty_subbatch({{}}); //Has one subbatch, which is empty.
	EXPECT_TRUE(empty_batch < empty_subbatch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_TRUE(empty_batch <= empty_subbatch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_FALSE(empty_batch > empty_subbatch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_FALSE(empty_batch >= empty_subbatch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_FALSE(empty_subbatch < empty_batch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_FALSE(empty_subbatch <= empty_batch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_TRUE(empty_subbatch > empty_batch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_TRUE(empty_subbatch >= empty_batch) << "The empty batch is a prefix of all other batches so it is always sorted first.";

	const BatchBase<BatchBase<int>> two_empty_subbatches({{}, {}});
	EXPECT_TRUE(empty_subbatch < two_empty_subbatches) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_TRUE(empty_subbatch <= two_empty_subbatches) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_FALSE(empty_subbatch > two_empty_subbatches) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_FALSE(empty_subbatch >= two_empty_subbatches) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_FALSE(two_empty_subbatches < empty_subbatch) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_FALSE(two_empty_subbatches <= empty_subbatch) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_TRUE(two_empty_subbatches > empty_subbatch) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_TRUE(two_empty_subbatches >= empty_subbatch) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";

	const BatchBase<BatchBase<int>> just_one({one});
	const BatchBase<BatchBase<int>> just_one_twice({one, one});
	EXPECT_TRUE(just_one < just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_TRUE(just_one <= just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_FALSE(just_one > just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_FALSE(just_one >= just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_FALSE(just_one_twice < just_one) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_FALSE(just_one_twice <= just_one) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_TRUE(just_one_twice > just_one) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_TRUE(just_one_twice >= just_one) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";

	const BatchBase<BatchBase<int>> just_one_many({one, one_through_nine});
	EXPECT_TRUE(just_one < just_one_many) << "The just_one batch is a prefix of the just_one_many batch, so it should be sorted first.";
	EXPECT_TRUE(just_one <= just_one_many) << "The just_one batch is a prefix of the just_one_many batch, so it should be sorted first.";
	EXPECT_FALSE(just_one > just_one_many) << "The just_one batch is a prefix of the just_one_many batch, so it should be sorted first.";
	EXPECT_FALSE(just_one >= just_one_many) << "The just_one batch is a prefix of the just_one_many batch, so it should be sorted first.";
	EXPECT_FALSE(just_one_many < just_one) << "The just_one batch is a prefix of the just_one_many batch, so it should be sorted first.";
	EXPECT_FALSE(just_one_many <= just_one) << "The just_one batch is a prefix of the just_one_many batch, so it should be sorted first.";
	EXPECT_TRUE(just_one_many > just_one) << "The just_one batch is a prefix of the just_one_many batch, so it should be sorted first.";
	EXPECT_TRUE(just_one_many >= just_one) << "The just_one batch is a prefix of the just_one_many batch, so it should be sorted first.";

	EXPECT_TRUE(just_one_twice < just_one_many) << "The last subbatch of just_one is a prefix of the last subbatch of just_one_many, so it should be sorted first.";
	EXPECT_TRUE(just_one_twice <= just_one_many) << "The last subbatch of just_one is a prefix of the last subbatch of just_one_many, so it should be sorted first.";
	EXPECT_FALSE(just_one_twice > just_one_many) << "The last subbatch of just_one is a prefix of the last subbatch of just_one_many, so it should be sorted first.";
	EXPECT_FALSE(just_one_twice >= just_one_many) << "The last subbatch of just_one is a prefix of the last subbatch of just_one_many, so it should be sorted first.";
	EXPECT_FALSE(just_one_many < just_one_twice) << "The last subbatch of just_one is a prefix of the last subbatch of just_one_many, so it should be sorted first.";
	EXPECT_FALSE(just_one_many <= just_one_twice) << "The last subbatch of just_one is a prefix of the last subbatch of just_one_many, so it should be sorted first.";
	EXPECT_TRUE(just_one_many > just_one_twice) << "The last subbatch of just_one is a prefix of the last subbatch of just_one_many, so it should be sorted first.";
	EXPECT_TRUE(just_one_many >= just_one_twice) << "The last subbatch of just_one is a prefix of the last subbatch of just_one_many, so it should be sorted first.";

	const BatchBase<BatchBase<int>> just_one_thrice({one, one, one});
	const BatchBase<BatchBase<int>> one_onetwo_zero({one, one_two, {0}});
	EXPECT_TRUE(just_one_thrice < one_onetwo_zero) << "The second subbatch is a prefix of its counterpart, so it should be sorted first, even though in the third subbatch the order is different.";
	EXPECT_TRUE(just_one_thrice <= one_onetwo_zero) << "The second subbatch is a prefix of its counterpart, so it should be sorted first, even though in the third subbatch the order is different.";
	EXPECT_FALSE(just_one_thrice > one_onetwo_zero) << "The second subbatch is a prefix of its counterpart, so it should be sorted first, even though in the third subbatch the order is different.";
	EXPECT_FALSE(just_one_thrice >= one_onetwo_zero) << "The second subbatch is a prefix of its counterpart, so it should be sorted first, even though in the third subbatch the order is different.";
	EXPECT_FALSE(one_onetwo_zero < just_one_thrice) << "The second subbatch is a prefix of its counterpart, so it should be sorted first, even though in the third subbatch the order is different.";
	EXPECT_FALSE(one_onetwo_zero <= just_one_thrice) << "The second subbatch is a prefix of its counterpart, so it should be sorted first, even though in the third subbatch the order is different.";
	EXPECT_TRUE(one_onetwo_zero > just_one_thrice) << "The second subbatch is a prefix of its counterpart, so it should be sorted first, even though in the third subbatch the order is different.";
	EXPECT_TRUE(one_onetwo_zero >= just_one_thrice) << "The second subbatch is a prefix of its counterpart, so it should be sorted first, even though in the third subbatch the order is different.";
}

/*!
 * Test the ordering of batches when the subelements of subbatches contain
 * different values, but the batches are otherwise the same.
 */
TEST_F(BatchOfBatchesFixture, CompareOrderValues) {
	const BatchBase<BatchBase<int>> just_one({{1}});
	const BatchBase<BatchBase<int>> just_two({{2}});
	EXPECT_TRUE(just_one < just_two) << "The number 1 is lower than the number 2.";
	EXPECT_TRUE(just_one <= just_two) << "The number 1 is lower than the number 2.";
	EXPECT_FALSE(just_one > just_two) << "The number 1 is lower than the number 2.";
	EXPECT_FALSE(just_one >= just_two) << "The number 1 is lower than the number 2.";
	EXPECT_FALSE(just_two < just_one) << "The number 1 is lower than the number 2.";
	EXPECT_FALSE(just_two <= just_one) << "The number 1 is lower than the number 2.";
	EXPECT_TRUE(just_two > just_one) << "The number 1 is lower than the number 2.";
	EXPECT_TRUE(just_two >= just_one) << "The number 1 is lower than the number 2.";

	const BatchBase<BatchBase<int>> onetwo_threefour({{1, 2}, {3, 4}});
	const BatchBase<BatchBase<int>> onetwo_fourthree({{1, 2}, {4, 3}});
	EXPECT_TRUE(onetwo_threefour < onetwo_fourthree) << "In the second subbatch, the number 3 is lower than the number 4.";
	EXPECT_TRUE(onetwo_threefour <= onetwo_fourthree) << "In the second subbatch, the number 3 is lower than the number 4.";
	EXPECT_FALSE(onetwo_threefour > onetwo_fourthree) << "In the second subbatch, the number 3 is lower than the number 4.";
	EXPECT_FALSE(onetwo_threefour >= onetwo_fourthree) << "In the second subbatch, the number 3 is lower than the number 4.";
	EXPECT_FALSE(onetwo_fourthree < onetwo_threefour) << "In the second subbatch, the number 3 is lower than the number 4.";
	EXPECT_FALSE(onetwo_fourthree <= onetwo_threefour) << "In the second subbatch, the number 3 is lower than the number 4.";
	EXPECT_TRUE(onetwo_fourthree > onetwo_threefour) << "In the second subbatch, the number 3 is lower than the number 4.";
	EXPECT_TRUE(onetwo_fourthree >= onetwo_threefour) << "In the second subbatch, the number 3 is lower than the number 4.";
}

/*!
 * Test whether the ordering of batches properly compares in lexicographic
 * order.
 *
 * For these tests the outcome would be different if later subelements or
 * subbatches were compared first, but due to the subelements being compared
 * front to back their order is as it is.
 */
TEST_F(BatchOfBatchesFixture, CompareOrderLexicographic) {
	const BatchBase<BatchBase<int>> one_two_three({{1}, {2}, {3}});
	const BatchBase<BatchBase<int>> one_three_two({{1}, {3}, {2}});
	EXPECT_TRUE(one_two_three < one_three_two) << "The third subbatch doesn't matter since the second subbatch already determines the order.";
	EXPECT_TRUE(one_two_three <= one_three_two) << "The third subbatch doesn't matter since the second subbatch already determines the order.";
	EXPECT_FALSE(one_two_three > one_three_two) << "The third subbatch doesn't matter since the second subbatch already determines the order.";
	EXPECT_FALSE(one_two_three >= one_three_two) << "The third subbatch doesn't matter since the second subbatch already determines the order.";
	EXPECT_FALSE(one_three_two < one_two_three) << "The third subbatch doesn't matter since the second subbatch already determines the order.";
	EXPECT_FALSE(one_three_two <= one_two_three) << "The third subbatch doesn't matter since the second subbatch already determines the order.";
	EXPECT_TRUE(one_three_two > one_two_three) << "The third subbatch doesn't matter since the second subbatch already determines the order.";
	EXPECT_TRUE(one_three_two >= one_two_three) << "The third subbatch doesn't matter since the second subbatch already determines the order.";
}

/*!
 * Test the assign method to assign multiple copies of a subbatch.
 */
TEST_F(BatchOfBatchesFixture, AssignCopies) {
	BatchBase<BatchBase<int>> batch; //Start off empty.

	batch.assign(20, one_two);
	EXPECT_EQ(batch.size(), 20);
	for(SubbatchView<int>& subbatch : batch) {
		EXPECT_EQ(subbatch, one_two);
	}

	//In this same batch, assign something else, confirming that the old content is erased.
	batch.assign(10, one);
	EXPECT_EQ(batch.size(), 10);
	for(SubbatchView<int>& subbatch : batch) {
		EXPECT_EQ(subbatch, one);
	}

	//Now assign something bigger.
	batch.assign(30, one_through_nine);
	EXPECT_EQ(batch.size(), 30);
	for(SubbatchView<int>& subbatch : batch) {
		EXPECT_EQ(subbatch, one_through_nine);
	}
}

/*!
 * Test assigning a range to the batch of batches defined by iterators.
 *
 * In this test, the iterators are random access, so it's possible to know
 * beforehand how many subbatches will be stored.
 */
TEST_F(BatchOfBatchesFixture, AssignRandomAccessIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases;

	batch.assign(power_increases.begin(), power_increases.end()); //power_increases is also a batch, which has a random-access iterator.
	EXPECT_EQ(batch, power_increases) << "Using the random-access iterator of power_increases, a complete copy of that batch was made.";

	batch.assign(power_increases.begin(), power_increases.begin() + 3); //Assign a part of the range of the original.
	EXPECT_EQ(batch.size(), 3) << "The range assigned to this batch contained only 3 elements. Not the entire power_increases batch.";
	EXPECT_EQ(batch[0], power_increases[0]) << "The first subbatch was copied.";
	EXPECT_EQ(batch[1], power_increases[1]) << "The second subbatch was copied.";
	EXPECT_EQ(batch[2], power_increases[2]) << "The third subbatch was copied.";

	batch.assign(linear_increases.begin() + 2, linear_increases.end()); //Assign the second half of this batch.
	EXPECT_EQ(batch.size(), linear_increases.size() - 2) << "The range assigned to this batch was everything except the first two elements (should be 3).";
	EXPECT_EQ(batch[0], linear_increases[2]) << "The third subbatch was copied.";
	EXPECT_EQ(batch[1], linear_increases[3]) << "The fourth subbatch was copied.";
	EXPECT_EQ(batch[2], linear_increases[4]) << "The fifth subbatch was copied.";
}

/*!
 * Test assigning a range to the batch of batches defined by iterators.
 *
 * In this test, the iterators are forward iterators, so it's impossible to know
 * beforehand how many items will be added, but we can iterate multiple times in
 * order to count how much.
 */
TEST_F(BatchOfBatchesFixture, AssignForwardIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases;

	//A linked list offers a bidirectional iterator, which is a forward iterator but not random access.
	const std::list<BatchBase<int>> batches = {empty, one, one_two, one_through_nine};
	batch.assign(batches.begin(), batches.end());
	EXPECT_EQ(batch.size(), 4) << "We put 4 subbatches in the list.";
	EXPECT_EQ(batch[0], empty) << "The first element is the empty subbatch.";
	EXPECT_EQ(batch[1], one) << "The second element is the subbatch with one element.";
	EXPECT_EQ(batch[2], one_two) << "The third element is the subbatch with two elements.";
	EXPECT_EQ(batch[3], one_through_nine) << "The fourth element is the subbatch with 9 digits.";

	std::list<BatchBase<int>>::const_iterator third = batches.begin();
	std::advance(third, 2);
	batch.assign(batches.begin(), third);
	EXPECT_EQ(batch.size(), 2) << "We iterated up until the third element. Everything before that should be included, not including the third element.";
	EXPECT_EQ(batch[0], empty) << "The first element is the empty subbatch.";
	EXPECT_EQ(batch[1], one) << "The second element is the subbatch with one element.";

	batch.assign(third, batches.end());
	EXPECT_EQ(batch.size(), 2) << "Starting from the third subbatch, there's only the third and fourth elements left.";
	EXPECT_EQ(batch[0], one_two) << "The third item in the list is the first in this batch.";
	EXPECT_EQ(batch[1], one_through_nine) << "The fourth item in the list is the second in this batch.";
}

/*!
 * Test assigning a range to the batch of batches defined by iterators.
 *
 * In this test, the iterators are input iterators, so it's impossible to know
 * beforehand how many items will be added, and we can only iterate over it
 * once, which also makes it impossible to go over it once to determine its
 * size.
 */
TEST_F(BatchOfBatchesFixture, AssignInputIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases;
	InputIteratorLimiter<BatchBase<BatchBase<int>>::const_iterator> begin(power_increases.begin());
	InputIteratorLimiter<BatchBase<BatchBase<int>>::const_iterator> third(power_increases.begin() + 2);
	InputIteratorLimiter<BatchBase<BatchBase<int>>::const_iterator> end(power_increases.end());

	batch.assign(begin, end); //Assign with this very limited type of iterator.
	EXPECT_EQ(batch, power_increases) << "We added the entire batch of batches to this batch.";

	batch.assign(begin, third);
	EXPECT_EQ(batch.size(), 2) << "The first and second subbatches are in, but the third marked the end.";
	EXPECT_EQ(batch[0], power_increases[0]) << "The first subbatch got placed in first place.";
	EXPECT_EQ(batch[1], power_increases[1]) << "The second subbatch got placed in second place.";

	batch.assign(third, end);
	EXPECT_EQ(batch.size(), 4) << "The third batch marks the start, and then the fourth, fifth and sixth are also in.";
	EXPECT_EQ(batch[0], power_increases[2]) << "The third subbatch got placed in first place.";
	EXPECT_EQ(batch[1], power_increases[3]) << "The fourth subbatch got placed in second place.";
	EXPECT_EQ(batch[2], power_increases[4]) << "The fifth subbatch got placed in third place.";
	EXPECT_EQ(batch[3], power_increases[5]) << "The last subbatch got placed in fourth place.";
}

/*!
 * Test assigning an initialiser list to the batch of batches.
 */
TEST_F(BatchOfBatchesFixture, AssignInitialiserList) {
	BatchBase<BatchBase<int>> batch = power_increases;

	batch.assign({one, one_through_nine, one_two});
	EXPECT_EQ(batch.size(), 3) << "We assigned a list of 3 subbatches to this batch.";
	EXPECT_EQ(batch[0], one) << "The first subbatch in the initialiser list was the \"one\" batch.";
	EXPECT_EQ(batch[1], one_through_nine) << "The second subbatch in the initialiser list was one_through_nine.";
	EXPECT_EQ(batch[2], one_two) << "The third subbatch in the initialiser list was one_two.";

	batch.assign({});
	EXPECT_EQ(batch.size(), 0) << "We assigned an empty initialiser list to this batch.";

	batch.assign({{1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11, 12}});
	EXPECT_EQ(batch.size(), 3) << "The initialiser list had 3 members.";
	EXPECT_EQ(batch[0], BatchBase<int>({1, 2, 3})) << "The first subbatch.";
	EXPECT_EQ(batch[1], BatchBase<int>({4, 5, 6, 7})) << "The second subbatch.";
	EXPECT_EQ(batch[2], BatchBase<int>({8, 9, 10, 11, 12})) << "The third subbatch.";
}

/*!
 * Test clearing batches of batches.
 */
TEST_F(BatchOfBatchesFixture, Clear) {
	linear_increases.clear();
	EXPECT_EQ(linear_increases.size(), 0) << "After clearing the batch, it should no longer have any items.";

	empty_batch.clear();
	EXPECT_EQ(empty_batch.size(), 0) << "The empty batch must stay empty after clearing it.";

	power_increases.clear();
	EXPECT_EQ(power_increases.size(), 0) << "After clearing the batch, it should no longer have any items.";

	BatchBase<BatchBase<int>> batch({one_two}); //A batch with a single item inside.
	batch.clear();
	EXPECT_EQ(batch.size(), 0) << "The one item in this batch must be erased by the clearing.";
}

/*!
 * Test getting an array of data for the subelements.
 *
 * This test looks whether the data contains all subelements in the batch.
 */
TEST_F(BatchOfBatchesFixture, DataSubelements) {
	std::vector<size_t> histogram; //Make a histogram of how often each element occurs in the subelement buffer.
	histogram.resize(6);
	for(size_t subelement_index = 0; subelement_index < linear_increases.size_subelements(); ++subelement_index) {
		histogram[*(linear_increases.data_subelements() + subelement_index)]++;
	}
	EXPECT_GE(histogram[1], 5) << "There were 5 subelements containing the number 1. More are allowed since the buffer can have dead spaces.";
	EXPECT_GE(histogram[2], 4) << "There were 4 subelements containing the number 2. More are allowed since the buffer can have dead spaces.";
	EXPECT_GE(histogram[3], 3) << "There were 3 subelements containing the number 3. More are allowed since the buffer can have dead spaces.";
	EXPECT_GE(histogram[4], 2) << "There were 2 subelements containing the number 4. More are allowed since the buffer can have dead spaces.";
	EXPECT_GE(histogram[5], 1) << "There was 1 subelement containing the number 5. More are allowed since the buffer can have dead spaces.";
}

/*!
 * Test emplacing an empty subbatch into the parent batch.
 */
TEST_F(BatchOfBatchesFixture, EmplaceEmpty) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace(batch.end()); //Emplace at the end.
	EXPECT_EQ(batch.size(), linear_increases.size() + 1) << "The number of subbatches grew by 1.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {}})) << "There's an empty subbatch at the end now.";

	batch.emplace(batch.begin()); //Emplace another one at the beginning.
	EXPECT_EQ(batch.size(), linear_increases.size() + 2) << "We added two batches to it, so it should have grown by 2.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{}, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {}})) << "There's an empty subbatch at the start and end now.";

	batch.emplace(batch.begin() + 3); //Emplace another one somewhere in the middle.
	EXPECT_EQ(batch.size(), linear_increases.size() + 3) << "We added three batches to it now.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{}, linear_increases[0], linear_increases[1], {}, linear_increases[2], linear_increases[3], linear_increases[4], {}})) << "There's still an empty subbatch at the start and end, and a new one in 4th place.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the constructor that
 * makes copies of a subvalue.
 */
TEST_F(BatchOfBatchesFixture, EmplaceCopies) {
	BatchBase<BatchBase<int>> batch = power_increases; //Make a copy so that we can compare with the original.

	batch.emplace(batch.end(), size_t(4), 1); //Add a subbatch with four 1's at the end.
	EXPECT_EQ(batch.size(), power_increases.size() + 1) << "The number of subbatches grew by 1.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({power_increases[0], power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5], {1, 1, 1, 1}})) << "There is a subbatch with four 1's appended to the end.";

	batch.emplace(batch.begin(), size_t(3), 2); //Add a subbatch with three 2's in the beginning.
	EXPECT_EQ(batch.size(), power_increases.size() + 2) << "This is the second time we added one.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{2, 2, 2}, power_increases[0], power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5], {1, 1, 1, 1}})) << "We added a subbatch with three 2's at the start.";

	batch.emplace(batch.begin() + 2, size_t(2), 3); //Add a subbatch with two 3's in the middle.
	EXPECT_EQ(batch.size(), power_increases.size() + 3) << "This is the third time we added one.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{2, 2, 2}, power_increases[0], {3, 3}, power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5], {1, 1, 1, 1}})) << "We added a subbatch with two 3's in the third place.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the constructor that
 * takes iterators.
 *
 * The iterators provided in this test are random access, meaning that the
 * distance between them can be obtained in constant time.
 */
TEST_F(BatchOfBatchesFixture, EmplaceRandomAccessIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace(batch.end(), one_through_nine.begin(), one_through_nine.begin() + 4); //Append a subbatch with 4 elements at the end.
	EXPECT_EQ(batch.size(), linear_increases.size() + 1) << "The number of subbatches grew by 1.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {1, 2, 3, 4}})) << "There is a part of the one_through_nine batch at the end.";

	batch.emplace(batch.begin(), batch[3].begin(), batch[3].end()); //Re-add a part of itself. Does the index get invalidated prematurely?
	EXPECT_EQ(batch.size(), linear_increases.size() + 2) << "This is the second time we added a subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[3], linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {1, 2, 3, 4}})) << "A part of the batch itself was added again. This should not invalidate the index in the batch until it's completed.";

	batch.emplace(batch.begin() + 4, one.begin(), one.begin()); //Add an empty subbatch through random access iterators.
	EXPECT_EQ(batch.size(), linear_increases.size() + 3) << "Even though the subbatch was empty, we still added a third extra subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[3], linear_increases[0], linear_increases[1], linear_increases[2], {}, linear_increases[3], linear_increases[4], {1, 2, 3, 4}})) << "We added an empty subbatch in the middle (no distance between start and end iterators).";
}

/*!
 * Test emplacing a subbatch into the parent batch with the constructor that
 * takes iterators.
 *
 * The iterators provided in this test are forward, meaning that you can't
 * easily calculate the distance between them, but you can iterate over it
 * multiple times.
 */
TEST_F(BatchOfBatchesFixture, EmplaceForwardIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	std::list<int> numbers(one_through_nine.begin(), one_through_nine.end()); //A linked list has bidirectional iterators, which is a forward iterator but not random access.

	batch.emplace(batch.end(), numbers.begin(), numbers.begin()); //Append an empty subbatch at the end.
	EXPECT_EQ(batch.size(), linear_increases.size() + 1) << "Even though the subbatch was empty, we still added a new subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {}})) << "There is an empty subbatch at the end now.";

	std::list<int>::const_iterator fifth = numbers.begin();
	std::advance(fifth, 4);
	batch.emplace(batch.begin(), numbers.cbegin(), fifth); //Prepend part of this list at the start.
	EXPECT_EQ(batch.size(), linear_increases.size() + 2) << "This is the second time we added a subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{1, 2, 3, 4}, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {}})) << "A part of the numbers list is added as a subbatch in front.";

	std::list<int>::const_iterator second = numbers.begin();
	std::advance(second, 1);
	batch.emplace(batch.begin() + 4, second, fifth); //Insert a subbatch in the middle too.
	EXPECT_EQ(batch.size(), linear_increases.size() + 3) << "This is the third time we added a subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{1, 2, 3, 4}, linear_increases[0], linear_increases[1], linear_increases[2], {2, 3, 4}, linear_increases[3], linear_increases[4], {}})) << "We added a new subbatch in the middle.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the constructor that
 * takes iterators.
 *
 * The iterators provided in this test are input iterators only, meaning that we
 * can't calculate the number of items beforehand.
 */
TEST_F(BatchOfBatchesFixture, EmplaceInputIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	InputIteratorLimiter<BatchBase<int>::const_iterator> begin(one_through_nine.begin()); //Create some very limited input iterators to test with.
	InputIteratorLimiter<BatchBase<int>::const_iterator> fifth(one_through_nine.begin() + 4);
	InputIteratorLimiter<BatchBase<int>::const_iterator> end(one_through_nine.end());

	batch.emplace(batch.end(), begin, fifth);
	EXPECT_EQ(batch.size(), linear_increases.size() + 1) << "We added a subbatch, so it's one larger now.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {1, 2, 3, 4}})) << "There is a new subbatch at the end.";

	batch.emplace(batch.begin(), fifth, fifth); //Empty range!
	EXPECT_EQ(batch.size(), linear_increases.size() + 2) << "Even though the subbatch was empty, we still added a new subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{}, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {1, 2, 3, 4}})) << "We added an empty subbatch at the start.";

	batch.emplace(batch.begin() + 4, fifth, end);
	EXPECT_EQ(batch.size(), linear_increases.size() + 3) << "This is the third time we added a subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{}, linear_increases[0], linear_increases[1], linear_increases[2], {5, 6, 7, 8, 9}, linear_increases[3], linear_increases[4], {1, 2, 3, 4}})) << "We added the second half of one_through_nine in the middle.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the copy constructor.
 *
 * Behaviourally this should be identical to just inserting the copy.
 */
TEST_F(BatchOfBatchesFixture, EmplaceCopy) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace(batch.begin(), one_through_nine);
	EXPECT_EQ(batch.size(), linear_increases.size() + 1) << "We added a new subbatch, so the size grew by one.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4]})) << "We added the one_through_nine batch at the front.";

	batch.emplace(batch.end(), one);
	EXPECT_EQ(batch.size(), linear_increases.size() + 2) << "We added another subbatch, so the size grew again.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], one})) << "We added the one batch at the end.";

	batch.emplace(batch.begin() + 2, one_two);
	EXPECT_EQ(batch.size(), linear_increases.size() + 3) << "We added a third subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], one_two, linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], one})) << "We added the one_two batch in the middle.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the move constructor.
 *
 * Behaviourally this should be identical to just inserting the subbatch.
 */
TEST_F(BatchOfBatchesFixture, EmplaceMove) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy of each input, so that we can compare with the original.
	BatchBase<int> one_through_nine_copy = one_through_nine;
	BatchBase<int> one_copy = one;
	BatchBase<int> one_two_copy = one_two;

	batch.emplace(batch.begin(), std::move(one_through_nine_copy));
	EXPECT_EQ(batch.size(), linear_increases.size() + 1) << "We added a new subbatch, so the size grew by one.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4]})) << "We added the one_through_nine batch at the front.";

	batch.emplace(batch.end(), std::move(one_copy));
	EXPECT_EQ(batch.size(), linear_increases.size() + 2) << "We added another subbatch, so the size grew again.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], one})) << "We added the one batch at the end.";

	batch.emplace(batch.begin() + 2, std::move(one_two_copy));
	EXPECT_EQ(batch.size(), linear_increases.size() + 3) << "We added a third subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], one_two, linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], one})) << "We added the one_two batch in the middle.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the constructor that
 * takes an initialiser list.
 */
TEST_F(BatchOfBatchesFixture, EmplaceInitialiserList) {
	BatchBase<BatchBase<int>> batch = power_increases; //Make a copy so that we can compare with the original.

	batch.emplace(batch.begin(), {9, 8, 7});
	EXPECT_EQ(batch.size(), power_increases.size() + 1) << "We added a new subbatch, so the size grew by one.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{9, 8, 7}, power_increases[0], power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5]})) << "We added {9, 8, 7} at the front.";

	batch.emplace(batch.end(), {6});
	EXPECT_EQ(batch.size(), power_increases.size() + 2) << "We added another subbatch, so the size grew again.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{9, 8, 7}, power_increases[0], power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5], {6}})) << "We added {6} at the end.";

	batch.emplace(batch.begin() + 3, {});
	EXPECT_EQ(batch.size(), power_increases.size() + 3) << "We added a third subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{9, 8, 7}, power_increases[0], power_increases[1], {}, power_increases[2], power_increases[3], power_increases[4], power_increases[5], {6}})) << "We added an empty subbatch in the middle.";

	batch.emplace(batch.begin() + 4, {5, 4, 3, 2, 1});
	EXPECT_EQ(batch.size(), power_increases.size() + 4) << "We added a fourth subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({{9, 8, 7}, power_increases[0], power_increases[1], {}, {5, 4, 3, 2, 1}, power_increases[2], power_increases[3], power_increases[4], power_increases[5], {6}})) << "We added {5, 4, 3, 2, 1} in the middle, just after the empty batch we added earlier.";
}

/*!
 * Test emplacing an empty subbatch onto the back.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackEmpty) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace_back();
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {}})) << "We added an empty subbatch at the end.";

	batch.emplace_back();
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {}, {}})) << "We added a second empty subbatch at the end.";

	empty_batch.emplace_back(); //Test starting from an empty batch.
	EXPECT_EQ(empty_batch, BatchBase<BatchBase<int>>({{}})) << "We added an empty subbatch to the previously empty batch.";
}

/*!
 * Test emplacing a new subbatch onto the back containing copies of a specific
 * subelement.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackCopies) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace_back(size_t(4), 1);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {1, 1, 1, 1}})) << "We constructed a subbatch containing four 1's at the end.";

	empty_batch.emplace_back(size_t(5), 2);
	EXPECT_EQ(empty_batch, BatchBase<BatchBase<int>>({{2, 2, 2, 2, 2}})) << "We constructed a subbatch containing five 2's at the end of the previously empty batch.";
}

/*!
 * Test emplacing a new subbatch onto the back from a pair of iterators defining
 * the contents of that subbatch.
 *
 * The iterators used in this test are random access, meaning we can directly
 * measure how many items are in that range.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackRandomAccessIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace_back(one_through_nine.begin() + 4, one_through_nine.end());
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {5, 6, 7, 8, 9}})) << "We constructed the second half of one_through_nine as a new subbatch at the end.";

	empty_batch.emplace_back(one_through_nine.begin() + 3, one_through_nine.begin() + 3); //Try an empty range.
	EXPECT_EQ(empty_batch, BatchBase<BatchBase<int>>({{}})) << "We added an empty range to this previously empty batch, so that it now contains one subbatch which is itself empty.";

	empty_batch.emplace_back(one_through_nine.begin(), one_through_nine.begin() + 4);
	EXPECT_EQ(empty_batch, BatchBase<BatchBase<int>>({{}, {1, 2, 3, 4}})) << "We appended the first half of one_through_nine to the batch that previously contained just an empty subbatch.";
}

/*!
 * Test emplacing a new subbatch onto the back from a pair of iterators defining
 * the contents of that subbatch.
 *
 * The iterators used in this test are forward, meaning we can't calculate how
 * many items are in the range without iterating over it, but we can iterate
 * multiple times.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackForwardIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	std::list<int> numbers(one_through_nine.begin(), one_through_nine.end()); //Linked list which has bidirectional iterators, which are just as good as forward iterators for our purpose.
	std::list<int>::iterator halfway = numbers.begin();
	std::advance(halfway, 4);

	batch.emplace_back(halfway, numbers.end());
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {5, 6, 7, 8, 9}})) << "We constructed the second half of numbers as a new subbatch at the end.";

	empty_batch.emplace_back(halfway, halfway); //Try an empty range.
	EXPECT_EQ(empty_batch, BatchBase<BatchBase<int>>({{}})) << "We added an empty range to this previously empty batch, so that it now contains one subbatch which is itself empty.";

	empty_batch.emplace_back(numbers.begin(), halfway);
	EXPECT_EQ(empty_batch, BatchBase<BatchBase<int>>({{}, {1, 2, 3, 4}})) << "We appended the first half of numbers to the batch that previously contained just an empty subbatch.";
}

/*!
 * Test emplacing a new subbatch onto the back from a pair of iterators defining
 * the contents of that subbatch.
 *
 * The iterators used in this test are input iterators, the most limited type of
 * iterators that input data. We can't calculate how much data will be given
 * beforehand, and we can't walk over it multiple times.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackInputIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	InputIteratorLimiter<BatchBase<int>::const_iterator> begin(one_through_nine.begin());
	InputIteratorLimiter<BatchBase<int>::const_iterator> middle(one_through_nine.begin() + 4);
	InputIteratorLimiter<BatchBase<int>::const_iterator> end(one_through_nine.end());

	batch.emplace_back(middle, end);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {5, 6, 7, 8, 9}})) << "We constructed the second half of one_through_nine as a new subbatch at the end.";

	empty_batch.emplace_back(middle, middle); //Try an empty range.
	EXPECT_EQ(empty_batch, BatchBase<BatchBase<int>>({{}})) << "We added an empty range to this previously empty batch, so that it now contains one subbatch which is itself empty.";

	empty_batch.emplace_back(begin, middle);
	EXPECT_EQ(empty_batch, BatchBase<BatchBase<int>>({{}, {1, 2, 3, 4}})) << "We appended the first half of one_through_nine to the batch that previously contained just an empty subbatch.";
}

/*!
 * Test emplacing a new subbatch onto the back using the copy constructor.
 *
 * A bit pointless, since the copy constructor makes a copy anyway so why going
 * through the trouble of using emplace rather than push_back, but it keeps the
 * interface consistent.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackCopy) {
	BatchBase<BatchBase<int>> batch = power_increases;

	batch.emplace_back(one_two);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({power_increases[0], power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5], one_two})) << "We added a copy of one_two to the end.";
	batch.emplace_back(one);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({power_increases[0], power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5], one_two, one})) << "We added a copy of one to the end.";
	batch.emplace_back(one_through_nine);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({power_increases[0], power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5], one_two, one, one_through_nine})) << "We added a copy of one_through_nine to the end.";
}

/*!
 * Test emplacing a new subbatch onto the back using the move constructor.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackMove) {
	BatchBase<BatchBase<int>> batch = power_increases;

	batch.emplace_back(std::move(one_two));
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({power_increases[0], power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5], one_two})) << "We moved one_two onto the end.";
	batch.emplace_back(std::move(one));
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({power_increases[0], power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5], one_two, one})) << "We moved one onto the end.";
	batch.emplace_back(std::move(one_through_nine));
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({power_increases[0], power_increases[1], power_increases[2], power_increases[3], power_increases[4], power_increases[5], one_two, one, one_through_nine})) << "We moved one_through_nine onto the end.";
}

/*!
 * Test emplacing a new subbatch onto the back using the constructor that takes
 * an initialiser list.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackInitialiserList) {
	BatchBase<BatchBase<int>> batch = linear_increases;

	batch.emplace_back({6, 5, 4});
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {6, 5, 4}})) << "We added {6, 5, 4} to the end.";
	batch.emplace_back({});
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {6, 5, 4}, {}})) << "We added an empty initialiser list to the end.";
	batch.emplace_back({1});
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], {6, 5, 4}, {}, {1}})) << "We added {1} to the end.";
}

/*!
 * Test inserting a subbatch into a batch by making a copy.
 */
TEST_F(BatchOfBatchesFixture, InsertCopy) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.insert(batch.begin(), one_through_nine);
	EXPECT_EQ(batch.size(), linear_increases.size() + 1) << "We added a new subbatch, so the size grew by one.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4]})) << "We added the one_through_nine batch at the front.";

	batch.insert(batch.end(), one);
	EXPECT_EQ(batch.size(), linear_increases.size() + 2) << "We added another subbatch, so the size grew again.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], one})) << "We added the one batch at the end.";

	batch.insert(batch.begin() + 2, one_two);
	EXPECT_EQ(batch.size(), linear_increases.size() + 3) << "We added a third subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], one_two, linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], one})) << "We added the one_two batch in the middle.";
}

/*!
 * Test inserting a subbatch into a batch by moving it in.
 */
TEST_F(BatchOfBatchesFixture, InsertMove) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy of each input, so that we can compare with the original.
	BatchBase<int> one_through_nine_copy = one_through_nine;
	BatchBase<int> one_copy = one;
	BatchBase<int> one_two_copy = one_two;

	batch.insert(batch.begin(), std::move(one_through_nine_copy));
	EXPECT_EQ(batch.size(), linear_increases.size() + 1) << "We added a new subbatch, so the size grew by one.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4]})) << "We added the one_through_nine batch at the front.";

	batch.insert(batch.end(), std::move(one_copy));
	EXPECT_EQ(batch.size(), linear_increases.size() + 2) << "We added another subbatch, so the size grew again.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], one})) << "We added the one batch at the end.";

	batch.insert(batch.begin() + 2, std::move(one_two_copy));
	EXPECT_EQ(batch.size(), linear_increases.size() + 3) << "We added a third subbatch.";
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({one_through_nine, linear_increases[0], one_two, linear_increases[1], linear_increases[2], linear_increases[3], linear_increases[4], one})) << "We added the one_two batch in the middle.";
}

/*!
 * Test inserting repeated copies of a subbatch into the batch.
 */
TEST_F(BatchOfBatchesFixture, InsertCopies) {
	BatchBase<BatchBase<int>> batch = power_increases; //Make a copy so that we can compare with the original.

	batch.insert(batch.begin() + 3, 3, one_two);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({power_increases[0], power_increases[1], power_increases[2], one_two, one_two, one_two, power_increases[3], power_increases[4], power_increases[5]})) << "We inserted three copies of one_two in the middle.";

	batch.insert(batch.begin() + 2, 0, one_through_nine);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({power_increases[0], power_increases[1], power_increases[2], one_two, one_two, one_two, power_increases[3], power_increases[4], power_increases[5]})) << "We inserted zero copies, so the batch is unchanged.";

	batch.insert(batch.end(), 4, one);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({power_increases[0], power_increases[1], power_increases[2], one_two, one_two, one_two, power_increases[3], power_increases[4], power_increases[5], one, one, one, one})) << "We appended four copies of one to the end.";
}

/*!
 * Test inserting a range of subbatches into the batch.
 *
 * This range is defined by random access iterators, meaning we can measure the
 * size of it with a simple subtraction.
 */
TEST_F(BatchOfBatchesFixture, InsertRandomAccessIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.insert(batch.begin(), power_increases.begin() + 2, power_increases.begin() + 4);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({
		power_increases[2],
		power_increases[3],
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We inserted two elements of power_increases at the start.";

	batch.insert(batch.begin() + 4, power_increases.begin(), power_increases.begin() + 1);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({
		power_increases[2],
		power_increases[3],
		linear_increases[0],
		linear_increases[1],
		power_increases[0],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We inserted the first subbatch of power_increases in the middle.";

	BatchBase<BatchBase<int>> original = batch; //Nothing should happen here, so keep an original to compare with.
	batch.insert(batch.begin() + 7, power_increases.begin() + 3, power_increases.begin() + 3);
	EXPECT_EQ(batch, original) << "We inserted an empty range, so the batch should remain unchanged.";
}

/*!
 * Test inserting a range of subbatches into the batch.
 *
 * This range is defined by forward iterators, meaning we can't directly measure
 * the size of the range, but we can iterate over it multiple times.
 */
TEST_F(BatchOfBatchesFixture, InsertForwardIterator) {
	BatchBase<BatchBase<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	std::list<BatchBase<int>> subbatches(power_increases.begin(), power_increases.end()); //Std::list uses bidirectional iterators, which are for our purposes just as powerful as forward iterators.
	std::list<BatchBase<int>>::const_iterator partway = subbatches.begin();
	std::list<BatchBase<int>>::const_iterator halfway = subbatches.begin();
	std::advance(partway, 2);
	std::advance(halfway, 4);

	batch.insert(batch.begin(), partway, halfway);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({
		power_increases[2],
		power_increases[3],
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We inserted two elements of power_increases at the start.";

	batch.insert(batch.begin() + 4, subbatches.begin(), partway);
	EXPECT_EQ(batch, BatchBase<BatchBase<int>>({
		power_increases[2],
		power_increases[3],
		linear_increases[0],
		linear_increases[1],
		power_increases[0],
		power_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We inserted the first two subbatches of power_increases in the middle.";

	BatchBase<BatchBase<int>> original = batch; //Nothing should happen here, so keep an original to compare with.
	batch.insert(batch.begin() + 7, partway, partway);
	EXPECT_EQ(batch, original) << "We inserted an empty range, so the batch should remain unchanged.";
}

}