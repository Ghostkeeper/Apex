/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON_GENERATORS
#define APEX_SIMPLE_POLYGON_GENERATORS

#include <cmath> //To construct an approximation of a circle.
#include <numbers> //For pi, to construct an approximation of a circle.

#include "apex/simple_polygon.hpp" //We're generating these simple polygons.

namespace apex {
namespace data {

/* This file contains some procedurally-generated simple polygons.

Test data should in theory be stored as an SVG file in the data folder. That way
they can be visualised and edited easily. However this isn't always practical,
because some tests need to be very big. For these, it's nicer to procedurally
generate the simple polygons directly, rather than parse megabytes of SVG files.
*/


/*!
 * An approximation of a circle. This is actually a regular n-gon with 1 million
 * vertices.
 *
 * The radius is also 1 million, causing vertices to be spaced apart by on
 * average about tau (6.28) units. The surface area of this circle should
 * approximate the surface area of a circle. However the circumference will be
 * significantly higher than expected due to the rounding of the coordinates.
 */
SimplePolygon<> generate_simple_polygon_circle();
const SimplePolygon simple_polygon_circle = generate_simple_polygon_circle();

}
}

#endif //APEX_SIMPLE_POLYGON_GENERATORS