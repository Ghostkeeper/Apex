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

/*!
 * A list of sizes for benchmarking with big batches of polygons.
 *
 * This is useful for benchmarking polygon operations that are fast or scale
 * well to big amounts of data.
 */
static std::vector<size_t> sizes_polygon_batch_big = {
	  0,  10,  20,  30,  40,  50,  60,  70,  80,  90,
	100, 110, 120, 130, 140, 150, 160, 170, 180, 190,
	200, 210, 220, 230, 240, 250, 260, 270, 280, 290,
	300, 310, 320, 330, 340, 350, 360, 370, 380, 390,
	400, 410, 420, 430, 440, 450, 460, 470, 480, 490,
	500, 510, 520, 530, 540, 550, 560, 570, 580, 590,
	600, 610, 620, 630, 640, 650, 660, 670, 680, 690,
	700, 710, 720, 730, 740, 750, 760, 770, 780, 790,
	800, 810, 820, 830, 840, 850, 860, 870, 880, 890,
	900, 910, 920, 930, 940, 950, 960, 970, 980, 990,
	1000
};

}

#endif //BENCHMARKER_SIZES