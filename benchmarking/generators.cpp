/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include "generators.hpp" //The definitions that this is implementing.

#include <cmath> //For trigonometry functions to construct an approximation of a circle.
#include <numbers> //For use of pi to construct an approximation of a circle by radians.

namespace benchmarker {

apex::Polygon generate_polygon_circle(const size_t num_vertices) {
	apex::Polygon result;
	const apex::coord_t radius = num_vertices * 4; //Prevent getting equal vertices by making them space out far enough.
	result.reserve(num_vertices);
	for(size_t vertex = 0; vertex < num_vertices; ++vertex) { //Construct a circle with lots of vertices.
		const apex::coord_t x = std::lround(std::cos(std::numbers::pi * 2 / num_vertices * vertex) * radius);
		const apex::coord_t y = std::lround(std::sin(std::numbers::pi * 2 / num_vertices * vertex) * radius);
		result.emplace_back(x, y);
	}
	return result;
}

apex::Batch<apex::Polygon> generate_polygon_batch_10gon(const size_t num_polygons) {
	apex::Batch<apex::Polygon> result;
	result.reserve(num_polygons);
	result.reserve_subelements(10 * num_polygons); //10 vertices for each polygon.

	apex::Polygon polygon; //A polygon to add to the batch repeatedly.
	for(size_t vertex = 0; vertex < 10; ++vertex) { //Construct a regular n-gon with 10 vertices.
		const apex::coord_t x = std::lround(std::cos(std::numbers::pi * 2 / 10 * vertex) * 40);
		const apex::coord_t y = std::lround(std::sin(std::numbers::pi * 2 / 10 * vertex) * 40);
		polygon.emplace_back(x, y);
	}

	for(size_t repeat = 0; repeat < num_polygons; ++repeat) {
		result.push_back(polygon); //Makes a copy.
	}
	return result;
}

}