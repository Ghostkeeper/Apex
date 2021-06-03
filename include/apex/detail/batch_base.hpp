/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_BATCH_BASE
#define APEX_BATCH_BASE

namespace apex {

/*!
 * This class forms the basis of all batch implementations.
 *
 * Batches are supposed to behave transparently like vectors. Inheriting from
 * vectors is unsafe if used polymorphously. Since we have no controls over how
 * the (public) batch classes are used, and we want to impose no restrictions,
 * we'll let the Batch implementations inherit from the private BatchBase class.
 * That class behaves like a vector, but uses composition rather than
 * inheritance to mimic its behaviour. That way, inheriting from this batch base
 * is safe.
 * \tparam Element The type of data stored in this batch.
 */
template<typename Element>
class BatchBase {
	protected:
		/*!
		 * The main data contained in the batch.
		 *
		 * The batch behaves almost exactly like this vector.
		 */
		std::vector<Element> elements;
};

}

#endif //APEX_BATCH_BASE

