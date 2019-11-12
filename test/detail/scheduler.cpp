/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include <gtest/gtest.h> //To run the tests.

#include "apex/detail/scheduler.hpp" //The code under test.

namespace apex {

/*
 * A container for a function that's being called.
 *
 * This container also tracks which functions get called and how often.
 */
class MockClass {
public:
	unsigned int func1_called = 0;
	unsigned int func2_called = 0;
	unsigned int func3_called = 0;
	void func1() {
		func1_called++;
	}
	void func2() {
		func2_called++;
	}
	void func3() {
		func3_called++;
	}
};

/*
 * Tests whether all scheduled jobs are actually executed.
 */
TEST(Scheduler, AllExecuted) {
	MockClass called;
	Scheduler scheduler;
	std::packaged_task<void()> task1([called = &called]() {
		called->func1();
	});
	std::packaged_task<void()> task2([called = &called]() {
		called->func2();
	});
	std::packaged_task<void()> task3([called = &called]() {
		called->func3();
	});
	scheduler.schedule(Job(task1, std::vector<const Job*>()));
	scheduler.schedule(Job(task2, std::vector<const Job*>()));
	scheduler.schedule(Job(task3, std::vector<const Job*>()));
	scheduler.run();
	EXPECT_EQ(called.func1_called, 1);
	EXPECT_EQ(called.func2_called, 1);
	EXPECT_EQ(called.func3_called, 1);
}

}