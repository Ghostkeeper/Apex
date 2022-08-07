/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef BENCHMARKER_GENERATORS
#define BENCHMARKER_GENERATORS

#include <apex/polygon.hpp>

namespace benchmarker {

/*!
 * Generate a regular polyhedron, approximating a circle, with a certain number
 * of vertices.
 */
apex::Polygon generate_polygon_circle(const size_t num_vertices);

}

#endif //BENCHMARKER_GENERATORS