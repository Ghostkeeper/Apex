/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <functional> //To bind a function inside the job.
#include <future> //To create the future task to execute once planned in.
#include <gtest/gtest.h> //To run the tests.
#include "apex/detail/job.hpp" //The code under test.

namespace apex {

/*
 * Example computational task that a job could be meant to execute.
 * \param a One of the numbers to sum.
 * \param b The other number to sum.
 */
static void sum(const int a, const int b, int& result) {
	result = a + b;
}

/*
 * Tests executing a job. No dependency checking is done, simply executing the
 * job.
 *
 * The code involved here is so simple and transparent that this basically just
 * serves as a test for the compiler...
 */
TEST(Job, Execution) {
	int result = 0; //Initialise it to a wrong value so we can be sure that the function gets executed.
	std::packaged_task<void()> task([result = std::ref(result)]() {
		sum(3, 4, result);
	});
	Job job(task, std::vector<const Job*>());
	job.task();
	EXPECT_EQ(result, 7);
}

}