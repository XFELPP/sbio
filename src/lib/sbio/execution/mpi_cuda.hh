/*
 * sbio - Stream Broker IO
 *
 * Copyright (C) 2025-2026 Gabriel Dorlhiac
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SBIO_EXECUTION_MPI_CUDA_HH
#define SBIO_EXECUTION_MPI_CUDA_HH

#include "sbio/execution/composed.hh"
#include "sbio/execution/cuda.hh"
#include "sbio/execution/mpi.hh"
#include "sbio/execution/mpi_threaded.hh"

namespace sbio {
  /**
   * Run MPI-based distribution with GPU support.
   *
   * The Data buffers for a rank live on the device. If `Config::devices` has been
   * used (as opposed to just `device`), the ranks for a node are spread out between
   * the selected GPUs. Index role buffers stay host-side.
   */
  using MPICudaExecution = Composed<MPIDistribution, CudaMemory>;

  /**
   * Run hybrid MPI/threads-based distribution with GPU support.
   *
   * The Data buffers for a rank live on the device. If `Config::devices` has been
   * used (as opposed to just `device`), the ranks for a node are spread out between
   * the selected GPUs. Selection is done rank-first then threads - for more details
   * refer to the implmentation of `MPIThreadedDistribution::unit_ordinal`.
   */
  using MPIThreadedCudaExecution = Composed<MPIThreadedDistribution, CudaMemory>;
} // namespace sbio

#endif // SBIO_EXECUTION_MPI_CUDA_HH
