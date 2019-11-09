/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2019 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_JOB
#define APEX_JOB

namespace apex {

/*
 * Represents a computational job that needs to be planned in.
 *
 * The job contains a function to call when the job is executed. This function
 * is most likely a bound function with all of its parameters included (though
 * not necessarily required).
 *
 * Since the job is planned in and executed later, not all input parameters of
 * the job may be available yet. They may be the result of other jobs. For that
 * reason, the output of the executed function needs to be an output parameter
 * as well, which can be constructed before starting the job and passed on to
 * future jobs before the job is executed.
 *
 * The job can also have dependencies, which are other jobs which must be
 * completed before this job is started. The scheduler will make sure that all
 * of these jobs are executed in sequence (as long as they don't contain
 * circular dependencies). Some jobs may be executed in parallel, if this is
 * conducive to better performance.
 */
class Job {
public:
	/*
	 * The task to execute. All of its parameters must be bound into this
	 * function.
	 */
	int task; //TODO: Type is temporary.

	/*
	 * Jobs that must be executed before this job is executed. The scheduler
	 * will ensure that these dependencies are met before executing this task.
	 */
	const std::vector<Job> dependencies;

	/*
	 * Constructs a new job.
	 *
	 * Keep in mind that this makes a copy of the task. The bound parameters in
	 * this task should be held by reference to prevent unnecessary data
	 * copying.
	 * \param task The function to execute. Its return type is ignored, so there
	 * should not be a return type. Instead, use an output parameter to return a
	 * value. This output parameter can then be used as input for subsequent
	 * jobs.
	 * \param dependencies Other jobs that must be completed before this job is
	 * started.
	 */
	Job(int task, const std::vector<Job> dependencies) : task(task), dependencies(dependencies) {};
};

}

#endif //APEX_JOB