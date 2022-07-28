/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "apex/detail/pairing_function.hpp" //The unit under test.

namespace apex {

namespace detail {

/*!
 * Test finding how many possible pairings there are in sets of certain sizes.
 *
 * In this test, adjacent numbers are allowed.
 */
TEST(PairingFunction, NumPairingsIncludingAdjacent) {
	EXPECT_EQ(num_pairings(0), 0) << "Without any items, there are no possible pairs of items either.";
	EXPECT_EQ(num_pairings(1), 0) << "With just 1 item, you can't make any pairs.";
	EXPECT_EQ(num_pairings(2), 1) << "With 2 items, you can form a pair. Don't create 2 flipped pairs.";
	EXPECT_EQ(num_pairings(3), 3) << "With 3 items, you can combine 0-1, 0-2 and 1-2.";
	EXPECT_EQ(num_pairings(4), 6) << "With 4 items, you can combine 0-1, 0-2, 0-3, 1-2, 1-3 and 2-3.";
	EXPECT_EQ(num_pairings(5), 10) << "With 5 items, there are 10 possible pairings.";
	EXPECT_EQ(num_pairings(10), 45) << "With 10 items, there are 45 possible pairings.";
}

}

}