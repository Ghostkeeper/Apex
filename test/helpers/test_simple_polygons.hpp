/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_TEST_FILES
#define APEX_TEST_FILES

#include "apex/simple_polygon.hpp" //We're defining these simple polygons here.
#include "test_case_loader.hpp" //Generate test polygons with this function.

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

}

}

#endif //APEX_TEST_FILES