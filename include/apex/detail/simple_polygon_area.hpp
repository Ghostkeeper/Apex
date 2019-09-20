/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON_AREA_HPP
#define APEX_SIMPLE_POLYGON_AREA_HPP

#include "../coordinate.hpp" //To return area_t.

namespace apex {

/*
 * Implements the Curiously Recurring Template Pattern to separate out the
 * private functions to compute the area of a simple polygon.
 * \tparam SimplePolygonBase An implementation of SimplePolygon's footprint,
 * including private members.
 */
template <typename SimplePolygonBase>
class SimplePolygonArea
{
public:
	/*
	 * Computes the surface area of the simple polygon.
	 *
	 * The sign of the area is linked to the polygon winding order. If the
	 * polygon is positive, the area will be positive too, and vice versa. If
	 * the polygon intersects itself, parts of the polygon will be subtracting
	 * from the area while other parts add up to the area.
	 *
	 * The area of the polygon is counted differently from the nonzero or
	 * even-odd fill rules. If a zone is looped around multiple times by the
	 * polygon, it will count to the total area multiple times as well.
	 * \return The surface area of the simple polygon.
	 */
	area_t area() const {
		return area_st();
	}

protected:
	/*
	 * Single-threaded implementation of ``area``.
	 *
	 * This uses the shoelace formula to compute the area.
	 * \return The surface area of the simple polygon.
	 */
	area_t area_st() const {
		area_t area = 0;
		for(size_t vertex = 0, previous = base().size() - 1; vertex < base().size(); vertex++) {
			area += static_cast<area_t>(base()[previous].x) * base()[vertex].y - static_cast<area_t>(base()[previous].y) * base()[vertex].x;
			previous = vertex;
		}
		return area / 2;
	}

private:
	/*
	 * Gives the base SimplePolygon instance via the template pattern, which is
	 * actually still this instance.
	 * \return This instance, cast to SimplePolygonBase.
	 */
	const SimplePolygonBase& base() const {
		return *static_cast<const SimplePolygonBase*>(this);
	}

};

}

#endif //APEX_SIMPLE_POLYGON_AREA_HPP