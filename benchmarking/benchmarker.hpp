/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef BENCHMARKER
#define BENCHMARKER

#include <functional>
#include <string>

namespace benchmarker {

class Benchmarker {
public:
	/*!
	 * Benchmark a function that performs differently based on the size of the
	 * input, with just 1 size parameter.
	 * \param name A human-readable name for the function to test.
	 * \param target A function to test the performance of. This function needs
	 * to accept a size and run a test of that size.
	 * \param sizes The size of inputs to the function to test with.
	 */
	static void bench_sized(const std::string name, const std::function<void(size_t)> target, const std::vector<size_t>& sizes);
};

}

#endif //BENCHMARKER