/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON_TEST_CASES
#define APEX_SIMPLE_POLYGON_TEST_CASES

#include "apex/simple_polygon.hpp" //We're defining these simple polygons here.

namespace apex {

/*!
 * This class defines a bunch of simple polygons for use in tests.
 *
 * Some of these simple polygons are loaded from SVG files. This is useful
 * because SVG files are easy to visualise for a maintainer. They can be opened
 * with any web browser and most image viewers. This is the preferable way to
 * define a test case, because it's easy to maintain. The SVG files are included
 * with a preprocessor directive and stored statically in the binary. To be
 * directly included, it is first pre-processed by CMake in the build directory.
 *
 * Some simple polygons are procedurally generated. This is necessary for cases
 * that are extremely big. It wouldn't be practical to load SVG files of
 * multiple megabytes. Nor would it be practical to store them under version
 * control. Nobody would read all of that or edit it directly to maintain it
 * anyway.
 */
class SimplePolygonTestCases {
public:
	const SimplePolygon empty;
	const SimplePolygon point;
	const SimplePolygon line;
	const SimplePolygon square_1000;
	const SimplePolygon square_1000_negative_x;
	const SimplePolygon square_1000_negative_y;
	const SimplePolygon square_1000_negative_xy;
	const SimplePolygon square_1000_centred;
	const SimplePolygon triangle_1000;
	const SimplePolygon thin_rectangle;
	const SimplePolygon arrowhead;
	const SimplePolygon negative_square;
	const SimplePolygon hourglass;
	const SimplePolygon zero_width;
	const SimplePolygon circle;

	/*!
	 * Constructs all of the test cases.
	 */
	SimplePolygonTestCases();

	/*!
	 * Load a test case as a simple polygon.
	 *
	 * This will take the contents of an SVG file and look for a ``<polygon>``
	 * tag and parse its coordinates. Those coordinates are used to construct a
	 * simple polygon. This way, the easy-to-visualise SVG files can be used to
	 * create test cases.
	 *
	 * Only the first polygon in the file is used. No transformations or CSS is
	 * applied. Other SVG elements such as ``<path>`` are not parsed. The parser
	 * in this loader is extremely simple and naive. This prevents needing a
	 * whole new library just to read test cases.
	 * \param svg The contents of an SVG file containing the polygon to load.
	 * \return A SimplePolygon with the loaded test data.
	 */
	SimplePolygon load_simple_polygon(const std::string& svg);

	/*!
	 * An approximation of a circle. This is actually a regular n-gon with 1
	 * million vertices.
	 *
	 * The radius is also 1 million, causing vertices to be spaced apart by on
	 * average about tau (6.28) units. The surface area of this n-gon should
	 * approximate the surface area of a circle. However the circumference will
	 * be significantly higher than expected due to the rounding of the
	 * coordinates.
	 * \return A high-resolution approximation of a circle.
	 */
	SimplePolygon generate_circle();
};

}

#endif //APEX_SIMPLE_POLYGON_TEST_CASES