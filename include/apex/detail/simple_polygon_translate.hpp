/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SIMPLE_POLYGON_TRANSLATE_HPP
#define APEX_SIMPLE_POLYGON_TRANSLATE_HPP

namespace apex {

/*
 * Implements the Curiously Recurring Template Pattern to separate out the
 * private functions to translate a simple polygon.
 * \tparam SimplePolygonBase An implementation of SimplePolygon's footprint,
 * including private members.
 */
template<typename SimplePolygonBase>
class SimplePolygonTranslate {
public:
	/*
	 * Moves this polygon with a certain offset.
	 *
	 * The polygon is moved in-place.
	 * \param dx How far to move the polygon in the X direction. This could be
	 * negative.
	 * \param dy How far to move the polygon in the Y direction. This could be
	 * negative.
	 */
	void translate(const coord_t dx, const coord_t dy) {
		return translate(Point2(dx, dy));
	}

	/*
	 * Moves this polygon with a certain offset.
	 *
	 * The polygon is moved in-place.
	 * \param delta The distance by which to move, representing both dimensions
	 * to move through as a single 2D vector.
	 */
	void translate(const Point2& delta) {
		return translate_st(delta);
	}

protected:
	/*
	 * Single-threaded implementation of ``translate``.
	 */
	void translate_st(const Point2& delta) {
		for(Point2& vertex : base()) {
			vertex += delta;
		}
	}

private:
	/*
	 * Gives the base SimplePolygon instance via the template pattern, which is
	 * actually still this instance.
	 * \return This instance, cast to SimplePolygonBase.
	 */
	SimplePolygonBase& base() {
		return *static_cast<SimplePolygonBase*>(this);
	}
};

}

#endif //APEX_SIMPLE_POLYGON_TRANSLATE_HPP