/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef BENCHMARKER
#define BENCHMARKER

#include <chrono> //To measure execution time.
#include <functional>

namespace benchmarker {

class Benchmarker {
public:
	/*!
	 * Run a benchmark with various size inputs.
	 *
	 * The benchmarked function takes a reference to test data as input. This
	 * function will be provided with various size inputs, generated through the
	 * generator provided.
	 *
	 * Each size in the given size list will first be run once as a warm-up
	 * round. This causes memory to be pre-allocated for the application so that
	 * the performance of the operating system memory allocation influences the
	 * measurements less. After that, they will be executed for real and the
	 * duration will be measured. The real test is repeated a number of times,
	 * defined by \ref repeats.
	 *
	 * The benchmarked function may not alter the input data in this version.
	 * To benchmark functions that do alter the input data, the data would have
	 * to be re-generated at every test.
	 *
	 * The outcome for each size is returned as a vector whose indices match the
	 * indices of the various sizes given in the input size array.
	 * \param generator A generator that generates test data objects with a
	 * certain size. The generator is a function that gets a size and needs to
	 * return a test object with that size.
	 * \param sizes A list of sizes to test with.
	 * \param benchmark A function to test the performance of. The function
	 * needs to accept a reference to a test object (as returned by the
	 * generator). It then needs to perform a task on that test object, and
	 * \emph only that task. Do not include any set-up or tear-down into this
	 * function. The benchmarked function will be measured as a whole for its
	 * performance.
	 * \return A vector of average execution times, equal to the length of the
	 * sizes given. This vector contains the average execution time of the
	 * benchmarked function given each size input. Execution times is in
	 * nanoseconds.
	 */
	template<typename TestData>
	static std::vector<double> run_const(const std::function<TestData(const size_t)> generator, const std::vector<size_t>& sizes, const std::function<void(const TestData&)> benchmark) {
		//First pre-generate the test data for each size.
		std::vector<TestData> test_datas;
		test_datas.reserve(sizes.size());
		for(const size_t size : sizes) {
			test_datas.push_back(generator(size));
		}

		//Perform a dry run with each test data object. This makes sure that memory is properly pre-allocated, reducing overhead and OS-dependence.
		for(const TestData& test_data : test_datas) {
			benchmark(test_data);
		}

		//Now for real. And measure the time it takes.
		std::vector<double> result_times;
		for(const TestData& test_data : test_datas) {
			//The counting of the for loop is some overhead within the measured period.
			//However stopping and re-starting the time measurement has bigger overhead, so we'll have to do the loop within the measured period.
			std::chrono::time_point start = std::chrono::steady_clock::now();
			for(size_t repeat = 0; repeat < repeats; ++repeat) {
				benchmark(test_data);
			}
			std::chrono::time_point end = std::chrono::steady_clock::now();
			std::chrono::duration nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>((end - start) / repeats);
			result_times.push_back(nanoseconds.count());
		}

		return result_times;
	}

	/*!
	 * Benchmark functions that calculate the area of a shape.
	 */
	static void bench_area();

protected:
	constexpr static size_t repeats = 10000; //How often to repeat each test. Increase for more accurate results.
};

}

#endif //BENCHMARKER