/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_POLYGON_PROPERTIES
#define APEX_POLYGON_PROPERTIES

#include "convexity.hpp"
#include "orientation.hpp"
#include "self_intersecting.hpp"

namespace apex {

/*!
 * A collection of geometric properties of a polygon.
 *
 * These properties must not add any new information that cannot be derived from
 * the polygon itself. It is only used to cache information for more efficient
 * computation.
 */
struct PolygonProperties {
	/*!
	 * Stores the state of these properties of a polygon.
	 *
	 * These properties all have limited amounts of optional values, such that
	 * they can each be stored in a few bits. While C++ provides us with the
	 * bit field language feature, this has a few problems:
	 * - The bit field can only be modified one field at a time. When editing a
	 * polygon, it is very common that all fields are modified at once. The bits
	 * in this custom bit field are arranged such that this operation can be
	 * done with a single bitwise operation.
	 * - When changing a C++ bitfield, the compiler has to mask a bitwise
	 * operation to affect only certain bits, and then apply an override there.
	 * Using knowledge of which values are the most common, we can assign those
	 * to the bit values of all 0's or all 1's, which can be written using a
	 * single bitwise operator instead of with two.
	 * - The layout of bit fields is implementation-defined by the compiler. The
	 * nature of heterogeneous programming make it quite common to use a mix of
	 * compilers, where one compiler is used for the accelerator while another
	 * is used on the CPU. If those compilers use different formats for their
	 * bit fields, it would cause strange bugs. Implementing our own format
	 * makes it consistent regardless of the compiler.
	 *
	 * The current bit layout of this field is as follows.
	 * - The 2 least significant bits indicate the convexity.
	 * - The next 2 least significant bits indicate self-intersection.
	 * - The next 2 least significant bits indicate orientation.
	 * - The remaining 2 bits are unused.
	 */
	uint8_t bitfield;

	PolygonProperties() : bitfield(0) {}

	/*!
	 * Whether this polygon is convex or concave.
	 * \return Whether this polygon is convex or concave.
	 */
	Convexity convexity() {
		return static_cast<Convexity>(bitfield & 0b00000011);
	}

	/*!
	 * Whether this polygon has any of its edges intersecting each other.
	 * \return Whether this polygon has any of its edges intersecting each
	 * other.
	 */
	SelfIntersecting self_intersecting() {
		return static_cast<SelfIntersecting>((bitfield & 0b00001100) >> 2);
	}

	/*!
	 * The winding orientation of this polygon: Clockwise, counter-clockwise or
	 * a mix. For a mix, polygons have to be self-intersecting.
	 * \return The winding orientation of this polygon.
	 */
	Orientation orientation() {
		return static_cast<Orientation>((bitfield & 0b00110000) >> 4);
	}

	/*!
	 * Makes all properties unknown again.
	 *
	 * This is a helper function for a common operation. It is common because
	 * when a polygon is modified arbitrarily, these usually become
	 * unknown.
	 */
	void reset() {
		bitfield = 0;
	}
};

}

#endif //APEX_POLYGON_PROPERTIES