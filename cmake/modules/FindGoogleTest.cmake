#Library for performing massively parallel computations on polygons.
#Copyright (C) 2018 Ghostkeeper
#This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
#You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.

#Find module for the Google Test testing suite.
#
#This module exports the following variables:
# - GOOGLETEST_FOUND: True if the Google Test suite was found, or False if it
#   wasn't.
# - GOOGLETEST_INCLUDE_DIRS: The location of Google Test's headers, if found.
# - GOOGLETEST_LIBRARIES: The library with Google Test's functions.
# - GOOGLETEST_MAIN_LIBRARIES: A library that allows creating a test as a
#   separate executable (with its own main() entry point).
# - GOOGLETEST_BOTH_LIBRARIES: Both the normal library and the main library.

#First try finding the library using CMake's built-in find script.
find_package(GTest QUIET)

if(GTEST_FOUND)
	if(NOT GoogleTest_FIND_QUIETLY)
		message(STATUS "Found GoogleTest.")
	endif()
	set(GOOGLETEST_FOUND TRUE)
	set(GOOGLETEST_INCLUDE_DIRS "${GTEST_INCLUDE_DIRS}")
	set(GOOGLETEST_LIBRARIES "${GTEST_LIBRARIES}")
	set(GOOGLETEST_MAIN_LIBRARIES "${GTEST_MAIN_LIBRARIES}")
	set(GOOGLETEST_BOTH_LIBRARIES "${GTEST_BOTH_LIBRARIES}")
	add_custom_target(GoogleTest) #Dummy target so that you can ensure GoogleTest gets built as dependency iff necessary.
else() #GTest was not found.
	#Give the option to build Google Test from source.
	option(BUILD_GOOGLETEST "Build Google Test from source." ON)
	if(BUILD_GOOGLETEST)
		if(NOT GoogleTest_FIND_QUIETLY)
			message(STATUS "Building Google Test from source.")
		endif()
		include(ExternalProject)
		ExternalProject_Add(GoogleTest
			GIT_REPOSITORY https://github.com/google/googletest
			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
			UPDATE_COMMAND "" #Don't update at all after building once. This speeds up compilation considerably while developing.
			INSTALL_COMMAND "" #If we want to build it just for this project, no need to install it.
		)
		set(GOOGLETEST_FOUND TRUE)
		set(GOOGLETEST_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/GoogleTest-prefix/src/GoogleTest/googletest/include")
		set(GOOGLETEST_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/GoogleTest-prefix/src/GoogleTest-build/lib/libgtest${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set(GOOGLETEST_MAIN_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/GoogleTest-prefix/src/GoogleTest-build/lib/libgtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set(GOOGLETEST_BOTH_LIBRARIES "${GOOGLETEST_LIBRARIES};${GOOGLETEST_MAIN_LIBRARIES}")
	elseif(NOT GoogleTest_FIND_QUIETLY)
		if(GoogleTest_FIND_REQUIRED)
			message(FATAL_ERROR "Could NOT find Google Test.")
		else()
			message(WARNING "Could NOT find Google Test.")
		endif()
	endif()
endif()

mark_as_advanced(GOOGLETEST_INCLUDE_DIRS)
mark_as_advanced(GOOGLETEST_LIBRARIES)
mark_as_advanced(GOOGLETEST_MAIN_LIBRARIES)
mark_as_advanced(GOOGLETEST_BOTH_LIBRARIES)