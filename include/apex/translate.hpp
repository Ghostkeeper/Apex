/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_TRANSLATE
#define APEX_TRANSLATE

#include "detail/geometry_concepts.hpp" //To disambiguate overloads.

namespace apex {

namespace detail {

//Declare the detail functions so that we can reference them from the public ones.
template<polygonal SimplePolygon>
void translate_st(SimplePolygon& polygon, const Point2& delta);

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

}

}

#endif //APEX_TRANSLATE