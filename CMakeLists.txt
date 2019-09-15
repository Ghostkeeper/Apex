#Library for performing massively parallel computations on polygons.
#Copyright (C) 2019 Ghostkeeper
#This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
#You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.12) #Oldest version it was tested with.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules") #Finding dependencies.

#Release configuration.
set(APEX_VERSION_MAJOR 0 CACHE STRING "Major release version. This must be incremented if there are changes that are not backwards compatible.")
set(APEX_VERSION_MINOR 0 CACHE STRING "Minor release version. This must be incremented if there are changes that are not forwards compatible.")
set(APEX_VERSION_PATCH 1 CACHE STRING "Patch release version. This must be incremented for every release where there are no changes to the API.")

project(Apex VERSION ${APEX_VERSION_MAJOR}.${APEX_VERSION_MINOR}.${APEX_VERSION_PATCH} DESCRIPTION "Library for performing massively parallel computations on polygons.")

#Tests.
option(BUILD_TESTS "Build tests to verify correctness of the library." OFF)
if(BUILD_TESTS)
	enable_testing()

	find_package(GoogleTest REQUIRED)

	#The names of all tests. Each must have a file called "test/<name>.cpp" as the source file.
	#Instead of slashes for the directories, use periods.
	set(test_names
		coordinate
	)

	#To make sure that the tests are built before running them, add the building of these tests as an additional test.
	add_custom_target(build_all_tests)
	add_test(BuildTests "${CMAKE_COMMAND}" --build ${CMAKE_CURRENT_BINARY_DIR} --target build_all_tests)

	foreach(test_case ${test_names})
		#Expand periods to slashes for the path to the test case, since CMake can't handle slashes in target names.
		string(REPLACE . / test_path ${test_case})
		#Make sure that the directory exists.
		get_filename_component(test_dirname ${test_path} DIRECTORY)
		file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${test_dirname})

		add_executable(${test_case} test/${test_path}.cpp ${test_helpers})
		target_include_directories(${test_case} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include") #Include Apex itself so that it can call the code under test.
		target_link_libraries(${test_case} ${GOOGLETEST_BOTH_LIBRARIES}) #Link with GoogleTest.
		target_include_directories(${test_case} PRIVATE ${GOOGLETEST_INCLUDE_DIRS})
		add_dependencies(build_all_tests ${test_case}) #Make sure that this gets built as part of the build_all_tests target.
		add_test(${test_case} ${test_case})
		set_tests_properties(${test_case} PROPERTIES DEPENDS BuildTests)
	endforeach()
endif()