/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON_AREA_HPP
#define APEX_SIMPLE_POLYGON_AREA_HPP

//This is an implementation detail of SimplePolygon. It must not be included outside of the SimplePolygon class!

/*
 * Single-threaded implementation of ``area``.
 *
 * This uses the shoelace formula to compute the area.
 * \return The surface area of the simple polygon.
 */
area_t area_st() const {
	area_t area = 0;
	for(size_t vertex = 0, previous = size() - 1; vertex < size(); vertex++) {
		area += static_cast<area_t>((*this)[previous].x) * (*this)[vertex].y - static_cast<area_t>((*this)[previous].y) * (*this)[vertex].x;
		previous = vertex;
	}
	return area / 2;
}

#endif //APEX_SIMPLE_POLYGON_AREA_HPP