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
 * A simple class that tracks how often each of its functions have been called.
 */
class Tracker {
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
 * Fixture with a few pre-defined jobs and a scheduler you can use.
 */
class SchedulerFixture : public testing::Test {
public:
	/*
	 * Task that, when executed, increments ``tracker.func1_called``.
	 */
	std::packaged_task<void()> task1;

	/*
	 * Task that, when executed, increments ``tracker.func2_called``.
	 */
	std::packaged_task<void()> task2;

	/*
	 * Task that, when executed, increments ``tracker.func3_called``.
	 */
	std::packaged_task<void()> task3;

	/*
	 * Job that, when executed, increments ``tracker.func1_called``.
	 */
	Job job1;

	/*
	 * Job that, when executed, increments ``tracker.func2_called``.
	 */
	Job job2;

	/*
	 * Job that, when executed, increments ``tracker.func3_called``.
	 */
	Job job3;

	/*
	 * Tracks how often each job is executed.
	 */
	Tracker tracker;

	/*
	 * Does the work of scheduling.
	 *
	 * Here in a convenient field so that we don't have to construct it each
	 * time.
	 */
	Scheduler scheduler;

	SchedulerFixture() :
		task1([tracker = &tracker]() {
			tracker->func1();
		}),
		task2([tracker = &tracker]() {
			tracker->func2();
		}),
		task3([tracker = &tracker]() {
			tracker->func3();
		}),
		job1(task1),
		job2(task2),
		job3(task3) {}

	/*
	 * Creates all of the basic fixtures for the next test.
	 */
	void SetUp() {
		tracker = Tracker(); //Reset state.
		scheduler = Scheduler();
	}
};

/*
 * Tests whether all scheduled jobs are actually executed.
 */
TEST_F(SchedulerFixture, AllExecuted) {
	scheduler.schedule(&job1);
	scheduler.schedule(&job2);
	scheduler.schedule(&job3);
	scheduler.run();
	EXPECT_EQ(tracker.func1_called, 1);
	EXPECT_EQ(tracker.func2_called, 1);
	EXPECT_EQ(tracker.func3_called, 1);
}

}