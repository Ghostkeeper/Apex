/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef MOCKSIMPLEPOLYGON_HPP
#define MOCKSIMPLEPOLYGON_HPP

#include <vector>

#include "apex/point2.hpp"
#include "apex/detail/simple_polygon_area.hpp"

namespace apex {

class MockSimplePolygon : public std::vector<Point2>,
	public SimplePolygonArea<MockSimplePolygon> {

};

}

#endif //MOCKSIMPLEPOLYGON_HPP