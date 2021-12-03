/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_TEST_FILES
#define APEX_TEST_FILES

#include <string>

/* This file defines a bunch of test files, included in the binary as strings.

These files are defined as inline raw string literals, using preprocessor
directives to define them as symbols in the program. The compiler will then put
them in the binary and make them accessible via a variable.

The include directive refers not directly to the file in the data directory, but
to a pre-processed file prepared by CMake.*/

namespace apex {

namespace data {

const std::string simple_polygon_square_1000 =
#include "test/data/simple_polygon/square_1000.svg"
;

}

}

#endif //APEX_TEST_FILES