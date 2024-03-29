/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_GPU_DATA_TRACKER
#define APEX_GPU_DATA_TRACKER

//All the types we need to be able to sync.
#include "apex/point2.hpp"

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
 *
 * Since OpenMP needs to know the types of data being handled, this class
 * features overloads for several data types that need to be synchronised to the
 * GPU. If a different data type needs to be added, it can easily be added.
 */
class GPUDataTracker {
public:
	/*!
	 * Signal that the point data has changed on the GPU.
	 *
	 * Any data on the host will be invalidated. If data is changed on the host
	 * and the GPU simultaneously, the latest change will be seen as leading.
	 */
	static void changed_on_gpu(const Point2* points) {
		sync_state[points] = GPUSyncState::DEVICE;
	}

	/*!
	 * Signal that the point data has changed on the host.
	 *
	 * Any data on the GPU will be invalidated. If data is changed on the host
	 * and the GPU simultaneously, the latest change will be seen as leading.
	 */
	static void changed_on_host(const Point2* points) {
		sync_state[points] = GPUSyncState::HOST;
	}

	/*!
	 * Synchronise point data to the GPU.
	 *
	 * If the GPU data was outdated, this ensures that the data on the GPU is
	 * up to date again. If the data on the GPU was already updated, this won't
	 * unnecessarily transfer anything.
	 */
	static void sync_to_gpu(const Point2* points, const size_t count) {
		std::unordered_map<const Point2*, GPUSyncState>::iterator current_sync_state = sync_state.find(points);
		if(current_sync_state != sync_state.end() && current_sync_state->second != GPUSyncState::HOST) { //GPU already has the most recent copy.
			return;
		}
		#pragma omp target enter data map(to:points[count])
		sync_state[points] = GPUSyncState::SYNC; //The host and device are now in sync.
	}

	/*!
	 * Synchronise point data to the host.
	 *
	 * If the host data was outdated, this ensures that the data on the host is
	 * up to date again. If the data on the host was already updated, this won't
	 * unnecessarily transfer anything.
	 */
	static void sync_to_host(const Point2* points, const size_t count) {
		std::unordered_map<const Point2*, GPUSyncState>::iterator current_sync_state = sync_state.find(points);
		if(current_sync_state == sync_state.end() || current_sync_state->second != GPUSyncState::DEVICE) { //Host already has the most recent copy.
			return;
		}
		#pragma omp target update from(points[count])
		sync_state[points] = GPUSyncState::SYNC; //The host and device are now in sync.
	}

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

protected:
	/*!
	 * These are the possible states of a piece of data being stored on the GPU
	 * or on the host.
	 *
	 * Either the host has the most recent copy, the GPU has the most recent
	 * copy, or both have the most recent copy (they are in sync).
	 */
	enum GPUSyncState {
		HOST, //The host currently has a more recent copy. The polygon may not be on the GPU at all.
		DEVICE, //The GPU device currently has a more recent copy.
		SYNC //The host and GPU are currently in sync. Both have a recent copy.
	};
	
	/*!
	 * For each memory object, tracks whether the object is currently up-to-date
	 * in the host, in the GPU, or both.
	 */
	static std::unordered_map<const Point2*, GPUSyncState> sync_state;
};

std::unordered_map<const Point2*, GPUDataTracker::GPUSyncState> GPUDataTracker::sync_state;

}

#endif //APEX_GPU_DATA_TRACKER