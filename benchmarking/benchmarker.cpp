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

#include <iomanip> //For std::setw.
#include <iostream> //To output the benchmark results to stdcout.
#include <numeric> //Easy summing for the CRC.
#include <vector>

#include "benchmarker.hpp"
#include "generators.hpp" //To generate test data.
#include "sizes.hpp" //For the sizes to test with.

namespace benchmarker {

void Benchmarker::bench_area() {
	std::cout << "________ AREA ________" << std::endl;
	std::vector<double> durations_st = run_const<apex::Polygon>(generate_polygon_circle, sizes_polygon_big, [](const apex::Polygon& polygon) {
		apex::detail::area_st(polygon);
	});
	std::vector<double> durations_mt = run_const<apex::Polygon>(generate_polygon_circle, sizes_polygon_big, [](const apex::Polygon& polygon) {
		apex::detail::area_mt(polygon);
	});
	std::vector<double> durations_gpu = run_const<apex::Polygon>(generate_polygon_circle, sizes_polygon_big, [](const apex::Polygon& polygon) {
		apex::detail::area_gpu(polygon);
	});

	//Output the results to terminal for now.
	std::cout << std::setw(10) << "SIZE" << std::setw(10) << "ST" << std::setw(10) << "MT" << std::setw(10) << "GPU" << std::endl;
	for(size_t i = 0; i < sizes_polygon_big.size(); ++i) {
		std::cout << std::setw(10) << sizes_polygon_big[i] << std::setw(10) << durations_st[i] << std::setw(10) << durations_mt[i] << std::setw(10) << durations_gpu[i] << std::endl;
	}

	//Repeat for the area of batches of polygons.
	std::cout << "_______ [AREA] _______" << std::endl;
	durations_st = run_const<apex::Batch<apex::Polygon>>(generate_polygon_batch_10gon, sizes_polygon_batch_big, [](const apex::Batch<apex::Polygon>& batch) {
		apex::detail::area_st(batch);
	});
	durations_mt = run_const<apex::Batch<apex::Polygon>>(generate_polygon_batch_10gon, sizes_polygon_batch_big, [](const apex::Batch<apex::Polygon>& batch) {
		apex::detail::area_mt(batch);
	});
	durations_gpu = run_const<apex::Batch<apex::Polygon>>(generate_polygon_batch_10gon, sizes_polygon_batch_big, [](const apex::Batch<apex::Polygon>& batch) {
		apex::detail::area_gpu(batch);
	});

	//Output the results to terminal for now.
	std::cout << std::setw(10) << "SIZE" << std::setw(10) << "ST" << std::setw(10) << "MT" << std::setw(10) << "GPU" << std::endl;
	for(size_t i = 0; i < sizes_polygon_big.size(); ++i) {
		std::cout << std::setw(10) << sizes_polygon_big[i] << std::setw(10) << durations_st[i] << std::setw(10) << durations_mt[i] << std::setw(10) << durations_gpu[i] << std::endl;
	}
}

}