#Library for performing massively parallel computations on polygons.
#Copyright (C) 2022 Ghostkeeper
#This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
#You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.10) #Oldest version it was tested with.
project(ApexBenchmark)

set(source_files
	"main.cpp"
	"generators.cpp"
)

#Always add GPU definition. We want to include the GPU in the benchmarks.
#In theory we might want to also allow benchmarking without GPU support. But that also makes the code more complex. Not for now.
add_definitions(-DGPU)

add_executable(apex_benchmark ${source_files})
target_link_libraries(apex_benchmark PRIVATE apex)
target_include_directories(apex_benchmark INTERFACE "${CMAKE_SOURCE_DIR}/include")