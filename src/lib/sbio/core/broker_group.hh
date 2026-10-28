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
#include "sbio/core/metadata.hh"
#include "sbio/core/result.hh"
#include "sbio/core/roles.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/stream.hh"
#include "sbio/core/sync.hh"
#include "sbio/core/topology.hh"
#include "sbio/core/types.hh"
#include "sbio/formats/format_traits.hh"

#include <ncarray/ncarrays.hh>
#include <ncarray/soarrays.hh>

#include <array>
#include <cassert>
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
     * The type of the general metadata object for the data format's Stream.
     */
    using StreamMetadata = MetadataInventory<FTraits>;

    /**
     * The type of the enumerator used to specify access patterns used for the format.
     */
    using DataAccessPtn = typename FTraits::DataAccessPtn;
    /**
     * The maximum number of segments (and therefore StreamBrokers) in the BrokerGroup.
     */
    static constexpr std::size_t SegmentCapacity { MaxSegments };
    /**
     * The type of a request object used to query for data.
     */
    using DataRequest = typename FTraits::DataRequest;
    /**
     * The type used to request a specific step from the Stream.
     *
     * This type is required and guaranteed to be convertible std::size_t; however,
     * different data format's may use different underlying types.
     */
    using StepIdxType = typename FTraits::StepIdxType;

    using PtrTableRequirements =
      RequirementsList<BufferDescriptor<roles::Table, 0, sizeof(void*)>>;
    using PtrStorageType = Storage<PtrTableRequirements, ExecutionPolicy>;

    /**
     * Based on choice of ExecutionPolicy, data will return in host or device buffers.
     *
     * As data is returned via ncarray array objects, the MemTag converts a MemorySpace
     * indicator in sbio to the tagging system used for host/device array constructs in
     * ncarray.
     */
    using MemTag = std::conditional_t<
      ExecutionPolicy::result_memory_space() == MemorySpace::Host,
      ncarray::HostTag,
      ncarray::DevTag
    >;

    using DataSegmentRef = SegmentRef<BrokerType>;

    // Default constructor for DataSource abstraction
    BrokerGroup() = default;

    BrokerGroup(const char* name,
                const char* type,
                std::size_t num_segments,
                DataSegmentRef* segments) {
      m_topology.num_segments = num_segments;

      safe_strncpy(m_topology.group_name, name, 256);
      safe_strncpy(m_topology.group_type, type, 256);

      std::uint32_t final_segment_indices[MaxSegments] {};
      if constexpr (FTraits::PartitioningStrategy != StreamPartitioningStrategy::Chronological) {
        if (num_segments > 0) {
          // Chronological streams are provided time-sorted already
          // For the other case -- sort the indices by the "segment_no" to handle the
          // possibility of missing segments but maintain the segment numbering where it
          // is significant
          std::uint32_t smallest { segments[0].format_segment_id };
          for (std::size_t j = 0; j < num_segments; ++j) {
            final_segment_indices[j] = j;
            if (segments[j].format_segment_id < smallest) {
              smallest = segments[j].format_segment_id;
            }
          }

          for (std::size_t i = 0; i < num_segments - 1; ++i) {
            std::uint32_t best { static_cast<std::uint32_t>(i) };
            for (std::size_t j = i + 1; j < num_segments; ++j) {
              std::uint32_t diff_best { segments[final_segment_indices[best]].format_segment_id - smallest };
              std::uint32_t diff_j { segments[final_segment_indices[j]].format_segment_id - smallest };

              if (diff_j < diff_best) {
                best = j;
              }
            }

            if (best != i) {
              std::uint32_t tmp { final_segment_indices[i] };
              final_segment_indices[i] = final_segment_indices[best];
              final_segment_indices[best] = tmp;
            }
          }
        }
      } else {
        for (std::size_t i = 0; i < num_segments; ++i) {
          final_segment_indices[i] = i;
        }
      }

      for (std::size_t n_seg = 0; n_seg < m_topology.num_segments; ++n_seg) {
        std::uint32_t working_idx { final_segment_indices[n_seg] };
        m_topology.set_segment(n_seg, segments[working_idx]);

        auto* broker { m_topology.broker_for_segment(n_seg) };
        auto& access_ptn { m_topology.pattern_for_segment(n_seg) };

        // Also store the broker pointer directly
        std::size_t n_broker { 0 };
        for (; n_broker < m_topology.num_stream_brokers; ++n_broker) {
          if (m_topology.stream_broker(n_broker) == broker) {
            break;
          }
        }
        if (n_broker == m_topology.num_stream_brokers) {
          m_topology.set_stream_broker(n_broker, broker, access_ptn);
        }
      }

      m_topology.count_logical_segments();

      std::size_t max_batch_count { 1 };
      if (!m_topology.empty() && m_topology.broker_for_segment(0) != nullptr) {
        max_batch_count = m_topology.broker_for_segment(0)->config().max_batch_size;
      }

      m_ptr_storage =
        ExecutionPolicy::template allocate_group_storage<IOPolicy, FTraits>(this->num_segments(),
                                                                            max_batch_count);
    }

    BrokerGroup(const GroupTopology<BrokerType, MaxSegments>& topo)
      : m_topology(topo)
    {

      std::size_t max_batch_count { 1 };
      if (!m_topology.empty() && m_topology.broker_for_segment(0) != nullptr) {
        max_batch_count = m_topology.broker_for_segment(0)->config().max_batch_size;
      }

      m_ptr_storage =
        ExecutionPolicy::template allocate_group_storage<IOPolicy, FTraits>(this->num_segments(),
                                                                            max_batch_count);
    }

    /**
     * Configure and set the step remapping policy for the managed SegmentRefs.
     *
     * @param[in] mapping The mapping policy to use.
     * @param[in] align_to The pattern to align to if using an aligning policy.
     */
    inline void set_step_mapping(StepMapping mapping, DataAccessPtn align_to = {}) {
      m_topology.step_mapping = mapping;
      m_topology.align_ptn = align_to;
    }

    const char* group_name() const { return m_topology.group_name; }
    const char* group_type() const { return m_topology.group_type; }

    /**
     * The number of (logical) segments of the group.
     *
     * @note For Chronological formats, every StreamBroker holds each segment.
     */
    inline std::size_t num_segments() const { return m_topology.num_logical_segments; }
    inline const DataSegmentRef* segments() const {
      if (!m_topology.empty()) {
        return &m_topology.segments[0];
      }

      return nullptr;
    }
    inline const DataSegmentRef& segment(std::size_t i) const {
      return m_topology.segments(i);
    }

    inline BrokerType** stream_brokers() {
      if (!m_topology.empty()) {
        return &m_topology.stream_brokers[0];
      }

      return nullptr;
    }
    SBIO_HD inline std::size_t num_stream_brokers() const { return m_topology.num_stream_brokers; }

    SBIO_HD inline IOStatus fetch_next_for(StepIdxType& step_idx, std::size_t broker_no) const {
      if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
        return chrono_fetch(step_idx, broker_no);
      }

      const auto& seg { m_topology.active_segment(step_idx, broker_no) };
      auto remapped_step_idx { m_topology.remap_step_idx(step_idx) };

      std::size_t ordinal { 0 };
      const auto status { m_topology.map_step(seg, remapped_step_idx, ordinal) };
      if (status != IOStatus::Success) {
        return status;
      }

      return seg.broker->fetch_step(ordinal, seg.access_ptn, seg.cursor.get());
    }

    /**
     * Fetch a batch of the group's steps for a StreamBroker `broker_no`.
     *
     * @note When using `StepMapping::AlignTo`, a StreamBroker of another access
     *       pattern fetches the range that must be mapped onto. Then the alignment
     *       is done.
     *
     * @param[in] steps The batch of steps [first, last)
     * @param[in] broker_no The StreamBroker to fetch for.
     * @returns The IOStatus for the fetch.
     */
    SBIO_HD inline IOStatus fetch_steps_for(std::initializer_list<StepIdxType> steps,
                                            std::size_t broker_no) const {
      if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
        const bool passed_step { steps.size() == 3 };
        const StepIdxType first { *steps.begin() };
        const StepIdxType last { passed_step ? *(steps.end() - 2) : *(steps.end() - 1) };

        ChronoBatch batch;
        const auto status { chrono_batch(first, last, batch) };
        if (status != IOStatus::Success) {
          return status;
        }
        return chrono_fetch_batch(batch, broker_no);
      }

      const auto& seg { m_topology.active_segment(*steps.begin(), broker_no) };
      if (!is_aligned(seg)) {
        return seg.broker->fetch_steps(steps, seg.access_ptn, seg.cursor.get());
      }

      // Otherwise we have to do the step mapping/alignment
      const bool passed_step { steps.size() == 3 };
      const StepIdxType first { *steps.begin() };
      const StepIdxType last { passed_step ? *(steps.end() - 2) : *(steps.end() - 1) };

      std::size_t first_ordinal { 0 };
      std::size_t end_ordinal { 0 };
      const auto status { map_batch(seg, first, last, first_ordinal, end_ordinal) };
      if (status != IOStatus::Success) {
        return status;
      }

      return seg.broker->fetch_steps({ static_cast<StepIdxType>(first_ordinal),
                                       static_cast<StepIdxType>(end_ordinal) },
                                     seg.access_ptn,
                                     seg.cursor.get());
    }

    template <class CBType>
    SBIO_HD inline DataResult get_data_for(DataRequest& req,
                                           std::size_t segment_no,
                                           CBType&& callback,
                                           std::size_t batch_idx = 0) const {
      req.segment_number = m_topology.segment(segment_no).format_segment_id;

      auto* stream_broker { m_topology.broker_for_segment(segment_no) };
      const auto& access_ptn { m_topology.pattern_for_segment(segment_no) };
      auto& cursor { m_topology.segment(segment_no).cursor.get() };

      auto res = stream_broker->get_data_in_buffer(req, access_ptn, cursor, batch_idx);

      if constexpr (requires { callback(res, segment_no); }) {
        callback(res, segment_no);
      } else if constexpr (requires { callback(res); }) {
        callback(res);
      }
      return res;
    }

    SBIO_HD inline DataResult get_data_for(DataRequest& req,
                                           std::size_t segment_no,
                                           std::size_t batch_idx = 0) const {
      req.segment_number = m_topology.segment(segment_no).format_segment_id;

      auto* stream_broker { m_topology.broker_for_segment(segment_no) };
      auto& access_ptn { m_topology.pattern_for_segment(segment_no) };
      auto& cursor { m_topology.segment(segment_no).cursor.get() };

      auto res = stream_broker->get_data_in_buffer(req, access_ptn, cursor, batch_idx);
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
     * @tparam CBType The type for the lambda callback to apply to each data segment
     *         independently.
     * @tparam Args... The variadic types for arguments to pass to the DataRequest
     *         constructor. This set of arguments depends on the FormatTraits request
     *         constructors (of which, there are possibly multiple).
     * @param[in] step_idx The index for the data to fetch.
     * @param[in] callback Optionally provide a callback to be run on every segment
     *            independently before the data is combined.
     * @param[in] args The arguments for the DataRequest constructor.
     * @returns array The requested data as an SOArrayView or SODevArrayView
     *          depending on whether MemTag is HostTag or DevTag, respectively.
     */
    template <class CBType, typename... Args>
    requires std::invocable<CBType, DataResult>
    inline ncarray::SOViewFor<MemTag> get_data(StepIdxType& step_idx,
                                               CBType&& callback,
                                               Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      // Save a result reference to capture the data in the lambdas
      DataResult ref_res;

      auto& ptr_buf { this->m_ptr_storage.template get<roles::Table>() };
      const void** ptr_tbl { reinterpret_cast<const void**>(ptr_buf.ptr()) };

      auto read_cb = [&](std::size_t i, auto&&... opt_step) {
        auto target_idx { step_idx };
        if constexpr (sizeof...(opt_step) > 0) {
          target_idx = std::get<0>(std::forward_as_tuple(opt_step...));
        }

        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          return chrono_fetch(target_idx, i);
        }

        const auto& seg { m_topology.active_segment(target_idx, i) };
        auto remapped_step_idx { m_topology.remap_step_idx(target_idx) };

        std::size_t ordinal { 0 };
        const auto status { m_topology.map_step(seg, remapped_step_idx, ordinal) };
        if (status != IOStatus::Success) {
          return status;
        }

        return seg.broker->fetch_step(ordinal, seg.access_ptn, seg.cursor.get());
      };

      auto get_data_cb = [&](std::size_t i) {
        const auto& seg { m_topology.segment(i) };

        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          // `i` is the logical segment, in the StreamBroker holding the step
          std::size_t broker_no { 0 };
          std::size_t ordinal { 0 };
          const std::size_t seg_idx {
            m_topology.locate(step_idx, broker_no, ordinal)
              ? m_topology.segment_index_for(broker_no, i)
              : TopologyType::NoSegment
          };

          ptr_tbl[i] = nullptr;
          if (seg_idx != TopologyType::NoSegment) {
            auto res = get_data_for(req, seg_idx, std::forward<CBType>(callback));
            ptr_tbl[i] = const_cast<void*>(res.data);
            if (ref_res.data == nullptr) {
              // This assumes all segments are same shape...
              ref_res.data = res.data;
              ref_res.size = res.size;
              ref_res.rank = res.rank;
              // TODO: Consider ways to avoid copy....
              for (std::uint16_t j = 0; j < res.rank; ++j) {
                ref_res.shape[j] = res.shape[j];
              }
              ref_res.dtype = res.dtype;
            }
          }
        } else {
          auto res = get_data_for(req,
                                  i,
                                  std::forward<CBType>(callback));
          // NOTE: The order of the segments may not be the "physical" order
          // -> Use the segment number from the request to populate m_ptrs
          // -> This way segment 0 gets put into m_ptrs[0]
          // HOWEVER - we pre-sort the segments during construction. This avoids
          // problems if only "segment 2" is provided without 0 and 1, e.g.
          ptr_tbl[seg.logical_slot] = const_cast<void*>(res.data);
          if (i == 0) {
            // This assumes all segments are same shape...
            ref_res.data = res.data;
            ref_res.size = res.size;
            ref_res.rank = res.rank;
            // TODO: Consider ways to avoid copy....
            for (std::uint16_t j = 0; j < res.rank; ++j) {
              ref_res.shape[j] = res.shape[j];
            }
            ref_res.dtype = res.dtype;
          }
        }
      };

      // For Chronological formats, only the StreamBroker holding a step fetches it
      std::size_t num_brokers { this->num_stream_brokers() };
      std::size_t num_segments { this->num_segments() };

      const IOStatus status {
        ExecutionPolicy::template get_data<FTraits>(step_idx,
                                                    read_cb,
                                                    num_brokers,
                                                    get_data_cb,
                                                    num_segments)
      };

      const void** ptr_table { const_cast<const void**>(ptr_tbl) };

      CompositeDataResult<MemTag> composite {
        ptr_table,
        status == IOStatus::Success ? this->num_segments() : 0,
        1,
        ref_res.rank,
        ref_res.shape,
        ref_res.dtype
      };
      return composite.to_array();
    }

    /**
     * Fetch data for a given index AND read into the fetched data to pull specific
     * data out.
     *
     * This is an identical get_data call to the other overload, without the application
     * of a lambda callback.
     *
     * @tparam Args... The variadic types for arguments to pass to the DataRequest
     *         constructor. This set of arguments depends on the FormatTraits request
     *         constructors (of which, there are possibly multiple).
     * @param[in] step_idx The index for the data to fetch.
     * @param[in] args The arguments for the DataRequest constructor.
     * @returns array The requested data as an SOArrayView or SODevArrayView
     *          depending on whether MemTag is HostTag or DevTag, respectively.
     */
    template <typename... Args>
    inline ncarray::SOViewFor<MemTag> get_data(StepIdxType& step_idx,
                                               Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      // Save a result reference to capture the data in the lambdas
      DataResult ref_res;

      auto& ptr_buf { this->m_ptr_storage.template get<roles::Table>() };
      const void** ptr_tbl { reinterpret_cast<const void**>(ptr_buf.ptr()) };

      auto read_cb = [&](std::size_t i, auto&&... opt_step) {
        auto target_idx { step_idx };
        if constexpr (sizeof...(opt_step) > 0) {
          target_idx = std::get<0>(std::forward_as_tuple(opt_step...));
        }

        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          return chrono_fetch(target_idx, i);
        }

        const auto& seg { m_topology.active_segment(target_idx, i) };
        auto remapped_step_idx { m_topology.remap_step_idx(target_idx) };

        std::size_t ordinal { 0 };
        const auto status { m_topology.map_step(seg, remapped_step_idx, ordinal) };
        if (status != IOStatus::Success) {
          return status;
        }

        return seg.broker->fetch_step(ordinal, seg.access_ptn, seg.cursor.get());
      };

      auto get_data_cb = [&](std::size_t i) {
        const auto& seg { m_topology.segment(i) };

        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          // `i` is the logical segment, in the StreamBroker holding the step
          std::size_t broker_no { 0 };
          std::size_t ordinal { 0 };
          const std::size_t seg_idx {
            m_topology.locate(step_idx, broker_no, ordinal)
              ? m_topology.segment_index_for(broker_no, i)
              : TopologyType::NoSegment
          };

          ptr_tbl[i] = nullptr;
          if (seg_idx != TopologyType::NoSegment) {
            auto res = get_data_for(req, seg_idx);
            ptr_tbl[i] = const_cast<void*>(res.data);
            if (ref_res.data == nullptr) {
              //  This assumes all segments are same shape...
              ref_res.data = res.data;
              ref_res.size = res.size;
              ref_res.rank = res.rank;
              // TODO: Consider ways to avoid copy....
              for (std::uint16_t j = 0; j < res.rank; ++j) {
                ref_res.shape[j] = res.shape[j];
              }
              ref_res.dtype = res.dtype;
            }
          }
        } else {
          auto res = get_data_for(req, i);
          // NOTE: The order of the segments may not be the "physical" order
          // -> Use the segment number from the request to populate m_ptrs
          // -> This way segment 0 gets put into m_ptrs[0]
          // HOWEVER - we pre-sort the segments during construction. This avoids
          // problems if only "segment 2" is provided without 0 and 1, e.g.
          ptr_tbl[seg.logical_slot] = const_cast<void*>(res.data);
          if (i == 0) {
            // This assumes all segments are same shape...
            ref_res.data = res.data;
            ref_res.size = res.size;
            ref_res.rank = res.rank;
            // TODO: Consider ways to avoid copy....
            for (std::uint16_t j = 0; j < res.rank; ++j) {
              ref_res.shape[j] = res.shape[j];
            }
            ref_res.dtype = res.dtype;
          }
        }
      };

      // For Chronological formats, only the StreamBroker holding a step fetches it
      std::size_t num_brokers { this->num_stream_brokers() };
      std::size_t num_segments { this->num_segments() };

      const IOStatus status {
        ExecutionPolicy::template get_data<FTraits>(step_idx,
                                                    read_cb,
                                                    num_brokers,
                                                    get_data_cb,
                                                    num_segments)
      };

      CompositeDataResult<MemTag> composite {
        ptr_tbl,
        status == IOStatus::Success ? this->num_segments() : 0,
        1,
        ref_res.rank,
        ref_res.shape,
        ref_res.dtype
      };

      return composite.to_array();
    }

    /**
     * Resolve data for a step from the already fetched buffers, without any IO.
     *
     * This is the data resolution half of `get_data`. The step's data must already be
     * in the StreamBrokers' buffers, e.g. fetched with `fetch_next_for` (directly or via
     * a zipped iteration). As with `get_data`, the result is valid until the next fetch
     * into the same buffers.
     *
     * @tparam Args... The variadic types for arguments to pass to the DataRequest
     *         constructor. This set of arguments depends on the FormatTraits request
     *         constructors (of which, there are possibly multiple).
     * @param[in] step_idx The index of the (already fetched) step.
     * @param[in] args The arguments for the DataRequest constructor.
     * @returns array The requested data as an SOArrayView or SODevArrayView
     *          depending on whether MemTag is HostTag or DevTag, respectively.
     */
    template <typename... Args>
    SBIO_HD inline ncarray::SOViewFor<MemTag> resolve_data(const StepIdxType& step_idx,
                                                           Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      DataResult ref_res;

      auto& ptr_buf { this->m_ptr_storage.template get<roles::Table>() };
      const void** ptr_tbl { reinterpret_cast<const void**>(ptr_buf.ptr()) };

      auto copy_ref = [&](const DataResult& res) {
        // This assumes all segments are same shape...
        ref_res.data = res.data;
        ref_res.size = res.size;
        ref_res.rank = res.rank;
        for (std::uint16_t j = 0; j < res.rank; ++j) {
          ref_res.shape[j] = res.shape[j];
        }
        ref_res.dtype = res.dtype;
      };

      if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
        std::size_t broker_no { 0 };
        std::size_t ordinal { 0 };
        const bool located { m_topology.locate(step_idx, broker_no, ordinal) };

        for (std::size_t l = 0; l < this->num_segments(); ++l) {
          const std::size_t seg_idx {
            located ? m_topology.segment_index_for(broker_no, l) : TopologyType::NoSegment
          };

          ptr_tbl[l] = nullptr;
          if (seg_idx != TopologyType::NoSegment) {
            auto res = get_data_for(req, seg_idx);
            ptr_tbl[l] = const_cast<void*>(res.data);
            if (ref_res.data == nullptr) {
              copy_ref(res);
            }
          }
        }
      } else {
        for (std::size_t i = 0; i < this->num_segments(); ++i) {
          const auto& seg { m_topology.segment(i) };
          auto res = get_data_for(req, i);

          ptr_tbl[seg.logical_slot] = const_cast<void*>(res.data);
          if (i == 0) {
            copy_ref(res);
          }
        }
      }

      CompositeDataResult<MemTag> composite {
        ptr_tbl,
        this->num_segments(),
        1,
        ref_res.rank,
        ref_res.shape,
        ref_res.dtype
      };
      return composite.to_array();
    }

    /**
     * Whether a fetch for this group's StreamBroker `broker_no` and a fetch for the
     * other group's StreamBroker `other_no` fill the same buffer with the same step.
     *
     * This is the case when both use the same StreamBroker, with the same access
     * pattern and the same step mapping. A zipped iteration then only fetches once.
     *
     * @param[in] broker_no The StreamBroker of this group.
     * @param[in] other The other group.
     * @param[in] other_no The StreamBroker of the other group.
     * @returns Whether the two fetches are identical.
     */
    SBIO_HD inline bool shares_fetch(std::size_t broker_no,
                                     const BrokerGroup& other,
                                     std::size_t other_no) const {
      // For Chronological formats, the steps a StreamBroker fetches depend only on the
      // merged order (shared by the DataSource's groups) and the step mapping
      return
        m_topology.stream_brokers[broker_no] == other.m_topology.stream_brokers[other_no] &&
        m_topology.access_ptn(broker_no) == other.m_topology.access_ptn(other_no)         &&
        m_topology.step_mapping == other.m_topology.step_mapping                          &&
        m_topology.align_ptn == other.m_topology.align_ptn;
    }

    /**
     * Check if a fetch for `broker_no` fills the same buffer as other's `other_no`.
     *
     * @note This occurs when both BrokerGroups use the same StreamBroker and access
     *       pattern. If using different step mappings (see shares_fetch), they may
     *       need different steps in the buffer.
     *
     * @param[in] broker_no The StreamBroker broker number for this BrokerGroup.
     * @param[in] other The other BrokerGroup.
     * @param[in] other_no The StreamBroker broker number for the `other` BrokerGroup.
     * @returns Whether the fetches fill the same buffer.
     */
    SBIO_HD inline bool shares_buffer(std::size_t broker_no,
                                      const BrokerGroup& other,
                                      std::size_t other_no) const {
      return
        m_topology.stream_brokers[broker_no] == other.m_topology.stream_brokers[other_no] &&
        m_topology.access_ptn(broker_no) == other.m_topology.access_ptn(other_no);
    }

    /**
     * Get the range of steps for a StreamBroker that correspond to a group's range.
     *
     * @note If there is no step alignment occurring, then the two ranges are identical
     *       and this is a normal fetch. Otherwise, it is a fetch over the range that
     *       the steps map onto.
     *
     * @returns Whether the steps could be mapped.
     */
    SBIO_HD inline IOStatus fetch_range_for(StepIdxType first,
                                            StepIdxType last,
                                            std::size_t broker_no,
                                            std::size_t& first_ordinal,
                                            std::size_t& end_ordinal) const {
      if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
        ChronoBatch batch;
        const auto status { chrono_batch(first, last, batch) };
        if (status != IOStatus::Success) {
          return status;
        }

        return chrono_ordinals(batch, broker_no, first_ordinal, end_ordinal);
      }

      const auto& seg { m_topology.active_segment(first, broker_no) };
      if (is_aligned(seg)) {
        return map_batch(seg, first, last, first_ordinal, end_ordinal);
      }

      const StepIdxType last_step {
        (last > first) ? static_cast<StepIdxType>(last - 1) : first
      };
      first_ordinal = m_topology.remap_step_idx(first);
      end_ordinal = m_topology.remap_step_idx(last_step) + 1;

      return IOStatus::Success;
    }

    template <class CBType, typename... Args>
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

      auto& ptr_buf { this->m_ptr_storage.template get<roles::Table>() };
      const void** ptr_tbl { reinterpret_cast<const void**>(ptr_buf.ptr()) };
      assert(count * this->num_segments() * sizeof(void *) <= ptr_buf.size() &&
             "A requested batch cannot be larger than configured max_batch_size!");

      ChronoBatch chrono {};
      IOStatus chrono_status { IOStatus::Success };
      if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
        chrono_status = chrono_batch(first, last, chrono);
      }

      auto read_cb = [&](std::size_t i, auto&&... opt_step) {
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          return chrono_status == IOStatus::Success ? chrono_fetch_batch(chrono, i) : chrono_status;
        } else {
          return fetch_steps_for(steps, i);
        }
      };

      auto get_data_cb = [&](std::size_t i, std::size_t cnt) {
        const auto& seg { m_topology.segment(i) };
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          // `i` is the logical segment, in the StreamBroker holding the step
          std::size_t seg_idx { TopologyType::NoSegment };
          std::size_t slot { 0 };
          chrono_slot(chrono, static_cast<StepIdxType>(first + cnt), i, seg_idx, slot);

          const std::size_t ptr_idx { cnt * this->num_segments() + i };
          ptr_tbl[ptr_idx] = nullptr;
          if (seg_idx != TopologyType::NoSegment) {
            auto res = get_data_for(req, seg_idx, std::forward<CBType>(callback), slot);
            ptr_tbl[ptr_idx] = const_cast<void*>(res.data);
            if (ref_res.data == nullptr) {
              ref_res.data = res.data;
              ref_res.size = res.size;
              ref_res.rank = res.rank;
              for (std::uint16_t j = 0; j < res.rank; ++j) {
                ref_res.shape[j] = res.shape[j];
              }
              ref_res.dtype = res.dtype;
            }
          }
        } else {
          auto res = get_data_for(req,
                                  i,
                                  std::forward<CBType>(callback),
                                  batch_slot(i, first, cnt));

          // NOTE: The order of the segments may not be the "physical" order
          // -> Use the segment number from the request to populate m_ptrs
          // -> This way segment 0 gets put into m_ptrs[0]
          // HOWEVER - we pre-sort the segments during construction. This avoids
          // problems if only "segment 2" is provided without 0 and 1, e.g.
          std::size_t ptr_idx { cnt * this->num_segments() + seg.logical_slot };
          ptr_tbl[ptr_idx] = const_cast<void*>(res.data);
          if (i == 0 && cnt == 0) {
            // This assumes all segments are same shape...
            // The batch axis is added by the CompositeDataResult
            ref_res.data = res.data;
            ref_res.size = res.size;
            ref_res.rank = res.rank;
            // TODO: Consider ways to avoid copy....
            for (std::uint16_t j = 0; j < res.rank - 1; ++j) {
              ref_res.shape[j] = res.shape[j];
            }
            ref_res.dtype = res.dtype;
          }
        }
      };

      // For Chronological formats, only the StreamBroker holding a step fetches it
      std::size_t num_brokers { this->num_stream_brokers() };
      std::size_t num_segments { this->num_segments() };

      const IOStatus status {
        ExecutionPolicy::template get_data_steps<FTraits>(steps,
                                                          read_cb,
                                                          num_brokers,
                                                          get_data_cb,
                                                          num_segments)
      };

      const void** ptr_table { const_cast<const void**>(ptr_tbl) };

      CompositeDataResult<MemTag> composite {
        ptr_table,
        status == IOStatus::Success ? this->num_segments() : 0,
        count,
        ref_res.rank,
        ref_res.shape,
        ref_res.dtype,
        true
      };

      return composite.to_array();
    }

    template <typename... Args>
    inline ncarray::SOViewFor<MemTag> get_multi_data(const std::initializer_list<StepIdxType>& steps,
                                                     Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      bool passed_step { steps.size() == 3 };
      StepIdxType first { *steps.begin() };
      StepIdxType last { passed_step ? *(steps.end() - 2) : *(steps.end() - 1) };
      std::size_t count { (last > first) ? (last - first) : 1 };

      // Save a result reference to capture the data in the lambdas
      DataResult ref_res;

      auto& ptr_buf = this->m_ptr_storage.template get<roles::Table>();
      const void** ptr_tbl = reinterpret_cast<const void**>(ptr_buf.ptr());
      assert(count * this->num_segments() * sizeof(void *) <= ptr_buf.size() &&
             "A requested batch cannot be larger than configured max_batch_size!");

      ChronoBatch chrono {};
      IOStatus chrono_status { IOStatus::Success };
      if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
        chrono_status = chrono_batch(first, last, chrono);
      }

      auto read_cb = [&](std::size_t i, auto&&... opt_step) {
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          return chrono_status == IOStatus::Success ? chrono_fetch_batch(chrono, i) : chrono_status;
        } else {
          return fetch_steps_for(steps, i);
        }
      };

      auto get_data_cb = [&](std::size_t i, std::size_t cnt) {
        const auto& seg { m_topology.segment(i) };
        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          // `i` is the logical segment, in the StreamBroker holding the step
          std::size_t seg_idx { TopologyType::NoSegment };
          std::size_t slot { 0 };
          chrono_slot(chrono, static_cast<StepIdxType>(first + cnt), i, seg_idx, slot);

          const std::size_t ptr_idx { cnt * this->num_segments() + i };
          ptr_tbl[ptr_idx] = nullptr;
          if (seg_idx != TopologyType::NoSegment) {
            auto res = get_data_for(req, seg_idx, slot);
            ptr_tbl[ptr_idx] = const_cast<void*>(res.data);
            if (ref_res.data == nullptr) {
              ref_res.data = res.data;
              ref_res.size = res.size;
              ref_res.rank = res.rank;
              for (std::uint16_t j = 0; j < res.rank; ++j) {
                ref_res.shape[j] = res.shape[j];
              }
              ref_res.dtype = res.dtype;
            }
          }
        } else {
          auto res = get_data_for(req, i, batch_slot(i, first, cnt));

          // NOTE: The order of the segments may not be the "physical" order
          // -> Use the segment number from the request to populate m_ptrs
          // -> This way segment 0 gets put into m_ptrs[0]
          // HOWEVER - we pre-sort the segments during construction. This avoids
          // problems if only "segment 2" is provided without 0 and 1, e.g.
          std::size_t ptr_idx { cnt * this->num_segments() + seg.logical_slot };
          ptr_tbl[ptr_idx] = const_cast<void*>(res.data);
          if (i == 0 && cnt == 0) {
            // This assumes all segments are same shape...
            // The batch axis is added by the CompositeDataResult
            ref_res.data = res.data;
            ref_res.size = res.size;
            ref_res.rank = res.rank;
            // TODO: Consider ways to avoid copy....
            for (std::uint16_t j = 0; j < res.rank; ++j) {
              ref_res.shape[j] = res.shape[j];
            }
            ref_res.dtype = res.dtype;
          }
        }
      };

      // For Chronological formats, only the StreamBroker holding a step fetches it
      std::size_t num_brokers { this->num_stream_brokers() };
      std::size_t num_segments { this->num_segments() };

      const IOStatus status {
        ExecutionPolicy::template get_data_steps<FTraits>(steps,
                                                          read_cb,
                                                          num_brokers,
                                                          get_data_cb,
                                                          num_segments)
      };

      CompositeDataResult<MemTag> composite {
        ptr_tbl,
        status == IOStatus::Success ? this->num_segments() : 0,
        count,
        ref_res.rank,
        ref_res.shape,
        ref_res.dtype,
        true
      };

      return composite.to_array();
    }

    /**
     * Resolve data for a contiguous batch of steps [first, last) from the already
     * fetched buffers, without any IO.
     *
     * This is the data resolution half of `get_multi_data`. The batch's data must
     * already be in the StreamBrokers' buffers, e.g. fetched with `fetch_steps_for`
     * (directly or via a zipped iteration over batches). The result always has a batch
     * axis.
     *
     * @tparam Args... The variadic types for arguments to pass to the DataRequest
     *         constructor. This set of arguments depends on the FormatTraits request
     *         constructors (of which, there are possibly multiple).
     * @param[in] first The first step of the (already fetched) batch.
     * @param[in] last One past the last step of the batch.
     * @param[in] args The arguments for the DataRequest constructor.
     * @returns array The requested data as an SOArrayView or SODevArrayView
     *          depending on whether MemTag is HostTag or DevTag, respectively.
     */
    template <typename... Args>
    SBIO_HD inline ncarray::SOViewFor<MemTag> resolve_multi_data(const StepIdxType& first,
                                                                 const StepIdxType& last,
                                                                 Args&&... args) const {
      DataRequest req(group_name(), group_type(), std::forward<Args>(args)...);

      const std::size_t count { (last > first) ? static_cast<std::size_t>(last - first) : 1 };

      DataResult ref_res;

      auto& ptr_buf { this->m_ptr_storage.template get<roles::Table>() };
      const void** ptr_tbl { reinterpret_cast<const void**>(ptr_buf.ptr()) };
      assert(count * this->num_segments() * sizeof(void *) <= ptr_buf.size() &&
             "A requested batch cannot be larger than configured max_batch_size!");

      if constexpr (FTraits::PartitioningStrategy != StreamPartitioningStrategy::Chronological) {
        for (std::size_t cnt = 0; cnt < count; ++cnt) {
          for (std::size_t i = 0; i < this->num_segments(); ++i) {
            const auto& seg { m_topology.segment(i) };
            auto res = get_data_for(req, i, batch_slot(i, first, cnt));

            ptr_tbl[cnt * this->num_segments() + seg.logical_slot] = const_cast<void*>(res.data);
            if (i == 0 && cnt == 0) {
              // This assumes all segments are same shape...
              ref_res.data = res.data;
              ref_res.size = res.size;
              ref_res.rank = res.rank;
              for (std::uint16_t j = 0; j < res.rank; ++j) {
                ref_res.shape[j] = res.shape[j];
              }
              ref_res.dtype = res.dtype;
            }
          }
        }
      } else {
        ChronoBatch chrono {};
        const bool batched { chrono_batch(first, last, chrono) == IOStatus::Success };

        for (std::size_t cnt = 0; cnt < count; ++cnt) {
          for (std::size_t l = 0; l < this->num_segments(); ++l) {
            std::size_t seg_idx { TopologyType::NoSegment };
            std::size_t slot { 0 };
            if (batched) {
              chrono_slot(chrono, static_cast<StepIdxType>(first + cnt), l, seg_idx, slot);
            }

            const std::size_t ptr_idx { cnt * this->num_segments() + l };
            ptr_tbl[ptr_idx] = nullptr;
            if (seg_idx != TopologyType::NoSegment) {
              auto res = get_data_for(req, seg_idx, slot);
              ptr_tbl[ptr_idx] = const_cast<void*>(res.data);
              if (ref_res.data == nullptr) {
                ref_res.data = res.data;
                ref_res.size = res.size;
               ref_res.rank = res.rank;
                for (std::uint16_t j = 0; j < res.rank; ++j) {
                  ref_res.shape[j] = res.shape[j];
                }
                ref_res.dtype = res.dtype;
              }
            }
          }
        }
      }

      CompositeDataResult<MemTag> composite {
        ptr_tbl,
        this->num_segments(),
        count,
        ref_res.rank,
        ref_res.shape,
        ref_res.dtype,
        true
      };
      return composite.to_array();
    }

  private:
    mutable const void* m_ptrs[MaxSegments]; // Final coalesced reads will be left here.

    /**
     * Whether a particular segment's steps are being aligned to a different access pattern.
     *
     * @note This alignment is only available when using the StreamBroker's generic
     *       indexing routines (or, at least, we can only know if its being done)
     */
    SBIO_HD inline bool is_aligned(const DataSegmentRef& seg) const {
      if constexpr (BrokerType::UsesGenericIndex) {
        return
          m_topology.step_mapping == StepMapping::AlignTo &&
          seg.access_ptn != m_topology.align_ptn;
      } else {
        return false;
      }
    }

    /**
     * Remap a segment's steps onto the batch of steps for the access pattern its aligend to.
     *
     * This maps [first_ordinal, end_ordinal) onto [first, last) that the segment was
     * configured to align to.
     *
     * @note The mapping is monotonic and runs from the ordinal of `first` to the ordinal
     *       of `last - 1`.
     *
     * @note Several steps may map to the same ordinal. In that case, it will only
     *       actually be fetched once.
     *
     * @note If the segment's steps are more frequent than those of the pattern it's
     *       being aligned to, the remapped range may end up being longer than the
     *       configured max batch size. It wouldn't fit into the Data buffer in that
     *       case and a PayloadTruncatedError would be returned.
     */
    SBIO_HD inline IOStatus map_batch(const DataSegmentRef& seg,
                                      StepIdxType first,
                                      StepIdxType last,
                                      std::size_t& first_ordinal,
                                      std::size_t& end_ordinal) const {
      const StepIdxType last_step {
        (last > first) ? static_cast<StepIdxType>(last - 1) : first
      };

      IOStatus status {
        m_topology.map_step(seg, m_topology.remap_step_idx(first), first_ordinal)
      };
      if (status != IOStatus::Success) {
        return status;
      }

      std::size_t last_ordinal { 0 };
      status =
        m_topology.map_step(seg, m_topology.remap_step_idx(last_step), last_ordinal);
      end_ordinal = last_ordinal + 1;

      return status;
    }

    /**
     * Determined where a step `first + cnt` is within a fetched batch of steps.
     *
     * If there are no step raemapping then its just `cnt`. If AlignTo was configured
     * though, then the remap must be considered and its the ordinal relative to the
     * first remapped ordinal.
     */
    SBIO_HD inline std::size_t batch_slot(std::size_t segment_no,
                                          StepIdxType first,
                                          std::size_t cnt) const {
      const auto& seg { m_topology.segment(segment_no) };
      if (!is_aligned(seg)) {
        return cnt;
      }

      std::size_t first_ordinal { 0 };
      std::size_t ordinal { 0 };
      const StepIdxType step { static_cast<StepIdxType>(first + cnt) };
      if (m_topology.map_step(seg, m_topology.remap_step_idx(first), first_ordinal) != IOStatus::Success ||
          m_topology.map_step(seg, m_topology.remap_step_idx(step), ordinal) != IOStatus::Success) {
        return cnt; // Not fetched (the fetch failed with the same mapping)
      }

      return ordinal - first_ordinal;
    }

    /// --- Chronological formats --- ///

    // Each step is in one StreamBroker (see GroupTopology::locate). A StreamBroker
    // fetches only the steps it holds.
    // A batch's steps held by one StreamBroker are consecutive there.

    using TopologyType = GroupTopology<BrokerType, MaxSegments>;

    /**
     * The steps of a batch held by each StreamBroker (in the group's step space).
     */
    struct ChronoBatch {
      std::array<std::size_t, MaxSegments> first {}; ///< First ordinal, per StreamBroker
      std::array<std::size_t, MaxSegments> last {};  ///< Last ordinal (inclusive), per StreamBroker
      std::array<bool, MaxSegments> any {};          ///< Whether the StreamBroker holds any
    };

    /**
     * The first SegmentRef of a StreamBroker.
     *
     * @note The chosen SegmentRef's cursor and pattern fetch for all.
     */
    SBIO_HD inline std::size_t first_segment_of(std::size_t broker_no) const {
      for (std::size_t n = 0; n < m_topology.num_segments; ++n) {
        if (m_topology.segment(n).broker == m_topology.stream_brokers[broker_no]) {
          return n;
        }
      }
      return TopologyType::NoSegment;
    }

    /**
     * Fetch a step, if StreamBroker `broker_no` holds it.
     */
    SBIO_HD inline IOStatus chrono_fetch(StepIdxType step_idx, std::size_t broker_no) const {
      std::size_t holder { 0 };
      std::size_t space_ordinal { 0 };
      if (!m_topology.locate(step_idx, holder, space_ordinal)) {
        return IOStatus::NoOffsetInData;
      }

      const std::size_t seg_idx { first_segment_of(broker_no) };
      if (holder != broker_no || seg_idx == TopologyType::NoSegment) {
        return IOStatus::Success; // Another StreamBroker holds the step
      }

      const auto& seg { m_topology.segment(seg_idx) };
      std::size_t ordinal { 0 };
      const auto status { m_topology.map_step(seg, space_ordinal, ordinal) };
      if (status != IOStatus::Success) {
        return status;
      }

      return seg.broker->fetch_step(ordinal, seg.access_ptn, seg.cursor.get());
    }

    /**
     * Find which steps of [first, last) each StreamBroker holds.
     */
    SBIO_HD inline IOStatus chrono_batch(StepIdxType first,
                                         StepIdxType last,
                                         ChronoBatch& batch) const {
      const StepIdxType end { (last > first) ? last : static_cast<StepIdxType>(first + 1) };
      for (StepIdxType step = first; step < end; ++step) {
        std::size_t broker_no { 0 };
        std::size_t ordinal { 0 };
        if (!m_topology.locate(step, broker_no, ordinal)) {
          return IOStatus::NoOffsetInData;
        }

        if (!batch.any[broker_no]) {
          batch.any[broker_no] = true;
          batch.first[broker_no] = ordinal;
        }
        batch.last[broker_no] = ordinal;
      }
      return IOStatus::Success;
    }

    /**
     * The ordinals [first, end) StreamBroker `broker_no` fetches for a batch.
     *
     * Empty (first == end == 0) if it holds none of the batch's steps.
     */
    SBIO_HD inline IOStatus chrono_ordinals(const ChronoBatch& batch,
                                            std::size_t broker_no,
                                            std::size_t& first_ordinal,
                                            std::size_t& end_ordinal) const {
      first_ordinal = 0;
      end_ordinal = 0;

      const std::size_t seg_idx { first_segment_of(broker_no) };
      if (!batch.any[broker_no] || seg_idx == TopologyType::NoSegment) {
        return IOStatus::Success;
      }

      const auto& seg { m_topology.segment(seg_idx) };
      std::size_t last_ordinal { 0 };
      IOStatus status { m_topology.map_step(seg, batch.first[broker_no], first_ordinal) };
      if (status == IOStatus::Success) {
        status = m_topology.map_step(seg, batch.last[broker_no], last_ordinal);
      }
      end_ordinal = last_ordinal + 1;

      return status;
    }

    /**
     * Fetch the steps of a batch StreamBroker `broker_no` holds.
     */
    SBIO_HD inline IOStatus chrono_fetch_batch(const ChronoBatch& batch,
                                               std::size_t broker_no) const {
      std::size_t first_ordinal { 0 };
      std::size_t end_ordinal { 0 };
      const auto status { chrono_ordinals(batch, broker_no, first_ordinal, end_ordinal) };
      if (status != IOStatus::Success || end_ordinal == first_ordinal) {
        return status;
      }

      const auto& seg { m_topology.segment(first_segment_of(broker_no)) };
      return seg.broker->fetch_steps({ static_cast<StepIdxType>(first_ordinal),
                                       static_cast<StepIdxType>(end_ordinal) },
                                     seg.access_ptn,
                                     seg.cursor.get());
    }

    /**
     * Where a step of a fetched batch is, for a logical segment.
     *
     * @param[out] seg_idx The SegmentRef (NoSegment if it cannot be found).
     * @param[out] slot The step's position in its StreamBroker's fetched batch.
     */
    SBIO_HD inline void chrono_slot(const ChronoBatch& batch,
                                    StepIdxType step,
                                    std::size_t logical,
                                    std::size_t& seg_idx,
                                    std::size_t& slot) const {
      seg_idx = TopologyType::NoSegment;
      slot = 0;

      std::size_t broker_no { 0 };
      std::size_t space_ordinal { 0 };
      if (!m_topology.locate(step, broker_no, space_ordinal) || !batch.any[broker_no]) {
        return;
      }

      const std::size_t idx { m_topology.segment_index_for(broker_no, logical) };
      if (idx == TopologyType::NoSegment) {
        return;
      }

      const auto& seg { m_topology.segment(idx) };
      std::size_t first_ordinal { 0 };
      std::size_t ordinal { 0 };
      if (m_topology.map_step(seg, batch.first[broker_no], first_ordinal) != IOStatus::Success ||
          m_topology.map_step(seg, space_ordinal, ordinal) != IOStatus::Success) {
        return; // Not fetched (the fetch failed with the same mapping)
      }

      seg_idx = idx;
      slot = ordinal - first_ordinal;
    }

    mutable GroupTopology<BrokerType, MaxSegments> m_topology;

    mutable PtrStorageType m_ptr_storage;
  };
} // namespace sbio

#endif // SBIO_CORE_BROKER_GROUP_HH
