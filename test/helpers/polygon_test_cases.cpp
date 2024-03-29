/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <cmath> //For trigonometry functions to construct an approximation of a circle.
#include <numbers> //For use of pi to construct an approximation of a circle by radians.

#include "polygon_test_cases.hpp" //The definition we're implementing.

namespace apex {

Polygon PolygonTestCases::empty() {
	return load_polygon(
#include "test/data/polygon/empty.svg"
	);
}

Polygon PolygonTestCases::point() {
	return load_polygon(
#include "test/data/polygon/point.svg"
	);
}

Polygon PolygonTestCases::line() {
	return load_polygon(
#include "test/data/polygon/line.svg"
	);
}

Polygon PolygonTestCases::square_1000() {
	return load_polygon(
#include "test/data/polygon/square_1000.svg"
	);
}

Polygon PolygonTestCases::square_1000_negative_x() {
	return load_polygon(
#include "test/data/polygon/square_1000_negative_x.svg"
	);
}

Polygon PolygonTestCases::square_1000_negative_y() {
	return load_polygon(
#include "test/data/polygon/square_1000_negative_y.svg"
	);
}

Polygon PolygonTestCases::square_1000_negative_xy() {
	return load_polygon(
#include "test/data/polygon/square_1000_negative_xy.svg"
	);
}

Polygon PolygonTestCases::square_1000_centred() {
	return load_polygon(
#include "test/data/polygon/square_1000_centred.svg"
	);
}

Polygon PolygonTestCases::thin_rectangle() {
	return load_polygon(
#include "test/data/polygon/thin_rectangle.svg"
	);
}

Polygon PolygonTestCases::touching_edge() {
	return load_polygon(
#include "test/data/polygon/touching_edge.svg"
	);
}

Polygon PolygonTestCases::touching_vertex() {
	return load_polygon(
#include "test/data/polygon/touching_vertex.svg"
	);
}

Polygon PolygonTestCases::triangle_1000() {
	return load_polygon(
#include "test/data/polygon/triangle_1000.svg"
	);
}

Polygon PolygonTestCases::arrowhead() {
	return load_polygon(
#include "test/data/polygon/arrowhead.svg"
	);
}

Polygon PolygonTestCases::negative_square() {
	return load_polygon(
#include "test/data/polygon/negative_square.svg"
	);
}

Polygon PolygonTestCases::hourglass() {
	return load_polygon(
#include "test/data/polygon/hourglass.svg"
	);
}

Polygon PolygonTestCases::zero_length_segments() {
	return load_polygon(
#include "test/data/polygon/zero_length_segments.svg"
	);
}

Polygon PolygonTestCases::zero_width() {
	return load_polygon(
#include "test/data/polygon/zero_width.svg"
	);
}

Polygon PolygonTestCases::zero_width_connection() {
	return load_polygon(
#include "test/data/polygon/zero_width_connection.svg"
	);
}

Polygon PolygonTestCases::circle() {
	Polygon result;
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

Polygon PolygonTestCases::load_polygon(const std::string& svg) {
	Polygon result;

	size_t position = 0;
	position = svg.find("<polygon ", position) + 9;
	if(position == std::string::npos) { //There is no polygon in this SVG file.
		return result;
	}
	position = svg.find("points=\"", position) + 8;
	if(position == std::string::npos) { //There is no points in this polygon (or maybe whitespace is not as we expect here).
		return result;
	}
	const size_t points_end = svg.find("\"", position);
	if(points_end == std::string::npos) { //The points never end. Malformed file or incomplete.
		return result;
	}

	std::vector<coord_t> coordinates; //Until we paired them up, store all single coordinates.
	while(position < points_end) {
		while((svg[position] == ',' || svg[position] == ' ') && position < points_end) {
			++position; //Skip over delimiter(s).
		}
		//We are now at the start of the coordinate. Find the end.
		const size_t coordinate_end = std::min(svg.find(",", position), std::min(svg.find(" ", position), svg.find("\"", position)));

		//Turn this substring into a coordinate and store it.
		const std::string coordinate_str = svg.substr(position, coordinate_end - position);
		try {
			coordinates.push_back(std::stoll(coordinate_str));
		} catch(const std::invalid_argument exception) { //Not a valid numerical coordinate here.
			//Ignore. Just continue setting the position to the end of this coordinate and parse the next one.
		}

		//Continue from the end with finding the next coordinate.
		position = coordinate_end;
	}

	for(size_t coordinate = 0; coordinate + 1 < coordinates.size(); coordinate += 2) {
		result.emplace_back(coordinates[coordinate], coordinates[coordinate + 1]); //Combine each pair of coordinates into a point.
	}

	return result;
}

}