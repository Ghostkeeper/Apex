/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.
#include <tuple> //Combinatoric test data.

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

/*!
 * Tests that run for all combinations of initial values of the properties.
 */
class PolygonPropertiesTest : public testing::TestWithParam<std::tuple<PolygonProperties::Convexity, PolygonProperties::SelfIntersecting, PolygonProperties::Orientation>> {};

INSTANTIATE_TEST_SUITE_P(AllCombinations, PolygonPropertiesTest, testing::Combine(
		testing::Values(PolygonProperties::Convexity::UNKNOWN, PolygonProperties::Convexity::CONVEX, PolygonProperties::Convexity::CONCAVE, PolygonProperties::Convexity::DEGENERATE),
		testing::Values(PolygonProperties::SelfIntersecting::UNKNOWN, PolygonProperties::SelfIntersecting::NO, PolygonProperties::SelfIntersecting::YES, PolygonProperties::SelfIntersecting::EDGE),
		testing::Values(PolygonProperties::Orientation::UNKNOWN, PolygonProperties::Orientation::POSITIVE, PolygonProperties::Orientation::NEGATIVE, PolygonProperties::Orientation::MIXED)
));

/*!
 * Tests setting all of the properties, then retrieving them again.
 */
TEST_P(PolygonPropertiesTest, SetAndGet) {
	//Create properties and set them according to the parameterised test.
	PolygonProperties properties;
	properties.set_convexity(std::get<0>(GetParam()));
	properties.set_self_intersecting(std::get<1>(GetParam()));
	properties.set_orientation(std::get<2>(GetParam()));

	//Now check if they are still what we set them to.
	EXPECT_EQ(properties.convexity(), std::get<0>(GetParam())) << "The convexity must remain what we have set it to.";
	EXPECT_EQ(properties.self_intersecting(), std::get<1>(GetParam())) << "The self-intersectingness must remain what we have set it to.";
	EXPECT_EQ(properties.orientation(), std::get<2>(GetParam())) << "The orientation must remain what we have set it to.";
}

/*!
 * Tests whether the values are unknown again after resetting it.
 */
TEST_P(PolygonPropertiesTest, Reset) {
	//Create properties and set them according to the parameterised test.
	PolygonProperties properties(
		static_cast<unsigned int>(std::get<0>(GetParam()))
		| static_cast<unsigned int>(std::get<1>(GetParam()))
		| static_cast<unsigned int>(std::get<2>(GetParam())));

	properties.reset();

	//Now check if it's all unknown.
	EXPECT_EQ(properties.convexity(), PolygonProperties::Convexity::UNKNOWN);
	EXPECT_EQ(properties.self_intersecting(), PolygonProperties::SelfIntersecting::UNKNOWN);
	EXPECT_EQ(properties.orientation(), PolygonProperties::Orientation::UNKNOWN);
}

}