#Library for performing massively parallel computations on polygons.
#Copyright (C) 2022 Ghostkeeper
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

prepare_string_literal("polygon/arrowhead.svg")
prepare_string_literal("polygon/empty.svg")
prepare_string_literal("polygon/hourglass.svg")
prepare_string_literal("polygon/line.svg")
prepare_string_literal("polygon/negative_square.svg")
prepare_string_literal("polygon/point.svg")
prepare_string_literal("polygon/square_1000.svg")
prepare_string_literal("polygon/square_1000_centred.svg")
prepare_string_literal("polygon/square_1000_negative_x.svg")
prepare_string_literal("polygon/square_1000_negative_xy.svg")
prepare_string_literal("polygon/square_1000_negative_y.svg")
prepare_string_literal("polygon/thin_rectangle.svg")
prepare_string_literal("polygon/triangle_1000.svg")
prepare_string_literal("polygon/zero_length_segments.svg")
prepare_string_literal("polygon/zero_width.svg")

prepare_string_literal("polygon_batch/edge_cases.svg")
prepare_string_literal("polygon_batch/empty.svg")
prepare_string_literal("polygon_batch/single_empty.svg")
prepare_string_literal("polygon_batch/single_line.svg")
prepare_string_literal("polygon_batch/single_point.svg")
prepare_string_literal("polygon_batch/single_square.svg")
prepare_string_literal("polygon_batch/square_triangle.svg")
prepare_string_literal("polygon_batch/square_triangle_square.svg")
prepare_string_literal("polygon_batch/two_squares.svg")