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

#ifndef SBIO_CORE_BROKER_GROUP_HH
#define SBIO_CORE_BROKER_GROUP_HH

#include "sbio/core/broker.hh"
#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/utility.hh"
#include "sbio/formats/format_traits.hh"

#include "ncarray/ncarrays.hh"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  template <
    class BrokerType,
    FormatTraits FTraits,
    std::size_t MaxSegments = 128
  >
  class BrokerGroup {
  public:
    using DataAccessPtn = typename FTraits::DataAccessPtn;
    using DataResult = typename FTraits::DataResult;
    using DataRequest = typename FTraits::DataRequest;
    using StepIdxType = typename FTraits::StepIdxType;

    /**
     * A reference to a specific piece of data, a numerical identifier, and pointer to its broker.
     *
     * The access_ptn enumerator is determined by the file format traits, and indicates
     * which kind of data it is.
     */
    struct DataSegmentRef {
      BrokerType* broker;
      std::uint32_t segment_no;
      DataAccessPtn access_ptn;
    };

    // Default constructor for DataSource abstraction
    SBIO_HD BrokerGroup() {
      m_name[0] = '\0';
      m_type[0] = '\0';
    }

    SBIO_HD BrokerGroup(const char* name,
                        const char* type,
                        std::size_t num_segments,
                        DataSegmentRef* segments) {
      m_num_segments = num_segments;

      std::size_t i { 0 };
      for (; i < FTraits::MaxNameSize - 1 && name[i] != '\0'; ++i) {
        m_name[i] = name[i];
      }
      m_name[i] = '\0';

      i = 0;
      for (; i < FTraits::MaxNameSize - 1 && type[i] != '\0'; ++i) {
        m_type[i] = type[i];
      }
      m_type[i] = '\0';

      for (std::size_t n_seg = 0; n_seg < m_num_segments; ++n_seg) {
        m_segments[n_seg] = segments[n_seg];

        auto* broker = m_segments[n_seg].broker;
        auto access_ptn = m_segments[n_seg].access_ptn;

        // Also store the broker pointer directly
        std::size_t n_broker { 0 };
        for (; n_broker < m_num_brokers; ++n_broker) {
          if (m_stream_brokers[n_broker] == broker) {
            break;
          }
        }
        if (n_broker == m_num_brokers) {
          m_stream_brokers[n_broker] = broker;
          m_access_ptns[n_broker] = access_ptn;
          m_num_brokers++;
        }
      }
    }

    SBIO_HD const char* group_name() const { return m_name; }
    SBIO_HD const char* group_type() const { return m_type; }

    SBIO_HD inline std::size_t num_segments() const { return m_num_segments; }
    SBIO_HD inline const DataSegmentRef* segments() const { return m_segments; }
    SBIO_HD inline const DataSegmentRef& segment(std::size_t i) const {
      return m_segments[i];
    }

    SBIO_HD inline BrokerType** stream_brokers() { return m_stream_brokers; }
    SBIO_HD inline std::size_t num_stream_brokers() const { return m_num_brokers; }

    SBIO_HD inline IOStatus fetch_next_for(StepIdxType& step_idx,
                                           std::size_t broker_no) const {
      auto* stream_broker = m_stream_brokers[broker_no];
      auto& access_ptn = m_access_ptns[broker_no];

      return stream_broker->fetch_step(step_idx, access_ptn);
    }

    template <class CBType>
    SBIO_HD inline DataResult get_data_for(DataRequest& req,
                                           std::size_t segment_no,
                                           CBType&& callback) const {
      req.segment_number = m_segments[segment_no].segment_no;

      auto* stream_broker = m_segments[segment_no].broker;
      auto& access_ptn = m_segments[segment_no].access_ptn;

      auto res = stream_broker->get_data_in_buffer(req, access_ptn);
      callback(res);
      return res;
    }

    SBIO_HD inline DataResult get_data_for(DataRequest& req,
                                           std::size_t segment_no) const {
      req.segment_number = m_segments[segment_no].segment_no;

      auto* stream_broker = m_segments[segment_no].broker;
      auto& access_ptn = m_segments[segment_no].access_ptn;

      auto res = stream_broker->get_data_in_buffer(req, access_ptn);
      return res;
    }

    /**
     * Fetch data for a given index AND read into the fetched data to pull specific
     * data out.
     *
     * NOTE: We add the concept restricting the CBType to invocable to avoid overload
     * ambiguities. This is likely not needed, as the partial ordering will treat the
     * Args pack as less-specialized, and prefer this overload. Adding it, though, does
     * not hurt and future-proofs against future overloads.
     *
     * @tparam MemTag The tag indicating host (CPU) or device (GPU) data access.
     * @tparam CBType The type for the lambda callback to apply to each data segment
     *         independently.
     * @tparam Args... The variadic types for arguments to pass to the DataRequest
     *         constructor. This set of arguments depends on the FormatTraits request
     *         constructors (of which, there are possibly multiple).
     * @param[in] step_idx The index for the data to fetch.
     * @param[in] callback Optionally provide a callback to be run on every segment
     *            independently before the data is combined.
     * @param[in] args The arguments for the DataRequest constructor.
     * @returns array The requested data as an NCArrayView or NCDevArrayView
     *          depending on whether MemTag is HostTag or DevTag, respectively.
     */
    template <typename MemTag = ncarray::HostTag, class CBType, typename... Args>
    requires std::invocable<CBType, DataResult>
    SBIO_HD inline ncarray::NCViewFor<MemTag> get_data(StepIdxType& step_idx,
                                                       CBType&& callback,
                                                       Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      // Save a result reference to capture the data in the lambdas
      DataResult ref_res;

      auto read_cb = [&](std::size_t i) {
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          auto active_stream_idx { step_idx % m_num_segments };
          auto adjusted_step_idx { step_idx / m_num_segments };
          auto& access_ptn = m_access_ptns[active_stream_idx];

          return m_stream_brokers[active_stream_idx]->fetch_step(adjusted_step_idx, access_ptn);
        } else {
          auto& access_ptn = m_access_ptns[i];
          return m_stream_brokers[i]->fetch_step(step_idx, access_ptn);
        }
      };

      auto get_data_cb = [&](std::size_t i) {
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          auto active_stream_idx { step_idx % m_num_segments };
          auto res = get_data_for(req,
                                  active_stream_idx,
                                  std::forward<CBType>(callback));

          m_ptrs[0] = const_cast<void*>(res.data);
          // This assumes all segments are same shape...
          ref_res.data = res.data;
          ref_res.size = res.size;
          ref_res.rank = res.rank;
          // TODO: Consider ways to avoid copy....
          for (std::uint16_t i = 0; i < res.rank; ++i) {
            ref_res.shape[i] = res.shape[i];
          }
          ref_res.dtype = res.dtype;
        } else {
          auto res = get_data_for(req,
                                  i,
                                  std::forward<CBType>(callback));
          // NOTE: The order of the segments may not be the "physical" order
          // -> Use the segment number from the request to populate m_ptrs
          // -> This way segment 0 gets put into m_ptrs[0]
          m_ptrs[req.segment_number] = const_cast<void*>(res.data);
          if (i == 0) {
            // This assumes all segments are same shape...
            ref_res.data = res.data;
            ref_res.size = res.size;
            ref_res.rank = res.rank;
            // TODO: Consider ways to avoid copy....
            for (std::uint16_t i = 0; i < res.rank; ++i) {
              ref_res.shape[i] = res.shape[i];
            }
            ref_res.dtype = res.dtype;
          }
        }
      };

      std::size_t num_brokers { m_num_brokers };
      std::size_t num_segments { m_num_segments };

      if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
        num_brokers = 1;
        num_segments = 1;
      }

      BrokerType::ExecutionPolicy::template get_data<FTraits>(step_idx,
                                                              read_cb,
                                                              num_brokers,
                                                              get_data_cb,
                                                              num_segments);

      return as_ncarray<MemTag>(m_ptrs, num_segments, ref_res);
    }

    /**
     * Fetch data for a given index AND read into the fetched data to pull specific
     * data out.
     *
     * This is an identical get_data call to the other overload, without the application
     * of a lambda callback.
     *
     * @tparam MemTag The tag indicating host (CPU) or device (GPU) data access.
     * @tparam Args... The variadic types for arguments to pass to the DataRequest
     *         constructor. This set of arguments depends on the FormatTraits request
     *         constructors (of which, there are possibly multiple).
     * @param[in] step_idx The index for the data to fetch.
     * @param[in] args The arguments for the DataRequest constructor.
     * @returns array The requested data as an NCArrayView or NCDevArrayView
     *          depending on whether MemTag is HostTag or DevTag, respectively.
     */
    template <typename MemTag = ncarray::HostTag, typename... Args>
    SBIO_HD inline ncarray::NCViewFor<MemTag> get_data(StepIdxType& step_idx,
                                                       Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      // Save a result reference to capture the data in the lambdas
      DataResult ref_res;

      auto read_cb = [&](std::size_t i) {
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          auto active_stream_idx { step_idx % m_num_segments };
          auto adjusted_step_idx { step_idx / m_num_segments };
          auto& access_ptn = m_access_ptns[active_stream_idx];

          return m_stream_brokers[active_stream_idx]->fetch_step(adjusted_step_idx, access_ptn);
        } else {
          auto& access_ptn = m_access_ptns[i];
          return m_stream_brokers[i]->fetch_step(step_idx, access_ptn);
        }
      };

      auto get_data_cb = [&](std::size_t i) {
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          auto active_stream_idx { step_idx % m_num_segments };
          auto res = get_data_for(req, active_stream_idx);

          m_ptrs[0] = const_cast<void*>(res.data);
          // This assumes all segments are same shape...
          ref_res.data = res.data;
          ref_res.size = res.size;
          ref_res.rank = res.rank;
          // TODO: Consider ways to avoid copy....
          for (std::uint16_t i = 0; i < res.rank; ++i) {
            ref_res.shape[i] = res.shape[i];
          }
          ref_res.dtype = res.dtype;
        } else {
          auto res = get_data_for(req, i);
          // NOTE: The order of the segments may not be the "physical" order
          // -> Use the segment number from the request to populate m_ptrs
          // -> This way segment 0 gets put into m_ptrs[0]
          m_ptrs[req.segment_number] = const_cast<void*>(res.data);
          if (i == 0) {
            // This assumes all segments are same shape...
            ref_res.data = res.data;
            ref_res.size = res.size;
            ref_res.rank = res.rank;
            // TODO: Consider ways to avoid copy....
            for (std::uint16_t i = 0; i < res.rank; ++i) {
              ref_res.shape[i] = res.shape[i];
            }
            ref_res.dtype = res.dtype;
          }
        }
      };

      std::size_t num_brokers { m_num_brokers };
      std::size_t num_segments { m_num_segments };

      if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
        num_brokers = 1;
        num_segments = 1;
      }

      BrokerType::ExecutionPolicy::template get_data<FTraits>(step_idx,
                                                              read_cb,
                                                              num_brokers,
                                                              get_data_cb,
                                                              num_segments);

      return as_ncarray<MemTag>(m_ptrs, num_segments, ref_res);
    }

  private:
    char m_name[FTraits::MaxNameSize];
    char m_type[FTraits::MaxNameSize];
    DataSegmentRef m_segments[MaxSegments];
    mutable std::size_t m_num_segments;

    BrokerType* m_stream_brokers[MaxSegments];
    DataAccessPtn m_access_ptns[MaxSegments];
    mutable std::size_t m_num_brokers { 0 };

    mutable const void* m_ptrs[MaxSegments]; // Final coalesced reads will be left here.
  };
} // namespace sbio

#endif // SBIO_CORE_BROKER_GROUP_HH
