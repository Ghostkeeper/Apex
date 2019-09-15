/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include "coordinate.hpp" //Used to store the coordinates of this point.

namespace apex {

/*
 * Defines a point in 2D space.
 *
 * We call the two dimensions X and Y, by convention.
 *
 * The coordinates are stored as coord_t, which is a 32-bit integer type.
 */
struct Point2 {
public:
	/*
	 * The projection of this point on the X dimension.
	 */
	coord_t x;

	/*
	 * The projection of this point on the Y dimension.
	 */
	coord_t y;

	/*
	 * Create a new point.
	 * \param x The projection of the desired point on the X dimension.
	 * \param y The projection of the desired point on the Y dimension.
	 */
	Point2(const coord_t x, const coord_t y) : x(x), y(y) {}
}

}