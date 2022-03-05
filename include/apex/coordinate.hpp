/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_COORDINATE
#define APEX_COORDINATE

#include <cstdint> //To use fixed-precision integers so that we are guaranteed a certain range of accuracy.

namespace apex {

/*!
 * The type used to store coordinates in space.
 *
 * This type is an integer type rather than a floating point type, so no partial
 * unit coordinates are possible. This is intended to prevent inaccuracies due
 * to build-up of rounding errors.
 */
typedef int32_t coord_t; //Must have 32 bits to allow for single-width entries in OpenCL with cl_int. Anything else kills performance.

/*!
 * The type used to store the surface area of 2-dimensional shapes.
 *
 * Areas can be negative. This is used to indicate the surface area of parts of
 * complex shapes and self-intersecting shapes.
 *
 * This type is meant to guarantee that every shape that can be represented by
 * the coordinate system of coord_t can have its area properly calculated.
 * However due to the limits of the available types and because it needs to hold
 * negative areas, it'd need a 65-bit integer to be able to properly hold the
 * maximum or minimum area. Instead, this area can properly hold up to half of
 * the full coordinate space.
 */
typedef int64_t area_t; //Preferably 65 bits, but this is the practical limit of the coordinate system. Going quadruple width would kill performance.

/*!
 * Divides a numerator by a denominator, and rounds the result to the nearest
 * integer.
 *
 * If the division ends up exactly halfway between two integers, the result is
 * rounded away from zero. For instance, 10 / 4 = 2.5 would get rounded to 3,
 * while -10 / 4 = -2.5 would get rounded to -3.
 *
 * This function never transitions to floating point numbers, so it can safely
 * be used with very big numbers. Overflows and underflows can happen, but only
 * if the numbers summed together would overflow.
 * \tparam IntegerLike This function works with any type of integer.
 * \param numerator The number to divide.
 * \param denominator The number to divide it by.
 * \return The numerator divided by the denominator, rounded to the nearest
 * integer.
 */
template<typename IntegerLike>
IntegerLike round_divide(const IntegerLike numerator, const IntegerLike denominator) {
	if((numerator < 0) ^ (denominator < 0)) { //Either the numerator or the denominator is negative, so the result must be negative.
		return (numerator - denominator / 2) / denominator; //Flip the .5 offset to do proper rounding in the negatives too.
	} else {
		return (numerator + denominator / 2) / denominator;
	}
}

}

#endif //APEX_COORDINATE