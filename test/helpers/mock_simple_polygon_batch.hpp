/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_MOCK_SIMPLE_POLYGON_BATCH
#define APEX_MOCK_SIMPLE_POLYGON_BATCH

#include <vector> //Provides the base mock implementation.

#include "apex/point2.hpp" //The coordinates of the polygons.
#include "apex/detail/area.hpp" //To implement the CRTP to allow these classes to be cast to the mock.
#include "mock_simple_polygon.hpp" //The contents of the batch are themselves mocked.

namespace apex {

/*!
 * This is a mock for the base SimplePolygonBatch class.
 *
 * It replaces the complex behaviour of the simple polygon batch with a much
 * simpler implementation based on a vector of vectors of points. That way you
 * can test the behaviour of an algorithm without depending on the actual
 * implementation of the ``SimplePolygonBatch`` class. If there are bugs in the
 * base class, they won't cause the tests for the algorithms to fail.
 *
 * This class also provides some easy constructors that fill the batch with some
 * default contents to test with. This way, it's much easier to construct a
 * batch with some simple shapes by way of a fixture. The algorithms will become
 * much slower though, since the efficient buffers with vertex data will be
 * created on the fly.
 */
class MockSimplePolygonBatch : public std::vector<MockSimplePolygon>,
		public SimplePolygonBatchArea<MockSimplePolygonBatch> {
public:
	/*!
	 * Some basic contents to initialise the batch with.
	 *
	 * These can be used in the constructor as a fixture of sorts to test with.
	 */
	enum Contents {
		EMPTY, //The batch is left empty (0 polygons).
		SINGLE_EMPTY, //The batch contains a single polygon with 0 vertices.
	};

	MockSimplePolygonBatch(const Contents contents = Contents::EMPTY);
};

}

#endif //APEX_MOCK_SIMPLE_POLYGON_BATCH