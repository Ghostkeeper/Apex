/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.
#include <list> //For linked lists, a data structure with inherently limited iterators, from which batches must be able to copy.

#include "apex/batch.hpp" //The code under test.
#include "helpers/fuzz_equal_behaviour.hpp" //To test whether batches behave equally to vectors.
#include "helpers/input_iterator_limiter.hpp" //To test with very limited iterator types.

namespace apex {

/*
You might expect there to be some tests for batches of simple elements here.
However the Batch<E> class simply inherits all of its functions from
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
	Batch<int> empty;

	/*!
	 * A batch without any subbatches.
	 */
	Batch<Batch<int>> empty_batch;

	/*!
	 * A batch of batches with linearly increasing sizes: 1, 2, 3, 4, 5
	 * elements.
	 */
	Batch<Batch<int>> linear_increases;

	/*!
	 * A batch with a single element, the number 1.
	 */
	Batch<int> one;

	/*!
	 * A batch with numbers 1 and 2.
	 */
	Batch<int> one_two;

	/*!
	 * A batch with numbers 1, 2, 3, 4, 5, 6, 7, 8 and 9.
	 */
	Batch<int> one_through_nine;

	/*!
	 * A batch of batches with increasing sizes: 1, 2, 4, 8, 16, 32 elements.
	 */
	Batch<Batch<int>> power_increases;

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
	const Batch<Batch<int>> empty;
	EXPECT_EQ(empty.size(), 0) << "The batch is empty after its creation.";
	EXPECT_TRUE(empty.empty()) << "The batch is empty after its creation.";
}

/*!
 * Test constructing a batch of batches with the default constructor for
 * subbatches.
 */
TEST(BatchOfBatches, ConstructDefault) {
	const Batch<Batch<int>> batch1(1);
	EXPECT_EQ(batch1.size(), 1) << "We added one empty subbatch.";
	EXPECT_TRUE(batch1[0].empty()) << "The subbatch was created with its default constructor, which should leave it empty.";

	const Batch<Batch<int>> batch4(4);
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
	const Batch<Batch<int>> batch1_1(1, one);
	EXPECT_EQ(batch1_1.size(), 1) << "There is one subbatch in this batch.";
	EXPECT_EQ(batch1_1[0], one) << "The subbatch is the one with one element.";

	const Batch<Batch<int>> batch4_1(4, one);
	EXPECT_EQ(batch4_1.size(), 4) << "There are four copies of the subbatch in this batch.";
	EXPECT_EQ(batch4_1[0], one) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_1[1], one) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_1[2], one) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_1[3], one) << "The subbatches are copies of the original.";

	const Batch<Batch<int>> batch4_12(4, one_two);
	EXPECT_EQ(batch4_12.size(), 4) << "There are four copies of the subbatch in this batch.";
	EXPECT_EQ(batch4_12[0], one_two) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_12[1], one_two) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_12[2], one_two) << "The subbatches are copies of the original.";
	EXPECT_EQ(batch4_12[3], one_two) << "The subbatches are copies of the original.";

	const Batch<Batch<int>> batch4_19(4, one_through_nine);
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
	const Batch<Batch<int>> batch_full(power_increases.begin(), power_increases.end()); //power_increases is also a batch, which has a random-access iterator.
	EXPECT_EQ(batch_full, power_increases) << "Using the random-access iterator of power_increases, a complete copy of that batch was made.";

	const Batch<Batch<int>> batch_partial(power_increases.begin(), power_increases.begin() + 3); //Construct from a part of the range of the original.
	EXPECT_EQ(batch_partial.size(), 3) << "The range this batch was constructed with contained only 3 elements. Not the entire power_increases batch.";
	EXPECT_EQ(batch_partial[0], power_increases[0]) << "The first subbatch was copied.";
	EXPECT_EQ(batch_partial[1], power_increases[1]) << "The second subbatch was copied.";
	EXPECT_EQ(batch_partial[2], power_increases[2]) << "The third subbatch was copied.";

	const Batch<Batch<int>> batch_last_third(power_increases.begin() + 4, power_increases.end()); //Construct from the last part of the range of the original.
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
	const std::list<Batch<int>> batches = {empty, one, one_two, one_through_nine};

	const Batch<Batch<int>> batch_full(batches.begin(), batches.end());
	EXPECT_EQ(batch_full.size(), 4) << "We put 4 subbatches in the list.";
	EXPECT_EQ(batch_full[0], empty) << "The first element is the empty subbatch.";
	EXPECT_EQ(batch_full[1], one) << "The second element is the subbatch with one element.";
	EXPECT_EQ(batch_full[2], one_two) << "The third element is the subbatch with two elements.";
	EXPECT_EQ(batch_full[3], one_through_nine) << "The fourth element is the subbatch with 9 digits.";

	std::list<Batch<int>>::const_iterator third = batches.begin();
	std::advance(third, 2);
	const Batch<Batch<int>> batch_partial(batches.begin(), third);
	EXPECT_EQ(batch_partial.size(), 2) << "We iterated up until the third element. Everything before that should be included, not including the third element.";
	EXPECT_EQ(batch_partial[0], empty) << "The first element is the empty subbatch.";
	EXPECT_EQ(batch_partial[1], one) << "The second element is the subbatch with one element.";

	const Batch<Batch<int>> batch_second_half(third, batches.end());
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
	InputIteratorLimiter<Batch<Batch<int>>::const_iterator> begin(power_increases.begin());
	InputIteratorLimiter<Batch<Batch<int>>::const_iterator> third(power_increases.begin() + 2);
	InputIteratorLimiter<Batch<Batch<int>>::const_iterator> end(power_increases.end());

	const Batch<Batch<int>> batch_full(begin, end); //Construct with very limited type of iterator.
	EXPECT_EQ(batch_full.size(), power_increases.size()) << "We added the entire batch of batches to this batch.";

	const Batch<Batch<int>> batch_partial(begin, third);
	EXPECT_EQ(batch_partial.size(), 2) << "The first and second subbatches are in, but the third marked the end.";
	EXPECT_EQ(batch_partial[0], power_increases[0]) << "The first subbatch got placed in first place.";
	EXPECT_EQ(batch_partial[1], power_increases[1]) << "The second subbatch got placed in second place.";

	const Batch<Batch<int>> batch_second_half(third, end);
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
	const Batch<Batch<int>> copy_empty(empty_batch);
	EXPECT_EQ(copy_empty, empty_batch);

	const Batch<Batch<int>> copy_filled(power_increases);
	EXPECT_EQ(copy_filled, power_increases);
}

/*!
 * Test the move constructor of batches of batches.
 *
 * This test includes the constraint that the actual element data didn't change
 * position, to give more certainty that the data wasn't actually copied.
 */
TEST_F(BatchOfBatchesFixture, ConstructMove) {
	const Batch<Batch<int>> original_batch(power_increases); //Make a copy so that we can compare the data in the batch without using the decommissioned moved batch.

	const int* original_position = &power_increases[5][5]; //Grab an arbitrary element in the array, noting its position in memory.
	const Batch<Batch<int>> moved_batch(std::move(power_increases));
	const int* new_position = &moved_batch[5][5];

	EXPECT_EQ(moved_batch, original_batch) << "After the move, all element data and subelement data is still unchanged.";
	EXPECT_EQ(original_position, new_position) << "The actual subelement data has not moved in the memory, eliding a copy for better performance.";
}

/*!
 * Test constructing batches of batches from initialiser lists.
 */
TEST(BatchOfBatches, ConstructInitialiserList) {
	const Batch<Batch<int>> empty({});
	EXPECT_EQ(empty.size(), 0) << "We constructed this from an empty initialiser list, so the batch must be empty too.";
	
	const Batch<Batch<int>> batch({{1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11, 12}});
	EXPECT_EQ(batch.size(), 3) << "The initialiser list had 3 members.";
	EXPECT_EQ(batch[0], Batch<int>({1, 2, 3})) << "The first subbatch.";
	EXPECT_EQ(batch[1], Batch<int>({4, 5, 6, 7})) << "The second subbatch.";
	EXPECT_EQ(batch[2], Batch<int>({8, 9, 10, 11, 12})) << "The third subbatch.";
}

/*!
 * Test assigning empty batches to other batches with copy-assignment.
 */
TEST_F(BatchOfBatchesFixture, AssignCopyEmpty) {
	Batch<Batch<int>> assign_empty_to_empty(empty_batch);
	assign_empty_to_empty = empty_batch; //Assign the empty batch again.
	EXPECT_EQ(assign_empty_to_empty, empty_batch) << "After assigning the empty batch, this batch is still empty.";

	Batch<Batch<int>> assign_empty_to_singular({one});
	assign_empty_to_singular = empty_batch;
	EXPECT_EQ(assign_empty_to_singular, empty_batch) << "After assigning the empty batch, this batch is empty.";

	Batch<Batch<int>> assign_empty_to_filled(power_increases);
	assign_empty_to_filled = empty_batch;
	EXPECT_EQ(assign_empty_to_filled, empty_batch) << "After assigning the empty batch, this batch is empty.";
}

/*!
 * Test assigning filled batches to other batches with copy-assignment.
 */
TEST_F(BatchOfBatchesFixture, AssignCopyFilled) {
	Batch<Batch<int>> assign_empty(empty_batch);
	assign_empty = power_increases; //Assign a filled batch to this empty batch.
	EXPECT_EQ(assign_empty, power_increases) << "After assigning this batch to the batch, it must be equal to the new batch.";

	Batch<Batch<int>> assign_singular({one});
	assign_singular = power_increases;
	EXPECT_EQ(assign_singular, power_increases) << "After assigning this batch, it must be equal to this batch.";

	Batch<Batch<int>> assign_filled(linear_increases);
	assign_filled = power_increases;
	EXPECT_EQ(assign_filled, power_increases) << "After assigning this batch, it must be equal to this batch.";
}

/*!
 * Test assigning empty batches to other batches with move-assignment.
 */
TEST_F(BatchOfBatchesFixture, AssignMoveEmpty) {
	Batch<Batch<int>> empty_copy_1(empty_batch); //Make a copy of the empty batch we're about to move, so we can still compare with the original.
	Batch<Batch<int>> assign_empty_to_empty(empty_batch);
	assign_empty_to_empty = std::move(empty_copy_1); //Assign the empty batch again.
	EXPECT_EQ(assign_empty_to_empty, empty_batch) << "After assigning the empty batch, this batch is still empty.";

	Batch<Batch<int>> empty_copy_2(empty_batch); //Don't re-use the moved batch ever again!
	Batch<Batch<int>> assign_empty_to_singular({one});
	assign_empty_to_singular = std::move(empty_copy_2);
	EXPECT_EQ(assign_empty_to_singular, empty_batch) << "After assigning the empty batch, this batch is empty.";

	Batch<Batch<int>> empty_copy_3(empty_batch); //Don't re-use the moved batch ever again!
	Batch<Batch<int>> assign_empty_to_filled(power_increases);
	assign_empty_to_filled = std::move(empty_copy_3);
	EXPECT_EQ(assign_empty_to_filled, empty_batch) << "After assigning the empty batch, this batch is empty.";
}

/*!
 * Test assigning filled batches to other batches with move-assignment.
 */
TEST_F(BatchOfBatchesFixture, AssignMoveFilled) {
	Batch<Batch<int>> powers_copy_1(power_increases); //Make a copy of the batch we're about to move, so we can still compare with the original.
	Batch<Batch<int>> assign_empty(empty_batch);
	assign_empty = std::move(powers_copy_1); //Assign a filled batch to this empty batch.
	EXPECT_EQ(assign_empty, power_increases) << "After assigning this batch to the batch, it must be equal to the new batch.";

	Batch<Batch<int>> powers_copy_2(power_increases); //Don't re-use the moved batch ever again!
	Batch<Batch<int>> assign_singular({one});
	assign_singular = std::move(powers_copy_2);
	EXPECT_EQ(assign_singular, power_increases) << "After assigning this batch, it must be equal to this batch.";

	Batch<Batch<int>> powers_copy_3(power_increases); //Don't re-use the moved batch ever again!
	Batch<Batch<int>> assign_filled(linear_increases);
	assign_filled = std::move(powers_copy_3);
	EXPECT_EQ(assign_filled, power_increases) << "After assigning this batch, it must be equal to this batch.";
}

/*!
 * Test equality of various types of batches of batches that are in fact equal.
 */
TEST_F(BatchOfBatchesFixture, CompareEqualityEqual) {
	//Empty containers are always equal.
	const Batch<Batch<int>> empty;
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
	const Batch<Batch<int>> left({{3, 2, 1}, {7, 6, 5, 4}, {}, {9, 8}});
	const Batch<Batch<int>> right({{3, 2, 1}, {7, 6, 5, 4}, {}, {9, 8}});
	EXPECT_TRUE(left == right) << "These batches are constructed from the same initialiser lists, so they must be equal.";
	EXPECT_FALSE(left != right) << "These batches are constructed from the same initialiser lists, so they must be equal.";
}

/*!
 * Test equality of batches that have different numbers of subbatches.
 */
TEST_F(BatchOfBatchesFixture, CompareEqualityDifferentSize) {
	const Batch<Batch<int>> empty_subbatch({{}}); //Has one subbatch, which is empty.
	EXPECT_FALSE(empty_batch == empty_subbatch) << "empty_subbatch contains one subbatch, while the other does not.";
	EXPECT_FALSE(empty_subbatch == empty_batch) << "empty_subbatch contains one subbatch, while the other does not.";
	EXPECT_TRUE(empty_batch != empty_subbatch) << "empty_subbatch contains one subbatch, while the other does not.";
	EXPECT_TRUE(empty_subbatch != empty_batch) << "empty_subbatch contains one subbatch, while the other does not.";

	const Batch<Batch<int>> two_empty_subbatches({{}, {}});
	EXPECT_FALSE(empty_subbatch == two_empty_subbatches) << "One batch has one empty subbatch, while the other has two empty subbatches.";
	EXPECT_FALSE(two_empty_subbatches == empty_subbatch) << "One batch has one empty subbatch, while the other has two empty subbatches.";
	EXPECT_TRUE(empty_subbatch != two_empty_subbatches) << "One batch has one empty subbatch, while the other has two empty subbatches.";
	EXPECT_TRUE(two_empty_subbatches != empty_subbatch) << "One batch has one empty subbatch, while the other has two empty subbatches.";

	const Batch<Batch<int>> just_one({one});
	const Batch<Batch<int>> just_one_twice({one, one});
	EXPECT_FALSE(just_one == just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch. Their size is different.";
	EXPECT_FALSE(just_one_twice == just_one) << "The just_one batch is a prefix of the just_one_twice batch. Their size is different.";
	EXPECT_TRUE(just_one != just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch. Their size is different.";
	EXPECT_TRUE(just_one_twice != just_one) << "The just_one batch is a prefix of the just_one_twice batch. Their size is different.";

	const Batch<Batch<int>> just_one_many({one, one_through_nine});
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
	const Batch<Batch<int>> just_one({one});
	const Batch<Batch<int>> just_one_two({one_two});
	EXPECT_FALSE(just_one == just_one_two) << "One batch has a subbatch with 1 element, while the other has a subbatch with 2 elements.";
	EXPECT_FALSE(just_one_two == just_one) << "One batch has a subbatch with 1 element, while the other has a subbatch with 2 elements.";
	EXPECT_TRUE(just_one != just_one_two) << "One batch has a subbatch with 1 element, while the other has a subbatch with 2 elements.";
	EXPECT_TRUE(just_one_two != just_one) << "One batch has a subbatch with 1 element, while the other has a subbatch with 2 elements.";

	const Batch<Batch<int>> one_onetwo({one, one_two});
	const Batch<Batch<int>> one_many({one, one_through_nine});
	EXPECT_FALSE(one_onetwo == one_many) << "The first subbatch is the same for each, but the second subbatch is longer in one batch.";
	EXPECT_FALSE(one_many == one_onetwo) << "The first subbatch is the same for each, but the second subbatch is longer in one batch.";
	EXPECT_TRUE(one_onetwo != one_many) << "The first subbatch is the same for each, but the second subbatch is longer in one batch.";
	EXPECT_TRUE(one_many != one_onetwo) << "The first subbatch is the same for each, but the second subbatch is longer in one batch.";
}

/*!
 * Test equality of batches where only the subvalues are different.
 */
TEST(BatchOfBatches, CompareEqualityDifferentValues) {
	const Batch<Batch<int>> just_one({{1}});
	const Batch<Batch<int>> just_two({{2}});
	EXPECT_FALSE(just_one == just_two) << "One subbatch has the number 1, the other has the number 2.";
	EXPECT_FALSE(just_two == just_one) << "One subbatch has the number 1, the other has the number 2.";
	EXPECT_TRUE(just_one != just_two) << "One subbatch has the number 1, the other has the number 2.";
	EXPECT_TRUE(just_two != just_one) << "One subbatch has the number 1, the other has the number 2.";

	const Batch<Batch<int>> onetwo_threefour({{1, 2}, {3, 4}});
	const Batch<Batch<int>> onetwo_fourthree({{1, 2}, {4, 3}});
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
	const Batch<Batch<int>> empty;
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
	const Batch<Batch<int>> left({{3, 2, 1}, {7, 6, 5, 4}, {}, {9, 8}});
	const Batch<Batch<int>> right({{3, 2, 1}, {7, 6, 5, 4}, {}, {9, 8}});
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
	const Batch<Batch<int>> empty_subbatch({{}}); //Has one subbatch, which is empty.
	EXPECT_TRUE(empty_batch < empty_subbatch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_TRUE(empty_batch <= empty_subbatch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_FALSE(empty_batch > empty_subbatch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_FALSE(empty_batch >= empty_subbatch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_FALSE(empty_subbatch < empty_batch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_FALSE(empty_subbatch <= empty_batch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_TRUE(empty_subbatch > empty_batch) << "The empty batch is a prefix of all other batches so it is always sorted first.";
	EXPECT_TRUE(empty_subbatch >= empty_batch) << "The empty batch is a prefix of all other batches so it is always sorted first.";

	const Batch<Batch<int>> two_empty_subbatches({{}, {}});
	EXPECT_TRUE(empty_subbatch < two_empty_subbatches) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_TRUE(empty_subbatch <= two_empty_subbatches) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_FALSE(empty_subbatch > two_empty_subbatches) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_FALSE(empty_subbatch >= two_empty_subbatches) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_FALSE(two_empty_subbatches < empty_subbatch) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_FALSE(two_empty_subbatches <= empty_subbatch) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_TRUE(two_empty_subbatches > empty_subbatch) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";
	EXPECT_TRUE(two_empty_subbatches >= empty_subbatch) << "The single subbatch is a prefix of the two subbatches, so it should be sorted first.";

	const Batch<Batch<int>> just_one({one});
	const Batch<Batch<int>> just_one_twice({one, one});
	EXPECT_TRUE(just_one < just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_TRUE(just_one <= just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_FALSE(just_one > just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_FALSE(just_one >= just_one_twice) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_FALSE(just_one_twice < just_one) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_FALSE(just_one_twice <= just_one) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_TRUE(just_one_twice > just_one) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";
	EXPECT_TRUE(just_one_twice >= just_one) << "The just_one batch is a prefix of the just_one_twice batch, so it should be sorted first.";

	const Batch<Batch<int>> just_one_many({one, one_through_nine});
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

	const Batch<Batch<int>> just_one_thrice({one, one, one});
	const Batch<Batch<int>> one_onetwo_zero({one, one_two, {0}});
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
	const Batch<Batch<int>> just_one({{1}});
	const Batch<Batch<int>> just_two({{2}});
	EXPECT_TRUE(just_one < just_two) << "The number 1 is lower than the number 2.";
	EXPECT_TRUE(just_one <= just_two) << "The number 1 is lower than the number 2.";
	EXPECT_FALSE(just_one > just_two) << "The number 1 is lower than the number 2.";
	EXPECT_FALSE(just_one >= just_two) << "The number 1 is lower than the number 2.";
	EXPECT_FALSE(just_two < just_one) << "The number 1 is lower than the number 2.";
	EXPECT_FALSE(just_two <= just_one) << "The number 1 is lower than the number 2.";
	EXPECT_TRUE(just_two > just_one) << "The number 1 is lower than the number 2.";
	EXPECT_TRUE(just_two >= just_one) << "The number 1 is lower than the number 2.";

	const Batch<Batch<int>> onetwo_threefour({{1, 2}, {3, 4}});
	const Batch<Batch<int>> onetwo_fourthree({{1, 2}, {4, 3}});
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
	const Batch<Batch<int>> one_two_three({{1}, {2}, {3}});
	const Batch<Batch<int>> one_three_two({{1}, {3}, {2}});
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
	Batch<Batch<int>> batch; //Start off empty.

	batch.assign(20, one_two);
	EXPECT_EQ(batch.size(), 20);
	for(Subbatch<int>& subbatch : batch) {
		EXPECT_EQ(subbatch, one_two);
	}

	//In this same batch, assign something else, confirming that the old content is erased.
	batch.assign(10, one);
	EXPECT_EQ(batch.size(), 10);
	for(Subbatch<int>& subbatch : batch) {
		EXPECT_EQ(subbatch, one);
	}

	//Now assign something bigger.
	batch.assign(30, one_through_nine);
	EXPECT_EQ(batch.size(), 30);
	for(Subbatch<int>& subbatch : batch) {
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
	Batch<Batch<int>> batch = linear_increases;

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
	Batch<Batch<int>> batch = linear_increases;

	//A linked list offers a bidirectional iterator, which is a forward iterator but not random access.
	const std::list<Batch<int>> batches = {empty, one, one_two, one_through_nine};
	batch.assign(batches.begin(), batches.end());
	EXPECT_EQ(batch.size(), 4) << "We put 4 subbatches in the list.";
	EXPECT_EQ(batch[0], empty) << "The first element is the empty subbatch.";
	EXPECT_EQ(batch[1], one) << "The second element is the subbatch with one element.";
	EXPECT_EQ(batch[2], one_two) << "The third element is the subbatch with two elements.";
	EXPECT_EQ(batch[3], one_through_nine) << "The fourth element is the subbatch with 9 digits.";

	std::list<Batch<int>>::const_iterator third = batches.begin();
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
	Batch<Batch<int>> batch = linear_increases;
	InputIteratorLimiter<Batch<Batch<int>>::const_iterator> begin(power_increases.begin());
	InputIteratorLimiter<Batch<Batch<int>>::const_iterator> third(power_increases.begin() + 2);
	InputIteratorLimiter<Batch<Batch<int>>::const_iterator> end(power_increases.end());

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
	Batch<Batch<int>> batch = power_increases;

	batch.assign({one, one_through_nine, one_two});
	EXPECT_EQ(batch.size(), 3) << "We assigned a list of 3 subbatches to this batch.";
	EXPECT_EQ(batch[0], one) << "The first subbatch in the initialiser list was the \"one\" batch.";
	EXPECT_EQ(batch[1], one_through_nine) << "The second subbatch in the initialiser list was one_through_nine.";
	EXPECT_EQ(batch[2], one_two) << "The third subbatch in the initialiser list was one_two.";

	batch.assign({});
	EXPECT_EQ(batch.size(), 0) << "We assigned an empty initialiser list to this batch.";

	batch.assign({{1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11, 12}});
	EXPECT_EQ(batch.size(), 3) << "The initialiser list had 3 members.";
	EXPECT_EQ(batch[0], Batch<int>({1, 2, 3})) << "The first subbatch.";
	EXPECT_EQ(batch[1], Batch<int>({4, 5, 6, 7})) << "The second subbatch.";
	EXPECT_EQ(batch[2], Batch<int>({8, 9, 10, 11, 12})) << "The third subbatch.";
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

	Batch<Batch<int>> batch({one_two}); //A batch with a single item inside.
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
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace(batch.end()); //Emplace at the end.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{}
	})) << "There's an empty subbatch at the end now.";

	batch.emplace(batch.begin()); //Emplace another one at the beginning.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{},
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{}
	})) << "There's an empty subbatch at the start and end now.";

	batch.emplace(batch.begin() + 3); //Emplace another one somewhere in the middle.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{},
		linear_increases[0],
		linear_increases[1],
		{},
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{}
	})) << "There's still an empty subbatch at the start and end, and a new one in 4th place.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the constructor that
 * makes copies of a subvalue.
 */
TEST_F(BatchOfBatchesFixture, EmplaceCopies) {
	Batch<Batch<int>> batch = power_increases; //Make a copy so that we can compare with the original.

	batch.emplace(batch.end(), size_t(4), 1); //Add a subbatch with four 1's at the end.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		{1, 1, 1, 1}})) << "There is a subbatch with four 1's appended to the end.";

	batch.emplace(batch.begin(), size_t(3), 2); //Add a subbatch with three 2's in the beginning.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{2, 2, 2},
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		{1, 1, 1, 1}
	})) << "We added a subbatch with three 2's at the start.";

	batch.emplace(batch.begin() + 2, size_t(2), 3); //Add a subbatch with two 3's in the middle.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{2, 2, 2},
		power_increases[0],
		{3, 3},
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		{1, 1, 1, 1}
	})) << "We added a subbatch with two 3's in the third place.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the constructor that
 * takes iterators.
 *
 * The iterators provided in this test are random access, meaning that the
 * distance between them can be obtained in constant time.
 */
TEST_F(BatchOfBatchesFixture, EmplaceRandomAccessIterator) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace(batch.end(), one_through_nine.begin(), one_through_nine.begin() + 4); //Append a subbatch with 4 elements at the end.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{1, 2, 3, 4}
	})) << "There is a part of the one_through_nine batch at the end.";

	batch.emplace(batch.begin(), batch[3].begin(), batch[3].end()); //Re-add a part of itself. Does the index get invalidated prematurely?
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[3],
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{1, 2, 3, 4}
	})) << "A part of the batch itself was added again. This should not invalidate the index in the batch until it's completed.";

	batch.emplace(batch.begin() + 4, one.begin(), one.begin()); //Add an empty subbatch through random access iterators.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[3],
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		{},
		linear_increases[3],
		linear_increases[4],
		{1, 2, 3, 4}
	})) << "We added an empty subbatch in the middle (no distance between start and end iterators).";
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
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	std::list<int> numbers(one_through_nine.begin(), one_through_nine.end()); //A linked list has bidirectional iterators, which is a forward iterator but not random access.

	batch.emplace(batch.end(), numbers.begin(), numbers.begin()); //Append an empty subbatch at the end.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{}
	})) << "There is an empty subbatch at the end now.";

	std::list<int>::const_iterator fifth = numbers.begin();
	std::advance(fifth, 4);
	batch.emplace(batch.begin(), numbers.cbegin(), fifth); //Prepend part of this list at the start.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{1, 2, 3, 4},
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{}
	})) << "A part of the numbers list is added as a subbatch in front.";

	std::list<int>::const_iterator second = numbers.begin();
	std::advance(second, 1);
	batch.emplace(batch.begin() + 4, second, fifth); //Insert a subbatch in the middle too.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{1, 2, 3, 4},
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		{2, 3, 4},
		linear_increases[3],
		linear_increases[4],
		{}
	})) << "We added a new subbatch in the middle.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the constructor that
 * takes iterators.
 *
 * The iterators provided in this test are input iterators only, meaning that we
 * can't calculate the number of items beforehand.
 */
TEST_F(BatchOfBatchesFixture, EmplaceInputIterator) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	InputIteratorLimiter<Batch<int>::const_iterator> begin(one_through_nine.begin()); //Create some very limited input iterators to test with.
	InputIteratorLimiter<Batch<int>::const_iterator> fifth(one_through_nine.begin() + 4);
	InputIteratorLimiter<Batch<int>::const_iterator> end(one_through_nine.end());

	batch.emplace(batch.end(), begin, fifth);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{1, 2, 3, 4}
	})) << "There is a new subbatch at the end.";

	batch.emplace(batch.begin(), fifth, fifth); //Empty range!
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{},
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{1, 2, 3, 4}
	})) << "We added an empty subbatch at the start.";

	batch.emplace(batch.begin() + 4, fifth, end);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{},
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		{5, 6, 7, 8, 9},
		linear_increases[3],
		linear_increases[4],
		{1, 2, 3, 4}
	})) << "We added the second half of one_through_nine in the middle.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the copy constructor.
 *
 * Behaviourally this should be identical to just inserting the copy.
 */
TEST_F(BatchOfBatchesFixture, EmplaceCopy) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace(batch.begin(), one_through_nine);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We added the one_through_nine batch at the front.";

	batch.emplace(batch.end(), one);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		one
	})) << "We added the one batch at the end.";

	batch.emplace(batch.begin() + 2, one_two);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		one_two,
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		one
	})) << "We added the one_two batch in the middle.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the move constructor.
 *
 * Behaviourally this should be identical to just inserting the subbatch.
 */
TEST_F(BatchOfBatchesFixture, EmplaceMove) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy of each input, so that we can compare with the original.
	Batch<int> one_through_nine_copy = one_through_nine;
	Batch<int> one_copy = one;
	Batch<int> one_two_copy = one_two;

	batch.emplace(batch.begin(), std::move(one_through_nine_copy));
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We added the one_through_nine batch at the front.";

	batch.emplace(batch.end(), std::move(one_copy));
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		one
	})) << "We added the one batch at the end.";

	batch.emplace(batch.begin() + 2, std::move(one_two_copy));
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		one_two,
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		one
	})) << "We added the one_two batch in the middle.";
}

/*!
 * Test emplacing a subbatch into the parent batch with the constructor that
 * takes an initialiser list.
 */
TEST_F(BatchOfBatchesFixture, EmplaceInitialiserList) {
	Batch<Batch<int>> batch = power_increases; //Make a copy so that we can compare with the original.

	batch.emplace(batch.begin(), {9, 8, 7});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{9, 8, 7},
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5]
	})) << "We added {9, 8, 7} at the front.";

	batch.emplace(batch.end(), {6});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{9, 8, 7},
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		{6}
	})) << "We added {6} at the end.";

	batch.emplace(batch.begin() + 3, {});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{9, 8, 7},
		power_increases[0],
		power_increases[1],
		{},
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		{6}
	})) << "We added an empty subbatch in the middle.";

	batch.emplace(batch.begin() + 4, {5, 4, 3, 2, 1});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		{9, 8, 7},
		power_increases[0],
		power_increases[1],
		{},
		{5, 4, 3, 2, 1},
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		{6}
	})) << "We added {5, 4, 3, 2, 1} in the middle, just after the empty batch we added earlier.";
}

/*!
 * Test emplacing an empty subbatch onto the back.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackEmpty) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace_back();
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{}
	})) << "We added an empty subbatch at the end.";

	batch.emplace_back();
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{},
		{}
	})) << "We added a second empty subbatch at the end.";

	empty_batch.emplace_back(); //Test starting from an empty batch.
	EXPECT_EQ(empty_batch, Batch<Batch<int>>({{}})) << "We added an empty subbatch to the previously empty batch.";
}

/*!
 * Test emplacing a new subbatch onto the back containing copies of a specific
 * subelement.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackCopies) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace_back(size_t(4), 1);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{1, 1, 1, 1}
	})) << "We constructed a subbatch containing four 1's at the end.";

	empty_batch.emplace_back(size_t(5), 2);
	EXPECT_EQ(empty_batch, Batch<Batch<int>>({{2, 2, 2, 2, 2}})) << "We constructed a subbatch containing five 2's at the end of the previously empty batch.";
}

/*!
 * Test emplacing a new subbatch onto the back from a pair of iterators defining
 * the contents of that subbatch.
 *
 * The iterators used in this test are random access, meaning we can directly
 * measure how many items are in that range.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackRandomAccessIterator) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.emplace_back(one_through_nine.begin() + 4, one_through_nine.end());
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{5, 6, 7, 8, 9}
	})) << "We constructed the second half of one_through_nine as a new subbatch at the end.";

	empty_batch.emplace_back(one_through_nine.begin() + 3, one_through_nine.begin() + 3); //Try an empty range.
	EXPECT_EQ(empty_batch, Batch<Batch<int>>({{}})) << "We added an empty range to this previously empty batch, so that it now contains one subbatch which is itself empty.";

	empty_batch.emplace_back(one_through_nine.begin(), one_through_nine.begin() + 4);
	EXPECT_EQ(empty_batch, Batch<Batch<int>>({{}, {1, 2, 3, 4}})) << "We appended the first half of one_through_nine to the batch that previously contained just an empty subbatch.";
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
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	std::list<int> numbers(one_through_nine.begin(), one_through_nine.end()); //Linked list which has bidirectional iterators, which are just as good as forward iterators for our purpose.
	std::list<int>::iterator halfway = numbers.begin();
	std::advance(halfway, 4);

	batch.emplace_back(halfway, numbers.end());
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{5, 6, 7, 8, 9}
	})) << "We constructed the second half of numbers as a new subbatch at the end.";

	empty_batch.emplace_back(halfway, halfway); //Try an empty range.
	EXPECT_EQ(empty_batch, Batch<Batch<int>>({{}})) << "We added an empty range to this previously empty batch, so that it now contains one subbatch which is itself empty.";

	empty_batch.emplace_back(numbers.begin(), halfway);
	EXPECT_EQ(empty_batch, Batch<Batch<int>>({{}, {1, 2, 3, 4}})) << "We appended the first half of numbers to the batch that previously contained just an empty subbatch.";
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
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	InputIteratorLimiter<Batch<int>::const_iterator> begin(one_through_nine.begin());
	InputIteratorLimiter<Batch<int>::const_iterator> middle(one_through_nine.begin() + 4);
	InputIteratorLimiter<Batch<int>::const_iterator> end(one_through_nine.end());

	batch.emplace_back(middle, end);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{5, 6, 7, 8, 9}
	})) << "We constructed the second half of one_through_nine as a new subbatch at the end.";

	empty_batch.emplace_back(middle, middle); //Try an empty range.
	EXPECT_EQ(empty_batch, Batch<Batch<int>>({{}})) << "We added an empty range to this previously empty batch, so that it now contains one subbatch which is itself empty.";

	empty_batch.emplace_back(begin, middle);
	EXPECT_EQ(empty_batch, Batch<Batch<int>>({{}, {1, 2, 3, 4}})) << "We appended the first half of one_through_nine to the batch that previously contained just an empty subbatch.";
}

/*!
 * Test emplacing a new subbatch onto the back using the copy constructor.
 *
 * A bit pointless, since the copy constructor makes a copy anyway so why going
 * through the trouble of using emplace rather than push_back, but it keeps the
 * interface consistent.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackCopy) {
	Batch<Batch<int>> batch = power_increases;

	batch.emplace_back(one_two);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		one_two
	})) << "We added a copy of one_two to the end.";
	batch.emplace_back(one);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		one_two,
		one
	})) << "We added a copy of one to the end.";
	batch.emplace_back(one_through_nine);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		one_two,
		one,
		one_through_nine
	})) << "We added a copy of one_through_nine to the end.";
}

/*!
 * Test emplacing a new subbatch onto the back using the move constructor.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackMove) {
	Batch<Batch<int>> batch = power_increases;

	batch.emplace_back(std::move(one_two));
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		one_two
	})) << "We moved one_two onto the end.";
	batch.emplace_back(std::move(one));
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		one_two,
		one
	})) << "We moved one onto the end.";
	batch.emplace_back(std::move(one_through_nine));
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		one_two,
		one,
		one_through_nine
	})) << "We moved one_through_nine onto the end.";
}

/*!
 * Test emplacing a new subbatch onto the back using the constructor that takes
 * an initialiser list.
 */
TEST_F(BatchOfBatchesFixture, EmplaceBackInitialiserList) {
	Batch<Batch<int>> batch = linear_increases;

	batch.emplace_back({6, 5, 4});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{6, 5, 4}
	})) << "We added {6, 5, 4} to the end.";
	batch.emplace_back({});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{6, 5, 4},
		{}
	})) << "We added an empty initialiser list to the end.";
	batch.emplace_back({1});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{6, 5, 4},
		{},
		{1}
	})) << "We added {1} to the end.";
}

/*!
 * Test inserting a subbatch into a batch by making a copy.
 */
TEST_F(BatchOfBatchesFixture, InsertCopy) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.insert(batch.begin(), one_through_nine);
	EXPECT_EQ(batch.size(), linear_increases.size() + 1) << "We added a new subbatch, so the size grew by one.";
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We added the one_through_nine batch at the front.";

	batch.insert(batch.end(), one);
	EXPECT_EQ(batch.size(), linear_increases.size() + 2) << "We added another subbatch, so the size grew again.";
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		one
	})) << "We added the one batch at the end.";

	batch.insert(batch.begin() + 2, one_two);
	EXPECT_EQ(batch.size(), linear_increases.size() + 3) << "We added a third subbatch.";
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		one_two,
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		one
	})) << "We added the one_two batch in the middle.";
}

/*!
 * Test inserting a subbatch into a batch by moving it in.
 */
TEST_F(BatchOfBatchesFixture, InsertMove) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy of each input, so that we can compare with the original.
	Batch<int> one_through_nine_copy = one_through_nine;
	Batch<int> one_copy = one;
	Batch<int> one_two_copy = one_two;

	batch.insert(batch.begin(), std::move(one_through_nine_copy));
	EXPECT_EQ(batch.size(), linear_increases.size() + 1) << "We added a new subbatch, so the size grew by one.";
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We added the one_through_nine batch at the front.";

	batch.insert(batch.end(), std::move(one_copy));
	EXPECT_EQ(batch.size(), linear_increases.size() + 2) << "We added another subbatch, so the size grew again.";
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		one
	})) << "We added the one batch at the end.";

	batch.insert(batch.begin() + 2, std::move(one_two_copy));
	EXPECT_EQ(batch.size(), linear_increases.size() + 3) << "We added a third subbatch.";
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_through_nine,
		linear_increases[0],
		one_two,
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		one
	})) << "We added the one_two batch in the middle.";
}

/*!
 * Test inserting repeated copies of a subbatch into the batch.
 */
TEST_F(BatchOfBatchesFixture, InsertCopies) {
	Batch<Batch<int>> batch = power_increases; //Make a copy so that we can compare with the original.

	batch.insert(batch.begin() + 3, 3, one_two);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		one_two,
		one_two,
		one_two,
		power_increases[3],
		power_increases[4],
		power_increases[5]
	})) << "We inserted three copies of one_two in the middle.";

	batch.insert(batch.begin() + 2, 0, one_through_nine);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		one_two,
		one_two,
		one_two,
		power_increases[3],
		power_increases[4],
		power_increases[5]
	})) << "We inserted zero copies, so the batch is unchanged.";

	batch.insert(batch.end(), 4, one);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		one_two,
		one_two,
		one_two,
		power_increases[3],
		power_increases[4],
		power_increases[5],
		one,
		one,
		one,
		one
	})) << "We appended four copies of one to the end.";
}

/*!
 * Test inserting a range of subbatches into the batch.
 *
 * This range is defined by random access iterators, meaning we can measure the
 * size of it with a simple subtraction.
 */
TEST_F(BatchOfBatchesFixture, InsertRandomAccessIterator) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.

	batch.insert(batch.begin(), power_increases.begin() + 2, power_increases.begin() + 4);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[2],
		power_increases[3],
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We inserted two elements of power_increases at the start.";

	batch.insert(batch.begin() + 4, power_increases.begin(), power_increases.begin() + 1);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[2],
		power_increases[3],
		linear_increases[0],
		linear_increases[1],
		power_increases[0],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We inserted the first subbatch of power_increases in the middle.";

	Batch<Batch<int>> original = batch; //Nothing should happen here, so keep an original to compare with.
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
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	std::list<Batch<int>> subbatches(power_increases.begin(), power_increases.end()); //Std::list uses bidirectional iterators, which are for our purposes just as powerful as forward iterators.
	std::list<Batch<int>>::const_iterator partway = subbatches.begin();
	std::list<Batch<int>>::const_iterator halfway = subbatches.begin();
	std::advance(partway, 2);
	std::advance(halfway, 4);

	batch.insert(batch.begin(), partway, halfway);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[2],
		power_increases[3],
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We inserted two elements of power_increases at the start.";

	batch.insert(batch.begin() + 4, subbatches.cbegin(), partway);
	EXPECT_EQ(batch, Batch<Batch<int>>({
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

	Batch<Batch<int>> original = batch; //Nothing should happen here, so keep an original to compare with.
	batch.insert(batch.begin() + 7, partway, partway);
	EXPECT_EQ(batch, original) << "We inserted an empty range, so the batch should remain unchanged.";
}

/*!
 * Test inserting a range of subbatches into the batch.
 *
 * This range is defined by input iterators, meaning we can't directly measure
 * the size of the range nor iterate multiple times over it. These are the most
 * limited type of iterators.
 */
TEST_F(BatchOfBatchesFixture, InsertInputIterator) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare with the original.
	InputIteratorLimiter<Batch<Batch<int>>::const_iterator> begin(power_increases.begin());
	InputIteratorLimiter<Batch<Batch<int>>::const_iterator> middle(power_increases.begin() + 4);
	InputIteratorLimiter<Batch<Batch<int>>::const_iterator> end(power_increases.end());

	batch.insert(batch.begin(), middle, end);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[4],
		power_increases[5],
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We inserted two elements of power_increases at the start.";

	batch.insert(batch.begin() + 4, begin, middle);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[4],
		power_increases[5],
		linear_increases[0],
		linear_increases[1],
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4]
	})) << "We inserted the first four subbatches of power_increases in the middle.";

	Batch<Batch<int>> original = batch; //Nothing should happen here, so keep an original to compare with.
	batch.insert(batch.begin() + 7, middle, middle);
	EXPECT_EQ(batch, original) << "We inserted an empty range, so the batch should remain unchanged.";
}

/*!
 * Test inserting a list of subbatches into the batch via initialiser lists.
 */
TEST_F(BatchOfBatchesFixture, InsertInitialiserList) {
	Batch<Batch<int>> batch = power_increases; //Make a copy so that we can compare with the original.

	batch.insert(batch.begin(), {one_two, one});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_two,
		one,
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5]
	})) << "We inserted a list of two subbatches at the start.";

	batch.insert(batch.begin() + 3, {one_through_nine, one, one});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_two,
		one,
		power_increases[0],
		one_through_nine,
		one,
		one,
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5]
	})) << "We inserted three more subbatches in the middle.";

	Batch<Batch<int>> original = batch; //Nothing should happen here, so keep an original to compare with.
	batch.insert(batch.begin() + 8, {});
	EXPECT_EQ(batch, original) << "We inserted an empty initialiser list, so the batch should remain unchanged.";

	batch.insert(batch.end(), {one_two});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_two,
		one,
		power_increases[0],
		one_through_nine,
		one,
		one,
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		one_two
	})) << "We inserted a subbatch at the end.";
}

/*!
 * Test appending a new subbatch to the end by making a copy of it.
 */
TEST_F(BatchOfBatchesFixture, PushBackCopy) {
	Batch<Batch<int>> batch;
	batch.push_back(one_two);
	EXPECT_EQ(batch, Batch<Batch<int>>({one_two})) << "We added a subbatch to the end of an empty batch, so now the batch contains just that one subbatch.";

	batch.push_back({});
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_two,
		{}
	})) << "We added an empty subbatch at the end.";

	batch = linear_increases; //Make a copy so that we can compare with the original.
	batch.push_back(one_through_nine);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		one_through_nine
	})) << "We appended one_through_nine to the end of it.";
}

/*!
 * Test appending a new subbatch to the end by moving it.
 */
TEST_F(BatchOfBatchesFixture, PushBackMove) {
	Batch<Batch<int>> batch;
	Batch<int> one_two_copy = one_two; //Make a copy that we can move while leaving the original intact to compare with.
	batch.push_back(std::move(one_two_copy));
	EXPECT_EQ(batch, Batch<Batch<int>>({one_two})) << "We added a subbatch to the end of an empty batch, so now the batch contains just that one subbatch.";

	Batch<int> empty;
	batch.push_back(std::move(empty));
	EXPECT_EQ(batch, Batch<Batch<int>>({
		one_two,
		{}
	})) << "We added an empty subbatch at the end.";

	batch = linear_increases; //Make a copy so that we can compare with the original.
	Batch<int> one_through_nine_copy = one_through_nine;
	batch.push_back(std::move(one_through_nine_copy));
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		one_through_nine
	})) << "We appended one_through_nine to the end of it.";
}

/*!
 * Test appending a new subbatch to the end by making a copy of it.
 */
TEST_F(BatchOfBatchesFixture, PushBackCopySubbatch) {
	Batch<Batch<int>> batch;
	batch.push_back(power_increases[3]);
	EXPECT_EQ(batch, Batch<Batch<int>>({power_increases[3]})) << "We added a subbatch to the end of an empty batch, so now the batch contains just that one subbatch.";

	batch = linear_increases; //Make a copy so that we can compare with the original.
	batch.push_back(power_increases[2]);
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		power_increases[2]
	})) << "We appended power_increases[2] to the end of it.";
}

/*!
 * Test appending a new subbatch to the end by moving it.
 */
TEST_F(BatchOfBatchesFixture, PushBackMoveSubbatch) {
	Batch<Batch<int>> batch;
	Batch<Batch<int>> power_increases_copy = power_increases; //Make a copy that we can move while leaving the original intact to compare with.
	batch.push_back(std::move(power_increases_copy[5]));
	EXPECT_EQ(batch, Batch<Batch<int>>({power_increases[5]})) << "We added a subbatch to the end of an empty batch, so now the batch contains just that one subbatch.";

	batch = linear_increases; //Make a copy so that we can compare with the original.
	batch.push_back(std::move(power_increases[3]));
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		power_increases[3]
	})) << "We appended power_increases[3] to the end of it.";
}

/*!
 * Test reserving memory for the subelements, by looking if iterators for
 * subelements are not invalidated for at least the reserved number of
 * subelements.
 */
TEST_F(BatchOfBatchesFixture, ReserveSubelementsRetainsIterators) {
	Batch<Batch<int>> batch = {one, one_two}; //Have something that we can refer to before reserving more subelements.
	batch.reserve_subelements(one.size() + one_two.size() + one_through_nine.size()); //Reserve enough to contain all data we'll put in it.

	//Now store iterators to the current data. They must not get invalidated when we add more data.
	Batch<int>::const_iterator first = batch[0].begin();
	Batch<int>::const_iterator second = batch[1].begin();
	Batch<int>::const_iterator third = batch[1].begin() + 1;

	batch.push_back(one_through_nine); //Much bigger array. This would normally cause reallocation, invalidating the iterators. But not now!

	EXPECT_EQ(*first, one[0]) << "We must still be able to read the contents in this iterator.";
	EXPECT_EQ(*second, one_two[0]) << "We must still be able to read the contents in this iterator.";
	EXPECT_EQ(*third, one_two[1]) << "We must still be able to read the contents in this iterator.";
}

/*!
 * Test resizing the batch of batches to a smaller size.
 *
 * The data at the start of the batch has to be retained, while subbatches at
 * the end are dropped then.
 */
TEST_F(BatchOfBatchesFixture, ResizeShrink) {
	Batch<Batch<int>> batch = power_increases; //Make a copy so that we have something to compare with.

	batch.resize(3); //Shrink to 3 subbatches.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2]
	})) << "The first three subbatches must have been retained, while the rest has been dropped.";

	batch.resize(0); //Shrink to 0 subbatches.
	EXPECT_TRUE(batch.empty()) << "We shrank to 0 subbatches, so it must now be an empty batch of batches.";
}

/*!
 * Test resizing the batch of batches to a bigger size.
 */
TEST_F(BatchOfBatchesFixture, ResizeGrow) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we can compare to the original.

	batch.resize(8); //Grow to 8 subbatches.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
		linear_increases[2],
		linear_increases[3],
		linear_increases[4],
		{},
		{},
		{}
	})) << "We grew from 5 to 8 subbatches, so there must now be 3 empty subbatches at the end.";
}

/*!
 * Test resizing the batch of batches to a smaller size while passing a default
 * element to fill in empty spots with.
 *
 * The default element should not get used.
 */
TEST_F(BatchOfBatchesFixture, ResizeShrinkWithDefault) {
	Batch<Batch<int>> batch = linear_increases; //Make a copy so that we have something to compare with.

	batch.resize(2, one); //Shrink to 2 subbatches.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		linear_increases[0],
		linear_increases[1],
	})) << "The first two subbatches must have been retained, while the rest has been dropped.";

	batch.resize(0, one_through_nine); //Shrink to 0 subbatches.
	EXPECT_TRUE(batch.empty()) << "We shrank to 0 subbatches, so it must now be an empty batch of batches.";
}

/*!
 * Test resizing the batch of batches to a bigger size, while passing a default
 * element to fill in empty spots with.
 */
TEST_F(BatchOfBatchesFixture, ResizeGrowWithDefault) {
	Batch<Batch<int>> batch = power_increases; //Make a copy so that we can compare to the original.

	batch.resize(9, one_two); //Grow to 9 subbatches.
	EXPECT_EQ(batch, Batch<Batch<int>>({
		power_increases[0],
		power_increases[1],
		power_increases[2],
		power_increases[3],
		power_increases[4],
		power_increases[5],
		one_two,
		one_two,
		one_two
	})) << "We grew from 6 to 9 subbatches, so the provided default subbatch must be appended 3 times at the end.";
}

/*!
 * Test whether ``shrink_to_fit`` reduces the reported memory usage (via
 * ``size_subelements``) to match the minimum used memory needed to store the
 * data.
 */
TEST_F(BatchOfBatchesFixture, ShrinkToFit) {
	//Find how many elements are stored in total.
	size_t minimum_memory = std::accumulate(power_increases.cbegin(), power_increases.cend(), size_t(0), [](const size_t current, const Subbatch<int>& subbatch) {
		return current + subbatch.size();
	});
	//Now add a few extra elements. This causes the subbatches to reallocate with doubling, adding superfluous extra capacity.
	power_increases[3].push_back(100);
	power_increases[4].push_back(101);
	power_increases[1].push_back(102);
	minimum_memory += 3; //Because we added 3 elements.
	Batch<Batch<int>> before_shrinking = power_increases; //Store a copy of this state. Shrinking shouldn't change the data, so we can compare to this.

	power_increases.shrink_to_fit();

	EXPECT_EQ(power_increases, before_shrinking) << "The data in the batch should not be changed by shrinking.";
	EXPECT_EQ(power_increases.size_subelements(), minimum_memory) << "The size of the subelement array should now equal the number of elements, since that is the minimum size needed to store the data.";
}

/*!
 * Test counting the size of the subelement array.
 */
TEST_F(BatchOfBatchesFixture, SizeSubelements) {
	size_t power_subelement_count = std::accumulate(power_increases.cbegin(), power_increases.cend(), size_t(0), [](const size_t current, const Subbatch<int>& subbatch) {
		return current + subbatch.size();
	});
	EXPECT_GE(power_increases.size_subelements(), power_subelement_count) << "The size of the subelement array must be at least as big as the number of subelements (but may be larger if there are dead spots).";
	//Check for each subelement if it occurs somewhere in the subelement buffer. If it does, remove it so we also properly count doubles.
	std::vector<int> subelement_buffer(power_increases.size_subelements());
	for(size_t i = 0; i < power_increases.size_subelements(); ++i) {
		subelement_buffer.push_back(power_increases.data_subelements()[i]);
	}

	for(Subbatch<int>& subbatch : power_increases) {
		for(int subelement : subbatch) {
			std::vector<int>::iterator found_element = std::find(subelement_buffer.begin(), subelement_buffer.end(), subelement);
			EXPECT_NE(found_element, subelement_buffer.end()) << "Every subelement of the batch of batches must be somewhere within the range [0, size_subelements()] in the subelement array.";
			if(found_element != subelement_buffer.end()) {
				(*found_element) = -1; //Remove this element so that we don't find it again if we're looking for the same integer.
			}
		}
	}
}

/*!
 * Test swapping the contents of batches of batches.
 */
TEST_F(BatchOfBatchesFixture, Swap) {
	Batch<Batch<int>> batch_a = linear_increases; //Make copies so that we can compare with their original contents.
	Batch<Batch<int>> batch_b = power_increases;

	batch_a.swap(batch_b);
	ASSERT_EQ(batch_a, power_increases) << "Batch A was swapped with Batch B, which contained power_increases at the time, so now Batch A must contain power_increases.";
	ASSERT_EQ(batch_b, linear_increases) << "Batch A, which contained linear_increases at the time, was swapped with Batch B, so now Batch B must contain linear_increases.";

	batch_a.swap(batch_b);
	ASSERT_EQ(batch_a, linear_increases) << "We're swapping them back, so now Batch A contains linear_increases again.";
	ASSERT_EQ(batch_b, power_increases) << "We're swapping them back, so now Batch B contains power_increases again.";

	batch_a = empty_batch;
	batch_b.swap(batch_a);
	ASSERT_EQ(batch_a, power_increases) << "Batch A was made empty and then swapped with the non-empty Batch B, so now Batch A must contain power_increases.";
	ASSERT_EQ(batch_b, empty_batch) << "Batch A was made empty and then swapped with Batch B, so now Batch B must be the empty one.";
}

/*!
 * Fuzz test that applies equivalent actions to a batch of batches and a vector
 * of vectors.
 *
 * This is effectively an integration test that tests the whole class. It
 * verifies that a batch of batches behaves the same way as a vector of vectors,
 * even though the implementation is very different.
 */
TEST(BatchOfBatches, VectorEquivalenceFuzz) {
	FuzzEqualBehaviour<Batch<Batch<int>>, std::vector<std::vector<int>>> fuzzer([](const Batch<Batch<int>>& batch, const std::vector<std::vector<int>>& vec) {
		if(batch.size() != vec.size()) { //If they have a different number of subbatches, they are not equal.
			return false;
		}
		for(size_t subbatch = 0; subbatch < batch.size(); ++subbatch) {
			if(batch[subbatch].size() != vec[subbatch].size()) { //If a subbatch has a different number of subelements, they are not equal.
				return false;
			}
			for(size_t subelement = 0; subelement < batch[subbatch].size(); ++subelement) {
				if(batch[subbatch][subelement] != vec[subbatch][subelement]) { //If the subelement in the same position has a different value, they are not equal.
					return false;
				}
			}
		}
		return true; //Everything checked. They are equal.
	});

	fuzzer.add_transformation("assign_operator",
		[](Batch<Batch<int>>& batch) { batch = Batch<Batch<int>>({{1, 2, 3}, {4, 5, 6}}); },
		[](std::vector<std::vector<int>>& vec) { vec = std::vector<std::vector<int>>({{1, 2, 3}, {4, 5, 6}}); },
		1.0);
	fuzzer.add_transformation("assign_empty",
		[](Batch<Batch<int>>& batch) { batch.assign({}); },
		[](std::vector<std::vector<int>>& vec) { vec.assign({}); },
		1.0);
	fuzzer.add_transformation("assign_singular",
		[](Batch<Batch<int>>& batch) { batch.assign({{1}}); },
		[](std::vector<std::vector<int>>& vec) { vec.assign({{1}}); },
		1.0);
	fuzzer.add_transformation("assign_copies",
		[](Batch<Batch<int>>& batch) { batch.assign(5, Batch<int>({1, 2, 3, 4})); },
		[](std::vector<std::vector<int>>& vec) { vec.assign(5, std::vector<int>({1, 2, 3, 4})); },
		1.0);
	fuzzer.add_transformation("assign_iterator",
		[](Batch<Batch<int>>& batch) {
			const std::vector<Batch<int>> source({{0}, {1, 2}, {3, 4, 5}, {6, 7, 8, 9}});
			batch.assign(source.cbegin(), source.cend());
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<std::vector<int>> source({{0}, {1, 2}, {3, 4, 5}, {6, 7, 8, 9}});
			vec.assign(source.cbegin(), source.cend());
		},
		1.0);
	fuzzer.add_transformation("clear",
		[](Batch<Batch<int>>& batch) { batch.clear(); },
		[](std::vector<std::vector<int>>& vec) { vec.clear(); },
		1.0);
	fuzzer.add_transformation("emplace_empty_begin",
		[](Batch<Batch<int>>& batch) { batch.emplace(batch.begin()); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace(vec.begin()); },
		10.0);
	fuzzer.add_transformation("emplace_empty_mid",
		[](Batch<Batch<int>>& batch) { batch.emplace(batch.begin() + batch.size() / 2); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace(vec.begin() + vec.size() / 2); },
		10.0);
	fuzzer.add_transformation("emplace_empty_end",
		[](Batch<Batch<int>>& batch) { batch.emplace(batch.end()); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace(vec.end()); },
		10.0);
	fuzzer.add_transformation("emplace_copies_begin",
		[](Batch<Batch<int>>& batch) { batch.emplace(batch.begin(), size_t(8), 42); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace(vec.begin(), size_t(8), 42); },
		10.0);
	fuzzer.add_transformation("emplace_copies_mid",
		[](Batch<Batch<int>>& batch) { batch.emplace(batch.begin() + batch.size() / 2, size_t(8), 42); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace(vec.begin() + vec.size() / 2, size_t(8), 42); },
		10.0);
	fuzzer.add_transformation("emplace_copies_end",
		[](Batch<Batch<int>>& batch) { batch.emplace(batch.end(), size_t(6), 69); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace(vec.end(), size_t(6), 69); },
		10.0);
	fuzzer.add_transformation("emplace_iterator_begin",
		[](Batch<Batch<int>>& batch) {
			const std::list<int> source({4, 3, 2, 1});
			batch.emplace(batch.cbegin(), source.cbegin(), source.cend());
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::list<int> source({4, 3, 2, 1});
			vec.emplace(vec.cbegin(), source.cbegin(), source.cend());
		},
		10.0);
	fuzzer.add_transformation("emplace_iterator_mid",
		[](Batch<Batch<int>>& batch) {
			const std::list<int> source({4, 3, 2, 1});
			batch.emplace(batch.cbegin() + batch.size() / 2, source.cbegin(), source.cend());
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::list<int> source({4, 3, 2, 1});
			vec.emplace(vec.cbegin() + vec.size() / 2, source.cbegin(), source.cend());
		},
		10.0);
	fuzzer.add_transformation("emplace_iterator_end",
		[](Batch<Batch<int>>& batch) {
			const std::list<int> source({4, 3, 2, 1});
			batch.emplace(batch.cend(), source.cbegin(), source.cend());
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::list<int> source({4, 3, 2, 1});
			vec.emplace(vec.cend(), source.cbegin(), source.cend());
		},
		10.0);
	fuzzer.add_transformation("emplace_copy_begin",
		[](Batch<Batch<int>>& batch) {
			const Batch<int> source({99});
			batch.emplace(batch.cbegin(), source);
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<int> source({99});
			vec.emplace(vec.cbegin(), source);
		},
		10.0);
	fuzzer.add_transformation("emplace_copy_mid",
		[](Batch<Batch<int>>& batch) {
			const Batch<int> source({99});
			batch.emplace(batch.cbegin() + batch.size() / 2, source);
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<int> source({99});
			vec.emplace(vec.cbegin() + vec.size() / 2, source);
		},
		10.0);
	fuzzer.add_transformation("emplace_copy_end",
		[](Batch<Batch<int>>& batch) {
			const Batch<int> source({88});
			batch.emplace(batch.cend(), source);
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<int> source({88});
			vec.emplace(vec.cend(), source);
		},
		10.0);
	fuzzer.add_transformation("emplace_move_begin",
		[](Batch<Batch<int>>& batch) {
			Batch<int> source({77});
			batch.emplace(batch.cbegin(), std::move(source));
		},
		[](std::vector<std::vector<int>>& vec) {
			std::vector<int> source({77});
			vec.emplace(vec.cbegin(), std::move(source));
		},
		10.0);
	fuzzer.add_transformation("emplace_move_mid",
		[](Batch<Batch<int>>& batch) {
			Batch<int> source({77});
			batch.emplace(batch.cbegin() + batch.size() / 2, std::move(source));
		},
		[](std::vector<std::vector<int>>& vec) {
			std::vector<int> source({77});
			vec.emplace(vec.cbegin() + vec.size() / 2, std::move(source));
		},
		10.0);
	fuzzer.add_transformation("emplace_move_end",
		[](Batch<Batch<int>>& batch) {
			Batch<int> source({66});
			batch.emplace(batch.cend(), std::move(source));
		},
		[](std::vector<std::vector<int>>& vec) {
			std::vector<int> source({66});
			vec.emplace(vec.cend(), std::move(source));
		},
		10.0);
	fuzzer.add_transformation("emplace_initialiser_list_begin",
		[](Batch<Batch<int>>& batch) { batch.emplace(batch.begin(), {1, 2, 3, 4, 5}); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace(vec.begin(), std::initializer_list<int>({1, 2, 3, 4, 5})); }, //The template argument deduction of the vector doesn't allow detecting the type, so explicitly specify it.
		10.0);
	fuzzer.add_transformation("emplace_initialiser_list_mid",
		[](Batch<Batch<int>>& batch) { batch.emplace(batch.begin() + batch.size() / 2, {1, 2, 3, 4, 5}); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace(vec.begin() + vec.size() / 2, std::initializer_list<int>({1, 2, 3, 4, 5})); }, //The template argument deduction of the vector doesn't allow detecting the type, so explicitly specify it.
		10.0);
	fuzzer.add_transformation("emplace_initialiser_list_end",
		[](Batch<Batch<int>>& batch) { batch.emplace(batch.end(), {1, 2, 3, 4, 5}); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace(vec.end(), std::initializer_list<int>({1, 2, 3, 4, 5})); }, //The template argument deduction of the vector doesn't allow detecting the type, so explicitly specify it.
		10.0);
	fuzzer.add_transformation("emplace_back_empty",
		[](Batch<Batch<int>>& batch) { batch.emplace_back(); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace_back(); },
		3.0);
	fuzzer.add_transformation("emplace_back_copies",
		[](Batch<Batch<int>>& batch) { batch.emplace_back(size_t(4), 1337); },
		[](std::vector<std::vector<int>>& vec) { vec.emplace_back(size_t(4), 1337); },
		3.0);
	fuzzer.add_transformation("emplace_back_iterator",
		[](Batch<Batch<int>>& batch) {
			const std::list<int> source({1, 2, 3, 5, 8, 13, 21});
			batch.emplace_back(source.cbegin(), source.cend());
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::list<int> source({1, 2, 3, 5, 8, 13, 21});
			vec.emplace_back(source.cbegin(), source.cend());
		},
		3.0);
	fuzzer.add_transformation("emplace_back_copy",
		[](Batch<Batch<int>>& batch) {
			const Batch<int> source({3, 1, 4, 1, 5, 9});
			batch.emplace_back(source);
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<int> source({3, 1, 4, 1, 5, 9});
			vec.emplace_back(source);
		},
		3.0);
	fuzzer.add_transformation("emplace_back_move",
		[](Batch<Batch<int>>& batch) {
			Batch<int> source({1, 2, 4, 8, 16, 32, 64});
			batch.emplace_back(std::move(source));
		},
		[](std::vector<std::vector<int>>& vec) {
			std::vector<int> source({1, 2, 4, 8, 16, 32, 64});
			vec.emplace_back(std::move(source));
		},
		3.0);
	fuzzer.add_transformation("erase_begin",
		[](Batch<Batch<int>>& batch) {
			if(!batch.empty()) {
				batch.erase(batch.cbegin());
			}
		},
		[](std::vector<std::vector<int>>& vec) {
			if(!vec.empty()) {
				vec.erase(vec.cbegin());
			}
		},
		10.0);
	fuzzer.add_transformation("erase_mid",
		[](Batch<Batch<int>>& batch) {
			if(!batch.empty()) {
				batch.erase(batch.cbegin() + batch.size() / 2);
			}
		},
		[](std::vector<std::vector<int>>& vec) {
			if(!vec.empty()) {
				vec.erase(vec.cbegin() + vec.size() / 2);
			}
		},
		10.0);
	fuzzer.add_transformation("erase_last",
		[](Batch<Batch<int>>& batch) {
			if(!batch.empty()) {
				batch.erase(batch.cend() - 1);
			}
		},
		[](std::vector<std::vector<int>>& vec) {
			if(!vec.empty()) {
				vec.erase(vec.cend() - 1);
			}
		},
		10.0);
	fuzzer.add_transformation("erase_first_half",
		[](Batch<Batch<int>>& batch) {
			if(batch.size() >= 2) {
				batch.erase(batch.cbegin(), batch.cbegin() + batch.size() / 2);
			}
		},
		[](std::vector<std::vector<int>>& vec) {
			if(vec.size() >= 2) {
				vec.erase(vec.cbegin(), vec.cbegin() + vec.size() / 2);
			}
		},
		10.0);
	fuzzer.add_transformation("erase_mid_half",
		[](Batch<Batch<int>>& batch) {
			if(batch.size() >= 2) {
				batch.erase(batch.cbegin() + batch.size() / 4, batch.cbegin() + batch.size() * 3 / 4);
			}
		},
		[](std::vector<std::vector<int>>& vec) {
			if(vec.size() >= 2) {
				vec.erase(vec.cbegin() + vec.size() / 4, vec.cbegin() + vec.size() * 3 / 4);
			}
		},
		10.0);
	fuzzer.add_transformation("erase_last_half",
		[](Batch<Batch<int>>& batch) {
			if(batch.size() >= 2) {
				batch.erase(batch.cbegin() + batch.size() / 2, batch.cend());
			}
		},
		[](std::vector<std::vector<int>>& vec) {
			if(vec.size() >= 2) {
				vec.erase(vec.cbegin() + vec.size() / 2, vec.cend());
			}
		},
		10.0);
	fuzzer.add_transformation("insert_copy_begin",
		[](Batch<Batch<int>>& batch) {
			const Batch<int> source({1, 4, 9, 16, 25});
			batch.insert(batch.cbegin(), source);
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<int> source({1, 4, 9, 16, 25});
			vec.insert(vec.cbegin(), source);
		},
		10.0);
	fuzzer.add_transformation("insert_copy_mid",
		[](Batch<Batch<int>>& batch) {
			const Batch<int> source({1, 3, 6, 10});
			batch.insert(batch.cbegin() + batch.size() / 2, source);
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<int> source({1, 3, 6, 10});
			vec.insert(vec.cbegin() + vec.size() / 2, source);
		},
		10.0);
	fuzzer.add_transformation("insert_copy_end",
		[](Batch<Batch<int>>& batch) {
			const Batch<int> source({1, 5, 12, 22});
			batch.insert(batch.cend(), source);
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<int> source({1, 5, 12, 22});
			vec.insert(vec.cend(), source);
		},
		10.0);
	fuzzer.add_transformation("insert_move_begin",
		[](Batch<Batch<int>>& batch) {
			Batch<int> source({1, 11, 21, 1211, 111221, 13112221});
			batch.insert(batch.cbegin(), std::move(source));
		},
		[](std::vector<std::vector<int>>& vec) {
			std::vector<int> source({1, 11, 21, 1211, 111221, 13112221});
			vec.insert(vec.cbegin(), std::move(source));
		},
		10.0);
	fuzzer.add_transformation("insert_move_mid",
		[](Batch<Batch<int>>& batch) {
			Batch<int> source({11, 121, 12321, 1234321});
			batch.insert(batch.cbegin() + batch.size() / 2, std::move(source));
		},
		[](std::vector<std::vector<int>>& vec) {
			std::vector<int> source({11, 121, 12321, 1234321});
			vec.insert(vec.cbegin() + vec.size() / 2, std::move(source));
		},
		10.0);
	fuzzer.add_transformation("insert_move_end",
		[](Batch<Batch<int>>& batch) {
			Batch<int> source({44, 33, 22, 11});
			batch.insert(batch.cend(), std::move(source));
		},
		[](std::vector<std::vector<int>>& vec) {
			std::vector<int> source({44, 33, 22, 11});
			vec.insert(vec.cend(), std::move(source));
		},
		10.0);
	fuzzer.add_transformation("insert_copies_begin",
		[](Batch<Batch<int>>& batch) {
			const Batch<int> source({1, -1, 1, -1, 2});
			batch.insert(batch.cbegin(), 3, source);
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<int> source({1, -1, 1, -1, 2});
			vec.insert(vec.cbegin(), 3, source);
		},
		10.0);
	fuzzer.add_transformation("insert_copies_mid",
		[](Batch<Batch<int>>& batch) {
			const Batch<int> source({1, -1, 2, -1, 2});
			batch.insert(batch.cbegin() + batch.size() / 2, 3, source);
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<int> source({1, -1, 2, -1, 2});
			vec.insert(vec.cbegin() + vec.size() / 2, 3, source);
		},
		10.0);
	fuzzer.add_transformation("insert_copies_end",
		[](Batch<Batch<int>>& batch) {
			const Batch<int> source({2, -1, 1, -1, 2});
			batch.insert(batch.cend(), 3, source);
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<int> source({2, -1, 1, -1, 2});
			vec.insert(vec.cend(), 3, source);
		},
		10.0);
	fuzzer.add_transformation("insert_iterator_begin",
		[](Batch<Batch<int>>& batch) {
			const Batch<Batch<int>> source({{1, 2}, {3, 4}, {4, 5}, {}, {}});
			batch.insert(batch.cbegin(), source.cbegin(), source.cend());
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<std::vector<int>> source({{1, 2}, {3, 4}, {4, 5}, {}, {}});
			vec.insert(vec.cbegin(), source.cbegin(), source.cend());
		},
		10.0);
	fuzzer.add_transformation("insert_iterator_mid",
		[](Batch<Batch<int>>& batch) {
			const Batch<Batch<int>> source({{1}, {}, {2, 3}, {}, {4, 5, 6}});
			batch.insert(batch.cbegin() + batch.size() / 2, source.cbegin(), source.cend());
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<std::vector<int>> source({{1}, {}, {2, 3}, {}, {4, 5, 6}});
			vec.insert(vec.cbegin() + vec.size() / 2, source.cbegin(), source.cend());
		},
		10.0);
	fuzzer.add_transformation("insert_iterator_end",
		[](Batch<Batch<int>>& batch) {
			const Batch<Batch<int>> source({{}, {1, 2, 3}, {2, 3, 4}, {}});
			batch.insert(batch.cend(), source.cbegin(), source.cend());
		},
		[](std::vector<std::vector<int>>& vec) {
			const std::vector<std::vector<int>> source({{}, {1, 2, 3}, {2, 3, 4}, {}});
			vec.insert(vec.cend(), source.cbegin(), source.cend());
		},
		10.0);
	fuzzer.add_transformation("insert_initialiser_list_begin",
		[](Batch<Batch<int>>& batch) { batch.insert(batch.begin(), {{2, 2}, {3, 3, 3}, {1}}); },
		[](std::vector<std::vector<int>>& vec) { vec.insert(vec.begin(), std::initializer_list<std::vector<int>>({{2, 2}, {3, 3, 3}, {1}})); }, //The template argument deduction of the vector doesn't allow detecting the type, so explicitly specify it.
		10.0);
	fuzzer.add_transformation("insert_initialiser_list_mid",
		[](Batch<Batch<int>>& batch) { batch.insert(batch.begin() + batch.size() / 2, {{5, 0, -5}, {}, {4, 0, -4}}); },
		[](std::vector<std::vector<int>>& vec) { vec.insert(vec.begin() + vec.size() / 2, std::initializer_list<std::vector<int>>({{5, 0, -5}, {}, {4, 0, -4}})); }, //The template argument deduction of the vector doesn't allow detecting the type, so explicitly specify it.
		10.0);
	fuzzer.add_transformation("insert_initialiser_list_end",
		[](Batch<Batch<int>>& batch) { batch.insert(batch.end(), {{12, 23, 34, 45}, {}}); },
		[](std::vector<std::vector<int>>& vec) { vec.insert(vec.end(), std::initializer_list<std::vector<int>>({{12, 23, 34, 45}, {}})); }, //The template argument deduction of the vector doesn't allow detecting the type, so explicitly specify it.
		10.0);
	fuzzer.add_transformation("push_back_copy",
		[](Batch<Batch<int>>& batch) { batch.push_back({1001, 2002}); },
		[](std::vector<std::vector<int>>& vec) { vec.push_back({1001, 2002}); },
		25.0);
	fuzzer.add_transformation("pop_back",
		[](Batch<Batch<int>>& batch) {
			if(!batch.empty()) {
				batch.pop_back();
			}
		},
		[](std::vector<std::vector<int>>& vec) {
			if(!vec.empty()) {
				vec.pop_back();
			}
		},
		50.0);
	fuzzer.add_transformation("push_back_move",
		[](Batch<Batch<int>>& batch) {
			Batch<int> source({2003, 3004});
			batch.push_back(std::move(source));
		},
		[](std::vector<std::vector<int>>& vec) {
			std::vector<int> source({2003, 3004});
			vec.push_back(std::move(source));
		},
		25.0);
	fuzzer.add_transformation("reserve_small",
		[](Batch<Batch<int>>& batch) { batch.reserve(10); },
		[](std::vector<std::vector<int>>& vec) { vec.reserve(10); },
		5.0);
	fuzzer.add_transformation("reserve_large",
		[](Batch<Batch<int>>& batch) { batch.reserve(100); },
		[](std::vector<std::vector<int>>& vec) { vec.reserve(100); },
		5.0);
	fuzzer.add_transformation("resize_empty",
		[](Batch<Batch<int>>& batch) { batch.resize(0); },
		[](std::vector<std::vector<int>>& vec) { vec.resize(0); },
		1.0);
	fuzzer.add_transformation("resize_small",
		[](Batch<Batch<int>>& batch) { batch.resize(4); },
		[](std::vector<std::vector<int>>& vec) { vec.resize(4); },
		4.0);
	fuzzer.add_transformation("resize_large",
		[](Batch<Batch<int>>& batch) { batch.resize(40); },
		[](std::vector<std::vector<int>>& vec) { vec.resize(40); },
		4.0);
	fuzzer.add_transformation("resize_default_small",
		[](Batch<Batch<int>>& batch) { batch.resize(4, {666, 66, 6}); },
		[](std::vector<std::vector<int>>& vec) { vec.resize(4, {666, 66, 6}); },
		4.0);
	fuzzer.add_transformation("resize_default_large",
		[](Batch<Batch<int>>& batch) { batch.resize(40, {31337, 1337, 5318008}); },
		[](std::vector<std::vector<int>>& vec) { vec.resize(40, {31337, 1337, 5318008}); },
		4.0);
	fuzzer.add_transformation("shrink_to_fit",
		[](Batch<Batch<int>>& batch) { batch.shrink_to_fit(); },
		[](std::vector<std::vector<int>>& vec) { vec.shrink_to_fit(); },
		10.0);
	fuzzer.add_transformation("swap",
		[](Batch<Batch<int>>& batch) {
			Batch<Batch<int>> source({{1}, {2, 2}, {3, 3, 3}, {4, 4, 4, 4}});
			batch.swap(source);
		},
		[](std::vector<std::vector<int>>& vec) {
			std::vector<std::vector<int>> source({{1}, {2, 2}, {3, 3, 3}, {4, 4, 4, 4}});
			vec.swap(source);
		},
		1.0);

	Batch<Batch<int>> batch;
	std::vector<std::vector<int>> vec;
	fuzzer.run(batch, vec, 100000);
}

}