/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_MOCK_SIMPLE_POLYGON_BATCH
#define APEX_MOCK_SIMPLE_POLYGON_BATCH

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
class MockSimplePolygonBatch : public std::vector<std::vector<Point2>>,
		public SimplePolygonBatchArea<MockSimplePolygonBatch> {
};

}

#endif //APEX_MOCK_SIMPLE_POLYGON_BATCH