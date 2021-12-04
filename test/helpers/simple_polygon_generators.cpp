/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include "simple_polygon_generators.hpp" //The definitions we're implementing.

namespace apex {
namespace data {

SimplePolygon<> generate_simple_polygon_circle() {
	SimplePolygon result;
	constexpr size_t num_vertices = 1000000;
	constexpr coord_t radius = 1000000; //Prevent getting equal vertices by making them space out far enough.
	result.reserve(num_vertices);
	for(size_t vertex = 0; vertex < num_vertices; ++vertex) { //Construct a circle with lots of vertices.
		const coord_t x = std::lround(std::cos(std::numbers::pi * 2 / num_vertices * vertex) * radius);
		const coord_t y = std::lround(std::sin(std::numbers::pi * 2 / num_vertices * vertex) * radius);
		result.emplace_back(x, y);
	}
	return result;
}

}
}