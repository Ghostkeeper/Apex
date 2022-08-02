#Library for performing massively parallel computations on polygons.
#Copyright (C) 2022 Ghostkeeper
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
#This supports version testing up to GoogleTest 1.10. The patch version cannot
#be detected and is thus ignored (in both the requirements from the find call
#and the output variables).
#
#This module exports the following variables:
# - GOOGLETEST_FOUND: True if the Google Test suite was found, or False if it
#   wasn't.
# - GOOGLETEST_INCLUDE_DIRS: The location of Google Test's headers, if found.
# - GOOGLETEST_LIBRARIES: The library with Google Test's functions.
# - GOOGLETEST_MAIN_LIBRARIES: A library that allows creating a test as a
#   separate executable (with its own main() entry point).
# - GOOGLETEST_BOTH_LIBRARIES: Both the normal library and the main library.
# - GOOGLETEST_VERSION_STRING: Version of Google Test found, e.g. "1.10".
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

	#Try to find the version number of GoogleTest by component testing.
	set(GOOGLETEST_VERSION_MAJOR 1) #Always 1.
	set(GOOGLETEST_VERSION_MINOR 0) #Start off under the assumption that it's at LEAST version 1.0, the oldest published version.

	find_package(Threads REQUIRED) #Threading library is required for GoogleTest if it's compiled with default parameters, so sadly it's also required to detect version of GoogleTest.

	#Test for v1.1.
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.1.cpp" "#include<gtest/gtest.h>\nTEST(Probe, ExceptionAssertion) { ASSERT_NO_THROW(1 == 0); }") #Testing for throwing is new in 1.1.
	try_compile(_googletest_is_1_1 "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe" "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.1.cpp" LINK_LIBRARIES "${GOOGLETEST_BOTH_LIBRARIES};${CMAKE_THREAD_LIBS_INIT}")
	if(_googletest_is_1_1)
		set(GOOGLETEST_VERSION_MINOR 1)
	endif()
	unset(_googletest_is_1_1)

	#Test for v1.2.
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.2.cpp" "#include<gtest/gtest.h>\nTEST(Probe, FatalFailureAssertion) { ASSERT_NO_FATAL_FAILURE(1 == 0); }") #Fatality assertions are new in 1.2.
	try_compile(_googletest_is_1_2 "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe" "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.2.cpp" LINK_LIBRARIES "${GOOGLETEST_BOTH_LIBRARIES};${CMAKE_THREAD_LIBS_INIT}")
	if(_googletest_is_1_2)
		set(GOOGLETEST_VERSION_MINOR 2)
	endif()
	unset(_googletest_is_1_2)

	#Test for v1.3.
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.3.cpp" "#include<gtest/gtest.h>\nTEST(Probe, DeathTest) { ASSERT_EXIT(1 == 0, ::testing::ExitedWithCode(0), \"\"); }") #Testing for death is new in 1.3.
	try_compile(_googletest_is_1_3 "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe" "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.3.cpp" LINK_LIBRARIES "${GOOGLETEST_BOTH_LIBRARIES};${CMAKE_THREAD_LIBS_INIT}")
	if(_googletest_is_1_3)
		set(GOOGLETEST_VERSION_MINOR 3)
	endif()
	unset(_googletest_is_1_3)

	#Test for v1.4 or v1.5.
	#Can't detect the difference between those since there were no new features in 1.5 that would make compilation break when using 1.4.
	#Only if threads are not available, but we require threads anyway, so we can't test this.
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.5.cpp" "#include<gtest/gtest.h>\nTEST(Probe, DeathIfSupported) { ASSERT_DEATH_IF_SUPPORTED(1 == 0, \"\"); }") #DeathIfSupported is new in 1.4.
	try_compile(_googletest_is_1_5 "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe" "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.5.cpp" LINK_LIBRARIES "${GOOGLETEST_BOTH_LIBRARIES};${CMAKE_THREAD_LIBS_INIT}")
	if(_googletest_is_1_5)
		set(GOOGLETEST_VERSION_MINOR 5)
	endif()
	unset(_googletest_is_1_5)

	#Test for v1.6 or v1.7.
	#Can't detect the difference between those since there were no new features in 1.7 that would make compilation break when using 1.6.
	#There were new features but only in the output of the program in the test log; since we only compile and don't run we can't test this.
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.7.cpp" "#include<gtest/gtest.h>\nTEST(Probe, AddFailureAt) { ADD_FAILURE_AT(\"googletest_probe_v1.7.cpp\", 1); }") #ADD_FAILURE_AT is new in 1.7.
	try_compile(_googletest_is_1_7 "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe" "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.7.cpp" LINK_LIBRARIES "${GOOGLETEST_BOTH_LIBRARIES};${CMAKE_THREAD_LIBS_INIT}")
	if(_googletest_is_1_7)
		set(GOOGLETEST_VERSION_MINOR 7)
	endif()
	unset(_googletest_is_1_7)

	#Test for v1.8.
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.8.cpp" "#include<gtest/gtest.h>\nTEST(Probe, Injection) { ASSERT_EQ(1, GTEST_HAS_STD_SHARED_PTR_); }") #Injection tests are new in 1.8.
	try_compile(_googletest_is_1_8 "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe" "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.8.cpp" LINK_LIBRARIES "${GOOGLETEST_BOTH_LIBRARIES};${CMAKE_THREAD_LIBS_INIT}")
	if(_googletest_is_1_8)
		set(GOOGLETEST_VERSION_MINOR 8)
	endif()
	unset(_googletest_is_1_8)

	#Test for v1.10.
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.10.cpp" "#include<gtest/gtest.h>\nclass TestSuite : public ::testing::TestWithParam<int> {};\nTEST_P(TestSuite, ParameterisedTest) {}\nINSTANTIATE_TEST_SUITE_P(ParameterisedInst, TestSuite, ::testing::Values());") #INSTANTIATE_TEST_SUITE_P is new in 1.10.
	try_compile(_googletest_is_1_10 "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe" "${CMAKE_CURRENT_BINARY_DIR}/googletest_probe/googletest_probe_v1.10.cpp" LINK_LIBRARIES "${GOOGLETEST_BOTH_LIBRARIES};${CMAKE_THREAD_LIBS_INIT}")
	if(_googletest_is_1_10)
		set(GOOGLETEST_VERSION_MINOR 10)
	endif()
	unset(_googletest_is_1_10)

	set(GOOGLETEST_VERSION_STRING "${GOOGLETEST_VERSION_MAJOR}.${GOOGLETEST_VERSION_MINOR}")
	if(NOT GoogleTest_FIND_QUIETLY)
		message(STATUS "Google Test version is: ${GOOGLETEST_VERSION_STRING}.")
	endif()

	#Check if we have the correct version then as per requirements.
	if(GoogleTest_FIND_VERSION_EXACT)
		if((NOT GoogleTest_FIND_VERSION_MAJOR EQUAL GOOGLETEST_VERSION_MAJOR) OR (NOT GoogleTest_FIND_VERSION_MINOR EQUAL GOOGLETEST_VERSION_MINOR))
			if(GoogleTest_FIND_REQUIRED)
				message(FATAL_ERROR "Google Test version needs to be exactly ${GoogleTest_FIND_VERSION_MAJOR}.${GoogleTest_FIND_VERSION_MINOR}, but found version ${GOOGLETEST_VERSION_MAJOR}.${GOOGLETEST_VERSION_MINOR}.")
			elseif(NOT GoogleTEST_FIND_QUIET)
				message(WARNING "Google Test version needs to be exactly ${GoogleTest_FIND_VERSION_MAJOR}.${GoogleTest_FIND_VERSION_MINOR}, but found version ${GOOGLETEST_VERSION_MAJOR}.${GOOGLETEST_VERSION_MINOR}.")
			endif()
		endif()
	else() #Inexact version matching.
		if(GoogleTest_FIND_VERSION_MAJOR GREATER GOOGLETEST_VERSION_MAJOR)
			if(GoogleTest_FIND_REQUIRED)
				message(FATAL_ERROR "Google Test version needs to be at least ${GoogleTest_FIND_VERSION_MAJOR}.${GoogleTest_FIND_VERSION_MINOR}, but found version ${GOOGLETEST_VERSION_MAJOR}.${GOOGLETEST_VERSION_MINOR}.")
			elseif(NOT GoogleTEST_FIND_QUIET)
				message(WARNING "Google Test version needs to be at least ${GoogleTest_FIND_VERSION_MAJOR}.${GoogleTest_FIND_VERSION_MINOR}, but found version ${GOOGLETEST_VERSION_MAJOR}.${GOOGLETEST_VERSION_MINOR}.")
			endif()
		elseif(GoogleTest_FIND_VERSION_MAJOR EQUAL GOOGLETEST_VERSION_MAJOR)
			if(GoogleTest_FIND_VERSION_MINOR GREATER GOOGLETEST_VERSION_MINOR)
				if(GoogleTest_FIND_REQUIRED)
					message(FATAL_ERROR "Google Test version needs to be at least ${GoogleTest_FIND_VERSION_MAJOR}.${GoogleTest_FIND_VERSION_MINOR}, but found version ${GOOGLETEST_VERSION_MAJOR}.${GOOGLETEST_VERSION_MINOR}.")
				elseif(NOT GoogleTEST_FIND_QUIET)
					message(WARNING "Google Test version needs to be at least ${GoogleTest_FIND_VERSION_MAJOR}.${GoogleTest_FIND_VERSION_MINOR}, but found version ${GOOGLETEST_VERSION_MAJOR}.${GOOGLETEST_VERSION_MINOR}.")
				endif()
			endif()
		endif()
	endif()
endif()

if(BUILD_GOOGLETEST)
	if(NOT GoogleTest_FIND_QUIETLY)
		message(STATUS "Building Google Test from source.")
	endif()

	#Translate the desired version number to a tag on Git.
	set(_googletest_tag "release-1.11.0")
	if(NOT GoogleTest_FIND_VERSION_MAJOR EQUAL 1)
		if(GoogleTest_FIND_REQUIRED)
			message(FATAL_ERROR "Unknown Google Test version ${GoogleTest_FIND_VERSION_MAJOR}.${GoogleTest_FIND_VERSION_MINOR} requested.")
		elseif(NOT GoogleTest_FIND_QUIET)
			message(WARNING "Unknown Google Test version ${GoogleTest_FIND_VERSION_MAJOR}.${GoogleTest_FIND_VERSION_MINOR} requested.")
		endif()
	endif()
	if(GoogleTest_FIND_VERSION_MINOR EQUAL 0)
		set(_googletest_tag "release-1.0.1")
	elseif(GoogleTest_FIND_VERSION_MINOR EQUAL 1)
		set(_googletest_tag "release-1.1.0")
	elseif(GoogleTest_FIND_VERSION_MINOR EQUAL 2)
		set(_googletest_tag "release-1.2.1")
	elseif(GoogleTest_FIND_VERSION_MINOR EQUAL 3)
		set(_googletest_tag "release-1.3.0")
	elseif(GoogleTest_FIND_VERSION_MINOR EQUAL 4)
		set(_googletest_tag "release-1.4.0")
	elseif(GoogleTest_FIND_VERSION_MINOR EQUAL 5)
		set(_googletest_tag "release-1.5.0")
	elseif(GoogleTest_FIND_VERSION_MINOR EQUAL 6)
		set(_googletest_tag "release-1.6.0")
	elseif(GoogleTest_FIND_VERSION_MINOR EQUAL 7)
		set(_googletest_tag "release-1.7.0")
	elseif(GoogleTest_FIND_VERSION_MINOR EQUAL 8)
		set(_googletest_tag "release-1.8.1")
	elseif(GoogleTest_FIND_VERSION_MINOR EQUAL 10)
		set(_googletest_tag "release-1.10.0")
	elseif(GoogleTest_FIND_VERSION_MINOR EQUAL 11)
		set(_googletest_tag "release-1.11.0")
	else()
		if(GoogleTest_FIND_REQUIRED)
			message(FATAL_ERROR "Unknown Google Test version ${GoogleTest_FIND_VERSION_MAJOR}.${GoogleTest_FIND_VERSION_MINOR} requested.")
		elseif(NOT GoogleTest_FIND_QUIET)
			message(WARNING "Unknown Google Test version ${GoogleTest_FIND_VERSION_MAJOR}.${GoogleTest_FIND_VERSION_MINOR} requested.")
		endif()
	endif()
	set(GOOGLETEST_VERSION_MAJOR ${GoogleTest_FIND_VERSION_MAJOR})
	set(GOOGLETEST_VERSION_MINOR ${GoogleTest_FIND_VERSION_MINOR})
	set(GOOGLETEST_VERSION_STRING "${GOOGLETEST_VERSION_MAJOR}.${GOOGLETEST_VERSION_MINOR}")

	include(ExternalProject)
	ExternalProject_Add(GoogleTest
		GIT_REPOSITORY https://github.com/google/googletest
		GIT_TAG ${_googletest_tag}
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
		UPDATE_COMMAND "" #Don't update at all after building once. This speeds up compilation considerably while developing.
		INSTALL_COMMAND "" #If we want to build it just for this project, no need to install it.
	)
	set(GOOGLETEST_FOUND TRUE)
	set(GOOGLETEST_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/GoogleTest-prefix/src/GoogleTest/googletest/include;${CMAKE_CURRENT_BINARY_DIR}/GoogleTest-prefix/src/GoogleTest/googlemock/include")
	set(GOOGLETEST_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/GoogleTest-prefix/src/GoogleTest-build/lib/libgtest${CMAKE_STATIC_LIBRARY_SUFFIX};${CMAKE_CURRENT_BINARY_DIR}/GoogleTest-prefix/src/GoogleTest-build/lib/libgmock${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(GOOGLETEST_MAIN_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/GoogleTest-prefix/src/GoogleTest-build/lib/libgtest_main${CMAKE_STATIC_LIBRARY_SUFFIX};${CMAKE_CURRENT_BINARY_DIR}/GoogleTest-prefix/src/GoogleTest-build/lib/libgmock_main${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(GOOGLETEST_BOTH_LIBRARIES "${GOOGLETEST_LIBRARIES};${GOOGLETEST_MAIN_LIBRARIES}")
endif()

if(NOT GOOGLETEST_FOUND)
	if(GoogleTest_FIND_REQUIRED)
		message(FATAL_ERROR "Could NOT find Google Test.")
	else()
		message(WARNING "Could NOT find Google Test.")
	endif()
endif()

mark_as_advanced(GOOGLETEST_INCLUDE_DIRS)
mark_as_advanced(GOOGLETEST_LIBRARIES)
mark_as_advanced(GOOGLETEST_MAIN_LIBRARIES)
mark_as_advanced(GOOGLETEST_BOTH_LIBRARIES)
