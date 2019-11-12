/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_SCHEDULER
#define APEX_SCHEDULER

#include "apex/detail/job.hpp" //The jobs scheduled by the scheduler.

namespace apex {

/*
 * This class makes sure that jobs are executed in order according to their
 * dependencies, as well as optimally in parallel.
 */
class Scheduler {
	/*
	 * The jobs that still need to be executed.
	 */
	std::vector<const Job*> jobs;

public:
	/*
	 * Schedule a new job to be run.
	 */
	void schedule(const Job* job) {
		jobs.push_back(job);
	}

	/*
	 * Runs all jobs.
	 *
	 * The order in which the jobs are ran is such that all dependencies are met
	 * before executing a job.
	 */
	void run() {
		//TODO: Test for dependencies.
		for(const Job* job : jobs) {
			job->task();
		}
	}
};

}

#endif //APEX_SCHEDULER