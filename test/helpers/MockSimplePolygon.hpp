/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef MOCKSIMPLEPOLYGON_HPP
#define MOCKSIMPLEPOLYGON_HPP

#include <vector> //Provides the base mock implementation.

#include "apex/point2.hpp" //The coordinates of the polygon.
#include "apex/detail/simple_polygon_area.hpp" //To implement the CRTP to allow these classes to be cast to the mock.
#include "apex/detail/simple_polygon_translate.hpp"

namespace apex {

/*
 * This is a mock for the base SimplePolygon class.
 *
 * It replaces the complex behaviour of the SimplePolygon with a much simpler
 * implementation based on a simple vector of coordinates.
 *
 * This class also provides easy constructors for some default shapes to test
 * with. This way, it's much easier to construct a polygon with a simple shape
 * by way of a fixture.
 */
class MockSimplePolygon : public std::vector<Point2>,
		public SimplePolygonArea<MockSimplePolygon>,
		public SimplePolygonTranslate<MockSimplePolygon> {
public:
	/*
	 * Basic shapes to initialise the polygon as.
	 *
	 * These can be used in the constructor as a fixture of sorts.
	 */
	enum Shape {
		EMPTY, //The polygon is left empty.
		SQUARE_1000, //The polygon is filled with a 1000x1000 square.
		SQUARE_1000_NEGATIVE_X, //1000x1000 square completely in the negative X.
		SQUARE_1000_NEGATIVE_Y, //1000x1000 square completely in the negative Y.
		SQUARE_1000_NEGATIVE_XY, //1000x1000 square completely in the negative X and Y.
		SQUARE_1000_CENTRED, //1000x1000 square centred around the origin.
		TRIANGLE_1000, //Triangle with a 1000-wide base. It's 1000 units high.
		THIN_RECTANGLE, //1000x1 rectangle. Very thin in the Y direction.
		ARROWHEAD, //Triangle pointing up with less tall triangle subtracted from it, forming an arrowhead.
		NEGATIVE_SQUARE, //1000x1000 square, but the winding order is wrong so it's a negative area.
	};

	MockSimplePolygon(const Shape shape = Shape::EMPTY);
};

}

#endif //MOCKSIMPLEPOLYGON_HPP