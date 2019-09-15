/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_COORDINATE
#define APEX_COORDINATE

#include <cstdint> //To use fixed-precision integers so that we are guaranteed a certain range of accuracy.

namespace apex {

/*
 * The type used to store coordinates in space.
 *
 * This type is an integer type rather than a floating point type, so no partial
 * unit coordinates are possible. This is intended to prevent inaccuracies due
 * to build-up of rounding errors.
 */
typedef int32_t coord_t; //Must have 32 bits to allow for single-width entries in OpenCL with cl_int. Anything else kills performance.

/*
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

}

#endif //APEX_COORDINATE