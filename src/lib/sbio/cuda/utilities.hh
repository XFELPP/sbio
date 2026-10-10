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

#ifndef SBIO_CUDA_UTILITIES_HH
#define SBIO_CUDA_UTILITIES_HH

#include "sbio/core/io.hh"

#include <cuda_runtime.h>
#include <cufile.h>

#include <iostream>

namespace sbio {
  /**
   * @def CHECK_CUDA_ERROR(val)
   * @brief Check the return value of the last CUDA host API call.
   * This will print the associated error message, if applicable, and aborts
   * if not successful.
   */
#define CHECK_CUDA_ERROR(val) check((val), #val, __FILE__, __LINE__)
  inline void check(cudaError_t err, const char* const func, const char* const file,
                    const int line) {
    if (err != cudaSuccess) {
      std::cerr << "CUDA Runtime Error at: " << file << ":" << line << std::endl;
      std::cerr << cudaGetErrorString(err) << " " << func << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  /**
   * @def CHECK_LAST_CUDA_ERROR()
   * @brief Check the last registered CUDA error.
   * This will interogate the result of the last API call to have been used.
   * If there was an error, this will print the associated error message, and abort.
   */
#define CHECK_LAST_CUDA_ERROR() checkLast(__FILE__, __LINE__)
  inline void checkLast(const char* const file, const int line) {
    cudaError_t const err{cudaGetLastError()};

    if (err != cudaSuccess) {
      std::cerr << "CUDA Runtime Error at: " << file << ":" << line << std::endl;
      std::cerr << cudaGetErrorString(err) << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  /**
   * A non-aborting check for CUDA errors given a status value from cuFile.
   *
   * @note The macro IS_CUDA_ERR is defined in the cuFile APIs.
   *
   * @param[in] status The returned status from the cuFile API.
   * @returns  0 if the status was not an error
   *          -1 if it could figure out the error and printed the message
   *          -2 if it was an unrecognized error.
   */
  int checkCuda(CUfileError_t status) {
    if (IS_CUDA_ERR(status)) {
      const char* str;
      CUresult strResult { cuGetErrorString(status.cu_err, &str) };

      if (strResult == CUDA_SUCCESS) {
        std::cerr << "CUDA ERROR: " << str << std::endl;
        return -1;
      } else {
        return -2;
      }
    }
    return 0;
  }

  /**
   * A non-aborting check for cuFile errors given a status value from cuFile.
   *
   * This checks cuFile errors first, and then will check for associated CUDA
   * runtime errors that may have occurred while executing cuFile calls. The return
   * value thus contains information about both direct cuFile errors and runtime
   * errors.
   *
   * @note The macros IS_CUFILE_ERR and CUFILE_ERRSTR are defined in the cuFile APIs.
   *
   * @param[in] status The returned status from the cuFile API.
   * @returns  0 if the status was not an error
   *          -1 if the was an interpretable runtime error and it printed the message
   *          -2 if there was an interpretable runtime error
   *          -3 if there was a cuFile error
   *          -4 if there was a cuFile error and interpretable runtime error
   *          -5 if there was a cuFile error and uninterpretable runtime error
   */
  int checkCuFile(CUfileError_t status) {
    if (IS_CUFILE_ERR(status.err)) {
      int cufile_err { -3 };

      std::cout << "cuFile ERROR "
                << status.err << ": " << CUFILE_ERRSTR(status.err) << std::endl;

      int cuda_err { 0 };
      if (status.err == 5011) {
        cuda_err = checkCuda(status);
      }

      return cufile_err + cuda_err;
    }

    return checkCuda(status);
  }
} // namespace sbio

#endif // SBIO_CUDA_UTILITIES_HH
