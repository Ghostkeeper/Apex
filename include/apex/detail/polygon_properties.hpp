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
	 * Whether this polygon is convex or concave.
	 */
	Convexity convexity : 2;

	/*!
	 * Whether this polygon has any of its edges intersecting each other.
	 */
	SelfIntersecting self_intersecting : 2;

	/*!
	 * The winding orientation of this polygon: Clockwise, counter-clockwise or
	 * a mix. For a mix, polygons have to be self-intersecting.
	 */
	Orientation orientation : 2;

	/*!
	 * Makes all properties unknown again.
	 *
	 * This is a helper function for a common operation. It is common because
	 * when a polygon is modified arbitrarily, these usually become
	 * unknown.
	 */
	void reset() {
		convexity = Convexity::UNKNOWN;
		self_intersecting = SelfIntersecting::UNKNOWN;
		orientation = Orientation::UNKNOWN;
	}
};

}

#endif //APEX_POLYGON_PROPERTIES