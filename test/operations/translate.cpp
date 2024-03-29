/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the test.

#include "../helpers/polygon_batch_test_cases.hpp" //To load testing batches of polygons to translate.
#include "../helpers/polygon_test_cases.hpp" //To load testing polygons to translate.
#include "apex/operations/translate.hpp" //The function under test.
#include "apex/point2.hpp" //To provide the delta vector to translate by.

namespace apex {

/*!
 * Test whether moving by 0,0 yields the original polygon.
 */
TEST(PolygonTranslate, MoveZero) {
	Polygon square_1000 = PolygonTestCases::square_1000();
	translate(square_1000, Point2(0, 0));
	EXPECT_EQ(square_1000, PolygonTestCases::square_1000()) << "The polygon may not have changed by moving 0,0.";

	square_1000 = PolygonTestCases::square_1000(); //Reset for the next test, just in case.
	detail::translate_st(square_1000, Point2(0, 0));
	EXPECT_EQ(square_1000, PolygonTestCases::square_1000()) << "The polygon may not have changed by moving 0,0.";

	square_1000 = PolygonTestCases::square_1000(); //Reset for the next test, just in case.
	detail::translate_mt(square_1000, Point2(0, 0));
	EXPECT_EQ(square_1000, PolygonTestCases::square_1000()) << "The polygon may not have changed by moving 0,0.";

#ifdef GPU
	square_1000 = PolygonTestCases::square_1000(); //Reset for the next test, just in case.
	detail::translate_gpu(square_1000, Point2(0, 0));
	EXPECT_EQ(square_1000, PolygonTestCases::square_1000()) << "The polygon may not have changed by moving 0,0.";
#endif
}

class TranslateByVector : public testing::TestWithParam<Point2> {};

INSTANTIATE_TEST_SUITE_P(TranslateInputs, TranslateByVector, testing::Values(Point2(250, 0), Point2(0, -300), Point2(-40, 70)));

/*!
 * Test moving a polygon by a certain offset.
 */
TEST_P(TranslateByVector, PolygonTranslateByVector) {
	const Polygon original = PolygonTestCases::square_1000(); //Keep a copy to compare to the original.
	const Point2 move_vector = GetParam();

	Polygon square_1000 = original;
	translate(square_1000, move_vector);
	ASSERT_EQ(square_1000.size(), original.size()) << "The polygon may not gain or lose any vertices by translating it.";
	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], original[i] + move_vector);
	}

	square_1000 = original;
	detail::translate_st(square_1000, move_vector);
	ASSERT_EQ(square_1000.size(), original.size()) << "The polygon may not gain or lose any vertices by translating it.";
	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], original[i] + move_vector);
	}

	square_1000 = original;
	detail::translate_mt(square_1000, move_vector);
	ASSERT_EQ(square_1000.size(), original.size()) << "The polygon may not gain or lose any vertices by translating it.";
	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], original[i] + move_vector);
	}

#ifdef GPU
	square_1000 = original;
	detail::translate_gpu(square_1000, move_vector);
	ASSERT_EQ(square_1000.size(), original.size()) << "The polygon may not gain or lose any vertices by translating it.";
	for(size_t i = 0; i < square_1000.size(); ++i) {
		EXPECT_EQ(square_1000[i], original[i] + move_vector);
	}
#endif
}

/*!
 * Test moving an empty polygon.
 *
 * Since the polygon stays empty, nothing should happen. But it shouldn't crash.
 */
TEST(PolygonTranslate, MoveEmpty) {
	Polygon empty = PolygonTestCases::empty();
	const Point2 move_vector(1337, -31337);

	translate(empty, move_vector);
	ASSERT_TRUE(empty.empty()) << "After translating it, the polygon is still empty.";

	detail::translate_st(empty, move_vector);
	ASSERT_TRUE(empty.empty()) << "After translating it, the polygon is still empty.";

	detail::translate_mt(empty, move_vector);
	ASSERT_TRUE(empty.empty()) << "After translating it, the polygon is still empty.";

#ifdef GPU
	detail::translate_gpu(empty, move_vector);
	ASSERT_TRUE(empty.empty()) << "After translating it, the polygon is still empty.";
#endif
}

/*!
 * Tests whether moving by 0,0 yields the original batch of polygons.
 */
TEST(PolygonBatchTranslate, MoveZero) {
	Batch<Polygon> square_triangle = PolygonBatchTestCases::square_triangle();
	translate(square_triangle, Point2(0, 0));
	EXPECT_EQ(square_triangle, PolygonBatchTestCases::square_triangle()) << "The polygons may not have changed by moving 0,0.";

	square_triangle = PolygonBatchTestCases::square_triangle(); //Reset for the next test, just in case.
	detail::translate_st(square_triangle, Point2(0, 0));
	EXPECT_EQ(square_triangle, PolygonBatchTestCases::square_triangle()) << "The polygons may not have changed by moving 0,0.";

	square_triangle = PolygonBatchTestCases::square_triangle(); //Reset for the next test, just in case.
	detail::translate_mt(square_triangle, Point2(0, 0));
	EXPECT_EQ(square_triangle, PolygonBatchTestCases::square_triangle()) << "The polygons may not have changed by moving 0,0.";

#ifdef GPU
	square_triangle = PolygonBatchTestCases::square_triangle(); //Reset for the next test, just in case.
	detail::translate_gpu(square_triangle, Point2(0, 0));
	EXPECT_EQ(square_triangle, PolygonBatchTestCases::square_triangle()) << "The polygons may not have changed by moving 0,0.";
#endif
}

/*!
 * Test moving a batch of polygons by a certain offset.
 */
TEST_P(TranslateByVector, PolygonBatchTranslateByVector) {
	const Batch<Polygon> original = PolygonBatchTestCases::square_triangle(); //Keep a copy to compare to the original.
	const Point2 move_vector = GetParam();

	Batch<Polygon> square_triangle = original;
	translate(square_triangle, move_vector);
	ASSERT_EQ(square_triangle.size(), original.size()) << "The number of polygons must remain the same.";
	for(size_t polygon = 0; polygon < square_triangle.size(); ++polygon) {
		ASSERT_EQ(square_triangle[polygon].size(), original[polygon].size()) << "The number of vertices in each polygon must remain the same.";
		for(size_t vertex = 0; vertex < square_triangle[polygon].size(); ++vertex) {
			EXPECT_EQ(square_triangle[polygon][vertex], original[polygon][vertex] + move_vector);
		}
	}

	square_triangle = original;
	detail::translate_st(square_triangle, move_vector);
	ASSERT_EQ(square_triangle.size(), original.size()) << "The number of polygons must remain the same.";
	for(size_t polygon = 0; polygon < square_triangle.size(); ++polygon) {
		ASSERT_EQ(square_triangle[polygon].size(), original[polygon].size()) << "The number of vertices in each polygon must remain the same.";
		for(size_t vertex = 0; vertex < square_triangle[polygon].size(); ++vertex) {
			EXPECT_EQ(square_triangle[polygon][vertex], original[polygon][vertex] + move_vector);
		}
	}

	square_triangle = original;
	detail::translate_mt(square_triangle, move_vector);
	ASSERT_EQ(square_triangle.size(), original.size()) << "The number of polygons must remain the same.";
	for(size_t polygon = 0; polygon < square_triangle.size(); ++polygon) {
		ASSERT_EQ(square_triangle[polygon].size(), original[polygon].size()) << "The number of vertices in each polygon must remain the same.";
		for(size_t vertex = 0; vertex < square_triangle[polygon].size(); ++vertex) {
			EXPECT_EQ(square_triangle[polygon][vertex], original[polygon][vertex] + move_vector);
		}
	}

#ifdef GPU
	square_triangle = original;
	detail::translate_gpu(square_triangle, move_vector);
	ASSERT_EQ(square_triangle.size(), original.size()) << "The number of polygons must remain the same.";
	for(size_t polygon = 0; polygon < square_triangle.size(); ++polygon) {
		ASSERT_EQ(square_triangle[polygon].size(), original[polygon].size()) << "The number of vertices in each polygon must remain the same.";
		for(size_t vertex = 0; vertex < square_triangle[polygon].size(); ++vertex) {
			EXPECT_EQ(square_triangle[polygon][vertex], original[polygon][vertex] + move_vector);
		}
	}
#endif
}

/*!
 * Test moving an empty batch.
 *
 * Since the batch stays empty, nothing should happen. But it shouldn't crash.
 */
TEST(PolygonBatchTranslate, MoveEmpty) {
	Batch<Polygon> empty = PolygonBatchTestCases::empty();
	const Point2 move_vector(1337, -31337);

	translate(empty, move_vector);
	ASSERT_TRUE(empty.empty()) << "After translating it, the batch is still empty.";

	detail::translate_st(empty, move_vector);
	ASSERT_TRUE(empty.empty()) << "After translating it, the batch is still empty.";

	detail::translate_mt(empty, move_vector);
	ASSERT_TRUE(empty.empty()) << "After translating it, the batch is still empty.";

#ifdef GPU
	detail::translate_gpu(empty, move_vector);
	ASSERT_TRUE(empty.empty()) << "After translating it, the batch is still empty.";
#endif
}

}