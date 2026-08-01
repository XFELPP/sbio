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
#include "sbio/core/stream.hh"
#include "sbio/core/sync.hh"
#include "sbio/core/utility.hh"
#include "sbio/formats/format_traits.hh"

#include <ncarray/ncarrays.hh>
#include <ncarray/soarrays.hh>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

namespace sbio {
  /**
   * The logical grouping of StreamBrokers which should be queried together.
   *
   * While the StreamBroker represents the smallest *logical* unit that makes
   * sense to query data for, the BrokerGroup represents the combination of
   * various StreamBrokers. Frequently, it makes more sense at higher levels of
   * abstraction to consider a group of StreamBrokers together as a single unit.
   * For example, each StreamBroker may be managing only a portion of a larger
   * data construct (a part of an image, e.g.). The BrokerGroup then provides
   * a single interface to query all logically related StreamBrokers simulatneously.
   * Note that a StreamBroker may belong to multiple BrokerGroups.
   *
   * @tparam BrokerType The type of the underlying StreamBroker in the group.
   * @tparam FTraits The data format being read.
   * @tparam MaxSegments The maximum number of members in the group.
   */
  template <
    IsStreamBroker BrokerType,
    FormatTraits<
      typename BrokerType::IOPolicy, typename BrokerType::ExecutionPolicy
    > FTraits,
    std::size_t MaxSegments = 128
  >
  class BrokerGroup {
  public:
    /**
     * The type of the IO strategy being used.
     */
    using IOPolicy = typename BrokerType::IOPolicy;
    /**
     * The Execution policy type.
     */
    using ExecutionPolicy = typename BrokerType::ExecutionPolicy;
    /**
     * The type of data being read.
     */
    using DataFormat = FTraits;

    /**
     * The type of Stream: I.e., the IO strategy and data format being read.
     */
    using StreamType = Stream<IOPolicy, FTraits>;

    /**
     * The type of the StreamBroker's Storage.
     */
    using SBStorageType = Storage<
      typename FTraits::BrokerBufferRequirements,
      ExecutionPolicy
    >;

    /**
     * The Execution policy configuration object type.
     *
     * `epolicy_config` objects configure the global behaviour of the Execution policy
     * being used. The Execution policy must be configured before any Streams are
     * opened as it controls all aspects of IO down to the allocation of Storage.
     */
    using EPolicyConfig = typename ExecutionPolicy::Config;
    /**
     * The individual Stream configuration object type.
     *
     * `stream_config` objects are used to set up each individual Stream so that
     * it can connect and read from its individual data.
     */
    using StreamConfig = typename FTraits::StreamParameters;
    /**
     * The DataSource configuration object type.
     *
     * `ds_config` objects are used for initial discovery and connection of the
     * full set of Streams.
     */
    using DSConfig = typename FTraits::DataSourceParameters;

    /**
     * The type of state tracking object for the data format's Stream.
     */
    using StreamState = typename FTraits::DiscoveryState;
    /**
     * The type of the general metadata object for the data format's Stream.
     */
    using StreamMetadata = typename FTraits::MetadataInventory;

    /**
     * The type of the enumerator used to specify access patterns used for the format.
     */
    using DataAccessPtn = typename FTraits::DataAccessPtn;
    /**
     * The type of a request object used to query for data.
     */
    using DataRequest = typename FTraits::DataRequest;
    /**
     * The type of a result object received as a response when querying for data.
     */
    using DataResult = typename FTraits::DataResult;
    /**
     * The type used to request a specific step from the Stream.
     *
     * This type is required and guaranteed to be convertible std::size_t; however,
     * different data format's may use different underlying types.
     */
    using StepIdxType = typename FTraits::StepIdxType;

    using PtrTableRequirements =
      TypeList<BufferDescriptor<TableRole, 0, sizeof(void*)>>;
    using PtrStorageType = Storage<PtrTableRequirements, ExecutionPolicy>;

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
    BrokerGroup() {
      m_name[0] = '\0';
      m_type[0] = '\0';
    }

    BrokerGroup(const char* name,
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

      std::size_t max_batch_count { 1 };
      if (m_num_brokers > 0 && m_segments[0].broker != nullptr) {
        max_batch_count = FTraits::max_batch_count(m_segments[0].broker->config());
      }

      m_ptr_storage =
        ExecutionPolicy::template allocate_group_storage<IOPolicy, FTraits>(m_num_segments,
                                                                            max_batch_count);
    }

    const char* group_name() const { return m_name; }
    const char* group_type() const { return m_type; }

    inline std::size_t num_segments() const { return m_num_segments; }
    inline const DataSegmentRef* segments() const { return m_segments; }
    inline const DataSegmentRef& segment(std::size_t i) const { return m_segments[i]; }

    inline BrokerType** stream_brokers() { return m_stream_brokers; }
    inline std::size_t num_stream_brokers() const { return m_num_brokers; }

    inline IOStatus fetch_next_for(StepIdxType& step_idx, std::size_t broker_no) const {
      auto* stream_broker = m_stream_brokers[broker_no];
      auto& access_ptn = m_access_ptns[broker_no];

      return stream_broker->fetch_step(step_idx, access_ptn);
    }

    inline IOStatus fetch_steps_for(std::initializer_list<StepIdxType> steps,
                                    std::size_t broker_no) const {
      auto* stream_broker = m_stream_brokers[broker_no];
      auto& access_ptn = m_access_ptns[broker_no];

      return stream_broker->fetch_steps(steps, access_ptn);
    }

    template <class CBType>
    inline DataResult get_data_for(DataRequest& req,
                                   std::size_t segment_no,
                                   CBType&& callback,
                                   std::size_t batch_idx = 0) const {
      req.segment_number = m_segments[segment_no].segment_no;

      auto* stream_broker = m_segments[segment_no].broker;
      auto& access_ptn = m_segments[segment_no].access_ptn;

      auto res = stream_broker->get_data_in_buffer(req, access_ptn, batch_idx);

      if constexpr (requires { callback(res, segment_no); }) {
        callback(res, segment_no);
      } else if constexpr (requires { callback(res); }) {
        callback(res);
      }
      return res;
    }

    inline DataResult get_data_for(DataRequest& req,
                                   std::size_t segment_no,
                                   std::size_t batch_idx = 0) const {
      req.segment_number = m_segments[segment_no].segment_no;

      auto* stream_broker = m_segments[segment_no].broker;
      auto& access_ptn = m_segments[segment_no].access_ptn;

      auto res = stream_broker->get_data_in_buffer(req, access_ptn, batch_idx);
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
    inline ncarray::SOViewFor<MemTag> get_data(StepIdxType& step_idx,
                                               CBType&& callback,
                                               Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      // Save a result reference to capture the data in the lambdas
      DataResult ref_res;

      auto& ptr_buf = this->m_ptr_storage.template get<TableRole>();
      const void** ptr_tbl = reinterpret_cast<const void**>(ptr_buf.ptr());

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

          ptr_tbl[0] = const_cast<void*>(res.data);
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
          // m_ptrs[req.segment_number] = const_cast<void*>(res.data);
          ptr_tbl[req.segment_number] = const_cast<void*>(res.data);
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

      ExecutionPolicy::template get_data<FTraits>(step_idx,
                                                  read_cb,
                                                  num_brokers,
                                                  get_data_cb,
                                                  num_segments);

      const void** ptr_table { const_cast<const void**>(ptr_tbl) };

      return as_ncarray<MemTag>(ptr_table, num_segments, ref_res);
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
    inline ncarray::SOViewFor<MemTag> get_data(StepIdxType& step_idx,
                                               Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      // Save a result reference to capture the data in the lambdas
      DataResult ref_res;

      auto& ptr_buf = this->m_ptr_storage.template get<TableRole>();
      const void** ptr_tbl = reinterpret_cast<const void**>(ptr_buf.ptr());

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

          ptr_tbl[0] = const_cast<void*>(res.data);
          //  This assumes all segments are same shape...
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
          ptr_tbl[req.segment_number] = const_cast<void*>(res.data);
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

      ExecutionPolicy::template get_data<FTraits>(step_idx,
                                                  read_cb,
                                                  num_brokers,
                                                  get_data_cb,
                                                  num_segments);

      return as_ncarray<MemTag>(ptr_tbl, num_segments, ref_res);
    }

    template <typename MemTag = ncarray::HostTag, class CBType, typename... Args>
    requires std::invocable<CBType, DataResult>
    inline ncarray::SOViewFor<MemTag> get_multi_data(const std::initializer_list<StepIdxType>& steps,
                                                     CBType&& callback,
                                                     Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      bool passed_step { steps.size() == 3 };
      StepIdxType first { *steps.begin() };
      StepIdxType last { passed_step ? *(steps.end() - 2) : *(steps.end() - 1) };
      std::size_t count { (last > first) ? (last - first) : 1 };

      // Save a result reference to capture the data in the lambdas
      DataResult ref_res;

      auto& ptr_buf = this->m_ptr_storage.template get<TableRole>();
      const void** ptr_tbl = reinterpret_cast<const void**>(ptr_buf.ptr());

      auto read_cb = [&](std::size_t i) {
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          /// TODO: Setup Chronological
        } else {
          auto& access_ptn = m_access_ptns[i];
          return m_stream_brokers[i]->fetch_steps(steps, access_ptn);
        }
      };

      auto get_data_cb = [&](std::size_t i, std::size_t cnt) {
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          /// TODO: Setup Chronological
        } else {
          auto res = get_data_for(req,
                                  i,
                                  std::forward<CBType>(callback),
                                  cnt);

          std::size_t ptr_idx { cnt * m_num_segments + req.segment_number };
          ptr_tbl[ptr_idx] = const_cast<void*>(res.data);
          if (i == 0) {
            // This assumes all segments are same shape...
            ref_res.data = res.data;
            ref_res.size = res.size;
            ref_res.rank = res.rank + 1;
            ref_res.shape[0] = count;
            // TODO: Consider ways to avoid copy....
            for (std::uint16_t i = 0; i < res.rank - 1; ++i) {
              ref_res.shape[i + 1] = res.shape[i];
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

      ExecutionPolicy::template get_data_steps<FTraits>(steps,
                                                        read_cb,
                                                        num_brokers,
                                                        get_data_cb,
                                                        num_segments);

      const void** ptr_table { const_cast<const void**>(ptr_tbl) };

      return as_ncarray<MemTag>(ptr_table, num_segments, ref_res, count);
    }

    template <typename MemTag = ncarray::HostTag, typename... Args>
    inline ncarray::SOViewFor<MemTag> get_multi_data(const std::initializer_list<StepIdxType>& steps,
                                                     Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      bool passed_step { steps.size() == 3 };
      StepIdxType first { *steps.begin() };
      StepIdxType last { passed_step ? *(steps.end() - 2) : *(steps.end() - 1) };
      std::size_t count { (last > first) ? (last - first) : 1 };

      // Save a result reference to capture the data in the lambdas
      DataResult ref_res;

      auto& ptr_buf = this->m_ptr_storage.template get<TableRole>();
      const void** ptr_tbl = reinterpret_cast<const void**>(ptr_buf.ptr());

      auto read_cb = [&](std::size_t i) {
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          // TODO: Setup Chronological
        } else {
          auto& access_ptn = m_access_ptns[i];
          return m_stream_brokers[i]->fetch_steps(steps, access_ptn);
        }
      };

      auto get_data_cb = [&](std::size_t i, std::size_t cnt) {
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          // TODO: Setup Chronological
        } else {
          auto res = get_data_for(req, i, cnt);

          std::size_t ptr_idx { cnt * m_num_segments + req.segment_number };
          ptr_tbl[ptr_idx] = const_cast<void*>(res.data);
          if (i == 0 && cnt == 0) {
            // This assumes all segments are same shape...
            ref_res.data = res.data;
            ref_res.size = res.size;
            ref_res.rank = res.rank;
            ref_res.shape[0] = count;
            // TODO: Consider ways to avoid copy....
            for (std::uint16_t i = 0; i < res.rank; ++i) {
              ref_res.shape[i + 1] = res.shape[i];
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

      ExecutionPolicy::template get_data_steps<FTraits>(steps,
                                                        read_cb,
                                                        num_brokers,
                                                        get_data_cb,
                                                        num_segments);

      return as_ncarray<MemTag>(ptr_tbl, num_segments, ref_res, count);
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

    mutable PtrStorageType m_ptr_storage;
  };
} // namespace sbio

#endif // SBIO_CORE_BROKER_GROUP_HH
