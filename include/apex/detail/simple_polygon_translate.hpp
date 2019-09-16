/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef SIMPLE_POLYGON_TRANSLATE_HPP
#define SIMPLE_POLYGON_TRANSLATE_HPP

/*
 * Single-threaded implementation of ``translate``.
 */
void translate_st(const Point2& delta) {
	for(Point2& vertex : *this) {
		vertex += delta;
	}
}

#endif //SIMPLE_POLYGON_TRANSLATE_HPP