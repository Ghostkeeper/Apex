/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <cmath> //To construct an octagon.
#include <gtest/gtest.h> //To run the test.

#include "apex/coordinate.hpp" //To construct an octagon.
#include "apex/simple_polygon.hpp" //The code under test.

namespace apex {

/*
 * Fixture that contains a few pre-constructed simple polygons.
 */
class SimplePolygonFixture : public ::testing::Test {
public:
	/*
	 * A simple shape with three vertices.
	 */
	SimplePolygon triangle;

	/*
	 * A regular octagon.
	 */
	SimplePolygon octagon;

	/*
	 * Constructs the fixture simple polygons.
	 */
	void SetUp() {
		triangle.emplace_back(20, 20);
		triangle.emplace_back(100, 20);
		triangle.emplace_back(60, 60);

		constexpr coord_t width = 1000;
		constexpr coord_t corner_size = width * (std::sqrt(2) - 1);
		octagon.emplace_back(corner_size, 0);
		octagon.emplace_back(width - corner_size, 0);
		octagon.emplace_back(width, corner_size);
		octagon.emplace_back(width, width - corner_size);
		octagon.emplace_back(width - corner_size, width);
		octagon.emplace_back(corner_size, width);
		octagon.emplace_back(0, width - corner_size);
		octagon.emplace_back(0, corner_size);
	}
};

/*
 * Tests constructing an empty simple polygon.
 */
TEST(SimplePolygon, ConstructEmpty) {
	SimplePolygon empty;
	EXPECT_EQ(empty.size(), 0);
}

/*
 * Tests copy-constructing a simple polygon.
 */
TEST_F(SimplePolygonFixture, ConstructCopy) {
	SimplePolygon copy(triangle); //Run the copy constructor.

	EXPECT_EQ(triangle, copy);

	copy[0].x += 1; //Modify the copy so that we're sure this is not held by reference or anything.
	EXPECT_NE(triangle, copy);
}

/*
 * Tests move-constructing a simple polygon.
 */
TEST_F(SimplePolygonFixture, ConstructMove) {
	SimplePolygon copy = triangle; //Make a copy to move so we keep the original fixture to compare against.
	SimplePolygon target = std::move(copy);

	EXPECT_EQ(triangle, target);
}

/*
 * Tests making a copy via assignment.
 */
TEST_F(SimplePolygonFixture, Assignment) {
	SimplePolygon copy = triangle; //Make a copy by assignment.

	EXPECT_EQ(triangle, copy);

	copy[0].x += 1; //Modify the copy so that we're sure this is not held by reference or anything.
	EXPECT_NE(triangle, copy);
}

/*
 * Tests accessing the vertices of the simple polygon by reference.
 */
TEST_F(SimplePolygonFixture, AccessReference) {
	Point2& vertex = triangle[0];
	vertex.x = 42; //Modify it.
	EXPECT_EQ(triangle[0].x, 42) << "The modified coordinates should be modified by reference, so it should be stored in the simple polygon as well.";

	triangle[0].y = 69;
	EXPECT_EQ(triangle[0].y, 69);
}

/*
 * Tests accessing individual vertices of the simple polygon by copy.
 */
TEST_F(SimplePolygonFixture, AccessCopy) {
	const Point2 vertex = triangle[1];
	EXPECT_EQ(vertex.x, 100);
	EXPECT_EQ(vertex.y, 20);
}

/*
 * Tests assigning an iterator range to the polygon that is smaller than the
 * polygon's current size.
 */
TEST_F(SimplePolygonFixture, AssignIteratorSmallerRange) {
	std::vector<Point2> source;
	source.emplace_back(10, 10);
	source.emplace_back(20, 10);
	source.emplace_back(20, 20);

	octagon.assign(source.begin(), source.end());

	ASSERT_EQ(octagon.size(), source.size());
	for(size_t i = 0; i < octagon.size(); ++i) {
		EXPECT_EQ(octagon[i], source[i]);
	}
}

/*
 * Tests assigning an iterator range to the polygon that is larger than the
 * polygon's current size.
 */
TEST_F(SimplePolygonFixture, AssignIteratorLargerRange) {
	std::vector<Point2> source;
	source.emplace_back(10, 10);
	source.emplace_back(20, 10);
	source.emplace_back(20, 20);
	source.emplace_back(10, 20);

	triangle.assign(source.begin(), source.end());
	ASSERT_EQ(triangle.size(), source.size());
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle[i], source[i]);
	}
}

/*
 * Tests assigning an initialiser list to the simple polygon.
 */
TEST_F(SimplePolygonFixture, AssignInitialiserList) {
	octagon.assign({Point2(10, 10), Point2(20, 10), Point2(20, 20)});
	ASSERT_EQ(octagon.size(), 3);
	EXPECT_EQ(octagon[0], Point2(10, 10));
	EXPECT_EQ(octagon[1], Point2(20, 10));
	EXPECT_EQ(octagon[2], Point2(20, 20));
}

/*
 * Tests accessing the vertices in the simple polygon with the ``at`` function.
 *
 * This test keeps the access within the range. There should be no exception.
 */
TEST_F(SimplePolygonFixture, AtInRange) {
	EXPECT_EQ(triangle.at(1), Point2(100, 20));
}

/*
 * Tests accessing the vertices in a const simple polygon with the ``at``
 * function.
 *
 * This test keeps the access within the range. There should be no exception.
 */
TEST_F(SimplePolygonFixture, AtInRangeConst) {
	const SimplePolygon copy(triangle); //Make it const via a copy constructor.
	EXPECT_EQ(triangle.at(1), Point2(100, 20));
}

/*
 * Tests accessing outside of the range of the simple polygon with the ``at``
 * function.
 */
TEST_F(SimplePolygonFixture, AtOutsideRange) {
	EXPECT_THROW(triangle.at(3), std::out_of_range);
	EXPECT_THROW(triangle.at(-1), std::out_of_range); //Probably doesn't matter really since it's unsigned anyway.
}

/*
 * Tests accessing outside of the range of a const simple polygon with the
 * ``at`` function.
 */
TEST_F(SimplePolygonFixture, AtOutsideRangeConst) {
	const SimplePolygon copy(triangle); //Make it const via a copy constructor.
	EXPECT_THROW(copy.at(3), std::out_of_range);
	EXPECT_THROW(copy.at(-1), std::out_of_range); //Probably doesn't matter really since it's unsigned anyway.
}

/*
 * Tests getting the back vertex.
 */
TEST_F(SimplePolygonFixture, Back) {
	EXPECT_EQ(triangle.back(), Point2(60, 60));

	const SimplePolygon copy(triangle);
	EXPECT_EQ(copy.back(), Point2(60, 60));
}

/*
 * Tests accessing and modifying the polygon via the ``data()`` function.
 */
TEST_F(SimplePolygonFixture, Data) {
	Point2* some_vertex = triangle.data();
	EXPECT_EQ(some_vertex->x, 20);
	some_vertex->x = 42;
	EXPECT_EQ(triangle[0].x, 42);
}

/*
 * Tests adding new vertices by emplacing their constructor arguments.
 *
 * Numerous other tests also depend on this, so if this fails it'll also fail
 * other tests. But if this one even fails, you can be sure there is something
 * very wrong with emplacing or the internal data.
 */
TEST_F(SimplePolygonFixture, EmplaceBack) {
	triangle.emplace_back(50, 50);

	ASSERT_EQ(triangle.size(), 4) << "There should now be one more vertex.";
	EXPECT_EQ(triangle[3].x, 50);
	EXPECT_EQ(triangle[3].y, 50);
}

/*
 * Tests whether an empty polygon is marked as empty and a filled polygon is
 * not.
 */
TEST(SimplePolygon, Empty) {
	SimplePolygon polygon;
	EXPECT_TRUE(polygon.empty()) << "The polygon must be empty at construction.";

	polygon.emplace_back(10, 20);
	EXPECT_FALSE(polygon.empty()) << "Once a vertex has been put in, the polygon is no longer empty.";
}

/*
 * Tests getting the front vertex.
 */
TEST_F(SimplePolygonFixture, Front) {
	EXPECT_EQ(triangle.front(), Point2(20, 20));

	const SimplePolygon copy(triangle);
	EXPECT_EQ(copy.front(), Point2(20, 20));
}

/*
 * Tests iterating around the simple polygon while reading the data.
 */
TEST_F(SimplePolygonFixture, IteratorConst) {
	SimplePolygon::const_iterator it = triangle.begin();
	EXPECT_EQ(triangle[0], *it) << "The iteration must begin at the first vertex.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex.";

	it++;
	EXPECT_EQ(triangle[2], *it) << "After iterating twice, it must represent the third vertex.";

	it++;
	EXPECT_EQ(triangle.end(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";
}

/*
 * Tests iterating around the simple polygon with the explicit const iterator.
 */
TEST_F(SimplePolygonFixture, IteratorCBegin) {
	SimplePolygon::const_iterator it = triangle.cbegin();
	EXPECT_EQ(triangle[0], *it) << "The iteration must begin at the first vertex.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex.";

	it++;
	EXPECT_EQ(triangle[2], *it) << "After iterating twice, it must represent the third vertex.";

	it++;
	EXPECT_EQ(triangle.end(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";
}

/*
 * Tests modifying the polygon by modifying the data in the iterator.
 */
TEST_F(SimplePolygonFixture, IteratorModification) {
	SimplePolygon::iterator it = triangle.begin();
	EXPECT_EQ(triangle[0], *it) << "The iteration must begin at the first vertex.";
	it->x = 42;
	EXPECT_EQ(triangle[0].x, 42) << "After the iterator has been changed by reference, the data must be stored in the simple polygon too.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex.";
	it->y = 69;
	EXPECT_EQ(triangle[1].y, 69) << "After the iterator has been changed by reference, the data must be stored in the simple polygon too.";

	it++;
	EXPECT_EQ(triangle[2], *it) << "After iterating twice, it must represent the third vertex.";
	it->x = 666;
	EXPECT_EQ(triangle[2].x, 666) << "After the iterator has been changed by reference, the data must be stored in the simple polygon too.";
}

/*
 * Tests iterating in reverse around the simple polygon while reading the data.
 */
TEST_F(SimplePolygonFixture, IteratorReverseConst) {
	SimplePolygon::const_reverse_iterator it = triangle.rbegin();
	EXPECT_EQ(triangle[2], *it) << "The iteration must begin at the last vertex.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second-to-last vertex.";

	it++;
	EXPECT_EQ(triangle[0], *it) << "After iterating twice, it must represent the third-to-last vertex.";

	it++;
	EXPECT_EQ(triangle.rend(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";
}

/*
 * Tests iterating in reverse around the simple polygon with the explicit const
 * iterator.
 */
TEST_F(SimplePolygonFixture, IteratorReverseCBegin) {
	SimplePolygon::const_reverse_iterator it = triangle.crbegin();
	EXPECT_EQ(triangle[2], *it) << "The iteration must begin at the last vertex.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second-to-last vertex.";

	it++;
	EXPECT_EQ(triangle[0], *it) << "After iterating twice, it must represent the third-to-last vertex.";

	it++;
	EXPECT_EQ(triangle.crend(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";
}

/*
 * Tests modifying the polygon by modifying the data in the reverse iterator.
 */
TEST_F(SimplePolygonFixture, IteratorReverseModification) {
	SimplePolygon::reverse_iterator it = triangle.rbegin();
	EXPECT_EQ(triangle[2], *it) << "The iteration must begin at the last vertex.";
	it->x = 42;
	EXPECT_EQ(triangle[2].x, 42) << "After the iterator has been changed by reference, the data must be stored in the simple polygon too.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second-to-last vertex.";
	it->y = 69;
	EXPECT_EQ(triangle[1].y, 69) << "After the iterator has been changed by reference, the data must be stored in the simple polygon too.";

	it++;
	EXPECT_EQ(triangle[0], *it) << "After iterating twice, it must represent the third-to-last vertex.";
	it->x = 666;
	EXPECT_EQ(triangle[0].x, 666) << "After the iterator has been changed by reference, the data must be stored in the simple polygon too.";
}

/*
 * Tests getting the maximum size of a simple polygon.
 */
TEST(SimplePolygon, MaxSize) {
	//In order to keep the test simple, we check for a minimum given by the OpenCL spec.
	//Otherwise we're just repeating the algorithm.
	//TODO: Mock out the OpenCL calls to give this a fixed maximum. This can be done only after the implementation is made to depend on OpenCL.
	EXPECT_GE(SimplePolygon().max_size(), 32 * 1024 / sizeof(Point2)) << "According to OpenCL specs, the global memory must be at least 32kB. This vector must be held in that global memory.";
}

/*
 * Tests getting the number of vertices.
 *
 * Numerous other tests also depend on this, so if this fails it'll also fail
 * other tests. But if this one even fails, you can be sure there is something
 * very wrong with the size or the internal data.
 */
TEST_F(SimplePolygonFixture, Size) {
	EXPECT_EQ(triangle.size(), 3);
}

}