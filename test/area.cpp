/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <cmath> //To construct a circle.
#include <gtest/gtest.h> //To run the test.

#include "apex/area.hpp" //The unit we're testing here.
#include "helpers/simple_polygon_batch_test_cases.hpp" //To load testing batches of polygons to compute the area of.
#include "helpers/simple_polygon_test_cases.hpp" //To load testing polygons to compute the area of.

#define PI 3.14159265358979 //To calculate the area of a regular N-gon.

namespace apex {

/*!
 * Tests whether the area of an empty polygon starts off at 0.
 */
TEST(SimplePolygonArea, InitialAreaIsZero) {
	constexpr area_t true_area = 0;
	EXPECT_EQ(area(SimplePolygonTestCases::empty()), true_area) << "The polygon is empty, so it has no surface area.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::empty()), true_area) << "The polygon is empty, so it has no surface area.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::empty()), true_area) << "The polygon is empty, so it has no surface area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::empty()), true_area) << "The polygon is empty, so it has no surface area.";
#endif
}

/*!
 * Tests the area of a basic 1000 by 1000 square.
 */
TEST(SimplePolygonArea, Square1000) {
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(area(SimplePolygonTestCases::square_1000()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::square_1000()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::square_1000()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::square_1000()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied.";
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square moved into the negative X space.
 */
TEST(SimplePolygonArea, Square1000NegativeX) {
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(area(SimplePolygonTestCases::square_1000_negative_x()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::square_1000_negative_x()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::square_1000_negative_x()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::square_1000_negative_x()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square moved into the negative Y space.
 */
TEST(SimplePolygonArea, Square1000NegativeY) {
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(area(SimplePolygonTestCases::square_1000_negative_y()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::square_1000_negative_y()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::square_1000_negative_y()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::square_1000_negative_y()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square moved into the negative X and
 * negative Y space.
 */
TEST(SimplePolygonArea, Square1000NegativeXY) {
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(area(SimplePolygonTestCases::square_1000_negative_xy()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::square_1000_negative_xy()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::square_1000_negative_xy()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::square_1000_negative_xy()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square around the origin.
 */
TEST(SimplePolygonArea, Square1000Centred) {
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(area(SimplePolygonTestCases::square_1000_centred()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::square_1000_centred()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::square_1000_centred()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::square_1000_centred()), true_area) << "It's a 1000 by 1000 square, so the area should be those multiplied, regardless of its position around the origin.";
#endif
}

/*!
 * Tests the area of a triangle with a 1000-unit wide base.
 */
TEST(SimplePolygonArea, Triangle1000) {
	constexpr area_t true_area = 1000 * 1000 / 2;
	EXPECT_EQ(area(SimplePolygonTestCases::triangle_1000()), true_area) << "This triangle has base 1000 and height 1000, so it should have half the surface area of those multiplied.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::triangle_1000()), true_area) << "This triangle has base 1000 and height 1000, so it should have half the surface area of those multiplied.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::triangle_1000()), true_area) << "This triangle has base 1000 and height 1000, so it should have half the surface area of those multiplied.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::triangle_1000()), true_area) << "This triangle has base 1000 and height 1000, so it should have half the surface area of those multiplied.";
#endif
}

/*!
 * Tests the area of a very long and thin rectangle.
 *
 * This catches some of the edge cases involved with rounding errors very well.
 */
TEST(SimplePolygonArea, ThinRectangle) {
	constexpr area_t true_area = 1000 * 1;
	EXPECT_EQ(area(SimplePolygonTestCases::thin_rectangle()), true_area) << "This is a 1000-long and 1-wide polygon.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::thin_rectangle()), true_area) << "This is a 1000-long and 1-wide polygon.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::thin_rectangle()), true_area) << "This is a 1000-long and 1-wide polygon.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::thin_rectangle()), true_area) << "This is a 1000-long and 1-wide polygon.";
#endif
}

/*!
 * Tests the area of a concave shape.
 */
TEST(SimplePolygonArea, Concave) {
	constexpr area_t true_area = 1000 * 1000 / 2 - 1000 * 500 / 2;
	EXPECT_EQ(area(SimplePolygonTestCases::arrowhead()), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::arrowhead()), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::arrowhead()), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::arrowhead()), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
#endif
}

/*!
 * Tests the area of a negative shape, where the winding order of the vertices
 * is the other way around.
 */
TEST(SimplePolygonArea, Negative) {
	constexpr area_t true_area = -1000 * 1000;
	EXPECT_EQ(area(SimplePolygonTestCases::negative_square()), true_area) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::negative_square()), true_area) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::negative_square()), true_area) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::negative_square()), true_area) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
#endif
}

/*!
 * Tests the area of a self-intersecting polygon, where half of it is negative.
 */
TEST(SimplePolygonArea, SelfIntersecting) {
	constexpr area_t true_area = 1000 * 500 / 2 - 1000 * 500 / 2;
	EXPECT_EQ(area(SimplePolygonTestCases::hourglass()), true_area) << "The negative area of this polygon exactly cancels out the positive area.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::hourglass()), true_area) << "The negative area of this polygon exactly cancels out the positive area.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::hourglass()), true_area) << "The negative area of this polygon exactly cancels out the positive area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::hourglass()), true_area) << "The negative area of this polygon exactly cancels out the positive area.";
#endif
}

/*!
 * Tests the area of a single point.
 */
TEST(SimplePolygonArea, Point) {
	EXPECT_EQ(area(SimplePolygonTestCases::point()), 0) << "Points don't have any surface area.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::point()), 0) << "Points don't have any surface area.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::point()), 0) << "Points don't have any surface area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::point()), 0) << "Points don't have any surface area.";
#endif
}

/*!
 * Tests the area of a line. It won't have any surface area.
 */
TEST(SimplePolygonArea, Line) {
	EXPECT_EQ(area(SimplePolygonTestCases::line()), 0) << "Lines don't have any surface area.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::line()), 0) << "Lines don't have any surface area.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::line()), 0) << "Lines don't have any surface area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::line()), 0) << "Lines don't have any surface area.";
#endif
}

/*!
 * Tests the area of a zero-width shape. It won't have any surface area.
 */
TEST(SimplePolygonArea, ZeroWidth) {
	EXPECT_EQ(area(SimplePolygonTestCases::zero_width()), 0) << "The shape has no width, so no surface area.";
	EXPECT_EQ(detail::area_st(SimplePolygonTestCases::zero_width()), 0) << "The shape has no width, so no surface area.";
	EXPECT_EQ(detail::area_mt(SimplePolygonTestCases::zero_width()), 0) << "The shape has no width, so no surface area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonTestCases::zero_width()), 0) << "The shape has no width, so no surface area.";
#endif
}

/*!
 * Tests computing the area of a regular simple polygon that consists of many
 * vertices.
 *
 * This is tested with a regular polygon that approaches a circle. The ground
 * truth is calculated with the formula for the area of a regular polygon:
 * 1/2 * n * r^2 * sin(2*pi / n)
 */
TEST(SimplePolygonArea, Circle) {
	const SimplePolygon circle = SimplePolygonTestCases::circle();
	const size_t num_vertices = circle.size();
	const coord_t radius = circle[0].x;

	const area_t ground_truth = num_vertices * radius * radius * std::sin(PI * 2 / num_vertices) / 2; //Formula for the area of a regular polygon.
	const area_t error_margin = std::sqrt(num_vertices) / num_vertices / 6 * (PI * radius * radius - PI * (radius - 1) * (radius - 1)); //Margin gets slowly smaller with more vertices, but larger with greater radius.

	EXPECT_NEAR(area(circle), ground_truth, error_margin) << "The area of the polygon must be close to the ideal area of the regular polygon, but can be different due to integer rounding of its vertices.";
	EXPECT_NEAR(detail::area_st(circle), ground_truth, error_margin) << "The area of the polygon must be close to the ideal area of the regular polygon, but can be different due to integer rounding of its vertices.";
	EXPECT_NEAR(detail::area_mt(circle), ground_truth, error_margin) << "The area of the polygon must be close to the ideal area of the regular polygon, but can be different due to integer rounding of its vertices.";
#ifdef GPU
	EXPECT_NEAR(detail::area_gpu(circle), ground_truth, error_margin) << "The area of the polygon must be close to the ideal area of the regular polygon, but can be different due to integer rounding of its vertices.";
#endif
}

/*!
 * Tests getting the area of an empty batch.
 */
TEST(SimplePolygonBatchArea, EmptyBatch) {
	const Batch<area_t> ground_truth;
	EXPECT_EQ(area(SimplePolygonBatchTestCases::empty()), ground_truth) << "The area of an empty batch is an empty list.";
	EXPECT_EQ(detail::area_st(SimplePolygonBatchTestCases::empty()), ground_truth) << "The area of an empty batch is an empty list.";
	EXPECT_EQ(detail::area_mt(SimplePolygonBatchTestCases::empty()), ground_truth) << "The area of an empty batch is an empty list.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonBatchTestCases::empty()), ground_truth) << "The area of an empty batch is an empty list.";
#endif
}

/*!
 * Tests getting the area of a batch with one polygon. That polygon is empty.
 *
 * This serves as a base case for whether the polygons in the batch are actually
 * used.
 */
TEST(SimplePolygonBatchArea, SingleEmpty) {
	const Batch<area_t> ground_truth = {0};
	EXPECT_EQ(area(SimplePolygonBatchTestCases::single_empty()), ground_truth) << "There is a single polygon in this batch, and it is empty.";
	EXPECT_EQ(detail::area_st(SimplePolygonBatchTestCases::single_empty()), ground_truth) << "There is a single polygon in this batch, and it is empty.";
	EXPECT_EQ(detail::area_mt(SimplePolygonBatchTestCases::single_empty()), ground_truth) << "There is a single polygon in this batch, and it is empty.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonBatchTestCases::single_empty()), ground_truth) << "There is a single polygon in this batch, and it is empty.";
#endif
}

/*!
 * Tests getting the area of a batch with one polygon. That polygon has just a
 * single vertex, without any surface area.
 */
TEST(SimplePolygonBatchArea, SinglePoint) {
	const Batch<area_t> ground_truth = {0};
	EXPECT_EQ(area(SimplePolygonBatchTestCases::single_point()), ground_truth) << "There is a single polygon in this batch, but it's just a point with no surface area.";
	EXPECT_EQ(detail::area_st(SimplePolygonBatchTestCases::single_point()), ground_truth) << "There is a single polygon in this batch, but it's just a point with no surface area.";
	EXPECT_EQ(detail::area_mt(SimplePolygonBatchTestCases::single_point()), ground_truth) << "There is a single polygon in this batch, but it's just a point with no surface area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonBatchTestCases::single_point()), ground_truth) << "There is a single polygon in this batch, but it's just a point with no surface area.";
#endif
}

/*!
 * Tests getting the area of a batch with one polygon. That polygon has just two
 * vertices, without any surface area.
 */
TEST(SimplePolygonBatchArea, SingleLine) {
	const Batch<area_t> ground_truth = {0};
	EXPECT_EQ(area(SimplePolygonBatchTestCases::single_line()), ground_truth) << "There is a single polygon in this batch, but it's just a line with no surface area.";
	EXPECT_EQ(detail::area_st(SimplePolygonBatchTestCases::single_line()), ground_truth) << "There is a single polygon in this batch, but it's just a line with no surface area.";
	EXPECT_EQ(detail::area_mt(SimplePolygonBatchTestCases::single_line()), ground_truth) << "There is a single polygon in this batch, but it's just a line with no surface area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonBatchTestCases::single_line()), ground_truth) << "There is a single polygon in this batch, but it's just a line with no surface area.";
#endif
}

/*!
 * Tests getting the area of a batch with one polygon.
 */
TEST(SimplePolygonBatchArea, SingleSquare) {
	const Batch<area_t> ground_truth = {1000 * 1000};
	EXPECT_EQ(area(SimplePolygonBatchTestCases::single_square()), ground_truth) << "There is a single polygon in this batch, and it's a 1000x1000 square.";
	EXPECT_EQ(detail::area_st(SimplePolygonBatchTestCases::single_square()), ground_truth) << "There is a single polygon in this batch, and it's a 1000x1000 square.";
	EXPECT_EQ(detail::area_mt(SimplePolygonBatchTestCases::single_square()), ground_truth) << "There is a single polygon in this batch, and it's a 1000x1000 square.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonBatchTestCases::single_square()), ground_truth) << "There is a single polygon in this batch, and it's a 1000x1000 square.";
#endif
}

/*!
 * Tests getting the area of a batch with a square and a triangle.
 *
 * Because the two shapes have different numbers of vertices and different
 * surface areas, this tests getting correct separation between the triangles of
 * the batch, and also that the results are properly formatted.
 */
TEST(SimplePolygonBatchArea, SquareTriangle) {
	const Batch<area_t> ground_truth = {1000 * 1000, 1000 * 1000 / 2};
	EXPECT_EQ(area(SimplePolygonBatchTestCases::square_triangle()), ground_truth) << "The square is 1000x1000. The triangle has a base and height of 1000, so an area of half of that.";
	EXPECT_EQ(detail::area_st(SimplePolygonBatchTestCases::square_triangle()), ground_truth) << "The square is 1000x1000. The triangle has a base and height of 1000, so an area of half of that.";
	EXPECT_EQ(detail::area_mt(SimplePolygonBatchTestCases::square_triangle()), ground_truth) << "The square is 1000x1000. The triangle has a base and height of 1000, so an area of half of that.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonBatchTestCases::square_triangle()), ground_truth) << "The square is 1000x1000. The triangle has a base and height of 1000, so an area of half of that.";
#endif
}

/*!
 * Tests getting the area of a batch with a square, a triangle and a square.
 *
 * This tests getting the area of a batch where the size of the polygons is both
 * reduced and increased.
 */
TEST(SimplePolygonBatchArea, SquareTriangleSquare) {
	const Batch<area_t> ground_truth = {1000 * 1000, 1000 * 1000 / 2, 1000 * 1000};
	EXPECT_EQ(area(SimplePolygonBatchTestCases::square_triangle_square()), ground_truth) << "The first and third polygons are squares of 1000x1000. The middle one is a triangle, with half of that area.";
	EXPECT_EQ(detail::area_st(SimplePolygonBatchTestCases::square_triangle_square()), ground_truth) << "The first and third polygons are squares of 1000x1000. The middle one is a triangle, with half of that area.";
	EXPECT_EQ(detail::area_mt(SimplePolygonBatchTestCases::square_triangle_square()), ground_truth) << "The first and third polygons are squares of 1000x1000. The middle one is a triangle, with half of that area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonBatchTestCases::square_triangle_square()), ground_truth) << "The first and third polygons are squares of 1000x1000. The middle one is a triangle, with half of that area.";
#endif
}

/*!
 * Tests getting the area of a batch with two identical squares.
 *
 * If any algorithm has trouble with the case where the entire batch is the same
 * it would fail this test.
 */
TEST(SimplePolygonBatchArea, TwoSquares) {
	const Batch<area_t> ground_truth = {1000 * 1000, 1000 * 1000};
	EXPECT_EQ(area(SimplePolygonBatchTestCases::two_squares()), ground_truth) << "The batch has two 1000x1000 squares.";
	EXPECT_EQ(detail::area_st(SimplePolygonBatchTestCases::two_squares()), ground_truth) << "The batch has two 1000x1000 squares.";
	EXPECT_EQ(detail::area_mt(SimplePolygonBatchTestCases::two_squares()), ground_truth) << "The batch has two 1000x1000 squares.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonBatchTestCases::two_squares()), ground_truth) << "The batch has two 1000x1000 squares.";
#endif
}

/*!
 * Tests all sorts of edge cases, to see if the batch version handles those the
 * same way as the normal algorithms.
 */
TEST(SimplePolygonBatchArea, EdgeCases) {
	const Batch<area_t> ground_truth = {-1000 * 1000, 0, 0, 0, 0, 0};
	EXPECT_EQ(area(SimplePolygonBatchTestCases::edge_cases()), ground_truth) << "The first element is a negative square. The second is self-intersecting which causes the negative area to compensate for the positive. The rest all has no surface.";
	EXPECT_EQ(detail::area_st(SimplePolygonBatchTestCases::edge_cases()), ground_truth) << "The first element is a negative square. The second is self-intersecting which causes the negative area to compensate for the positive. The rest all has no surface.";
	EXPECT_EQ(detail::area_mt(SimplePolygonBatchTestCases::edge_cases()), ground_truth) << "The first element is a negative square. The second is self-intersecting which causes the negative area to compensate for the positive. The rest all has no surface.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(SimplePolygonBatchTestCases::edge_cases()), ground_truth) << "The first element is a negative square. The second is self-intersecting which causes the negative area to compensate for the positive. The rest all has no surface.";
#endif
}

/*!
 * Tests getting the area of some polygons with many vertices.
 */
TEST(SimplePolygonBatchArea, TwoCircles) {
	const Batch<SimplePolygon> two_circles = SimplePolygonBatchTestCases::two_circles();
	const size_t num_vertices = two_circles[0].size();
	const coord_t radius = two_circles[0][0].x;

	const area_t true_area = num_vertices * radius * radius * std::sin(PI * 2 / num_vertices) / 2; //Formula for the area of a regular polygon.
	const area_t error_margin = std::sqrt(num_vertices) / num_vertices / 6 * (PI * radius * radius - PI * (radius - 1) * (radius - 1)); //Margin gets slowly smaller with more vertices, but larger with greater radius.

	Batch<area_t> calculated_areas = area(two_circles);
	for(area_t calculated_area : calculated_areas) {
		EXPECT_NEAR(calculated_area, true_area, error_margin) << "The area of the polygons must be close to the ideal area of the regular polygon, but can be different due to integer rounding of its vertices.";
	}
	calculated_areas = detail::area_st(two_circles);
	for(area_t calculated_area : calculated_areas) {
		EXPECT_NEAR(calculated_area, true_area, error_margin) << "The area of the polygons must be close to the ideal area of the regular polygon, but can be different due to integer rounding of its vertices.";
	}
	calculated_areas = detail::area_mt(two_circles);
	for(area_t calculated_area : calculated_areas) {
		EXPECT_NEAR(calculated_area, true_area, error_margin) << "The area of the polygons must be close to the ideal area of the regular polygon, but can be different due to integer rounding of its vertices.";
	}
#ifdef GPU
	calculated_areas = detail::area_gpu(two_circles);
	for(area_t calculated_area : calculated_areas) {
		EXPECT_NEAR(calculated_area, true_area, error_margin) << "The area of the polygons must be close to the ideal area of the regular polygon, but can be different due to integer rounding of its vertices.";
	}
#endif
}

}