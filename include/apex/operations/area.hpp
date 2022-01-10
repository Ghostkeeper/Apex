/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_AREA
#define APEX_AREA

#include <omp.h> //To do parallel processing.
#include <vector> //Returning the results of batch operations.

#include "../batch.hpp" //To return batches of areas.
#include "../coordinate.hpp" //To return area_t.
#include "../detail/geometry_concepts.hpp" //To disambiguate overloads.
#include "../point2.hpp" //To access coordinates of vertices.

namespace apex {

namespace detail {

//Declare the detail functions so that we can reference them from the public ones.
template<polygonal SimplePolygon>
area_t area_st(const SimplePolygon& polygon);

template<multi_polygonal SimplePolygonBatch>
Batch<area_t> area_st(const SimplePolygonBatch&);

template<polygonal SimplePolygon>
area_t area_mt(const SimplePolygon& polygon);

template<multi_polygonal SimplePolygonBatch>
Batch<area_t> area_mt(const SimplePolygonBatch&);

#ifdef GPU
template<polygonal SimplePolygon>
area_t area_gpu(const SimplePolygon& polygon);

template<multi_polygonal SimplePolygonBatch>
Batch<area_t> area_gpu(const SimplePolygonBatch&);
#endif //GPU

};

/*!
 * Computes the surface area of a simple polygon.
 *
 * The sign of the area is linked to the polygon winding order. If the polygon
 * is positive, the area will be positive too, and vice versa. If the polygon
 * intersects itself, parts of the polygon will be subtracting from the area
 * while other parts add up to the area.
 *
 * The area of the polygon is counted differently from the nonzero or even-odd
 * fill rules. If a zone is looped around multiple times by the polygon, it will
 * count to the total area multiple times as well.
 * \tparam SimplePolygon A class that behaves like a simple polygon.
 * \param polygon The simple polygon to calculate the area of.
 * \return The surface area of the simple polygon.
 */
template<polygonal SimplePolygon>
area_t area(const SimplePolygon& polygon) {
	if(polygon.size() < 400) {
		return detail::area_st(polygon);
	}
#ifdef GPU
	if(polygon.size() >= 3000) {
		return detail::area_gpu(polygon);
	}
#endif //GPU
	return detail::area_mt(polygon);
}

/*!
 * Computes the surface areas of each simple polygon in a batch.
 *
 * The sign of the area is linked to the polygon winding order. If the polygon
 * is positive, the area will be positive too, and vice versa. If the polygon
 * intersects itself, parts of the polygon will be subtracting from the area
 * while other parts add up to the area.
 *
 * The area of the polygon is counted differently from the nonzero or even-odd
 * fill rules. If a zone is looped around multiple times by the polygon, it will
 * count to the total area multiple times as well.
 * \tparam SimplePolygonBatch A class that behaves like a batch of simple
 * polygons.
 * \param batch A batch of simple polygons to calculate the areas of.
 * \return A list of areas, one for each simple polygon, in the same order as
 * the order of those polygons in the batch.
 */
template<multi_polygonal SimplePolygonBatch>
Batch<area_t> area(const SimplePolygonBatch& batch) {
	if(batch.size() + batch.size_subelements() < 200) {
		return detail::area_st(batch);
	}
	return detail::area_mt(batch);
	//Currently, there doesn't seem to be a case where the GPU is faster in this algorithm.
}

namespace detail {

/*!
 * Single-threaded implementation of ``area``.
 *
 * This uses the shoelace formula to compute the area. The shoelace formula sums
 * the areas of the individual triangles formed by two adjacent vertices and the
 * coordinate origin.
 *
 * To calculate the area of a triangle with one vertex on the origin, we'll
 * calculate the area of a parallelogram formed by the original triangle and
 * that triangle mirrored around the line segment we're calculating the area
 * for. Visualise this:
 *
 * @image html res/shoelace_algorithm_parallelogram.svg
 *
 * The area of the parallelogram can be visualised by starting with a rectangle
 * that encloses the original triangle like this. The green areas fall outside
 * of the parallelogram and shouldn't be counted towards its area.
 *
 * @image html res/shoelace_algorithm_rectangle_overlay.svg
 *
 * The green areas are not part of the parallelogram here, but they can be
 * shifted towards the missing part that falls outside of the rectangle like
 * this.
 *
 * @image html res/shoelace_algorithm_multiple_rectangles.svg
 *
 * This forms a second rectangle, in this case a smaller one in the upper right
 * hand corner. The two green triangles partially overlap and go partially
 * outside of the parallelogram we're trying to get the area of. The part that
 * is overlap plus the part that goes outside of the parallelogram together
 * forms an area of \f$x_1 \cdot y_2\f$.
 *
 * The total area of the parallelogram then becomes the area of the rectangle
 * formed by \f$x_2 \cdot y_1\f$ minus the area formed by the other rectangle
 * formed by \f$x_1 \cdot y_2\f$. In other words, the area of the parallelogram
 * is \f$x_2 \cdot y_1 - x_1 \cdot y_2\f$. This needs to be divided by two to
 * arrive at the area of the triangle. The surface area of a simple polygon is
 * the sum of all of these triangles. This is the shoelace formula.
 *
 * In this implementation, the areas of these parallelograms are calculated in
 * sequence and summed together to get the area of the simple polygon.
 * \tparam SimplePolygon A class that behaves like a simple polygon.
 * \param polygon The simple polygon to calculate the area of.
 * \return The surface area of the simple polygon.
*/
template<polygonal SimplePolygon>
area_t area_st(const SimplePolygon& polygon) {
   area_t area = 0;
   for(size_t vertex = 0, previous = polygon.size() - 1; vertex < polygon.size(); vertex++) {
	   area += static_cast<area_t>(polygon[previous].x) * polygon[vertex].y - static_cast<area_t>(polygon[previous].y) * polygon[vertex].x;
	   previous = vertex;
   }
   return area / 2; //Instead of dividing each triangle's area by 2, simply divide the total by 2 afterwards.
}

/*!
 * Single-threaded implementation of ``area``.
 *
 * This single-threaded implementation simply computes the area for each simple
 * polygon in the batch in sequence, and adds them to the resulting vector.
 * \tparam SimplePolygonBatch A class that behaves like a batch of simple
 * polygons.
 * \param batch The batch of simple polygons to compute the areas of.
 * \return A list of areas, one for each simple polygon, in the same order as
 * the order of those polygons in the batch.
 */
template<multi_polygonal SimplePolygonBatch>
Batch<area_t> area_st(const SimplePolygonBatch& batch) {
	Batch<area_t> result;
	result.reserve(batch.size());
	for(typename SimplePolygonBatch::const_iterator it = batch.begin(); it != batch.end(); ++it) {
		result.push_back(detail::area_st(*it));
	}
	return result;
}

/*!
 * Multi-threaded implementation of ``area``.
 *
 * This uses the shoelace formula to compute the area. The shoelace formula sums
 * the areas of the individual triangles formed by two adjacent vertices and the
 * coordinate origin.
 *
 * To calculate the area of a triangle with one vertex on the origin, we'll
 * calculate the area of a parallelogram formed by the original triangle and
 * that triangle mirrored around the line segment we're calculating the area
 * for. Visualise this:
 *
 * @image html res/shoelace_algorithm_parallelogram.svg
 *
 * The area of the parallelogram can be visualised by starting with a rectangle
 * that encloses the original triangle like this. The green areas fall outside
 * of the parallelogram and shouldn't be counted towards its area.
 *
 * @image html res/shoelace_algorithm_rectangle_overlay.svg
 *
 * The green areas are not part of the parallelogram here, but they can be
 * shifted towards the missing part that falls outside of the rectangle like
 * this.
 *
 * @image html res/shoelace_algorithm_multiple_rectangles.svg
 *
 * This forms a second rectangle, in this case a smaller one in the upper right
 * hand corner. The two green triangles partially overlap and go partially
 * outside of the parallelogram we're trying to get the area of. The part that
 * is overlap plus the part that goes outside of the parallelogram together
 * forms an area of \f$x_1 \cdot y_2\f$.
 *
 * The total area of the parallelogram then becomes the area of the rectangle
 * formed by \f$x_2 \cdot y_1\f$ minus the area formed by the other rectangle
 * formed by \f$x_1 \cdot y_2\f$. In other words, the area of the parallelogram
 * is \f$x_2 \cdot y_1 - x_1 \cdot y_2\f$. This needs to be divided by two to
 * arrive at the area of the triangle. The surface area of a simple polygon is
 * the sum of all of these triangles. This is the shoelace formula.
 *
 * In this implementation, the areas of these parallelograms are calculated in
 * parallel. This is summed with a parallel reduction, to produce the area of
 * the complete polygon.
 * \tparam SimplePolygon A class that behaves like a simple polygon.
 * \param polygon The simple polygon to calculate the area of.
 * \return The surface area of the simple polygon.
 */
template<polygonal SimplePolygon>
area_t area_mt(const SimplePolygon& polygon) {
	area_t area = 0;
	const size_t size = polygon.size();
	#pragma omp parallel for simd reduction(+:area)
	for(size_t vertex = 0; vertex < size; ++vertex) {
		size_t previous = (vertex - 1 + size) % size;
		area += static_cast<area_t>(polygon[previous].x) * polygon[vertex].y - static_cast<area_t>(polygon[previous].y) * polygon[vertex].x;
	}
	return area / 2;
}

/*!
 * Multi-threaded implementation of ``area``.
 *
 * This uses the shoelace formula to compute the area. The shoelace formula sums
 * the areas of the individual triangles formed by two adjacent vertices and the
 * coordinate origin.
 *
 * To calculate the area of a triangle with one vertex on the origin, we'll
 * calculate the area of a parallelogram formed by the original triangle and
 * that triangle mirrored around the line segment we're calculating the area
 * for. Visualise this:
 *
 * @image html res/shoelace_algorithm_parallelogram.svg
 *
 * The area of the parallelogram can be visualised by starting with a rectangle
 * that encloses the original triangle like this. The green areas fall outside
 * of the parallelogram and shouldn't be counted towards its area.
 *
 * @image html res/shoelace_algorithm_rectangle_overlay.svg
 *
 * The green areas are not part of the parallelogram here, but they can be
 * shifted towards the missing part that falls outside of the rectangle like
 * this.
 *
 * @image html res/shoelace_algorithm_multiple_rectangles.svg
 *
 * This forms a second rectangle, in this case a smaller one in the upper right
 * hand corner. The two green triangles partially overlap and go partially
 * outside of the parallelogram we're trying to get the area of. The part that
 * is overlap plus the part that goes outside of the parallelogram together
 * forms an area of \f$x_1 \cdot y_2\f$.
 *
 * The total area of the parallelogram then becomes the area of the rectangle
 * formed by \f$x_2 \cdot y_1\f$ minus the area formed by the other rectangle
 * formed by \f$x_1 \cdot y_2\f$. In other words, the area of the parallelogram
 * is \f$x_2 \cdot y_1 - x_1 \cdot y_2\f$. This needs to be divided by two to
 * arrive at the area of the triangle. The surface area of a simple polygon is
 * the sum of all of these triangles. This is the shoelace formula.
 *
 * In this implementation, the areas of these parallelograms are calculated in
 * parallel. This is summed with a parallel reduction, to produce the area of
 * the complete polygon. This is repeated in parallel for each polygon in the
 * batch.
 * \tparam SimplePolygonBatch A class that behaves like a batch of simple
 * polygons.
 * \param batch The batch of simple polygons to calculate the areas of.
 * \return A list of areas, one for each simple polygon, in the same order as
 * the order of those polygons in the batch.
 */
template<multi_polygonal SimplePolygonBatch>
Batch<area_t> area_mt(const SimplePolygonBatch& batch) {
	Batch<area_t> result;
	result.resize(batch.size()); //Resize, so that all threads can enter their data in parallel.

	#pragma omp parallel for
	for(size_t polygon_index = 0; polygon_index < batch.size(); ++polygon_index) {
		area_t area = 0;
		const auto& polygon = batch[polygon_index]; //Instantiates auto with whatever type the batch indexes.
		const size_t size = polygon.size();

		//While task loops may be more efficient in this case, they cause segfaults with GCC when the loop is empty.
		#pragma omp parallel for simd reduction(+:area)
		for(size_t vertex = 0; vertex < size; ++vertex) {
			size_t previous = (vertex - 1 + size) % size;
			area += static_cast<area_t>(polygon[previous].x) * polygon[vertex].y - static_cast<area_t>(polygon[previous].y) * polygon[vertex].x;
		}
		result[polygon_index] = area / 2;
	}
	return result;
}

#ifdef GPU
/*!
 * Implementation of ``area`` that runs on the graphics card, if available.
 *
 * This uses the shoelace formula to compute the area. The shoelace formula sums
 * the areas of the individual triangles formed by two adjacent vertices and the
 * coordinate origin.
 *
 * To calculate the area of a triangle with one vertex on the origin, we'll
 * calculate the area of a parallelogram formed by the original triangle and
 * that triangle mirrored around the line segment we're calculating the area
 * for. Visualise this:
 *
 * @image html res/shoelace_algorithm_parallelogram.svg
 *
 * The area of the parallelogram can be visualised by starting with a rectangle
 * that encloses the original triangle like this. The green areas fall outside
 * of the parallelogram and shouldn't be counted towards its area.
 *
 * @image html res/shoelace_algorithm_rectangle_overlay.svg
 *
 * The green areas are not part of the parallelogram here, but they can be
 * shifted towards the missing part that falls outside of the rectangle like
 * this.
 *
 * @image html res/shoelace_algorithm_multiple_rectangles.svg
 *
 * This forms a second rectangle, in this case a smaller one in the upper right
 * hand corner. The two green triangles partially overlap and go partially
 * outside of the parallelogram we're trying to get the area of. The part that
 * is overlap plus the part that goes outside of the parallelogram together
 * forms an area of \f$x_1 \cdot y_2\f$.
 *
 * The total area of the parallelogram then becomes the area of the rectangle
 * formed by \f$x_2 \cdot y_1\f$ minus the area formed by the other rectangle
 * formed by \f$x_1 \cdot y_2\f$. In other words, the area of the parallelogram
 * is \f$x_2 \cdot y_1 - x_1 \cdot y_2\f$. This needs to be divided by two to
 * arrive at the area of the triangle. The surface area of a simple polygon is
 * the sum of all of these triangles. This is the shoelace formula.
 *
 * In this implementation, the areas of these parallelograms are calculated on
 * the GPU, if available. This is summed with a parallel reduction, to produce
 * the area of the complete polygon.
 * \tparam SimplePolygon A class that behaves like a simple polygon.
 * \param polygon The simple polygon to calculate the area of.
 * \return The surface area of the simple polygon.
 */
template<polygonal SimplePolygon>
area_t area_gpu(const SimplePolygon& polygon) {
	area_t area = 0;
	const size_t size = polygon.size();
	const Point2* vertices = polygon.data();
	#pragma omp target teams distribute parallel for map(to:vertices[0:size]) map(tofrom:area) reduction(+:area)
	for(size_t vertex = 0; vertex < size; ++vertex) {
		size_t previous = (vertex - 1 + size) % size;
		area += static_cast<area_t>(vertices[previous].x) * vertices[vertex].y - static_cast<area_t>(vertices[previous].y) * vertices[vertex].x;
	}
	return area / 2;
}

/*!
 * Implementation of ``area`` that runs on the graphics card, if available.
 *
 * This uses the shoelace formula to compute the area. The shoelace formula sums
 * the areas of the individual triangles formed by two adjacent vertices and the
 * coordinate origin.
 *
 * To calculate the area of a triangle with one vertex on the origin, we'll
 * calculate the area of a parallelogram formed by the original triangle and
 * that triangle mirrored around the line segment we're calculating the area
 * for. Visualise this:
 *
 * @image html res/shoelace_algorithm_parallelogram.svg
 *
 * The area of the parallelogram can be visualised by starting with a rectangle
 * that encloses the original triangle like this. The green areas fall outside
 * of the parallelogram and shouldn't be counted towards its area.
 *
 * @image html res/shoelace_algorithm_rectangle_overlay.svg
 *
 * The green areas are not part of the parallelogram here, but they can be
 * shifted towards the missing part that falls outside of the rectangle like
 * this.
 *
 * @image html res/shoelace_algorithm_multiple_rectangles.svg
 *
 * This forms a second rectangle, in this case a smaller one in the upper right
 * hand corner. The two green triangles partially overlap and go partially
 * outside of the parallelogram we're trying to get the area of. The part that
 * is overlap plus the part that goes outside of the parallelogram together
 * forms an area of \f$x_1 \cdot y_2\f$.
 *
 * The total area of the parallelogram then becomes the area of the rectangle
 * formed by \f$x_2 \cdot y_1\f$ minus the area formed by the other rectangle
 * formed by \f$x_1 \cdot y_2\f$. In other words, the area of the parallelogram
 * is \f$x_2 \cdot y_1 - x_1 \cdot y_2\f$. This needs to be divided by two to
 * arrive at the area of the triangle. The surface area of a simple polygon is
 * the sum of all of these triangles. This is the shoelace formula.
 *
 * In this implementation, the areas of these parallelograms are calculated on
 * the GPU, if available. This is summed with a parallel reduction, to produce
 * the area of the complete polygon. This is repeated in parallel for each
 * polygon in the batch.
 * \tparam SimplePolygonBatch A class that behaves like a batch of simple
 * polygons.
 * \param batch The batch of simple polygons to calculate the areas of.
 * \return A list of areas, one for each simple polygon, in the same order as
 * the order of those polygons in the batch.
 */
template<multi_polygonal SimplePolygonBatch>
Batch<area_t> area_gpu(const SimplePolygonBatch& batch) {
	const size_t batch_size = batch.size();
	Batch<area_t> result;
	result.resize(batch_size); //Resize, so that all threads can enter their data in parallel.
	area_t* result_data = result.data();

	const Subbatch<Point2>* polygons = batch.data();
	const Point2* vertices = batch.data_subelements();
	const size_t vertices_size = batch.size_subelements();
	#pragma omp target teams distribute parallel for map(to:vertices[0:vertices_size]) map(to:polygons[0:batch_size]) map(from:result_data[0:batch_size])
	for(size_t polygon_index = 0; polygon_index < batch_size; ++polygon_index) {
		area_t area = 0;
		const auto& polygon = polygons[polygon_index];
		const size_t size = polygon.size();

		//On the GPU we'll spawn new threads for each sub-loop too.
		#pragma omp parallel for reduction(+:area)
		for(size_t vertex = 0; vertex < size; ++vertex) {
			size_t previous = (vertex - 1 + size) % size;
			area += static_cast<area_t>(polygon[previous].x) * polygon[vertex].y - static_cast<area_t>(polygon[previous].y) * polygon[vertex].x;
		}
		result_data[polygon_index] = area / 2;
	}
	return result;
}
#endif //GPU

}

}

#endif //APEX_AREA