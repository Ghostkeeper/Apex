/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include "mock_simple_polygon_batch.hpp" //The class we're implementing.

namespace apex {

MockSimplePolygonBatch::MockSimplePolygonBatch(const MockSimplePolygonBatch::Contents contents) {
	std::vector<MockSimplePolygon> polygons;
	switch(contents) {
		case EMPTY: {
			break;
		}
		case SINGLE_EMPTY: {
			polygons.emplace_back(MockSimplePolygon::Shape::EMPTY);
			break;
		}
		case SINGLE_POINT: {
			polygons.emplace_back(MockSimplePolygon::Shape::POINT);
			break;
		}
		case SINGLE_LINE: {
			polygons.emplace_back(MockSimplePolygon::Shape::LINE);
			break;
		}
		case SINGLE_SQUARE: {
			polygons.emplace_back(MockSimplePolygon::Shape::SQUARE_1000);
			break;
		}
		case SQUARE_TRIANGLE: {
			polygons.emplace_back(MockSimplePolygon::Shape::SQUARE_1000);
			polygons.emplace_back(MockSimplePolygon::Shape::TRIANGLE_1000);
			break;
		}
		case SQUARE_TRIANGLE_SQUARE: {
			polygons.emplace_back(MockSimplePolygon::Shape::SQUARE_1000);
			polygons.emplace_back(MockSimplePolygon::Shape::TRIANGLE_1000);
			polygons.emplace_back(MockSimplePolygon::Shape::SQUARE_1000);
			break;
		}
		case TWO_SQUARES: {
			polygons.emplace_back(MockSimplePolygon::Shape::SQUARE_1000);
			polygons.emplace_back(MockSimplePolygon::Shape::SQUARE_1000);
			break;
		}
		case EDGE_CASES: {
			polygons.emplace_back(MockSimplePolygon::Shape::NEGATIVE_SQUARE);
			polygons.emplace_back(MockSimplePolygon::Shape::HOURGLASS);
			polygons.emplace_back(MockSimplePolygon::Shape::ZERO_WIDTH);
			polygons.emplace_back(MockSimplePolygon::Shape::LINE);
			polygons.emplace_back(MockSimplePolygon::Shape::POINT);
			polygons.emplace_back(MockSimplePolygon::Shape::EMPTY);
			break;
		}
	}
	(*this).swap(polygons);
}

}