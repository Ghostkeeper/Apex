/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "apex/batch.hpp" //The code under test.

namespace apex {

/*
You might expect there to be some tests for batches of simple elements here.
However the Batch<E> class simply passes all of its function calls on to an
underlying vector data structure. These functions are so trivial that testing
the batch can be considered transparently testing the vector implementation of
your compiler. That is not in scope for this test, so those tests are left out
in favour of testing the more interesting specialisations of batches.
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
     * A batch with a single element, the number 1.
     */
    Batch<int> one;

    /*!
     * A batch with numbers 1 and 2.
     */
    Batch<int> one_two;

    /*!
     * Constructs the fixture batches.
     */
    void SetUp() {
        one.assign({1});
        one_two.assign({1, 2});
    }
};

/*!
 * Tests constructing a batch of batches without any batches.
 *
 * This also serves as a basic test for whether the class can be used at all.
 */
TEST(BatchOfBatches, ConstructEmpty) {
    Batch<Batch<int>> empty;
    EXPECT_EQ(empty.size(), 0) << "The batch is empty after its creation.";
    EXPECT_TRUE(empty.empty()) << "The batch is empty after its creation.";
}

}