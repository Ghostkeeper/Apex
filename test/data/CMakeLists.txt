#Library for performing massively parallel computations on polygons.
#Copyright (C) 2021 Ghostkeeper
#This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
#This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
#You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.

#Prepares the contents of a file for inclusion as a string literal.
#Once prepared, the file can be defined as a string in C++ as follows:
# constexpr char* some_file =
# #include "data/path/file.txt"
# ;
#This causes the variable to be filled with the contents of that file.
#The prepared files will be placed in the build directory of CMake, where the
#compiler can find them.
function(prepare_string_literal filename)
	file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${filename}" contents)
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${filename}" "R\"!!!(${contents})!!!\"")
endfunction(prepare_string_literal)

prepare_string_literal("simple_polygon/square_1000.svg")