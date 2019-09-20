/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include "MockSimplePolygon.hpp"

namespace apex {

MockSimplePolygon::MockSimplePolygon(const MockSimplePolygon::Shape shape) {
	std::vector<Point2> vertices;
	switch(shape) {
		case EMPTY: {
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
	}
	(*this).swap(vertices);
}

}