/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_GEOMETRY_CONCEPTS
#define APEX_GEOMETRY_CONCEPTS

#include "../point2.hpp" //To require return types.

namespace apex {

/*!
 * A concept for objects that are like polygons.
 *
 * Polygons are two-dimensional shapes represented by a finite number of
 * incident line segments. In our implementation, those line segments are
 * represented by a list of vertices (\ref Point2).
 */
template<typename T>
concept polygonal = requires(T object) {
	{ object.size() } -> std::unsigned_integral;
	{ object[1] } -> std::convertible_to<Point2>;
};

/*!
 * A concept for objects that are like collections of polygons or complex
 * polygons.
 *
 * These are lists of polygons, be it batches of them or a complex polygon.
 */
template<typename T>
concept multi_polygonal = requires(T object) {
	{ object.size() } -> std::unsigned_integral;
	{ object[1] } -> polygonal;
};

}

#endif //APEX_GEOMETRY_CONCEPTS