/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2022 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef BENCHMARKER_SIZES
#define BENCHMARKER_SIZES

#include <vector>

namespace benchmarker {

/*!
 * A list of sizes for benchmarking with big polygons.
 *
 * This is useful for benchmarking polygon operations that are fast or scale
 * well to big polygons.
 */
static std::vector<size_t> sizes_polygon_big = {
	   0,  100,  200,  300,  400,  500,  600,  700,  800,  900,
	1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900,
	2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900,
	3000, 3100, 3200, 3300, 3400, 3500, 3600, 3700, 3800, 3900,
	4000, 4100, 4200, 4300, 4400, 4500, 4600, 4700, 4800, 4900,
	5000, 5100, 5200, 5300, 5400, 5500, 5600, 5700, 5800, 5900,
	6000, 6100, 6200, 6300, 6400, 6500, 6600, 6700, 6800, 6900,
	7000, 7100, 7200, 7300, 7400, 7500, 7600, 7700, 7800, 7900,
	8000, 8100, 8200, 8300, 8400, 8500, 8600, 8700, 8800, 8900,
	9000, 9100, 9200, 9300, 9400, 9500, 9600, 9700, 9800, 9900,
	10000
};

}

#endif //BENCHMARKER_SIZES