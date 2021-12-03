/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include "apex/coordinate.hpp" //To store coordinates until we create testing polygons out of them.
#include "test_case_loader.hpp" //The functions we're implementing.

namespace apex {

SimplePolygon<> load_simple_polygon(const std::string& svg) {
	SimplePolygon<> result;

	size_t position = 0;
	position = svg.find("<polygon ", position) + 9;
	position = svg.find("points=\"", position) + 8;
	const size_t points_end = svg.find("\"", position);

	std::vector<coord_t> coordinates; //Until we paired them up, store all single coordinates.
	while(position < points_end) {
		while(svg[position] == ',' || svg[position] == ' ') {
			++position; //Skip over delimiter(s).
		}
		//We are now at the start of the coordinate. Find the end.
		const size_t coordinate_end = std::min(svg.find(","), svg.find(" "));

		//Turn this substring into a coordinate and store it.
		const std::string coordinate_str = svg.substr(position, coordinate_end - position);
		coordinates.push_back(std::stoll(coordinate_str));

		//Continue from the end with finding the next coordinate.
		position = coordinate_end;
	}

	for(size_t coordinate = 0; coordinate < coordinates.size() - 1; coordinate += 2) {
		result.emplace_back(coordinates[coordinate], coordinates[coordinate + 1]); //Combine each pair of coordinates into a point.
	}

	return result;
}

}