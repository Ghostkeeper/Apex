/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <apex/polygon.hpp> //To test performance of using polygons.
#include <iostream> //To print out some progress/metadata information.

#include "benchmarker.hpp" //To execute tests.
#include "generators.hpp" //To generate test objects.
#include "sizes.hpp" //To determine how big the test objects are.

int main(int argc, char** argv) {
	std::cout << "Apex benchmarking application.\n" << std::endl;

	std::cout << "________ AREA ________" << std::endl;
	std::vector<double> durations_st = benchmarker::Benchmarker::run_const<apex::Polygon>("Area ST", benchmarker::generate_polygon_circle, benchmarker::sizes_polygon_big, [](const apex::Polygon& polygon) {
		apex::detail::area_st(polygon);
	});
	std::vector<double> durations_mt = benchmarker::Benchmarker::run_const<apex::Polygon>("Area MT", benchmarker::generate_polygon_circle, benchmarker::sizes_polygon_big, [](const apex::Polygon& polygon) {
		apex::detail::area_mt(polygon);
	});
	std::vector<double> durations_gpu = benchmarker::Benchmarker::run_const<apex::Polygon>("Area GPU", benchmarker::generate_polygon_circle, benchmarker::sizes_polygon_big, [](const apex::Polygon& polygon) {
		apex::detail::area_gpu(polygon);
	});

	//Output the results to terminal for now.
	benchmarker::Benchmarker::output_cout<3>({"ST", "MT", "GPU"}, benchmarker::sizes_polygon_big, {durations_st, durations_mt, durations_gpu});

	//Repeat for the area of batches of polygons.
	std::cout << "_______ [AREA] _______" << std::endl;
	durations_st = benchmarker::Benchmarker::run_const<apex::Batch<apex::Polygon>>("[Area] ST", benchmarker::generate_polygon_batch_10gon, benchmarker::sizes_polygon_batch_big, [](const apex::Batch<apex::Polygon>& batch) {
		apex::detail::area_st(batch);
	});
	durations_mt = benchmarker::Benchmarker::run_const<apex::Batch<apex::Polygon>>("[Area] MT", benchmarker::generate_polygon_batch_10gon, benchmarker::sizes_polygon_batch_big, [](const apex::Batch<apex::Polygon>& batch) {
		apex::detail::area_mt(batch);
	});
	durations_gpu = benchmarker::Benchmarker::run_const<apex::Batch<apex::Polygon>>("[Area] GPU", benchmarker::generate_polygon_batch_10gon, benchmarker::sizes_polygon_batch_big, [](const apex::Batch<apex::Polygon>& batch) {
		apex::detail::area_gpu(batch);
	});

	//Output the results to terminal for now.
	benchmarker::Benchmarker::output_cout<3>({"ST", "MT", "GPU"}, benchmarker::sizes_polygon_batch_big, {durations_st, durations_mt, durations_gpu});

	return 0;
}