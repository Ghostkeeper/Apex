/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <apex/operations/area.hpp>
#include <apex/polygon.hpp>
#include <apex/point2.hpp>

#include <chrono> //To measure execution time.
#include <iomanip> //For std::setw.
#include <iostream> //To output the benchmark results to stdcout.
#include <numeric> //Easy summing for the CRC.
#include <vector>

#include "benchmarker.hpp"

namespace benchmarker {

void Benchmarker::bench_area() {
	std::cout << "________ AREA ________" << std::endl;
	//std::cout << std::setw(10) << "SIZE" << std::setw(10) << "ST" << std::setw(10) << "MT" << std::setw(10) << "GPU" << std::setw(10) << "CRC" << std::endl;
	std::cout << "SIZE;ST;MT;GPU;CRC" << std::endl;

	//Sizes to test with.
	std::vector<size_t> sizes;
	for(size_t i = 0; i < 10000; i += 100) {
		sizes.push_back(i);
	}
	//Pre-generate the polygons.
	std::vector<apex::Polygon> polys;
	for(size_t size : sizes) {
		polys.emplace_back(size, apex::Point2(0, 0));
		for(size_t i = 0; i < size; ++i) {
			polys.back()[i] = apex::Point2(i, i);
		}
	}

	//First do a dry run with the largest size, to get more accurate results without system calls to allocate more memory.
	{
		apex::detail::area_st(polys.back());
		apex::detail::area_mt(polys.back());
		apex::detail::area_gpu(polys.back());
	}

	for(size_t i = 0; i < sizes.size(); ++i) {
		apex::Polygon& poly = polys[i];
		size_t size = sizes[i];

		apex::area_t sum = 0; //Calculate this sum so that compilers can't optimise the repeats away.
		std::chrono::time_point start = std::chrono::steady_clock::now();
		for(size_t repeat = 0; repeat < repeats; ++repeat) {
			sum += apex::detail::area_st(poly);
		}
		std::chrono::time_point end = std::chrono::steady_clock::now();
		std::chrono::duration st_time = std::chrono::duration_cast<std::chrono::nanoseconds>((end - start) / repeats);

		start = std::chrono::steady_clock::now();
		for(size_t repeat = 0; repeat < repeats; ++repeat) {
			sum += apex::detail::area_mt(poly);
		}
		end = std::chrono::steady_clock::now();
		std::chrono::duration mt_time = std::chrono::duration_cast<std::chrono::nanoseconds>((end - start) / repeats);

		start = std::chrono::steady_clock::now();
		for(size_t repeat = 0; repeat < repeats; ++repeat) {
			sum += apex::detail::area_gpu(poly);
		}
		end = std::chrono::steady_clock::now();
		std::chrono::duration gpu_time = std::chrono::duration_cast<std::chrono::nanoseconds>((end - start) / repeats);

		//std::cout << std::setw(10) << size << std::setw(10) << st_time.count() << std::setw(10) << mt_time.count() << std::setw(10) << gpu_time.count() << std::setw(10) << sum << std::endl;
		std::cout << size << ";" << st_time.count() << ";" << mt_time.count() << ";" << gpu_time.count() << ";" << sum << std::endl;
	}

	//Repeat for the area of batches of polygons.
	std::cout << "_______ [AREA] _______" << std::endl;
	//std::cout << std::setw(10) << "SIZE" << std::setw(10) << "ST" << std::setw(10) << "MT" << std::setw(10) << "GPU" << std::setw(10) << "CRC" << std::endl;
	std::cout << "SIZE;ST;MT;GPU;CRC" << std::endl;

	//Sizes to test with.
	sizes.clear();
	for(size_t i = 0; i < 1000; i += 10) {
		sizes.push_back(i);
	}

	//Pre-generate the batches.
	std::vector<apex::Batch<apex::Polygon>> batches;
	apex::Polygon poly(10, apex::Point2(0, 0)); //Each batch repeats this one polygon multiple times.
	for(size_t i = 0; i < poly.size(); ++i) {
		poly[i] = apex::Point2(i, i);
	}
	for(size_t size : sizes) {
		apex::Batch<apex::Polygon> batch(size, poly);
		batches.push_back(batch);
	}

	//First do a dry run with the largest size, to get more accurate results without system calls to allocate more memory.
	{
		apex::detail::area_st(batches.back());
		apex::detail::area_mt(batches.back());
		apex::detail::area_gpu(batches.back());
	}

	for(size_t i = 0; i < sizes.size(); ++i) {
		apex::Batch<apex::Polygon>& batch = batches[i];
		size_t size = sizes[i];

		apex::area_t sum = 0; //Calculate this sum so that compilers can't optimise the repeats away.
		apex::Batch<apex::area_t> area_results;
		area_results.reserve(size);
		std::chrono::time_point start = std::chrono::steady_clock::now();
		for(size_t repeat = 0; repeat < repeats; ++repeat) {
			area_results = apex::detail::area_st(batch);
		}
		std::chrono::time_point end = std::chrono::steady_clock::now();
		std::chrono::duration st_time = std::chrono::duration_cast<std::chrono::nanoseconds>((end - start) / repeats);
		sum = std::accumulate(area_results.begin(), area_results.end(), sum);

		start = std::chrono::steady_clock::now();
		for(size_t repeat = 0; repeat < repeats; ++repeat) {
			area_results = apex::detail::area_mt(batch);
		}
		end = std::chrono::steady_clock::now();
		std::chrono::duration mt_time = std::chrono::duration_cast<std::chrono::nanoseconds>((end - start) / repeats);
		sum = std::accumulate(area_results.begin(), area_results.end(), sum);

		start = std::chrono::steady_clock::now();
		for(size_t repeat = 0; repeat < repeats; ++repeat) {
			area_results = apex::detail::area_gpu(batch);
		}
		end = std::chrono::steady_clock::now();
		std::chrono::duration gpu_time = std::chrono::duration_cast<std::chrono::nanoseconds>((end - start) / repeats);
		sum = std::accumulate(area_results.begin(), area_results.end(), sum);

		//std::cout << std::setw(10) << size << std::setw(10) << st_time.count() << std::setw(10) << mt_time.count() << std::setw(10) << gpu_time.count() << std::setw(10) << sum << std::endl;
		std::cout << size << ";" << st_time.count() << ";" << mt_time.count() << ";" << gpu_time.count() << ";" << sum << std::endl;
	}
}

}