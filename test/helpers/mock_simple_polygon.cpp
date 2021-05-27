/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <cmath> //To construct an approximation of a circle.

#include "mock_simple_polygon.hpp"

#define PI 3.14159265358979 //To construct an approximation of a circle.

namespace apex {

MockSimplePolygon::MockSimplePolygon(const MockSimplePolygon::Shape shape) {
	std::vector<Point2> vertices;
	switch(shape) {
		case EMPTY: {
			break;
		}
		case POINT: {
			vertices = {
				Point2(25, 25)
			};
			break;
		}
		case LINE: {
			vertices = {
				Point2(50, 100),
				Point2(100, 200)
			};
			break;
		}
		case SQUARE_1000: {
			vertices = {
				Point2(0, 0),
				Point2(1000, 0),
				Point2(1000, 1000),
				Point2(0, 1000)
			};
			break;
		}
		case SQUARE_1000_NEGATIVE_X: {
			vertices = {
				Point2(-1024, 0),
				Point2(-24, 0),
				Point2(-24, 1000),
				Point2(-1024, 1000)
			};
			break;
		}
		case SQUARE_1000_NEGATIVE_Y: {
			vertices = {
				Point2(0, -1024),
				Point2(1000, -1024),
				Point2(1000, -24),
				Point2(0, -24)
			};
			break;
		}
		case SQUARE_1000_NEGATIVE_XY: {
			vertices = {
				Point2(-1024, -1024),
				Point2(-24, -1024),
				Point2(-24, -24),
				Point2(-1024, -24)
			};
			break;
		}
		case SQUARE_1000_CENTRED: {
			vertices = {
				Point2(-500, -500),
				Point2(500, -500),
				Point2(500, 500),
				Point2(-500, 500)
			};
			break;
		}
		case TRIANGLE_1000: {
			vertices = {
				Point2(24, 24),
				Point2(1024, 24),
				Point2(524, 1024)
			};
			break;
		}
		case THIN_RECTANGLE: {
			vertices = {
				Point2(0, 0),
				Point2(1000, 0),
				Point2(1000, 1),
				Point2(0, 1)
			};
			break;
		}
		case ARROWHEAD: {
			vertices = {
				Point2(10, 10),
				Point2(510, 510),
				Point2(1010, 10),
				Point2(510, 1010)
			};
			break;
		}
		case NEGATIVE_SQUARE: {
			vertices = {
				Point2(0, 0),
				Point2(0, 1000),
				Point2(1000, 1000),
				Point2(1000, 0)
			};
			break;
		}
		case HOURGLASS: {
			vertices = {
				Point2(0, 0),
				Point2(1000, 1000),
				Point2(0, 1000),
				Point2(1000, 0)
			};
			break;
		}
		case ZERO_WIDTH: {
			vertices = {
				Point2(0, 0),
				Point2(1000, 0),
				Point2(1000, 0),
				Point2(0, 0)
			};
			break;
		}
		case CIRCLE: {
			constexpr size_t num_vertices = 1000000;
			constexpr coord_t radius = 1000000; //Prevent getting equal vertices by making them space out far enough.
			vertices.reserve(num_vertices);
			for(size_t vertex = 0; vertex < num_vertices; ++vertex) { //Construct a circle with lots of vertices.
				const coord_t x = std::lround(std::cos(PI * 2 / num_vertices * vertex) * radius);
				const coord_t y = std::lround(std::sin(PI * 2 / num_vertices * vertex) * radius);
				vertices.emplace_back(x, y);
			}
		}
	}
	(*this).swap(vertices);
}

}