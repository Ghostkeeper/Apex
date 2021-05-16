#Library for performing massively parallel computations on polygons.
#Copyright (C) 2021 Ghostkeeper
#This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
#You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.10) #Oldest version it was tested with.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules") #Finding dependencies.

#Release configuration.
set(APEX_VERSION_MAJOR 0 CACHE STRING "Major release version. This must be incremented if there are changes that are not backwards compatible.")
set(APEX_VERSION_MINOR 0 CACHE STRING "Minor release version. This must be incremented if there are changes that are not forwards compatible.")
set(APEX_VERSION_PATCH 1 CACHE STRING "Patch release version. This must be incremented for every release where there are no changes to the API.")

project(Apex VERSION ${APEX_VERSION_MAJOR}.${APEX_VERSION_MINOR}.${APEX_VERSION_PATCH} DESCRIPTION "Library for performing massively parallel computations on polygons.")

#The main target.
add_library(apex INTERFACE)
target_include_directories(apex INTERFACE "${CMAKE_SOURCE_DIR}/include")

#Make sure the compiler is up to the task.
set(CMAKE_CXX_STANDARD 17)
target_compile_features(apex INTERFACE cxx_std_17)
target_compile_features(apex INTERFACE cxx_constexpr)
target_compile_features(apex INTERFACE cxx_generalized_initializers)
target_compile_features(apex INTERFACE cxx_noexcept)
target_compile_features(apex INTERFACE cxx_relaxed_constexpr)
target_compile_features(apex INTERFACE cxx_rvalue_references)
target_compile_features(apex INTERFACE cxx_right_angle_brackets)
if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
	message(FATAL_ERROR "GCC version must be at least 8.0 due to fixes in const overload selection!")
endif()

#Dependencies.
option(WITH_OPENMP "Use OpenMP to parallelise computation." ON)
if(WITH_OPENMP)
	find_package(OpenMP REQUIRED)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

#Tests.
option(BUILD_TESTS "Build tests to verify correctness of the library." OFF)
if(BUILD_TESTS)
	enable_testing()

	find_package(GoogleTest 1.10 REQUIRED)
	find_package(Threads REQUIRED) #Threading library is required for GoogleTest if it's compiled with default parameters.

	#The main library then needs to be told to link to GoogleTest to define friend tests.
	add_definitions(-DBUILD_TESTS)
	target_include_directories(apex INTERFACE ${GOOGLETEST_INCLUDE_DIRS})
	target_link_libraries(apex INTERFACE ${GOOGLETEST_BOTH_LIBRARIES})
	add_dependencies(apex GoogleTest)

	#Only perform GPU tests if we have a GPU and basic operations are supported.
	#E.g. on some computers parallel distribution works, but reductions don't. So include a reduction in the test.
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gpu_probe/gpu_probe.cpp" "#include <omp.h>\nint main() {\nint x = 0;\n#pragma omp teams distribute parallel for reduction(+:x)\nfor(int i = 0; i < 1; ++i) {x += 1;}\nreturn x;}")
	try_run(gpu_supported gpu_compile_result "${CMAKE_CURRENT_BINARY_DIR}/gpu_probe" "${CMAKE_CURRENT_BINARY_DIR}/gpu_probe/gpu_probe.cpp" CMAKE_FLAGS "-DCMAKE_CXX_FLAGS:string=${OpenMP_CXX_FLAGS}")
	if(gpu_supported EQUAL 1)
		message(STATUS "Found GPU support on this computer. Building GPU tests.")
		add_definitions(-DGPU_TESTS)
	endif()

	#Library of helper functions and classes to help running the tests (e.g. mocks, gropers).
	set(test_helpers_sources
		mock_simple_polygon.cpp
	)
	set(test_helpers_paths "")
	foreach(test_helpers_source ${test_helpers_sources})
		list(APPEND test_helpers_paths "${CMAKE_SOURCE_DIR}/test/helpers/${test_helpers_source}")
	endforeach()
	add_library(apex_test_helpers ${test_helpers_paths})
	target_link_libraries(apex_test_helpers apex)

	#The names of all tests. Each must have a file called "test/<name>.cpp" as the source file.
	#Instead of slashes for the directories, use periods.
	set(test_names
		coordinate
		point2
		simple_polygon
		simple_polygon_batch
		simple_polygon_batch_view
		detail.area
		detail.translate
	)

	#To make sure that the tests are built before running them, add the building of these tests as an additional test.
	add_custom_target(build_all_tests)
	add_test(BuildTests "${CMAKE_COMMAND}" --build ${CMAKE_CURRENT_BINARY_DIR} --target build_all_tests)

	include(GoogleTest) #Defines the gtest_discover_tests command.

	foreach(test_case ${test_names})
		#Expand periods to slashes for the path to the test case, since CMake can't handle slashes in target names.
		string(REPLACE . / test_path ${test_case})
		#Make sure that the directory exists.
		get_filename_component(test_dirname ${test_path} DIRECTORY)
		file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${test_dirname})

		add_executable(${test_case} test/${test_path}.cpp ${test_helpers})
		target_link_libraries(${test_case} PRIVATE apex apex_test_helpers ${GOOGLETEST_BOTH_LIBRARIES} "${CMAKE_THREAD_LIBS_INIT}")
		target_include_directories(${test_case} PRIVATE ${GOOGLETEST_INCLUDE_DIRS})
		add_dependencies(${test_case} GoogleTest apex_test_helpers)
		add_dependencies(build_all_tests ${test_case}) #Make sure that this gets built as part of the build_all_tests target.
		gtest_discover_tests(${test_case} PROPERTIES DEPENDS BuildTests)
	endforeach()
endif()

#Generating documentation.
option(BUILD_DOCUMENTATION "Construct HTML-based API documentation pages." OFF)
if(BUILD_DOCUMENTATION)
	find_package(Doxygen REQUIRED dot)

	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
	add_custom_target(doc ALL
		COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		COMMENT "Generating API documentation."
	)
endif()