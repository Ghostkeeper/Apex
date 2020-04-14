#Library for performing massively parallel computations on polygons.
#Copyright (C) 2020 Ghostkeeper
#This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
#You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.

#Find module for the Google Test testing suite.
#
#This module expands on CMake's built-in find module in three ways:
# - It gives the option to download GoogleTest sources from the internet and use
#   that instead of using the installed libraries on your system. This is
#   required since we need a version of GoogleTest that is not yet installed by
#   default through the package distribution of most distro's yet.
# - It allows requiring certain versions of the GoogleTest library.
# - It outputs the version that was actually found.
#
# This supports version testing up to GoogleTest 1.10. The patch version cannot
# be detected and is thus ignored (in both the requirements from the find call
# and the output variables).
#
#This module exports the following variables:
# - GOOGLETEST_FOUND: True if the Google Test suite was found, or False if it
#   wasn't.
# - GOOGLETEST_INCLUDE_DIRS: The location of Google Test's headers, if found.
# - GOOGLETEST_LIBRARIES: The library with Google Test's functions.
# - GOOGLETEST_MAIN_LIBRARIES: A library that allows creating a test as a
#   separate executable (with its own main() entry point).
# - GOOGLETEST_BOTH_LIBRARIES: Both the normal library and the main library.
# - GOOGLETEST_VERSION_STRING: Version of Google Test found, e.g. "1.10.0".
# - GOOGLETEST_VERSION_MAJOR: Major version number of Google Test found.
# - GOOGLETEST_VERSION_MINOR: Minor version number of Google Test found.

#How to obtain GoogleTest.
option(USE_SYSTEM_GOOGLETEST "Use Google Test libraries installed on your computer." TRUE)
option(BUILD_GOOGLETEST "Download the source code for Google Test, build that and use it in this project." FALSE)
if(GoogleTest_FIND_REQUIRED AND NOT USE_SYSTEM_GOOGLETEST AND NOT BUILD_GOOGLETEST)
	message(FATAL_ERROR "Need to have either USE_SYSTEM_GOOGLETEST or BUILD_GOOGLETEST enabled, since GoogleTest is required.")
endif()
if(USE_SYSTEM_GOOGLETEST AND BUILD_GOOGLETEST)
	message(FATAL_ERROR "Need to have either USE_SYSTEM_GOOGLETEST or BUILD_GOOGLETEST enabled, not both, or I don't know which one you want me to use.")
endif()

if(USE_SYSTEM_GOOGLETEST)
	#First try finding the library using CMake's built-in find script.
	find_package(GTest QUIET)
	if(GTEST_FOUND)
		if(NOT GoogleTest_FIND_QUIETLY)
			message(STATUS "Found Google Test on the system.")
		endif()
		set(GOOGLETEST_FOUND TRUE)
		set(GOOGLETEST_INCLUDE_DIRS "${GTEST_INCLUDE_DIRS}")
		set(GOOGLETEST_LIBRARIES "${GTEST_LIBRARIES}")
		set(GOOGLETEST_MAIN_LIBRARIES "${GTEST_MAIN_LIBRARIES}")
		set(GOOGLETEST_BOTH_LIBRARIES "${GTEST_BOTH_LIBRARIES}")
		add_custom_target(GoogleTest) #Dummy target so that you can ensure GoogleTest gets built as dependency iff necessary.
	endif()
endif()

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
endif()

if(NOT GOOGLETEST_FOUND)
	if(NOT GoogleTest_FIND_QUIETLY)
		if(GoogleTest_FIND_REQUIRED)
			message(FATAL_ERROR "Could NOT find Google Test.")
		else()
			message(WARNING "Could NOT find Google Test.")
		endif()
	endif()
endif()

#Try to find the version number of GoogleTest by component testing.
set(GOOGLETEST_VERSION_MAJOR 1) #Always 1.
set(GOOGLETEST_VERSION_MINOR 0) #Start off under the assumption that it's at LEAST version 1.0, the oldest published version.

find_package(Threads REQUIRED) #Threading library is required for GoogleTest if it's compiled with default parameters, so sadly it's also required to detect version of GoogleThread.

#Test for v1.1.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.1.cpp" "#include<gtest/gtest.h>\nTEST(Probe, ExceptionAssertion) { int i = 0; ASSERT_NO_THROW(i == 0); }")
try_compile(_googletest_is_1_1 "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe" "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.1.cpp" LINK_LIBRARIES "${GOOGLETEST_BOTH_LIBRARIES};${CMAKE_THREAD_LIBS_INIT}")
if(_googletest_is_1_1)
	set(GOOGLETEST_VERSION_MINOR 1)
endif()

set(GOOGLETEST_VERSION_STRING "${GOOGLETEST_VERSION_MAJOR}.${GOOGLETEST_VERSION_MINOR}")
if(NOT GoogleTest_FIND_QUIETLY)
	message(STATUS "Google Test version is: ${GOOGLETEST_VERSION_STRING}.")
endif()

mark_as_advanced(GOOGLETEST_INCLUDE_DIRS)
mark_as_advanced(GOOGLETEST_LIBRARIES)
mark_as_advanced(GOOGLETEST_MAIN_LIBRARIES)
mark_as_advanced(GOOGLETEST_BOTH_LIBRARIES)