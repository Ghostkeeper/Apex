/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <algorithm> //To test the specialisation of std::swap.
#include <cmath> //To construct an octagon.
#include <gtest/gtest.h> //To run the test.

#include "apex/coordinate.hpp" //To construct an octagon.
#include "apex/polygon.hpp" //The code under test.

namespace apex {

/*!
 * Fixture that contains a few pre-constructed polygons.
 */
class PolygonFixture : public ::testing::Test {
public:
	/*!
	 * A simple shape with three vertices.
	 */
	Polygon triangle;

	/*!
	 * A regular octagon.
	 */
	Polygon octagon;

	/*!
	 * Constructs the fixture polygons.
	 */
	void SetUp() {
		triangle.emplace_back(20, 20);
		triangle.emplace_back(100, 20);
		triangle.emplace_back(60, 60);

		constexpr coord_t width = 1000;
		const coord_t corner_size = width * (std::sqrt(2) - 1);
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

/*!
 * Tests constructing an empty polygon.
 */
TEST(Polygon, ConstructEmpty) {
	Polygon empty;
	EXPECT_EQ(empty.size(), 0);
}

/*!
 * Tests constructing a polygon with one vertex repeated many times.
 */
TEST(Polygon, ConstructRepeated) {
	Polygon repeated(10, Point2(66, 66));

	ASSERT_EQ(repeated.size(), 10) << "There should now be 10 vertices in the polygon.";
	for(size_t i = 0; i < repeated.size(); ++i) {
		EXPECT_EQ(repeated[i], Point2(66, 66));
	}
}

/*!
 * Tests copy-constructing a polygon.
 */
TEST_F(PolygonFixture, ConstructCopy) {
	Polygon copy(triangle); //Run the copy constructor.

	EXPECT_EQ(triangle, copy);

	copy[0].x += 1; //Modify the copy so that we're sure this is not held by reference or anything.
	EXPECT_NE(triangle, copy);
}

/*!
 * Tests move-constructing a polygon.
 */
TEST_F(PolygonFixture, ConstructMove) {
	Polygon copy = triangle; //Make a copy to move so we keep the original fixture to compare against.
	Polygon target = std::move(copy);

	EXPECT_EQ(triangle, target);
}

/*!
 * Tests making a copy via assignment.
 */
TEST_F(PolygonFixture, Assignment) {
	Polygon copy = triangle; //Make a copy by assignment.

	EXPECT_EQ(triangle, copy);

	copy[0].x += 1; //Modify the copy so that we're sure this is not held by reference or anything.
	EXPECT_NE(triangle, copy);
}

/*!
 * Tests accessing the vertices of the polygon by reference.
 */
TEST_F(PolygonFixture, AccessReference) {
	Point2& vertex = triangle[0];
	vertex.x = 42; //Modify it.
	EXPECT_EQ(triangle[0].x, 42) << "The modified coordinates should be modified by reference, so it should be stored in the polygon as well.";

	triangle[0].y = 69;
	EXPECT_EQ(triangle[0].y, 69);
}

/*!
 * Tests accessing individual vertices of the polygon by copy.
 */
TEST_F(PolygonFixture, AccessCopy) {
	const Point2 vertex = triangle[1];
	EXPECT_EQ(vertex.x, 100);
	EXPECT_EQ(vertex.y, 20);
}

/*!
 * Tests assigning a repeated vertex to the polygon.
 */
TEST_F(PolygonFixture, AssignRepeated) {
	triangle.assign(10, Point2(42, 42));
	ASSERT_EQ(triangle.size(), 10);
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle[i], Point2(42, 42));
	}
}

/*!
 * Tests assigning an iterator range to the polygon that is smaller than the
 * polygon's current size.
 */
TEST_F(PolygonFixture, AssignIteratorSmallerRange) {
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

/*!
 * Tests assigning an iterator range to the polygon that is larger than the
 * polygon's current size.
 */
TEST_F(PolygonFixture, AssignIteratorLargerRange) {
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

/*!
 * Tests assigning an initialiser list to the polygon.
 */
TEST_F(PolygonFixture, AssignInitialiserList) {
	octagon.assign({Point2(10, 10), Point2(20, 10), Point2(20, 20)});
	ASSERT_EQ(octagon.size(), 3);
	EXPECT_EQ(octagon[0], Point2(10, 10));
	EXPECT_EQ(octagon[1], Point2(20, 10));
	EXPECT_EQ(octagon[2], Point2(20, 20));
}

/*!
 * Tests accessing the vertices in the polygon with the ``at`` function.
 *
 * This test keeps the access within the range. There should be no exception.
 */
TEST_F(PolygonFixture, AtInRange) {
	EXPECT_EQ(triangle.at(1), Point2(100, 20));
}

/*!
 * Tests accessing the vertices in a const polygon with the ``at`` function.
 *
 * This test keeps the access within the range. There should be no exception.
 */
TEST_F(PolygonFixture, AtInRangeConst) {
	const Polygon copy(triangle); //Make it const via a copy constructor.
	EXPECT_EQ(copy.at(1), Point2(100, 20));
}

/*!
 * Tests accessing outside of the range of the polygon with the ``at`` function.
 */
TEST_F(PolygonFixture, AtOutsideRange) {
	EXPECT_THROW(triangle.at(3), std::out_of_range);
	EXPECT_THROW(triangle.at(-1), std::out_of_range); //Probably doesn't matter really since it's unsigned anyway.
}

/*!
 * Tests accessing outside of the range of a const polygon with the ``at``
 * function.
 */
TEST_F(PolygonFixture, AtOutsideRangeConst) {
	const Polygon copy(triangle); //Make it const via a copy constructor.
	EXPECT_THROW(copy.at(3), std::out_of_range);
	EXPECT_THROW(copy.at(-1), std::out_of_range); //Probably doesn't matter really since it's unsigned anyway.
}

/*!
 * Tests getting the back vertex.
 */
TEST_F(PolygonFixture, Back) {
	EXPECT_EQ(triangle.back(), Point2(60, 60));

	const Polygon copy(triangle);
	EXPECT_EQ(copy.back(), Point2(60, 60));
}

/*!
 * Tests clearing a polygon.
 */
TEST_F(PolygonFixture, Clear) {
	Polygon empty;
	empty.clear();
	EXPECT_EQ(empty.size(), 0) << "Size should still be 0.";

	triangle.clear();
	EXPECT_EQ(triangle.size(), 0) << "Clearing it should've removed all vertices.";
}

/*!
 * Tests accessing and modifying the polygon via the ``data()`` function.
 */
TEST_F(PolygonFixture, Data) {
	Point2* some_vertex = triangle.data();
	EXPECT_EQ(some_vertex->x, 20);
	some_vertex->x = 42;
	EXPECT_EQ(triangle[0].x, 42);
}

/*!
 * Tests emplacing a vertex at the beginning of the vertex list.
 */
TEST_F(PolygonFixture, EmplaceStart) {
	Polygon copy = triangle; //Modify a copy so that we can compare against the original triangle.
	copy.emplace(copy.begin(), 42, 42);

	ASSERT_EQ(copy.size(), triangle.size() + 1) << "There should now be one more vertex.";
	EXPECT_EQ(copy[0], Point2(42, 42));
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + 1], triangle[i]) << "The original triangle's elements have been shifted.";
	}
}

/*!
 * Tests emplacing a vertex in the middle of the vertex list.
 */
TEST_F(PolygonFixture, EmplaceMiddle) {
	Polygon copy = triangle; //Modify a copy so that we can compare against the original triangle.
	Polygon::const_iterator position = copy.begin();
	position++;
	copy.emplace(position, 42, 42);

	ASSERT_EQ(copy.size(), triangle.size() + 1) << "There should now be one more vertex.";
	EXPECT_EQ(copy[0], triangle[0]) << "The first vertex of the triangle was skipped.";
	EXPECT_EQ(copy[1], Point2(42, 42)) << "The new vertex was placed in the second place.";
	for(size_t i = 1; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + 1], triangle[i]) << "The rest of the triangle's elements have been shifted.";
	}
}

/*!
 * Tests emplacing a vertex at the end of the vertex list.
 */
TEST_F(PolygonFixture, EmplaceEnd) {
	Polygon copy = triangle; //Modify a copy so that we can compare against the original triangle.
	copy.emplace(copy.end(), 42, 42);

	ASSERT_EQ(copy.size(), triangle.size() + 1) << "There should now be one more vertex.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i], triangle[i]) << "The original triangle's elements didn't move.";
	}
	EXPECT_EQ(copy[triangle.size()], Point2(42, 42)) << "The new vertex is at the end.";
}

/*!
 * Tests adding new vertices by emplacing their constructor arguments.
 *
 * Numerous other tests also depend on this, so if this fails it'll also fail
 * other tests. But if this one even fails, you can be sure there is something
 * very wrong with emplacing or the internal data.
 */
TEST_F(PolygonFixture, EmplaceBack) {
	Polygon copy = triangle;
	copy.emplace_back(50, 50);

	ASSERT_EQ(copy.size(), triangle.size() + 1) << "There should now be one more vertex.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i], triangle[i]);
	}
	EXPECT_EQ(copy[triangle.size()], Point2(50, 50));
}

/*!
 * Tests whether an empty polygon is marked as empty and a filled polygon is
 * not.
 */
TEST(Polygon, Empty) {
	Polygon polygon;
	EXPECT_TRUE(polygon.empty()) << "The polygon must be empty at construction.";

	polygon.emplace_back(10, 20);
	EXPECT_FALSE(polygon.empty()) << "Once a vertex has been put in, the polygon is no longer empty.";
}

/*!
 * Tests erasing a single vertex from the beginning of the vertex list.
 */
TEST_F(PolygonFixture, EraseSingleBegin) {
	Polygon copy = octagon; //Modify a copy rather than the original, so we can compare with the original.
	copy.erase(copy.begin());
	ASSERT_EQ(copy.size(), octagon.size() - 1) << "One vertex has been removed.";
	for(size_t i = 0; i < octagon.size() - 1; ++i) {
		EXPECT_EQ(copy[i], octagon[i + 1]) << "All of the vertices have been shifted.";
	}
}

/*!
 * Tests erasing a single vertex from the middle of the vertex list.
 */
TEST_F(PolygonFixture, EraseSingleMiddle) {
	Polygon copy = octagon; //Modify a copy rather than the original, so we can compare with the original.
	Polygon::const_iterator second_vertex = copy.begin();
	second_vertex++;
	copy.erase(second_vertex);
	ASSERT_EQ(copy.size(), octagon.size() - 1) << "One vertex has been removed.";
	EXPECT_EQ(copy[0], octagon[0]) << "The first vertex is still in its place.";
	for(size_t i = 1; i < octagon.size() - 1; ++i) {
		EXPECT_EQ(copy[i], octagon[i + 1]) << "The rest of the vertices have been shifted.";
	}
}

/*!
 * Tests erasing a single vertex from the end of the vertex list.
 */
TEST_F(PolygonFixture, EraseSingleEnd) {
	Polygon copy = octagon; //Modify a copy rather than the original, so we can compare with the original.
	Polygon::const_iterator last_vertex = copy.end();
	last_vertex--; //Does this work? Good test!
	copy.erase(last_vertex);
	ASSERT_EQ(copy.size(), octagon.size() - 1) << "One vertex has been removed.";
	for(size_t i = 0; i < octagon.size() - 2; ++i) {
		EXPECT_EQ(copy[i], octagon[i]) << "None of the vertices have been shifted.";
	}
}

/*!
 * Tests erasing a range of vertices from the beginning of the vertex list.
 */
TEST_F(PolygonFixture, EraseRangeBegin) {
	Polygon copy = octagon; //Modify a copy rather than the original, so we can compare with the original.
	Polygon::const_iterator third_vertex = copy.begin(); //From the beginning to the third vertex is a range of 2 vertices (because the 3rd one is not erased along).
	third_vertex++;
	third_vertex++;
	copy.erase(copy.begin(), third_vertex);
	ASSERT_EQ(copy.size(), octagon.size() - 2) << "Two vertices have been removed.";
	for(size_t i = 0; i < octagon.size() - 3; ++i) {
		EXPECT_EQ(copy[i], octagon[i + 2]) << "All of the vertices shift by 2.";
	}
}

/*!
 * Tests erasing a range of vertices from the middle of the vertex list.
 */
TEST_F(PolygonFixture, EraseRangeMiddle) {
	Polygon copy = octagon; //Modify a copy rather than the original, so we can compare with the original.
	Polygon::const_iterator second_vertex = copy.begin(); //From the second to the fourth vertex is a range of 2 vertices (because the 3rd one is not erased along).
	second_vertex++;
	Polygon::const_iterator fourth_vertex = second_vertex;
	fourth_vertex++;
	fourth_vertex++;
	copy.erase(second_vertex, fourth_vertex);
	ASSERT_EQ(copy.size(), octagon.size() - 2) << "Two vertices have been removed.";
	EXPECT_EQ(copy[0], octagon[0]) << "The first vertex was not erased and hasn't shifted.";
	for(size_t i = 1; i < octagon.size() - 3; ++i) {
		EXPECT_EQ(copy[i], octagon[i + 2]) << "The rest of the vertices shift by 2.";
	}
}

/*!
 * Tests erasing a range of vertices from the end of the vertex list.
 */
TEST_F(PolygonFixture, EraseRangeEnd) {
	Polygon copy = octagon; //Modify a copy rather than the original, so we can compare with the original.
	Polygon::const_iterator second_to_last = copy.end(); //From second-to-last until the end is 2 vertices.
	second_to_last--;
	second_to_last--;
	copy.erase(second_to_last, copy.end());
	ASSERT_EQ(copy.size(), octagon.size() - 2) << "Two vertices have been removed.";
	for(size_t i = 0; i < octagon.size() - 3; ++i) {
		EXPECT_EQ(copy[i], octagon[i]) << "None of the vertices have shifted any.";
	}
}

/*!
 * Tests erasing all vertices as a range.
 */
TEST_F(PolygonFixture, EraseRangeAll) {
	octagon.erase(octagon.begin(), octagon.end());
	EXPECT_EQ(octagon.size(), 0) << "All vertices have been erased.";
}

/*!
 * Tests getting the front vertex.
 */
TEST_F(PolygonFixture, Front) {
	EXPECT_EQ(triangle.front(), Point2(20, 20));

	const Polygon copy(triangle);
	EXPECT_EQ(copy.front(), Point2(20, 20));
}

/*!
 * Tests inserting vertices by copying a vertex in front of all other vertices.
 */
TEST_F(PolygonFixture, InsertCopyFront) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	copy.insert(copy.begin(), Point2(42, 69)); //Insert a new vertex before everything else.
	ASSERT_EQ(copy.size(), triangle.size() + 1);
	EXPECT_EQ(copy[0], Point2(42, 69)) << "The inserted vertex must have been the first vertex now.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + 1], triangle[i]) << "All vertices must have shifted by inserting a vertex in front.";
	}
}

/*!
 * Tests inserting vertices by copying a vertex in the middle between the other
 * vertices.
 */
TEST_F(PolygonFixture, InsertCopyMiddle) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	Polygon::const_iterator second_vertex = copy.begin();
	second_vertex++;
	copy.insert(second_vertex, Point2(42, 69)); //Insert a new vertex in the second location.
	ASSERT_EQ(copy.size(), triangle.size() + 1);
	EXPECT_EQ(copy[0], triangle[0]) << "The first vertex must remain untouched since it was inserted after it.";
	EXPECT_EQ(copy[1], Point2(42, 69)) << "The inserted vertex must be the new the second vertex.";
	for(size_t i = 1; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + 1], triangle[i]) << "All remaining vertices must have been shifted by inserting a vertex in front.";
	}
}

/*!
 * Tests inserting vertices by copying a vertex at the end of the vertex list.
 */
TEST_F(PolygonFixture, InsertCopyBack) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	copy.insert(copy.end(), Point2(42, 69)); //Insert a new vertex at the very end.
	ASSERT_EQ(copy.size(), triangle.size() + 1);
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i], triangle[i]) << "All vertices are in front of the inserted vertex, so they shouldn't be shifted.";
	}
	EXPECT_EQ(copy[triangle.size()], Point2(42, 69)) << "The new vertex must now be at the end.";
}

/*!
 * Tests inserting vertices by moving a vertex in front of all other vertices.
 */
TEST_F(PolygonFixture, InsertMoveFront) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	Point2&& rvalue = Point2(42, 69);
	copy.insert(copy.begin(), rvalue); //Insert a new vertex before everything else.
	ASSERT_EQ(copy.size(), triangle.size() + 1);
	EXPECT_EQ(copy[0], Point2(42, 69)) << "The inserted vertex must have been the first vertex now.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + 1], triangle[i]) << "All vertices must have shifted by inserting a vertex in front.";
	}
}

/*!
 * Tests inserting vertices by moving a vertex in the middle between the other
 * vertices.
 */
TEST_F(PolygonFixture, InsertMoveMiddle) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	Polygon::const_iterator second_vertex = copy.begin();
	second_vertex++;
	Point2&& rvalue = Point2(42, 69);
	copy.insert(second_vertex, rvalue); //Insert a new vertex in the second location.
	ASSERT_EQ(copy.size(), triangle.size() + 1);
	EXPECT_EQ(copy[0], triangle[0]) << "The first vertex must remain untouched since it was inserted after it.";
	EXPECT_EQ(copy[1], Point2(42, 69)) << "The inserted vertex must be the new the second vertex.";
	for(size_t i = 1; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + 1], triangle[i]) << "All remaining vertices must have been shifted by inserting a vertex in front.";
	}
}

/*!
 * Tests inserting vertices by moving a vertex to the end of the vertex list.
 */
TEST_F(PolygonFixture, InsertMoveBack) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	Point2&& rvalue = Point2(42, 69);
	copy.insert(copy.end(), rvalue); //Insert a new vertex at the very end.
	ASSERT_EQ(copy.size(), triangle.size() + 1);
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i], triangle[i]) << "All vertices are in front of the inserted vertex, so they shouldn't be shifted.";
	}
	EXPECT_EQ(copy[triangle.size()], Point2(42, 69)) << "The new vertex must now be at the end.";
}

/*!
 * Tests inserting multiple copies of a vertex at once at the beginning of the
 * vertex list.
 */
TEST_F(PolygonFixture, InsertMultipleFront) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	copy.insert(copy.begin(), 42, Point2(99, 88));
	ASSERT_EQ(copy.size(), triangle.size() + 42) << "There must now be 42 new vertices.";
	for(size_t i = 0; i < 42; ++i) {
		EXPECT_EQ(copy[i], Point2(99, 88)) << "All 42 copies must be copies of the original vertex.";
	}
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + 42], triangle[i]) << "All vertices must have shifted by 42 to make space.";
	}
}

/*!
 * Tests inserting multiple copies of a vertex at once in the middle of the
 * vertex list.
 */
TEST_F(PolygonFixture, InsertMultipleMiddle) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	Polygon::const_iterator second_vertex = copy.begin();
	second_vertex++;
	copy.insert(second_vertex, 42, Point2(88, 77));
	ASSERT_EQ(copy.size(), triangle.size() + 42) << "There must now be 42 new vertices.";
	EXPECT_EQ(copy[0], triangle[0]) << "The first vertex must remain untouched since it was inserted after it.";
	for(size_t i = 0; i < 42; ++i) {
		EXPECT_EQ(copy[i + 1], Point2(88, 77)) << "All 42 copies must be copies of the original vertex.";
	}
	for(size_t i = 1; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + 42], triangle[i]) << "All remaining vertices must have been shifted by inserting vertices in front.";
	}
}

/*!
 * Tests inserting multiple copies of a vertex at once at the end of the vertex
 * list.
 */
TEST_F(PolygonFixture, InsertMultipleEnd) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	copy.insert(copy.end(), 42, Point2(77, 66));
	ASSERT_EQ(copy.size(), triangle.size() + 42) << "There must now be 42 new vertices.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i], triangle[i]) << "All vertices are in front of the inserted vertices, so they shouldn't be shifted.";
	}
	for(size_t i = 0; i < 42; ++i) {
		EXPECT_EQ(copy[triangle.size() + i], Point2(77, 66)) << "All 42 copies must be copies of the original vertex.";
	}
}

/*!
 * Tests inserting a range of vertices indicated by two iterators in the front
 * of the list of vertices.
 */
TEST_F(PolygonFixture, InsertIteratorsFront) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	copy.insert(copy.begin(), octagon.begin(), octagon.end());
	ASSERT_EQ(copy.size(), triangle.size() + octagon.size());
	for(size_t i = 0; i < octagon.size(); ++i) {
		EXPECT_EQ(copy[i], octagon[i]) << "The octagon must appear at the beginning of the combined shape.";
	}
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[octagon.size() + i], triangle[i]) << "The triangle must appear at the end of the combined shape.";
	}
}

/*!
 * Tests inserting a range of vertices indicated by two iterators in the middle
 * between the other vertices.
 */
TEST_F(PolygonFixture, InsertIteratorsMiddle) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	Polygon::const_iterator second_vertex = copy.begin();
	second_vertex++;
	copy.insert(second_vertex, octagon.begin(), octagon.end());
	ASSERT_EQ(copy.size(), triangle.size() + octagon.size());
	EXPECT_EQ(copy[0], triangle[0]) << "The octagon must have been inserted after the first vertex of the triangle.";
	for(size_t i = 0; i < octagon.size(); ++i) {
		EXPECT_EQ(copy[i + 1], octagon[i]) << "The octagon must appear in the middle of the combined shape.";
	}
	for(size_t i = 1; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + octagon.size()], triangle[i]) << "The rest of the triangle must appear at the end of the combined shape.";
	}
}

/*!
 * Tests inserting a range of vertices indicated by two iterators at the end of
 * the vertex list.
 */
TEST_F(PolygonFixture, InsertIteratorsBack) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	copy.insert(copy.end(), octagon.begin(), octagon.end());
	ASSERT_EQ(copy.size(), triangle.size() + octagon.size());
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i], triangle[i]) << "The octagon was inserted after the triangle vertices, so these vertices are still intact.";
	}
	for(size_t i = 0; i < octagon.size(); ++i) {
		EXPECT_EQ(copy[i + triangle.size()], octagon[i]) << "The octagon was inserted at the end, so it must be shifted by the length of the triangle.";
	}
}

/*!
 * Tests inserting an initialiser list of vertices at the start of a polygon.
 */
TEST_F(PolygonFixture, InsertInitialiserListFront) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	copy.insert(copy.begin(), {Point2(99, 88), Point2(42, 69)});
	ASSERT_EQ(copy.size(), triangle.size() + 2);
	EXPECT_EQ(copy[0], Point2(99, 88));
	EXPECT_EQ(copy[1], Point2(42, 69));
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + 2], triangle[i]) << "The triangle's vertices were shifted by inserting two vertices before them.";
	}
}

/*!
 * Tests inserting an initialiser list of vertices in the middle of a polygon.
 */
TEST_F(PolygonFixture, InsertInitialiserListMiddle) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	Polygon::const_iterator second_vertex = copy.begin();
	second_vertex++;
	copy.insert(second_vertex, {Point2(99, 88), Point2(42, 69)});
	ASSERT_EQ(copy.size(), triangle.size() + 2);
	EXPECT_EQ(copy[0], triangle[0]) << "The first vertex is still in front since the new vertices were inserted after it.";
	EXPECT_EQ(copy[1], Point2(99, 88));
	EXPECT_EQ(copy[2], Point2(42, 69));
	for(size_t i = 1; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i + 2], triangle[i]) << "The rest of the vertices were shifted by inserting two vertices before them.";
	}
}

/*!
 * Tests inserting an initialiser list of vertices at the end of a polygon.
 */
TEST_F(PolygonFixture, InsertInitialiserListEnd) {
	Polygon copy = triangle; //Modify a copy rather than the original, so we can compare with the original.
	copy.insert(copy.end(), {Point2(99, 88), Point2(42, 69)});
	ASSERT_EQ(copy.size(), triangle.size() + 2) << "Two new vertices were added.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(copy[i], triangle[i]) << "All of the vertices are still in their original place, since it only added anything at the end.";
	}
	EXPECT_EQ(copy[triangle.size()], Point2(99, 88)) << "The first new vertex was added at the end.";
	EXPECT_EQ(copy[triangle.size() + 1], Point2(42, 69)) << "The second new vertex was added after it.";
}

/*!
 * Tests iterating around the polygon while reading the data.
 */
TEST_F(PolygonFixture, IteratorConst) {
	Polygon::const_iterator it = triangle.begin();
	EXPECT_EQ(triangle[0], *it) << "The iteration must begin at the first vertex.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex.";

	it++;
	EXPECT_EQ(triangle[2], *it) << "After iterating twice, it must represent the third vertex.";

	it++;
	EXPECT_EQ(triangle.end(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";
}

/*!
 * Tests iterating around the polygon with the explicit const iterator.
 */
TEST_F(PolygonFixture, IteratorCBegin) {
	Polygon::const_iterator it = triangle.cbegin();
	EXPECT_EQ(triangle[0], *it) << "The iteration must begin at the first vertex.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex.";

	it++;
	EXPECT_EQ(triangle[2], *it) << "After iterating twice, it must represent the third vertex.";

	it++;
	EXPECT_EQ(triangle.cend(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";
}

/*!
 * Tests modifying the polygon by modifying the data in the iterator.
 */
TEST_F(PolygonFixture, IteratorModification) {
	Polygon::iterator it = triangle.begin();
	EXPECT_EQ(triangle[0], *it) << "The iteration must begin at the first vertex.";
	it->x = 42;
	EXPECT_EQ(triangle[0].x, 42) << "After the iterator has been changed by reference, the data must be stored in the polygon too.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second vertex.";
	it->y = 69;
	EXPECT_EQ(triangle[1].y, 69) << "After the iterator has been changed by reference, the data must be stored in the polygon too.";

	it++;
	EXPECT_EQ(triangle[2], *it) << "After iterating twice, it must represent the third vertex.";
	it->x = 666;
	EXPECT_EQ(triangle[2].x, 666) << "After the iterator has been changed by reference, the data must be stored in the polygon too.";
}

/*!
 * Tests iterating in reverse around the polygon while reading the data.
 */
TEST_F(PolygonFixture, IteratorReverseConst) {
	Polygon::const_reverse_iterator it = triangle.rbegin();
	EXPECT_EQ(triangle[2], *it) << "The iteration must begin at the last vertex.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second-to-last vertex.";

	it++;
	EXPECT_EQ(triangle[0], *it) << "After iterating twice, it must represent the third-to-last vertex.";

	it++;
	EXPECT_EQ(triangle.rend(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";
}

/*!
 * Tests iterating in reverse around the polygon with the explicit const
 * iterator.
 */
TEST_F(PolygonFixture, IteratorReverseCBegin) {
	Polygon::const_reverse_iterator it = triangle.crbegin();
	EXPECT_EQ(triangle[2], *it) << "The iteration must begin at the last vertex.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second-to-last vertex.";

	it++;
	EXPECT_EQ(triangle[0], *it) << "After iterating twice, it must represent the third-to-last vertex.";

	it++;
	EXPECT_EQ(triangle.crend(), it) << "The triangle has only three vertices, so after iterating thrice it must arrive at the end.";
}

/*!
 * Tests modifying the polygon by modifying the data in the reverse iterator.
 */
TEST_F(PolygonFixture, IteratorReverseModification) {
	Polygon::reverse_iterator it = triangle.rbegin();
	EXPECT_EQ(triangle[2], *it) << "The iteration must begin at the last vertex.";
	it->x = 42;
	EXPECT_EQ(triangle[2].x, 42) << "After the iterator has been changed by reference, the data must be stored in the polygon too.";

	it++;
	EXPECT_EQ(triangle[1], *it) << "After iterating once, it must represent the second-to-last vertex.";
	it->y = 69;
	EXPECT_EQ(triangle[1].y, 69) << "After the iterator has been changed by reference, the data must be stored in the polygon too.";

	it++;
	EXPECT_EQ(triangle[0], *it) << "After iterating twice, it must represent the third-to-last vertex.";
	it->x = 666;
	EXPECT_EQ(triangle[0].x, 666) << "After the iterator has been changed by reference, the data must be stored in the polygon too.";
}

/*!
 * Tests getting the maximum size of a polygon.
 */
TEST(Polygon, MaxSize) {
	//In order to keep the test simple, we check for a minimum given by the OpenCL spec.
	//Otherwise we're just repeating the algorithm.
	//TODO: Mock out the OpenCL calls to give this a fixed maximum. This can be done only after the implementation is made to depend on OpenCL.
	EXPECT_GE(Polygon().max_size(), 32 * 1024 / sizeof(Point2)) << "According to OpenCL specs, the global memory must be at least 32kB. This vector must be held in that global memory.";
}

/*!
 * Tests removing the last vertex of the vertex list.
 */
TEST_F(PolygonFixture, PopBack) {
	Polygon copy = triangle; //Modify a copy so that we can compare against the original.
	copy.pop_back();
	ASSERT_EQ(copy.size(), triangle.size() - 1);
	for(size_t i = 0; i < copy.size(); ++i) {
		EXPECT_EQ(copy[i], triangle[i]) << "All other vertices are still intact.";
	}
}

/*!
 * Tests push_back by letting it copy the vertex.
 */
TEST_F(PolygonFixture, PushBackCopy) {
	triangle.push_back(Point2(42, 42));
	ASSERT_EQ(triangle.size(), 4) << "One vertex was added.";
	EXPECT_EQ(triangle[3], Point2(42, 42)) << "The new vertex was appended at the end.";
}

/*!
 * Tests push_back by moving the vertex into the memory of the polygon.
 */
TEST_F(PolygonFixture, PushBackMove) {
	Point2&& rvalue = Point2(42, 42);
	triangle.push_back(rvalue);
	ASSERT_EQ(triangle.size(), 4) << "One vertex was added.";
	EXPECT_EQ(triangle[3], Point2(42, 42)) << "The new vertex was appended at the end.";
}

/*!
 * Tests reserving memory for the polygon and the resulting capacity.
 */
TEST_F(PolygonFixture, ReserveCapacity) {
	EXPECT_GE(triangle.capacity(), triangle.size()) << "The capacity must at least be the number of elements currently contained.";

	triangle.reserve(256);
	EXPECT_GE(triangle.capacity(), 256) << "We reserved 256 elements, so there must now be at least room for 256.";

	triangle.reserve(128);
	EXPECT_GE(triangle.capacity(), 256) << "Reducing the reservation shouldn't have any effect as long as we don't shrink to size.";

	triangle.reserve(257);
	EXPECT_GE(triangle.capacity(), 257); //Try with a number that's not a nice multiple of 2 as well.
}

/*!
 * Tests resizing the polygon to something smaller.
 */
TEST_F(PolygonFixture, ResizeSmaller) {
	Polygon triangle_copy = triangle; //Make a copy so that we can compare the edited polygon with the original.
	triangle_copy.resize(2);
	ASSERT_EQ(triangle_copy.size(), 2) << "We resized it to contain only 2 vertices.";
	for(size_t i = 0; i < triangle_copy.size(); ++i) {
		EXPECT_EQ(triangle_copy[i], triangle[i]) << "The remaining vertices must still be in their places.";
	}
}

/*!
 * Tests resizing the polygon to something larger by default initialisation.
 */
TEST_F(PolygonFixture, ResizeLargerDefault) {
	Polygon triangle_copy = triangle; //Make a copy so that we can compare the edited polygon with the original.
	triangle_copy.resize(5);
	ASSERT_EQ(triangle_copy.size(), 5) << "We resized it to contain 5 vertices.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_copy[i], triangle[i]) << "The original vertices are still there.";
	}
	for(size_t i = triangle.size(); i < triangle_copy.size(); ++i) {
		EXPECT_EQ(triangle_copy[i], Point2(0, 0)) << "The new vertices must be zero-initialised.";
	}
}

/*!
 * Tests resizing the polygon to something larger with a customised default
 * vertex.
 */
TEST_F(PolygonFixture, ResizeLargerCustom) {
	Polygon triangle_copy = triangle; //Make a copy so that we can compare the edited polygon with the original.
	triangle_copy.resize(7, Point2(13, 37));
	ASSERT_EQ(triangle_copy.size(), 7) << "We resized it to contain 7 vertices.";
	for(size_t i = 0; i < triangle.size(); ++i) {
		EXPECT_EQ(triangle_copy[i], triangle[i]) << "The original vertices are still there.";
	}
	for(size_t i = triangle.size(); i < triangle_copy.size(); ++i) {
		EXPECT_EQ(triangle_copy[i], Point2(13, 37)) << "The new vertices must be equal to the default we provided.";
	}
}

/*!
 * Tests whether shrink_to_fit doesn't destroy anything.
 *
 * The shrink_to_fit method doesn't give any guarantees that the capacity is
 * actually reduced. That is up to the compiler. So we can't test that the
 * capacity is reduced. But we can at least verify that the capacity doesn't cut
 * off any data.
 */
TEST_F(PolygonFixture, ShrinkToFit) {
	triangle.reserve(256);
	triangle.shrink_to_fit();
	EXPECT_GE(triangle.capacity(), triangle.size());
}

/*!
 * Tests getting the number of vertices.
 *
 * Numerous other tests also depend on this, so if this fails it'll also fail
 * other tests. But if this one even fails, you can be sure there is something
 * very wrong with the size or the internal data.
 */
TEST_F(PolygonFixture, Size) {
	EXPECT_EQ(triangle.size(), 3);
}

/*!
 * Tests swapping the contents of two polygons.
 */
TEST_F(PolygonFixture, Swap) {
	Polygon copy_triangle = triangle; //Make copies so that we can compare to the original triangle and octagon.
	Polygon copy_octagon = octagon;
	copy_triangle.swap(copy_octagon);
	EXPECT_EQ(copy_triangle, octagon) << "Since the triangle was swapped with the octagon, it must now contain the octagon.";
	EXPECT_EQ(copy_octagon, triangle) << "Since the octagon was swapped with the triangle, it must now contain the triangle.";
}

/*!
 * Tests swapping the contents of two polygons via the std::swap function.
 */
TEST_F(PolygonFixture, SwapStd) {
	Polygon copy_triangle = triangle; //Make copies so that we can compare to the original triangle and octagon.
	Polygon copy_octagon = octagon;
	std::swap(copy_triangle, copy_octagon);
	EXPECT_EQ(copy_triangle, octagon) << "Since the triangle was swapped with the octagon, it must now contain the octagon.";
	EXPECT_EQ(copy_octagon, triangle) << "Since the octagon was swapped with the triangle, it must now contain the triangle.";
}

}