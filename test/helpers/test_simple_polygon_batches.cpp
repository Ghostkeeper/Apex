/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include "test_simple_polygon_batches.hpp" //The definitions we're implementing.
#include "test_simple_polygons.hpp" //Generate individual polygons that we put in these batches.

namespace apex {
namespace data {

Batch<SimplePolygon> load_simple_polygon_batch(const std::string& svg) {
	Batch<SimplePolygon> result;

	size_t position = 0;
	while((position = svg.find("<polygon ", position)) != std::string::npos) {
		position += 9; //Skip over the "<polygon " part.
		result.emplace_back();

		position = svg.find("points=\"", position) + 8;
		if(position == std::string::npos) { //There is no points in this polygon or any following (or maybe whitespace is not as we expect here).
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
			result.back().emplace_back(coordinates[coordinate], coordinates[coordinate + 1]); //Combine each pair of coordinates into a point.
		}
	}

	return result;
}

Batch<SimplePolygon> generate_simple_polygon_batch_two_circles() {
	Batch<SimplePolygon> result;

	result.push_back(generate_simple_polygon_circle());
	result.push_back(generate_simple_polygon_circle());

	//Move the second circle 1000 points to positive X.
	for(Point2& vertex : result.back()) {
		vertex.x += 1000;
	}
	return result;
}

}
}