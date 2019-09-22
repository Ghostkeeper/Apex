/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "apex/simple_polygon.hpp" //The code under test.

namespace apex {

/*
 * Tests constructing an empty simple polygon.
 */
TEST(SimplePolygon, ConstructEmpty) {
	SimplePolygon empty;
	EXPECT_EQ(empty.size(), 0);
}

/*
 * Tests copy-constructing a simple polygon.
 */
TEST(SimplePolygon, ConstructCopy) {
	SimplePolygon original;
	original.emplace_back(20, 20);
	original.emplace_back(100, 20);
	original.emplace_back(60, 60);

	SimplePolygon copy(original); //Run the copy constructor.

	ASSERT_EQ(original.size(), copy.size());
	for(size_t i = 0; i < original.size(); ++i) {
		EXPECT_EQ(original[i], copy[i]);
	}
}

}