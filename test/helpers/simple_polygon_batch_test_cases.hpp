/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON_BATCH_TEST_CASES
#define APEX_SIMPLE_POLYGON_BATCH_TEST_CASES

#include "apex/batch.hpp" //We're going to provide test cases of batches.
#include "apex/simple_polygon.hpp" //We're going to put simple polygons in the test batches.

namespace apex {

/*!
 * This class defines a bunch of batches of simple polygons for use in tests.
 *
 * Some of these batches are loaded from SVG files. This is useful because SVG
 * files are easy to visualise for a maintainer. They can be opened with any web
 * browser and most image viewers. This is the preferable way to define a test
 * case, because it's easy to maintain. The SVG files are included with a
 * preprocessor directive and stored statically in the binary. To be directly
 * included, it is first pre-processed by CMake in the build directory.
 *
 * Some batches are procedurally generated. This is necessary for cases that are
 * extremely big. It wouldn't be practical to load SVG files of multiple
 * megabytes. Nor would it be practical to store them under version control.
 * Nobody would read all of that or edit it directly to maintain them anyway.
 */
class SimplePolygonBatchTestCases {
public:
	const Batch<SimplePolygon> empty;
	const Batch<SimplePolygon> single_empty;
	const Batch<SimplePolygon> single_point;
	const Batch<SimplePolygon> single_line;
	const Batch<SimplePolygon> single_square;
	const Batch<SimplePolygon> square_triangle;
	const Batch<SimplePolygon> square_triangle_square;
	const Batch<SimplePolygon> two_squares;
	const Batch<SimplePolygon> edge_cases;
	const Batch<SimplePolygon> two_circles;

	/*!
	 * Constructs all of the test cases.
	 */
	SimplePolygonBatchTestCases();

	/*!
	 * Load a test case as a batch of simple polygons.
	 *
	 * This will take the contents of an SVG file and look for all ``<polygon>``
	 * tags and parse their coordinates. Those coordinates are used to construct
	 * a simple polygon, which is added to the batch. This way, the
	 * easy-to-visualise SVG files can be used to create test cases.
	 *
	 * No transformations or CSS is applied. Other SVG elements such as
	 * ``<path>`` are not parsed. The parser in this loader is extremely simple
	 * and naive. This prevents needing a whole new library just to read test
	 * cases.
	 * \param svg The contents of an SVG file containing the polygons to load.
	 * \return A ``Batch`` of ``SimplePolygon`` with the loaded test data.
	 */
	Batch<SimplePolygon> load_simple_polygon_batch(const std::string& svg);

	/*!
	 * Generates a batch with two approximations of a circle. They are actually
	 * regular n-gons with 1 million vertices each.
	 *
	 * The radius is also 1 million, causing vertices to be spaced apart by on
	 * average about tau (6.28) units. The surface area of these n-gons should
	 * approximate the surface area of a circle. However the circumference will
	 * be significantly higher than expected due to the rounding of the
	 * coordinates.
	 * The first polygon is centred around the origin. The second is offset by
	 * 1000 units to positive X.
	 * \return A batch containing two high-resolution approximations of circles.
	 */
	Batch<SimplePolygon> generate_two_circles();
};

}

#endif //APEX_TEST_SIMPLE_POLYGON_BATCHES