/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_POINT2
#define APEX_POINT2

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
	constexpr Point2(const coord_t x, const coord_t y) : x(x), y(y) {}

	/*
	 * Adds two points together element-wise.
	 *
	 * This is equivalent to translating this point by the coordinates of the
	 * other point (or vice versa).
	 * \param other The point to add to this point.
	 * \return A new point with the coordinates of both points summed together.
	 */
	constexpr Point2 operator +(const Point2& other) const {
		return Point2(x + other.x, y + other.y);
	}

	/*
	 * Adds another point to this point in-place.
	 *
	 * This is equivalent to translating this point by the coordinates of the
	 * other point.
	 * \param other The point to add to this point.
	 * \return This point.
	 */
	Point2& operator +=(const Point2& other) {
		x += other.x;
		y += other.y;
		return *this;
	}

	/*
	 * Subtracts the specified point from this point element-wise.
	 *
	 * This is equivalent to translating this point by the inverted coordinates
	 * of the other point.
	 * \param other The point to subtract from this point.
	 * \return A new point with the coordinates of the other point subtracted
	 * from this one.
	 */
	constexpr Point2 operator -(const Point2& other) const {
		return Point2(x - other.x, y - other.y);
	}

	/*
	 * Subtracts the specified point from this point in-place.
	 *
	 * This is equivalent to translating this point by the inverted coordinates
	 * of the other point.
	 * \param other The point to subtract from this point.
	 * \return This point.
	 */
	Point2& operator -=(const Point2& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	/*
	 * Compares two points for whether they point to the same location in space.
	 * \param other The point to compare with.
	 * \return ``true`` if the two points are equal, or ``false`` otherwise.
	 */
	constexpr bool operator ==(const Point2& other) const {
		return x == other.x && y == other.y;
	}

	/*
	 * Compares two points for whether they point to a different location in
	 * space.
	 * \param other The point to compare with.
	 * \return ``true`` if the two points are different, or ``false`` otherwise.
	 */
	constexpr bool operator !=(const Point2& other) const {
		return !((*this) == other);
	}
};

}

#endif //APEX_POINT2