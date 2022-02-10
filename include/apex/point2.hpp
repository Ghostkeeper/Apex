/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_POINT2
#define APEX_POINT2

#include <ostream> //To overload printing the point to a stream.

#include "coordinate.hpp" //Used to store the coordinates of this point.

namespace apex {

/*!
 * Defines a point in 2D space.
 *
 * We call the two dimensions X and Y, by convention.
 *
 * The coordinates are stored as coord_t, which is a 32-bit integer type.
 */
struct Point2 {
public:
	/*!
	 * The projection of this point on the X dimension.
	 */
	coord_t x;

	/*!
	 * The projection of this point on the Y dimension.
	 */
	coord_t y;

	/*!
	 * Create a new point at position [0,0].
	 */
	constexpr Point2() : x(0), y(0) {}

	/*!
	 * Create a new point.
	 * \param x The projection of the desired point on the X dimension.
	 * \param y The projection of the desired point on the Y dimension.
	 */
	constexpr Point2(const coord_t x, const coord_t y) : x(x), y(y) {}

	/*!
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

	/*!
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

	/*!
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

	/*!
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

	/*!
	 * Compares two points for whether they point to the same location in space.
	 * \param other The point to compare with.
	 * \return ``true`` if the two points are equal, or ``false`` otherwise.
	 */
	constexpr bool operator ==(const Point2& other) const {
		return x == other.x && y == other.y;
	}

	/*!
	 * Compares two points for whether they point to a different location in
	 * space.
	 * \param other The point to compare with.
	 * \return ``true`` if the two points are different, or ``false`` otherwise.
	 */
	constexpr bool operator !=(const Point2& other) const {
		return !((*this) == other);
	}

	/*!
	 * Overloads streaming this point.
	 *
	 * This is useful for debugging, since it allows printing the point to a stream
	 * directly, giving you a reasonably readable output.
	 */
	friend std::ostream& operator <<(std::ostream& output_stream, const Point2& point) {
		return output_stream << "[" << point.x << "," << point.y << "]";
	}

	/*!
	 * Compute the magnitude of the cross product of this 2D vector with another
	 * 2D vector.
	 *
	 * The cross product is normally defined only for 3D vectors. This function
	 * takes the cross product between two 3D vectors where the third coordinate
	 * is 0. The cross product would then be the 3D vector that is perpendicular
	 * to both of these vectors. As both of these vectors are on the plane of
	 * those first two dimensions, the cross product would end up on position
	 * [0, 0], but with a third dimension. The third dimension is returned here.
	 *
	 * The direction of the cross product is characterised with the right-hand
	 * rule, with the two vectors of the input vectors as the index and middle
	 * fingers, and the resulting cross product in the direction of the thumb.
	 * It is anticommutative, meaning that taking the cross product of this
	 * point with another will yield an inverted result to taking the cross
	 * product of that point with this point.
	 *
	 * The magnitude of the cross product is the area of the parallelogram
	 * containing the two vectors as two of its sides. The result of this
	 * function is an area, as a result. The result could be so big that it
	 * cannot be represented as an actual vector in this library. This geometric
	 * property can be useful to work with the angle between the two vectors.
	 * \param other The other point, considered as a 2D vector, to compute the
	 * cross product with.
	 * \return The magnitude of the cross product of this vector with the other
	 * vector.
	 */
	area_t cross_product(const Point2& other) {
		return 0; //TODO
	}

	/*!
	 * The possible orientations that a point could have with respect to a line.
	 */
	enum LineOrientation {
		/*!
		 * The point is left of the line, when seen from the line's start to its
		 * end position.
		 */
		LEFT,

		/*!
		 * The point is right of the line, when seen from the line's start to
		 * its end position.
		 */
		RIGHT,

		/*!
		 * The point is exactly on the line.
		 */
		COLINEAR
	};

	/*!
	 * Find the orientation of this point with respect to a line.
	 *
	 * The line is considered infinite, going through the two given points. The
	 * line has a direction, going from the given \p line_start to the given
	 * \p line_end position.
	 *
	 * The line's start and end positions are not allowed to be equal. The
	 * direction of the line would be ambiguous then. The result of such a
	 * computation is left undefined.
	 * \param line_start One point on the line of which to check the
	 * orientation.
	 * \param line_end The second point on the line of which to check the
	 * orientation.
	 */
	constexpr LineOrientation orientation(const Point2& line_start, const Point2& line_end) {
		return LineOrientation::COLINEAR; //TODO: Implement.
	}
};

}

#endif //APEX_POINT2