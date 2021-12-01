/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_TEST_CASE_LOADER
#define APEX_TEST_CASE_LOADER

#include <string>

#include "apex/simple_polygon.hpp" //We can construct simple polygons here.

namespace apex {

/*!
 * Load a test case from a file as a simple polygon.
 *
 * This will search through the ``simple_polygon`` folder in the test cases for
 * an SVG file with the given name, followed by .svg. In that SVG file it will
 * look for a ``<polygon>`` tag and parse its coordinates. Those coordinates are
 * used to construct a simple polygon. This way, the easy-to-visualise SVG files
 * can be used to create test cases.
 *
 * Only the first polygon in the file is used. No transformations or CSS is
 * applied. Other SVG elements such as ``<path>`` are not parsed. The parser in
 * this loader is extremely simple and naive. This prevents needing a whole new
 * library just to read test cases.
 * \param name The name of the test case to load.
 * \return A SimplePolygon with the loaded test data.
 */
SimplePolygon<> load_simple_polygon(const std::string& name);

/*!
 * Load a file and return its entire contents as a string.
 *
 * This is a helper function for other loaders.
 *
 * If the file could not be loaded, an empty string is returned.
 *
 * The file is interpreted using the system default encoding. Using special
 * characters in the file should be considered unreliable.
 * \param path The path to the file to load.
 * \return The contents of the file at that location.
 */
std::string load_string(const std::string& path);

}

#endif //APEX_TEST_CASE_LOADER