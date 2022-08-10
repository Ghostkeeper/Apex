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
#include <functional> //To accept functions to benchmark with.
#include <iomanip> //For std::setw.
#include <iostream> //To output progress during benchmarking.

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
	 * \tparam TestData A testing object provided for the benchmarked function,
	 * which is also the output of the generator.
	 * \param name A name to display in the terminal while this benchmark is
	 * running.
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
	static std::vector<double> run_const(const std::string name, const std::function<TestData(const size_t)> generator, const std::vector<size_t>& sizes, std::function<void(const TestData&)> benchmark) {
		std::cout << name << " | Preparing..." << std::flush;
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
		std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b"; //Erase 'Preparing...'
		std::cout << "[          ]";
		std::cout << "\b\b\b\b\b\b\b\b\b\b\b"; //Return cursor to the start of the progress bar.
		std::cout << std::flush;

		//Now for real. And measure the time it takes.
		std::vector<double> result_times;
		size_t progress_printed = 0;
		for(size_t test_case = 0; test_case < test_datas.size(); ++test_case) {
			const TestData& test_data = test_datas[test_case];
			//The counting of the for loop is some overhead within the measured period.
			//However stopping and re-starting the time measurement has bigger overhead, so we'll have to do the loop within the measured period.
			std::chrono::time_point start = std::chrono::steady_clock::now();
			for(size_t repeat = 0; repeat < repeats; ++repeat) {
				benchmark(test_data);
			}
			std::chrono::time_point end = std::chrono::steady_clock::now();
			std::chrono::duration nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>((end - start) / repeats);
			result_times.push_back(nanoseconds.count());

			//Update progress bar.
			const float progress = static_cast<float>(test_case) / test_datas.size();
			while(progress * 10 > progress_printed) {
				std::cout << "▓" << std::flush;
				progress_printed += 1;
			}
		}
		//Erase progress bar.
		for(size_t i = 0; i < 14 + name.length(); ++i) {
			std::cout << "\b \b" << std::flush;
		}

		return result_times;
	}

	/*!
	 * Print the benchmark results in COUT, to read them in the terminal.
	 * \tparam The number of different benchmarks to compare in the output. For
	 * instance, you could compare 3 different tests when comparing a single-
	 * threaded, a multi-threaded and a GPU implementation of an algorithm.
	 * \param names The names to give to each test. Names must be 9 or fewer
	 * characters long, or it will not align well in the output.
	 * \param sizes The test sizes that were provided to each test.
	 * \param durations For each test, a list of the execution times of each
	 * input size in the list of sizes.
	 */
	template<size_t NumTests>
	static void output_cout(const std::array<std::string, NumTests>& names, const std::vector<size_t>& sizes, const std::array<std::vector<double>, NumTests>& durations) {
		//Print the header bar.
		std::cout << std::setw(10) << "SIZE";
		for(const std::string name : names) {
			std::cout << std::setw(10) << name;
		}
		std::cout << std::endl;

		//Print the data for each size.
		for(size_t size_index = 0; size_index < sizes.size(); ++size_index) {
			std::cout << std::setw(10) << sizes[size_index];
			for(size_t test = 0; test < durations.size(); ++test) {
				std::cout << std::setw(10) << durations[test][size_index];
			}
			std::cout << std::endl;
		}
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