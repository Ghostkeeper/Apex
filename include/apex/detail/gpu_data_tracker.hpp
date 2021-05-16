/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_GPU_DATA_TRACKER
#define APEX_GPU_DATA_TRACKER

namespace apex {

/*!
 * The ``GPUDataTracker`` tracks what data is currently on the GPU and what is
 * currently on the host.
 *
 * This prevents having to move data back and forth between the GPU and the
 * host if it is not necessary. Data transfer between host and accelerator is
 * lazy. As soon as an algorithm is requested to be called on the GPU, the
 * tracker is queried to see whether data needs to be transferred. If the GPU
 * already had the most up-to-date data, nothing is transferred. Otherwise, the
 * data is updated on the GPU. Similarly, if data is requested from the host, it
 * requests whether the data on the host is up-to-date, and transfers it if
 * needed. This ensures minimal traffic between the host and the GPU, which
 * improves performance considerably.
 *
 * The ``GPUDataTracker`` tracks a synchronisation state for each piece of data.
 * This state tracks whether the data on the host is the most up-to-date
 * version, the data on the GPU is the most up-to-date, or whether the two are
 * in sync.
 *
 * Being a resource management system, it is inappropriate to have multiple
 * copies tracking the synchronisation state of memory objects. For that reason,
 * this class is completely static. It is impossible to instantiate the class.
 * The class exists only as a scope to provide namespacing and to allow tests to
 * substitute the class for a mock.
 */
class GPUDataTracker {
public:
	/*!
	 * This object may not be instantiated.
	 *
	 * Being a resource management system, it is inappropriate to have multiple
	 * copies tracking the synchronisation state of memory objects. For that
	 * reason, this class is completely static. It is impossible to instantiate
	 * the class. The class exists only as a scope to provide namespacing and to
	 * allow tests to substitute the class for a mock.
	 */
	GPUDataTracker() = delete;

	/*!
	 * This object may not be copied.
	 */
	GPUDataTracker(const GPUDataTracker& original) = delete;

	/*!
	 * This object may not be assigned.
	 */
	void operator =(const GPUDataTracker& original) = delete;
};

}

#endif //APEX_GPU_DATA_TRACKER