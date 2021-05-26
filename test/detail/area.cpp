/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <cmath> //To construct a circle.
#include <gtest/gtest.h> //To run the test.

#include "../helpers/mock_simple_polygon.hpp" //Mock away the base SimplePolygon class. Also provides the area function under test.
#include "../helpers/mock_simple_polygon_batch.hpp" //Mock away the base SimplePolygonBatch class.

#define PI 3.14159265358979 //To construct and calculate the area of a regular N-gon.

namespace apex {

/*!
 * Tests whether the area of an empty polygon starts off at 0.
 */
TEST(SimplePolygonArea, InitialAreaIsZero) {
	const MockSimplePolygon empty_polygon;
	constexpr area_t true_area = 0;
	EXPECT_EQ(empty_polygon.area(), true_area);
	EXPECT_EQ(empty_polygon.area_st(), true_area);
	EXPECT_EQ(empty_polygon.area_mt(), true_area);
#ifdef GPU_TESTS
	EXPECT_EQ(empty_polygon.area_gpu(), true_area);
#endif
}

/*!
 * Tests the area of a basic 1000 by 1000 square.
 */
TEST(SimplePolygonArea, Square1000) {
	const MockSimplePolygon square_1000(MockSimplePolygon::Shape::SQUARE_1000);
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(square_1000.area(), true_area);
	EXPECT_EQ(square_1000.area_st(), true_area);
	EXPECT_EQ(square_1000.area_mt(), true_area);
#ifdef GPU_TESTS
	EXPECT_EQ(square_1000.area_gpu(), true_area);
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square moved into the negative X space.
 */
TEST(SimplePolygonArea, Square1000NegativeX) {
	const MockSimplePolygon negative_x(MockSimplePolygon::Shape::SQUARE_1000_NEGATIVE_X);
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(negative_x.area(), true_area);
	EXPECT_EQ(negative_x.area_st(), true_area);
	EXPECT_EQ(negative_x.area_mt(), true_area);
#ifdef GPU_TESTS
	EXPECT_EQ(negative_x.area_gpu(), true_area);
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square moved into the negative Y space.
 */
TEST(SimplePolygonArea, Square1000NegativeY) {
	const MockSimplePolygon negative_y(MockSimplePolygon::Shape::SQUARE_1000_NEGATIVE_Y);
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(negative_y.area(), true_area);
	EXPECT_EQ(negative_y.area_st(), true_area);
	EXPECT_EQ(negative_y.area_mt(), true_area);
#ifdef GPU_TESTS
	EXPECT_EQ(negative_y.area_gpu(), true_area);
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square moved into the negative X and
 * negative Y space.
 */
TEST(SimplePolygonArea, Square1000NegativeXY) {
	const MockSimplePolygon negative_xy(MockSimplePolygon::Shape::SQUARE_1000_NEGATIVE_XY);
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(negative_xy.area(), true_area);
	EXPECT_EQ(negative_xy.area_st(), true_area);
	EXPECT_EQ(negative_xy.area_mt(), true_area);
#ifdef GPU_TESTS
	EXPECT_EQ(negative_xy.area_gpu(), true_area);
#endif
}

/*!
 * Tests the area of a 1000 by 1000 square around the origin.
 */
TEST(SimplePolygonArea, Square1000Centred) {
	const MockSimplePolygon centred(MockSimplePolygon::Shape::SQUARE_1000_CENTRED);
	constexpr area_t true_area = 1000 * 1000;
	EXPECT_EQ(centred.area(), true_area);
	EXPECT_EQ(centred.area_st(), true_area);
	EXPECT_EQ(centred.area_mt(), true_area);
#ifdef GPU_TESTS
	EXPECT_EQ(centred.area_gpu(), true_area);
#endif
}

/*!
 * Tests the area of a triangle with a 1000-unit wide base.
 */
TEST(SimplePolygonArea, Triangle1000) {
	const MockSimplePolygon triangle(MockSimplePolygon::Shape::TRIANGLE_1000);
	constexpr area_t true_area = 1000 * 1000 / 2;
	EXPECT_EQ(triangle.area(), true_area);
	EXPECT_EQ(triangle.area_st(), true_area);
	EXPECT_EQ(triangle.area_mt(), true_area);
#ifdef GPU_TESTS
	EXPECT_EQ(triangle.area_gpu(), true_area);
#endif
}

/*!
 * Tests the area of a very long and thin rectangle.
 *
 * This catches some of the edge cases involved with rounding errors very well.
 */
TEST(SimplePolygonArea, ThinRectangle) {
	const MockSimplePolygon thin_rectangle(MockSimplePolygon::Shape::THIN_RECTANGLE);
	constexpr area_t true_area = 1000 * 1;
	EXPECT_EQ(thin_rectangle.area(), true_area);
	EXPECT_EQ(thin_rectangle.area_st(), true_area);
	EXPECT_EQ(thin_rectangle.area_mt(), true_area);
#ifdef GPU_TESTS
	EXPECT_EQ(thin_rectangle.area_gpu(), true_area);
#endif
}

/*!
 * Tests the area of a concave shape.
 */
TEST(SimplePolygonArea, Concave) {
	const MockSimplePolygon arrowhead(MockSimplePolygon::Shape::ARROWHEAD);
	constexpr area_t true_area = 1000 * 1000 / 2 - 1000 * 500 / 2;
	EXPECT_EQ(arrowhead.area(), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
	EXPECT_EQ(arrowhead.area_st(), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
	EXPECT_EQ(arrowhead.area_mt(), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
#ifdef GPU_TESTS
	EXPECT_EQ(arrowhead.area_gpu(), true_area) << "The arrowhead has a 1000-wide base and 1000 height, subtracting 1000-wide base with 500 height from the shape.";
#endif
}

/*!
 * Tests the area of a negative shape, where the winding order of the vertices
 * is the other way around.
 */
TEST(SimplePolygonArea, Negative) {
	const MockSimplePolygon negative(MockSimplePolygon::Shape::NEGATIVE_SQUARE);
	constexpr area_t true_area = -1000 * 1000;
	EXPECT_EQ(negative.area(), -1000 * 1000) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
	EXPECT_EQ(negative.area_st(), -1000 * 1000) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
	EXPECT_EQ(negative.area_mt(), -1000 * 1000) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
#ifdef GPU_TESTS
	EXPECT_EQ(negative.area_gpu(), -1000 * 1000) << "Since the winding order is the other way around, this shape is negative and should have a negative area.";
#endif
}

/*!
 * Tests the area of a self-intersecting polygon, where half of it is negative.
 */
TEST(SimplePolygonArea, SelfIntersecting) {
	const MockSimplePolygon hourglass(MockSimplePolygon::Shape::HOURGLASS);
	constexpr area_t true_area = 1000 * 500 / 2 - 1000 * 500 / 2;
	EXPECT_EQ(hourglass.area(), true_area);
	EXPECT_EQ(hourglass.area_st(), true_area);
	EXPECT_EQ(hourglass.area_mt(), true_area);
#ifdef GPU_TESTS
	EXPECT_EQ(hourglass.area_gpu(), true_area);
#endif
}

/*!
 * Tests the area of a single point.
 */
TEST(SimplePolygonArea, Point) {
	const MockSimplePolygon point(MockSimplePolygon::Shape::POINT);
	EXPECT_EQ(point.area(), 0) << "Points don't have any surface area.";
	EXPECT_EQ(point.area_st(), 0) << "Points don't have any surface area.";
	EXPECT_EQ(point.area_mt(), 0) << "Points don't have any surface area.";
#ifdef GPU_TESTS
	EXPECT_EQ(point.area_gpu(), 0) << "Points don't have any surface area.";
#endif
}

/*!
 * Tests the area of a line. It won't have any surface area.
 */
TEST(SimplePolygonArea, Line) {
	const MockSimplePolygon line(MockSimplePolygon::Shape::LINE);
	EXPECT_EQ(line.area(), 0) << "Lines don't have any surface area.";
	EXPECT_EQ(line.area_st(), 0) << "Lines don't have any surface area.";
	EXPECT_EQ(line.area_mt(), 0) << "Lines don't have any surface area.";
#ifdef GPU_TESTS
	EXPECT_EQ(line.area_gpu(), 0) << "Lines don't have any surface area.";
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
	constexpr size_t num_vertices = 1000000;
	constexpr coord_t radius = 1000000;
	MockSimplePolygon circle;
	circle.reserve(num_vertices);
	for(size_t vertex = 0; vertex < num_vertices; ++vertex) { //Construct a circle with lots of vertices.
		const coord_t x = std::lround(std::cos(PI * 2 / num_vertices * vertex) * radius); //This rounding naturally introduces error, so we must allow some lenience in the output.
		const coord_t y = std::lround(std::sin(PI * 2 / num_vertices * vertex) * radius);
		circle.emplace_back(x, y);
	}

	const area_t ground_truth = num_vertices * radius * radius * std::sin(PI * 2 / num_vertices) / 2; //Formula for the area of a regular polygon.
	const area_t error_margin = std::sqrt(num_vertices) / num_vertices / 6 * (PI * radius * radius - PI * (radius - 1) * (radius - 1)); //Margin gets slowly smaller with more vertices, but larger with greater radius.

	EXPECT_NEAR(circle.area(), ground_truth, error_margin);
	EXPECT_NEAR(circle.area_st(), ground_truth, error_margin);
	EXPECT_NEAR(circle.area_mt(), ground_truth, error_margin);
#ifdef GPU_TESTS
	EXPECT_NEAR(circle.area_gpu(), ground_truth, error_margin);
#endif
}

/*!
 * Tests getting the area of an empty batch.
 */
TEST(SimplePolygonBatchArea, EmptyBatch) {
	MockSimplePolygonBatch empty;
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
	MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::SINGLE_EMPTY);

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
	MockSimplePolygonBatch batch(MockSimplePolygonBatch::Contents::SINGLE_LINE);

	const std::vector<area_t> ground_truth = {0};
	EXPECT_EQ(batch.area(), ground_truth) << "There is a single polygon in this batch, but it's just a line with no surface area.";
}

}