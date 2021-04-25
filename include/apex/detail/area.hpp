/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON_AREA_HPP
#define APEX_SIMPLE_POLYGON_AREA_HPP

#include "../coordinate.hpp" //To return area_t.

namespace apex {

/*!
 * Implements the Curiously Recurring Template Pattern to separate out the
 * private functions to compute the area of a simple polygon.
 * \tparam SimplePolygonBase An implementation of SimplePolygon's footprint,
 * including private members.
 */
template <typename SimplePolygonBase>
class SimplePolygonArea
{
public:
	/*!
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
	/*!
	 * Single-threaded implementation of ``area``.
	 *
	 * This uses the shoelace formula to compute the area. The shoelace formula
	 * sums the areas of the individual triangles formed by two adjacent
	 * vertices and the coordinate origin.
	 *
	 * To calculate the area of a triangle with one vertex on the origin, we'll
	 * calculate the area of a parallelogram formed by the original triangle and
	 * that triangle mirrored around the line segment we're calculating the area
	 * for. Visualise this:
	 *
	 * @image html res/shoelace_algorithm_parallelogram.svg
	 *
	 * The area of the parallelogram can be visualised by starting with a
	 * rectangle that encloses the original triangle like this. The green areas
	 * fall outside of the parallelogram and shouldn't be counted towards its
	 * area.
	 *
	 * @image html res/shoelace_algorithm_rectangle_overlay.svg
	 *
	 * The green areas are not part of the parallelogram here, but they can be
	 * shifted towards the missing part that falls outside of the rectangle like
	 * this.
	 *
	 * @image html res/shoelace_algorithm_multiple_rectangles.svg
	 *
	 * This forms a second rectangle, in this case a smaller one in the upper
	 * right hand corner. The two green triangles partially overlap and go
	 * partially outside of the parallelogram we're trying to get the area of.
	 * The part that is overlap plus the part that goes outside of the
	 * parallelogram together forms an area of \f$x_1 \cdot y_2\f$.
	 *
	 * The total area of the parallelogram then becomes the area of the
	 * rectangle formed by \f$x_2 \cdot y_1\f$ minus the area formed by the
	 * other rectangle formed by \f$x_1 \cdot y_2\f$. In other words, the area
	 * of the parallelogram is \f$x_2 \cdot y_1 - x_1 \cdot y_2\f$. This needs
	 * to be divided by two to arrive at the area of the triangle. This is the
	 * shoelace formula.
	 * \return The surface area of the simple polygon.
	 */
	area_t area_st() const {
		area_t area = 0;
		for(size_t vertex = 0, previous = base().size() - 1; vertex < base().size(); vertex++) {
			area += static_cast<area_t>(base()[previous].x) * base()[vertex].y - static_cast<area_t>(base()[previous].y) * base()[vertex].x;
			previous = vertex;
		}
		return area / 2; //Instead of dividing each triangle's area by 2, simply divide the total by 2 afterwards.
	}

private:
	/*!
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