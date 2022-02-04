/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "apex/detail/polygon_properties.hpp" //The code under test.

namespace apex {

/*!
 * Tests whether the properties are initially set to unknown.
 */
TEST(PolyProperties, InitUnknown) {
	const PolygonProperties properties;
	EXPECT_EQ(properties.convexity(), PolygonProperties::Convexity::UNKNOWN) << "The convexity must be initialised as unknown.";
	EXPECT_EQ(properties.self_intersecting(), PolygonProperties::SelfIntersecting::UNKNOWN) << "The self-intersectingness must be initialised as unknown.";
	EXPECT_EQ(properties.orientation(), PolygonProperties::Orientation::UNKNOWN) << "The orientation must be initialised as unknown.";
}

}