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
template<polygonal SimplePolygon>
void translate_st(SimplePolygon& polygon, const Point2& delta);

template<multi_polygonal SimplePolygonBatch>
void translate_st(SimplePolygonBatch& batch, const Point2& delta);

template<polygonal SimplePolygon>
void translate_mt(SimplePolygon& polygon, const Point2& delta);

#ifdef GPU
template<polygonal SimplePolygon>
void translate_gpu(SimplePolygon& polygon, const Point2& delta);
#endif

}

/*!
 * Moves a polygon with a certain offset.
 *
 * The polygon is moved in-place.
 * \tparam SimplePolygon A class that behaves like a simple polygon.
 * \param polygon The polygon to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<polygonal SimplePolygon>
void translate(SimplePolygon& polygon, const Point2& delta) {
	detail::translate_st(polygon, delta);
}

/*!
 * Moves all polygons in a batch of polygons with a certain offset.
 *
 * The polygons are moved in-place. All polygons are moved with the same offset.
 * \tparam SimplePolygonBatch A class that behaves like a batch of simple
 * polygons.
 * \param batch The batch of simple polygons to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<multi_polygonal SimplePolygonBatch>
void translate(SimplePolygonBatch& batch, const Point2& delta) {
	detail::translate_st(batch, delta);
}

namespace detail {

/*!
 * Single-threaded implementation of \ref translate.
 *
 * This implementation simply adds the delta vector to each vertex in turn.
 * \tparam SimplePolygon A class that behaves like a simple polygon.
 * \param polygon The polygon to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<polygonal SimplePolygon>
void translate_st(SimplePolygon& polygon, const Point2& delta) {
	for(Point2& vertex : polygon) {
		vertex += delta;
	}
}

/*!
 * Single-threaded implementation of \ref translate for batches of polygons.
 *
 * This implementation simply translates each polygon in turn.
 * \tparam SimplePolygonBatch A class that behaves like a batch of simple
 * polygons.
 * \param batch The batch of simple polygons to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<multi_polygonal SimplePolygonBatch>
void translate_st(SimplePolygonBatch& batch, const Point2& delta) {
	for(typename SimplePolygonBatch::iterator polygon = batch.begin(); polygon != batch.end(); ++polygon) {
		translate_st(*polygon, delta);
	}
}

/*!
 * Multi-threaded implementation of \ref translate.
 *
 * This implementation simply modifies all vertices in parallel.
 * \tparam SimplePolygon A class that behaves like a simple polygon.
 * \param polygon The polygon to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<polygonal SimplePolygon>
void translate_mt(SimplePolygon& polygon, const Point2& delta) {
	#pragma omp parallel for simd
	for(size_t vertex = 0; vertex < polygon.size(); ++vertex) {
		polygon[vertex] += delta;
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
 * \tparam SimplePolygon A class that behaves like a simple polygon.
 * \param polygon The polygon to translate.
 * \param delta The distance by which to move, representing both dimensions to
 * move through as a single 2D vector.
 */
template<polygonal SimplePolygon>
void translate_gpu(SimplePolygon& polygon, const Point2& delta) {
	Point2* vertices = polygon.data();
	const size_t size = polygon.size();
	#pragma omp target teams distribute parallel for map(tofrom:vertices[0:size])
	for(size_t vertex = 0; vertex < size; ++vertex) {
		vertices[vertex] += delta;
	}
}
#endif

}

}

#endif //APEX_TRANSLATE