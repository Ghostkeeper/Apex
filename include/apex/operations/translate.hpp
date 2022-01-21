/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_TRANSLATE
#define APEX_TRANSLATE

#include "../detail/geometry_concepts.hpp" //To disambiguate overloads.

namespace apex {

namespace detail {

//Declare the detail functions so that we can reference them from the public ones.
template<polygonal Polygon>
void translate_st(Polygon& polygon, const Point2& delta);

template<multi_polygonal PolygonBatch>
void translate_st(PolygonBatch& batch, const Point2& delta);

template<polygonal Polygon>
void translate_mt(Polygon& polygon, const Point2& delta);

#ifdef GPU
template<polygonal Polygon>
void translate_gpu(Polygon& polygon, const Point2& delta);
#endif

}

/*!
 * Moves a polygon with a certain offset.
 *
 * The polygon is moved in-place.
 * \tparam Polygon A class that behaves like a polygon.
 * \param polygon The polygon to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<polygonal Polygon>
void translate(Polygon& polygon, const Point2& delta) {
	detail::translate_st(polygon, delta);
}

/*!
 * Moves all polygons in a batch of polygons with a certain offset.
 *
 * The polygons are moved in-place. All polygons are moved with the same offset.
 * \tparam PolygonBatch A class that behaves like a batch of polygons.
 * \param batch The batch of polygons to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<multi_polygonal PolygonBatch>
void translate(PolygonBatch& batch, const Point2& delta) {
	detail::translate_st(batch, delta);
}

namespace detail {

/*!
 * Single-threaded implementation of \ref translate.
 *
 * This implementation simply adds the delta vector to each vertex in turn.
 * \tparam Polygon A class that behaves like a polygon.
 * \param polygon The polygon to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<polygonal Polygon>
void translate_st(Polygon& polygon, const Point2& delta) {
	for(Point2& vertex : polygon) {
		vertex += delta;
	}
}

/*!
 * Single-threaded implementation of \ref translate for batches of polygons.
 *
 * This implementation simply translates each polygon in turn.
 * \tparam PolygonBatch A class that behaves like a batch of polygons.
 * \param batch The batch of polygons to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<multi_polygonal PolygonBatch>
void translate_st(PolygonBatch& batch, const Point2& delta) {
	for(typename PolygonBatch::iterator polygon = batch.begin(); polygon != batch.end(); ++polygon) {
		translate_st(*polygon, delta);
	}
}

/*!
 * Multi-threaded implementation of \ref translate.
 *
 * This implementation simply modifies all vertices in parallel.
 * \tparam Polygon A class that behaves like a polygon.
 * \param polygon The polygon to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<polygonal Polygon>
void translate_mt(Polygon& polygon, const Point2& delta) {
	#pragma omp parallel for simd
	for(size_t vertex = 0; vertex < polygon.size(); ++vertex) {
		polygon[vertex] += delta;
	}
}

/*!
 * Multi-threaded implementation of \ref translate for batches of polygons.
 *
 * This implementation will translate all polygons in parallel. The vertices of
 * each polygon may be translated in parallel as well, if there are enough of
 * them. Small polygons will simply be processed thread-locally.
 * \tparam PolygonBatch A class that behaves like a batch of polygons.
 * \param batch The batch of polygons to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<multi_polygonal PolygonBatch>
void translate_mt(PolygonBatch& batch, const Point2& delta) {
	const int parallel_threshold = omp_get_num_procs() * 2; //A polygon is considered "big" if it's larger than twice the number of available polygons. We'll parallelise that then.
	#pragma omp parallel for
	for(size_t polygon = 0; polygon < batch.size(); ++polygon) {
		if(batch[polygon].size() < parallel_threshold) {
			translate_st(batch[polygon], delta);
		} else {
			#pragma omp parallel for simd
			for(size_t vertex = 0; vertex < batch[polygon].size(); ++vertex) {
				batch[polygon][vertex] += delta;
			}
		}
	}
}

#ifdef GPU
/*!
 * GPU-accelerated implementation of \ref translate.
 *
 * This implementation simply modifies all vertices in parallel.
 *
 * While this alternative is likely never the fastest option on its own, using
 * this may allow the data to remain on the GPU without communicating to the
 * host for just a translate operation, which may improve performance if handled
 * well.
 * \tparam Polygon A class that behaves like a polygon.
 * \param polygon The polygon to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<polygonal Polygon>
void translate_gpu(Polygon& polygon, const Point2& delta) {
	Point2* vertices = polygon.data();
	const size_t size = polygon.size();
	#pragma omp target teams distribute parallel for simd map(tofrom:vertices[0:size])
	for(size_t vertex = 0; vertex < size; ++vertex) {
		vertices[vertex] += delta;
	}
}

/*!
 * GPU-accelerated implementation of \ref translate for batches of polygons.
 *
 * This implementation ignores the boundaries of polygons and simply shifts all
 * vertices in the batch by the given delta, even those in unused regions of the
 * vertex buffer. This works well on batches which are space-optimised.
 * \tparam PolygonBatch A class that behaves like a batch of polygons.
 * \param batch The batch of polygons to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<multi_polygonal PolygonBatch>
void translate_gpu(PolygonBatch& batch, const Point2& delta) {
	Point2* vertices = batch.data_subelements();
	const size_t vertices_size = batch.size_subelements();
	#pragma omp target teams distribute parallel for simd map(tofrom:vertices[0:vertices_size])
	for(size_t vertex = 0; vertex < vertices_size; ++vertex) {
		vertices[vertex] += delta;
	}
}
#endif

}

}

#endif //APEX_TRANSLATE