/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_TEST_SIMPLE_POLYGONS
#define APEX_TEST_SIMPLE_POLYGONS

#include "apex/simple_polygon.hpp" //We're defining these simple polygons here.

/* This file defines a bunch of simple polygons for use in tests.

 */
/* This file defines a bunch of test files, included in the binary as strings.

These files are defined as inline raw string literals, using preprocessor
directives to define them as symbols in the program. The compiler will then put
them in the binary and make them accessible via a variable.

The include directive refers not directly to the file in the data directory, but
to a pre-processed file prepared by CMake.*/

namespace apex {
namespace data {

/*!
 * Load a test case as a simple polygon.
 *
 * This will take the contents of an SVG file and look for a ``<polygon>`` tag
 * and parse its coordinates. Those coordinates are used to construct a simple
 * polygon. This way, the easy-to-visualise SVG files can be used to create test
 * cases.
 *
 * Only the first polygon in the file is used. No transformations or CSS is
 * applied. Other SVG elements such as ``<path>`` are not parsed. The parser in
 * this loader is extremely simple and naive. This prevents needing a whole new
 * library just to read test cases.
 * \param svg The contents of an SVG file containing the polygon to load.
 * \return A SimplePolygon with the loaded test data.
 */
SimplePolygon<> load_simple_polygon(const std::string& svg);

const SimplePolygon<> simple_polygon_point = load_simple_polygon(
#include "test/data/simple_polygon/point.svg"
);
const SimplePolygon<> simple_polygon_line = load_simple_polygon(
#include "test/data/simple_polygon/line.svg"
);
const SimplePolygon<> simple_polygon_square_1000 = load_simple_polygon(
#include "test/data/simple_polygon/square_1000.svg"
);
const SimplePolygon<> simple_polygon_square_1000_negative_x = load_simple_polygon(
#include "test/data/simple_polygon/square_1000_negative_x.svg"
);
const SimplePolygon<> simple_polygon_square_1000_negative_y = load_simple_polygon(
#include "test/data/simple_polygon/square_1000_negative_y.svg"
);
const SimplePolygon<> simple_polygon_square_1000_negative_xy = load_simple_polygon(
#include "test/data/simple_polygon/square_1000_negative_xy.svg"
);
const SimplePolygon<> simple_polygon_square_1000_centred = load_simple_polygon(
#include "test/data/simple_polygon/square_1000_centred.svg"
);
const SimplePolygon<> simple_polygon_triangle_1000 = load_simple_polygon(
#include "test/data/simple_polygon/triangle_1000.svg"
);
const SimplePolygon<> simple_polygon_thin_rectangle = load_simple_polygon(
#include "test/data/simple_polygon/thin_rectangle.svg"
);
const SimplePolygon<> simple_polygon_arrowhead = load_simple_polygon(
#include "test/data/simple_polygon/arrowhead.svg"
);
const SimplePolygon<> simple_polygon_negative_square = load_simple_polygon(
#include "test/data/simple_polygon/negative_square.svg"
);
const SimplePolygon<> simple_polygon_hourglass = load_simple_polygon(
#include "test/data/simple_polygon/hourglass.svg"
);
const SimplePolygon<> simple_polygon_zero_width = load_simple_polygon(
#include "test/data/simple_polygon/zero_width.svg"
);

/* Below are some procedurally generated polygons.

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

#endif //APEX_TEST_SIMPLE_POLYGONS