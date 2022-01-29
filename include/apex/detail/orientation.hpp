/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_ORIENTATION
#define APEX_ORIENTATION

namespace apex {

/*!
 * A property of shapes that indicates the winding orientation of the shape.
 *
 * The winding orientation is a convention in geometrical algorithms where
 * shapes wind counter-clockwisely if they are solid shapes, but clockwisely if
 * they are negative shapes, meant to subtract from other shapes.
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

}

#endif //APEX_ORIENTATION