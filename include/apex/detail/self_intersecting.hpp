/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SELF_INTERSECTING
#define APEX_SELF_INTERSECTING

namespace apex {

/*!
 * A property of shapes to indicate whether the border of the shape intersects
 * itself.
 *
 * A self-intersecting shape can be considered degenerate in some applications,
 * as it cannot exist in reality and makes it ambiguous what the inside of the
 * shape is. The inside of the shape depends on the fill rule applied to it.
 */
enum class SelfIntersecting {
	/*!
	 * It is not known if this shape is self-intersecting.
	 */
	UNKNOWN,

	/*!
	 * The shape is definitely self-intersecting.
	 */
	YES,

	/*!
	 * The shape is definitely not self-intersecting.
	 */
	NO,

	/*!
	 * While the shape does not properly intersect itself, it does intersect
	 * itself counting edge cases.
	 *
	 * This could be just single vertices touching, or parts of line segments,
	 * or the tips of curves touching tangentially.
	 */
	EDGE
};

}

#endif //APEX_SELF_INTERSECTING