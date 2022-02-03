/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_POLYGON_PROPERTIES
#define APEX_POLYGON_PROPERTIES

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
	 * This geometric property conveys whether a shape is convex or not.
	 *
	 * A shape is convex when any straight line segment starting and ending
	 * inside the shape stays completely inside the shape. If a straight line
	 * segment exists that starts and ends inside the shape, but halfway along
	 * exits the shape, the shape is not convex but concave.
	 */
	enum class Convexity {
		/*!
		 * No convexity information has been calculated yet about this shape.
		 */
		UNKNOWN = 0,

		/*!
		 * The shape is convex. No straight line between any points inside the
		 * shape will intersect the border of the shape.
		 */
		CONVEX = 1,

		/*!
		 * The shape is concave. There are straight lines between points inside
		 * the shape that intersect with its edges.
		 */
		CONCAVE = 2,

		/*!
		 * The shape is degenerate. Convexity has no meaning with this shape.
		 *
		 * For instance, the shape could be a point or a line, which has no
		 * positive area to draw lines in, or it could be self-intersecting.
		 */
		DEGENERATE = 3
	};

	/*!
	 * A property of shapes to indicate whether the border of the shape
	 * intersects itself.
	 *
	 * A self-intersecting shape can be considered degenerate in some
	 * applications, as it cannot exist in reality and makes it ambiguous what
	 * the inside of the shape is. The inside of the shape depends on the fill
	 * rule applied to it.
	 */
	enum class SelfIntersecting {
		/*!
		 * It is not known if this shape is self-intersecting.
		 */
		UNKNOWN = 0,

		/*!
		 * The shape is definitely self-intersecting.
		 */
		YES = 4,

		/*!
		 * The shape is definitely not self-intersecting.
		 */
		NO = 8,

		/*!
		 * While the shape does not properly intersect itself, it does intersect
		 * itself counting edge cases.
		 *
		 * This could be just single vertices touching, or parts of line
		 * segments, or the tips of curves touching tangentially.
		 */
		EDGE = 12
	};

	/*!
	 * A property of shapes that indicates the winding orientation of the shape.
	 *
	 * The winding orientation is a convention in geometrical algorithms where
	 * shapes wind counter-clockwisely if they are solid shapes, but clockwisely
	 * if they are negative shapes, meant to subtract from other shapes.
	 */
	enum class Orientation {
		/*!
		 * The orientation of this shape is unknown.
		 */
		UNKNOWN = 0,

		/*!
		 * The shape is entirely positive. There are no negative areas.
		 */
		POSITIVE = 16,

		/*!
		 * The shape is entirely negative. There are no positive areas.
		 */
		NEGATIVE = 32,

		/*!
		 * The shape contains both positive and negative areas.
		 */
		MIXED = 48
	};

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
	 * - The remaining bits of the int (however long that is) are unused.
	 * This uses an integer as bits, since it is by definition the same type as
	 * the enums that it is composed of, preventing the need for copies in order
	 * to cast. This makes at least 16 bits available (of which currently only 6
	 * are used).
	 */
	unsigned int bitfield;

	PolygonProperties() : bitfield(0) {}

	/*!
	 * Whether this polygon is convex or concave.
	 * \return Whether this polygon is convex or concave.
	 */
	Convexity convexity() const {
		return static_cast<Convexity>(bitfield & 0b11);
	}

	/*!
	 * Change the stored convexity value.
	 * \param convexity The new convexity to store.
	 */
	void set_convexity(const Convexity convexity) {
		bitfield = (bitfield & (~0b11)) + static_cast<int>(convexity); //Clear the convexity bits, then add the new convexity to them.
	}

	/*!
	 * Whether this polygon has any of its edges intersecting each other.
	 * \return Whether this polygon has any of its edges intersecting each
	 * other.
	 */
	SelfIntersecting self_intersecting() const {
		return static_cast<SelfIntersecting>(bitfield & 0b1100);
	}

	/*!
	 * Change the stored self-intersecting value.
	 * \param self_intersecting The new self-intersecting value to store.
	 */
	void set_self_intersecting(const SelfIntersecting self_intersecting) {
		bitfield = (bitfield & (~0b1100)) + static_cast<int>(self_intersecting); //Clear the self-intersecting bits, then add the new value to them.
	}

	/*!
	 * The winding orientation of this polygon: Clockwise, counter-clockwise or
	 * a mix. For a mix, polygons have to be self-intersecting.
	 * \return The winding orientation of this polygon.
	 */
	Orientation orientation() const {
		return static_cast<Orientation>(bitfield & 0b110000);
	}

	/*!
	 * Change the stored orientation value.
	 * \param orientation The new orientation to store.
	 */
	void set_orientation(const Orientation orientation) {
		bitfield = (bitfield & (~0b110000)) + static_cast<int>(orientation); //Clear the orientation bits, then add the new orientation to them.
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