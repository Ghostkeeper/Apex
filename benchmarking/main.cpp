/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include "benchmarker.hpp" //To execute tests.
#include <apex/simple_polygon.hpp> //The library under test.
#include <iostream> //To print out some progress/metadata information.

int main(int argc, char** argv) {
	std::cout << "Apex benchmarking application.\n" << std::endl;

	benchmarker::Benchmarker::bench_area();
}