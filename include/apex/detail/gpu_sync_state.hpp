/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_GPU_SYNC_STATE
#define APEX_GPU_SYNC_STATE

namespace apex {

/*!
 * These are the possible states of a piece of data being stored on the GPU or
 * on the host.
 *
 * Either the host has the most recent copy, the GPU has the most recent copy,
 * or both have the most recent copy (they are in sync).
 */
enum GPUSyncState {
	HOST, //The host currently has a more recent copy. The polygon may not be on the GPU at all.
	DEVICE, //The GPU device currently has a more recent copy.
	SYNC //The host and GPU are currently in sync. Both have a recent copy.
};

}

#endif //APEX_GPU_SYNC_STATE