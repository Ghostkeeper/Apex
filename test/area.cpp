/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <cmath> //To construct a circle.
#include <gtest/gtest.h> //To run the test.

#include "apex/area.hpp"
#include "apex/simple_polygon.hpp"
#include "helpers/mock_simple_polygon.hpp" //Mock away the base SimplePolygon class.
#include "helpers/mock_simple_polygon_batch.hpp" //Mock away the base SimplePolygonBatch class.
#include "helpers/test_simple_polygons.hpp" //To load testing polygons to compute the area of.

#define PI 3.14159265358979 //To calculate the area of a regular N-gon.

namespace apex {

/*!
 * Tests whether the area of an empty polygon starts off at 0.
 */
TEST(SimplePolygonArea, InitialAreaIsZero) {
	constexpr area_t true_area = 0;
	EXPECT_EQ(area(data::simple_polygon_empty), true_area);
	EXPECT_EQ(detail::area_st(data::simple_polygon_empty), true_area);
	EXPECT_EQ(detail::area_mt(data::simple_polygon_empty), true_area);
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_empty), true_area);
#endif
}

/*!
 * Tests the area of a basic 1000 by 1000 square.
 */
TEST(SimplePolygonArea, Square1000) {
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(area(data::simple_polygon_square_1000), true_area);
	EXPECT_EQ(detail::area_st(data::simple_polygon_square_1000), true_area);
	EXPECT_EQ(detail::area_mt(data::simple_polygon_square_1000), true_area);
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_square_1000), true_area);
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square moved into the negative X space.
 */
TEST(SimplePolygonArea, Square1000NegativeX) {
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(area(data::simple_polygon_square_1000_negative_x), true_area);
	EXPECT_EQ(detail::area_st(data::simple_polygon_square_1000_negative_x), true_area);
	EXPECT_EQ(detail::area_mt(data::simple_polygon_square_1000_negative_x), true_area);
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_square_1000_negative_x), true_area);
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square moved into the negative Y space.
 */
TEST(SimplePolygonArea, Square1000NegativeY) {
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(area(data::simple_polygon_square_1000_negative_y), true_area);
	EXPECT_EQ(detail::area_st(data::simple_polygon_square_1000_negative_y), true_area);
	EXPECT_EQ(detail::area_mt(data::simple_polygon_square_1000_negative_y), true_area);
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_square_1000_negative_y), true_area);
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square moved into the negative X and
 * negative Y space.
 */
TEST(SimplePolygonArea, Square1000NegativeXY) {
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(area(data::simple_polygon_square_1000_negative_xy), true_area);
	EXPECT_EQ(detail::area_st(data::simple_polygon_square_1000_negative_xy), true_area);
	EXPECT_EQ(detail::area_mt(data::simple_polygon_square_1000_negative_xy), true_area);
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_square_1000_negative_xy), true_area);
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square around the origin.
 */
TEST(SimplePolygonArea, Square1000Centred) {
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(area(data::simple_polygon_square_1000_centred), true_area);
	EXPECT_EQ(detail::area_st(data::simple_polygon_square_1000_centred), true_area);
	EXPECT_EQ(detail::area_mt(data::simple_polygon_square_1000_centred), true_area);
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_square_1000_centred), true_area);
#endif
}

/*!
 * Tests the area of a triangle with a 1000-unit wide base.
 */
TEST(SimplePolygonArea, Triangle1000) {
	constexpr area_t true_area = 1000 * 1000 / 2;
	EXPECT_EQ(area(data::simple_polygon_triangle_1000), true_area);
	EXPECT_EQ(detail::area_st(data::simple_polygon_triangle_1000), true_area);
	EXPECT_EQ(detail::area_mt(data::simple_polygon_triangle_1000), true_area);
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_triangle_1000), true_area);
#endif
}

/*!
 * Tests the area of a very long and thin rectangle.
 *
 * This catches some of the edge cases involved with rounding errors very well.
 */
TEST(SimplePolygonArea, ThinRectangle) {
	constexpr area_t true_area = 1000 * 1;
	EXPECT_EQ(area(data::simple_polygon_thin_rectangle), true_area);
	EXPECT_EQ(detail::area_st(data::simple_polygon_thin_rectangle), true_area);
	EXPECT_EQ(detail::area_mt(data::simple_polygon_thin_rectangle), true_area);
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_thin_rectangle), true_area);
#endif
}

/*!
 * Tests the area of a concave shape.
 */
TEST(SimplePolygonArea, Concave) {
	constexpr area_t true_area = 1000 * 1000 / 2 - 1000 * 500 / 2;
	EXPECT_EQ(area(data::simple_polygon_arrowhead), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
	EXPECT_EQ(detail::area_st(data::simple_polygon_arrowhead), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
	EXPECT_EQ(detail::area_mt(data::simple_polygon_arrowhead), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_arrowhead), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
#endif
}

/*!
 * Tests the area of a negative shape, where the winding order of the vertices
 * is the other way around.
 */
TEST(SimplePolygonArea, Negative) {
	constexpr area_t true_area = -1000 * 1000;
	EXPECT_EQ(area(data::simple_polygon_negative_square), true_area) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
	EXPECT_EQ(detail::area_st(data::simple_polygon_negative_square), true_area) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
	EXPECT_EQ(detail::area_mt(data::simple_polygon_negative_square), true_area) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_negative_square), true_area) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
#endif
}

/*!
 * Tests the area of a self-intersecting polygon, where half of it is negative.
 */
TEST(SimplePolygonArea, SelfIntersecting) {
	constexpr area_t true_area = 1000 * 500 / 2 - 1000 * 500 / 2;
	EXPECT_EQ(area(data::simple_polygon_hourglass), true_area);
	EXPECT_EQ(detail::area_st(data::simple_polygon_hourglass), true_area);
	EXPECT_EQ(detail::area_mt(data::simple_polygon_hourglass), true_area);
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_hourglass), true_area);
#endif
}

/*!
 * Tests the area of a single point.
 */
TEST(SimplePolygonArea, Point) {
	EXPECT_EQ(area(data::simple_polygon_point), 0) << "Points don't have any surface area.";
	EXPECT_EQ(detail::area_st(data::simple_polygon_point), 0) << "Points don't have any surface area.";
	EXPECT_EQ(detail::area_mt(data::simple_polygon_point), 0) << "Points don't have any surface area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_point), 0) << "Points don't have any surface area.";
#endif
}

/*!
 * Tests the area of a line. It won't have any surface area.
 */
TEST(SimplePolygonArea, Line) {
	EXPECT_EQ(area(data::simple_polygon_line), 0) << "Lines don't have any surface area.";
	EXPECT_EQ(detail::area_st(data::simple_polygon_line), 0) << "Lines don't have any surface area.";
	EXPECT_EQ(detail::area_mt(data::simple_polygon_line), 0) << "Lines don't have any surface area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_line), 0) << "Lines don't have any surface area.";
#endif
}

/*!
 * Tests the area of a zero-width shape. It won't have any surface area.
 */
TEST(SimplePolygonArea, ZeroWidth) {
	EXPECT_EQ(area(data::simple_polygon_zero_width), 0) << "The shape has no width, so no surface area.";
	EXPECT_EQ(detail::area_st(data::simple_polygon_zero_width), 0) << "The shape has no width, so no surface area.";
	EXPECT_EQ(detail::area_mt(data::simple_polygon_zero_width), 0) << "The shape has no width, so no surface area.";
#ifdef GPU
	EXPECT_EQ(detail::area_gpu(data::simple_polygon_zero_width), 0) << "The shape has no width, so no surface area.";
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
	const size_t num_vertices = data::simple_polygon_circle.size();
	const coord_t radius = data::simple_polygon_circle[0].x;

	const area_t ground_truth = num_vertices * radius * radius * std::sin(PI * 2 / num_vertices) / 2; //Formula for the area of a regular polygon.
	const area_t error_margin = std::sqrt(num_vertices) / num_vertices / 6 * (PI * radius * radius - PI * (radius - 1) * (radius - 1)); //Margin gets slowly smaller with more vertices, but larger with greater radius.

	EXPECT_NEAR(area(data::simple_polygon_circle), ground_truth, error_margin);
	EXPECT_NEAR(detail::area_st(data::simple_polygon_circle), ground_truth, error_margin);
	EXPECT_NEAR(detail::area_mt(data::simple_polygon_circle), ground_truth, error_margin);
#ifdef GPU
	EXPECT_NEAR(detail::area_gpu(data::simple_polygon_circle), ground_truth, error_margin);
#endif
}

/*!
 * Tests getting the area of an empty batch.
 */
TEST(SimplePolygonBatchArea, EmptyBatch) {
	const MockSimplePolygonBatch empty;
	const std::vector<area_t> ground_truth;
	EXPECT_EQ(empty.area(), ground_truth) << "The area of an empty batch is an empty list.";
}

/*!
 * Tests getting the area of a batch with one polygon. That polygon is empty.
 *
 * This serves as a base case for whether the polygons in the batch are actually
 * used.
 */
TEST(SimplePolygonBatchArea, SingleEmpty) {
	const MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::SINGLE_EMPTY);

	const std::vector<area_t> ground_truth = {0};
	EXPECT_EQ(batch.area(), ground_truth) << "There is a single polygon in this batch, and it is empty.";
}

/*!
 * Tests getting the area of a batch with one polygon. That polygon has just a
 * single vertex, without any surface area.
 */
TEST(SimplePolygonBatchArea, SinglePoint) {
	MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::SINGLE_POINT);

	const std::vector<area_t> ground_truth = {0};
	EXPECT_EQ(batch.area(), ground_truth) << "There is a single polygon in this batch, but it's just a point with no surface area.";
}

/*!
 * Tests getting the area of a batch with one polygon. That polygon has just two
 * vertices, without any surface area.
 */
TEST(SimplePolygonBatchArea, SingleLine) {
	const MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::SINGLE_LINE);

	const std::vector<area_t> ground_truth = {0};
	EXPECT_EQ(batch.area(), ground_truth) << "There is a single polygon in this batch, but it's just a line with no surface area.";
}

/*!
 * Tests getting the area of a batch with one polygon.
 */
TEST(SimplePolygonBatchArea, SingleSquare) {
	const MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::SINGLE_SQUARE);

	const std::vector<area_t> ground_truth = {1000 * 1000};
	EXPECT_EQ(batch.area(), ground_truth) << "There is a single polygon in this batch, and it's a 1000x1000 square.";
}

/*!
 * Tests getting the area of a batch with a square and a triangle.
 *
 * Because the two shapes have different numbers of vertices and different
 * surface areas, this tests getting correct separation between the triangles of
 * the batch, and also that the results are properly formatted.
 */
TEST(SimplePolygonBatchArea, SquareTriangle) {
	const MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::SQUARE_TRIANGLE);

	const std::vector<area_t> ground_truth = {1000 * 1000, 1000 * 1000 / 2};
	EXPECT_EQ(batch.area(), ground_truth) << "The square is 1000x1000. The triangle has a base and height of 1000, so an area of half of that.";
}

/*!
 * Tests getting the area of a batch with a square, a triangle and a square.
 *
 * This tests getting the area of a batch where the size of the polygons is both
 * reduced and increased.
 */
TEST(SimplePolygonBatchArea, SquareTriangleSquare) {
	const MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::SQUARE_TRIANGLE_SQUARE);

	const std::vector<area_t> ground_truth = {1000 * 1000, 1000 * 1000 / 2, 1000 * 1000};
	EXPECT_EQ(batch.area(), ground_truth) << "The first and third polygons are squares of 1000x1000. The middle one is a triangle, with half of that area.";
}

/*!
 * Tests getting the area of a batch with two identical squares.
 *
 * If any algorithm has trouble with the case where the entire batch is the same
 * it would fail this test.
 */
TEST(SimplePolygonBatchArea, TwoSquares) {
	const MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::TWO_SQUARES);

	const std::vector<area_t> ground_truth = {1000 * 1000, 1000 * 1000};
	EXPECT_EQ(batch.area(), ground_truth) << "The batch has two 1000x1000 squares.";
}

/*!
 * Tests all sorts of edge cases, to see if the batch version handles those the
 * same way as the normal algorithms.
 */
TEST(SimplePolygonBatchArea, EdgeCases) {
	const MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::EDGE_CASES);

	const std::vector<area_t> ground_truth = {-1000 * 1000, 0, 0, 0, 0, 0};
	EXPECT_EQ(batch.area(), ground_truth) << "The first element is a negative square. The second is self-intersecting which causes the negative area to compensate for the positive. The rest all has no surface.";
}

/*!
 * Tests getting the area of some polygons with many vertices.
 */
TEST(SimplePolygonBatchArea, TwoCircles) {
	const MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::TWO_CIRCLES);
	const size_t num_vertices = batch[0].size();
	const coord_t radius = batch[0][0].x;

	const area_t true_area = num_vertices * radius * radius * std::sin(PI * 2 / num_vertices) / 2; //Formula for the area of a regular polygon.
	const area_t error_margin = std::sqrt(num_vertices) / num_vertices / 6 * (PI * radius * radius - PI * (radius - 1) * (radius - 1)); //Margin gets slowly smaller with more vertices, but larger with greater radius.

	const std::vector<area_t> calculated_areas = batch.area();
	for(area_t calculated_area : calculated_areas) {
		EXPECT_NEAR(calculated_area, true_area, error_margin);
	}
}

}