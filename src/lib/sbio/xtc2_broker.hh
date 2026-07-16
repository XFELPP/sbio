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

#ifndef SBIO_CORE_XTC2_BROKER_HH
#define SBIO_CORE_XTC2_BROKER_HH

#include "sbio/core/access_style.hh"
#include "sbio/core/broker.hh"
#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/stream.hh"
#include "sbio/core/sync.hh"
#ifdef SBIO_HAS_CUDA
#include "sbio/cuda/storage.hh"
#endif
#include "sbio/formats/xtc2/xtc2.hh"
#include "sbio/formats/xtc2/xtc2_traits.hh"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace sbio {
  /**
   * A Broker implementation for managing XTC2 data streams.
   *
   * This Broker will use both a 'small data' and a 'big data' stream to read data.
   * The 'small data' stream is used for indexing the 'big data' stream which contains
   * the actual bulk data.
   */
  template <IOTraits IO, class EPolicy>
  class XTC2StreamBroker
    : public StreamBroker<XTC2StreamBroker<IO, EPolicy>, IO, EPolicy, XTC2Traits> {
  public:
    using Base = StreamBroker<XTC2StreamBroker<IO, EPolicy>, IO, EPolicy, XTC2Traits>;
    using StorageT = Storage<typename XTC2Traits::BrokerBufferRequirements, EPolicy>;
    using Config = typename XTC2Traits::StreamParameters;
    using DiscoveryState = typename XTC2Traits::DiscoveryState;
    using MetadataInventory = typename XTC2Traits::MetadataInventory;

    using DataResult = typename XTC2Traits::DataResult;
    using DataRequest = typename XTC2Traits::DataRequest;
    using DataAccessPtn = typename XTC2Traits::DataAccessPtn;

    XTC2StreamBroker()
      : Base()
    {}

    XTC2StreamBroker(const Config& cfg)
      : Base(cfg)
    {}

    /**
     * Allocate necessary buffers for reading data.
     *
     * @returns The request for allocations to be fulfilled by the Execution policy.
     */
    inline AllocationRequest<XTC2Traits> get_allocation_request() const {
      // These follow the order layed out in the XTC2Traits BuffRequirements TypeList
      AllocationRequest<XTC2Traits> request;
      request.size_requests[0] = this->m_config.max_dgram_size; // Transition buf
      request.size_requests[1] = // Scratch buffer for reading ahead in smd file
        this->m_config.events_per_read * (sizeof(XTC2::Dgram) + 80);
      request.size_requests[2] = this->m_config.max_dgram_size; // Event buf
      request.size_requests[3] =
        this->m_config.events_per_read * sizeof(XTC2Traits::EventOffset);
      request.size_requests[4] =
        this->m_config.events_per_read * sizeof(XTC2Traits::TransitionOffset);

      return request;
    }

    /**
     * The CONNECT stage will open both the brokered streams.
     *
     * @returns The IOStatus from opening the data streams.
     */
    inline IOStatus open_data_stream_impl() {
      return setup();
    }

    /**
     * The discovery stage reads the XTC2 data from CONFIGURE to determine what
     * is available in the brokered streams.
     *
     * @returns The IOStatus from reading the metadata.
     */
    inline IOStatus discover_metadata_impl() {
      return read_xtc_metadata();
    }

    /**
     * The INDEXING stage indexes the 'big data' stream by reading the 'small data' one.
     *
     * @returns The IOStatus from indexing the streams.
     */
    inline IOStatus index_stream_impl() {
      // Provide the XTC2 implementation's stream state with relevant config info
      this->m_stream_state.events_per_read = this->m_config.events_per_read;

      std::size_t read_size =
        (sizeof(XTC2::Dgram) + 80) * this->m_config.events_per_read;

      std::size_t missing_chunk { 0 };
      if (m_curr_smd_offset) {
        std::size_t last_bytes_read = m_smd_stream.read_count();
        missing_chunk = last_bytes_read - m_curr_smd_offset;
      }

      // Read a lot into a scratch buffer
      auto& smd_buf = this->m_storage.template get<MetadataRole, 1>();
      IOStatus status = m_smd_stream.read_batch(smd_buf.ptr(), read_size, missing_chunk);

      if (status == IOStatus::ZeroBytesRead) {
        // End of file - need to broadcast that we no longer have new events.
        // May need to adjust for "live mode"
        m_num_events = 0;
        m_num_transitions = 0;
        return IOStatus::Success;
      } else if (status != IOStatus::Success) {
        return status;
      }

      auto l1_offsets = this->get_l1_offsets_buffer();
      auto trans_offsets = this->get_transition_offsets_buffer();

      std::size_t bytes_read = m_smd_stream.read_count();
      m_curr_smd_offset = 0;
      std::size_t n_events { 0 };
      std::size_t n_transitions { 0 };
      status = IOStatus::Success;
      while (m_curr_smd_offset < bytes_read) {
        if (n_events >= this->m_config.events_per_read) {
          status = IOStatus::AllRequestedRead;
          break;
        }

        if (m_curr_smd_offset + sizeof(XTC2::Dgram) > bytes_read) {
          // Will want to deal with this differently for handling "Live Mode"
          status = IOStatus::TruncatedRead;
          break;
        }

        auto* dg =
          reinterpret_cast<XTC2::Dgram*>(reinterpret_cast<char*>(smd_buf.ptr()) + m_curr_smd_offset);

        // From the scratch buffer above, pull out the offset data and populate
        // The l1 and transition offset buffers
        m_curr_smd_offset += XTC2Traits::populate_offsets(
            dg,
            this->m_stream_state,
            0,
            l1_offsets,
            trans_offsets,
            m_curr_smd_offset
        );

        if (dg->service() == XTC2::TransitionId::L1Accept) {
          n_events++;
        } else {
          n_transitions++;
        }
      }
      m_num_events = n_events;
      m_num_transitions = n_transitions;
      // Eventually, want to return that status variable populated above.
      return IOStatus::Success;
    }

    // ---- Currently Unused ---- //
    template <typename CBType>
    inline IOStatus process(CBType&& callback) { return IOStatus::Success; }
    inline IOStatus run() { return IOStatus::Success; }
    template <typename CBType>
    inline IOStatus step(CBType&& callback) { return IOStatus::Success; }
    // -------------------------- //

    /**
     * Retrieve the current indexed capacity.
     *
     * @returns The current indexed capacity (how many offsets are known).
     */
    inline std::size_t capacity() const { return this->m_num_events; }

    /**
     * The currently read data buffer.
     *
     * @returns The pointer to the most recently read data.
     */
    inline void* current_buffer() {
      return this->m_storage.template get<DataRole>().ptr();
    }

    /**
     * Retrieve data for the specified step using the provided access pattern.
     *
     * @param[in] step_index The index to read data for.
     * @param[in] ptn The access pattern for reading (L1Accept, transition, etc.)
     * @returns The IOStatus for reading/fetching the data from the stream.
     */
    inline IOStatus fetch_step_impl(std::size_t step_index, const DataAccessPtn ptn) {
      if (ptn == XTC2Traits::DataAccessPtn::L1Accept) {
        return fetch_next_l1(step_index);
      } else if (ptn == XTC2Traits::DataAccessPtn::SlowUpdate) {
        return fetch_next_transition(step_index, XTC2::TransitionId::SlowUpdate);
      } else if (ptn == XTC2Traits::DataAccessPtn::BeginStep) {
        return fetch_next_transition(step_index, XTC2::TransitionId::BeginStep);
      }

      return IOStatus::GeneralIOError;
    }

    /**
     * AFTER fetching data, pull specific pieces of interest out.
     *
     * @param[in] req The specification for the requested data.
     * @param[in] ptn The access pattern for the spec (transition, L1Accept, etc).
     * @returns The result of inspecting the data (including the data itself).
     */
    inline DataResult get_data_in_buffer(const DataRequest& req,
                                         const DataAccessPtn ptn) {
      if (ptn == XTC2Traits::DataAccessPtn::L1Accept) {
        // "Normal" detector resolution.
        auto& bd_buf { this->m_storage.template get<DataRole>() };

        auto broker_view = EPolicy::acquire_broker_view(bd_buf);
        XTC2::DataResult broker_res = XTC2Traits::resolve_data(broker_view, this->metadata(), req);
        XTC2::DataResult user_res = EPolicy::release_broker_view(bd_buf, broker_view, broker_res);

        return user_res;
        if constexpr (requires { bd_buf.host_ptr(); }) {
#ifdef SBIO_HAS_CUDA
          if (bd_buf.has_cached_offset()) {
            XTC2::DataResult res;
            res.data =
              reinterpret_cast<const char*>(bd_buf.device_ptr()) + bd_buf.cached_offset();

            const auto& cache { bd_buf.cached_result() };
            res.size = cache.size;
            res.rank = cache.rank;
            std::memcpy(res.shape, cache.shape, sizeof(res.shape));
            res.dtype = static_cast<XTC2::DType>(cache.dtype);

            return res;
          } else {
            // Traverse the host-mirrored metadata header
            XTC2::DataResult res =
              XTC2Traits::resolve_data(bd_buf.host_ptr(), this->metadata(), req);
            std::size_t offset =
              reinterpret_cast<const char*>(res.data) - reinterpret_cast<const char*>(bd_buf.host_ptr());
            bd_buf.set_cached_offset(offset);

            CudaDeviceBuffer::CachedResult cache;
            cache.size = res.size;
            cache.rank = res.rank;
            std::memcpy(cache.shape, res.shape, sizeof(cache.shape));
            cache.dtype = static_cast<int>(res.dtype);
            bd_buf.set_cached_result(cache);
            res.data = reinterpret_cast<const char*>(bd_buf.device_ptr()) + offset;

            return res;
          }
#endif
        }

        return XTC2Traits::resolve_data(this->current_buffer(), this->metadata(), req);
      } else if (ptn == XTC2Traits::DataAccessPtn::SlowUpdate) {
        // Semantic Mapping: Requested name is actually a PV field in "epics"
        DataRequest epics_req { req };

        const char* epics_det_name { "epics" };
        std::size_t i { 0 };
        for (; i < 5; ++i) {
          epics_req.detector_name[i] = epics_det_name[i];
        }
        epics_req.detector_name[i] = '\0';

        i = 0;
        for (; i < XTC2Traits::MaxNameSize - 1 && req.detector_name[i] != '\0'; ++i) {
          epics_req.field_name[i] = req.detector_name[i];
        }
        epics_req.field_name[i] = '\0';

        return XTC2Traits::resolve_data(this->current_buffer(), this->metadata(), epics_req);
      } else if (ptn == XTC2Traits::DataAccessPtn::BeginStep) {
        // `scan` should be like a "normal" detector. However, it must be read from
        // the BeginStep transition buffers.
        // Those buffers will have:
        // - `step_value`     : INT64
        // - `step_docstring` : CHARSTR, optional (but usually present)
        // - `scan_var_xxx`   : ANY (the actual scanned variable - may have multiple)
        // We also allow for people to pass `scan_var_namexxx` as the name directly
        // So in the case we have Scan type access, but the name is not "scan" we must
        // do a rewrite
        DataRequest scan_req { req };

        if (std::strcmp(scan_req.detector_name, "scan") != 0) {
          const char* scan_det_name { "scan" };
          std::size_t i { 0 };
          for (; i < 4; ++i) {
            scan_req.detector_name[i] = scan_det_name[i];
          }
          scan_req.detector_name[i] = '\0';

          i = 0;
          for (; i < XTC2Traits::MaxNameSize - 1 && req.detector_name[i] != '\0'; ++i) {
            scan_req.field_name[i] = req.detector_name[i];
          }
          scan_req.field_name[i] = '\0';
        }
        return XTC2Traits::resolve_data(this->current_buffer(), this->metadata(), scan_req);
      }
      return {};
    }

    /**
     * The following variables are specified as requiring synchronization.
     *
     * Depending on the execution policy, it may be needed to perform explicit
     * synchronizations when there are components running in parallel. The
     * function sync_vars returns the group of such variables.
     *
     * @returns The sync group for the event and transition count.
     */
    inline auto sync_vars() {
      return make_sync_group(this->m_num_events, this->m_num_transitions);
    }

  private:
    /**
     * The first of the `IndexRole` buffers will be used for event offsets.
     */
    inline auto get_l1_offsets_buffer() {
      return reinterpret_cast<XTC2Traits::EventOffset*>(this->m_storage.template get<IndexRole, 0>().ptr());
    }

    /**
     * The second of the `IndexRole` buffers will be used for transition offsets.
     */
    inline auto get_transition_offsets_buffer() {
      return reinterpret_cast<XTC2Traits::TransitionOffset*>(this->m_storage.template get<IndexRole, 1>().ptr());
    }

    inline IOStatus fetch_next_l1(std::size_t step_index) {
      std::size_t adjusted_index { step_index % this->m_config.events_per_read };
      if (adjusted_index >= m_num_events) {
        return IOStatus::AllRequestedRead;
      }
      auto l1_offsets = this->get_l1_offsets_buffer();

      auto& offset = l1_offsets[adjusted_index];
      std::size_t file_offset { offset.offset };
      std::size_t read_size { offset.size };

      auto& bd_buf = this->m_storage.template get<DataRole>();
      IOStatus status = m_bd_stream.read_at(bd_buf.ptr(), file_offset, read_size);

      if (status != IOStatus::Success) {
        return status;
      }

      bd_buf.set_dirty(true);

      if constexpr (requires { bd_buf.host_ptr(); }) {
        if (!bd_buf.has_cached_offset()) {
          std::size_t copy_size { std::min(read_size, static_cast<std::size_t>(4096)) };
#ifdef SBIO_HAS_CUDA
          cudaMemcpy(bd_buf.host_ptr(),
                     bd_buf.device_ptr(),
                     copy_size,
                     cudaMemcpyDeviceToHost);
#endif
        }
      }

      std::size_t read_count = m_bd_stream.read_count();
      if (read_count == 0) {
        return IOStatus::ZeroBytesRead;
      }

      return IOStatus::Success;
    }

    inline IOStatus fetch_next_transition(std::size_t step_index,
                                          XTC2::TransitionId transition_id) {
      std::size_t adjusted_index { step_index % this->m_config.events_per_read };
      if (adjusted_index >= m_num_transitions) {
        return IOStatus::AllRequestedRead;
      }
      auto transition_offsets = this->get_transition_offsets_buffer();

      auto& offset = transition_offsets[m_curr_transition_index];
      while (offset.transition_id != transition_id) {
        m_curr_transition_index++;
        if (m_curr_transition_index >= m_num_transitions) {
          return IOStatus::AllRequestedRead;
        }
        offset = transition_offsets[m_curr_transition_index];
      }
      std::int64_t prev_l1_index { offset.previous_l1_index };

      std::size_t read_size { 0 };
      std::size_t file_offset { 0 };
      if (prev_l1_index <= static_cast<std::int64_t>(step_index) &&
          m_curr_transition_index < m_num_transitions) {
        read_size = offset.size;
        if (prev_l1_index == -1) {
          auto& l1_offset = this->get_l1_offsets_buffer()[0];
          std::size_t total_offset_from_l1 = read_size;
          std::size_t transition_index = m_curr_transition_index;

          auto& next_transition_offset = transition_offsets[transition_index];
          while (next_transition_offset.previous_l1_index == -1) {
            total_offset_from_l1 += next_transition_offset.size;
            transition_index++;
            next_transition_offset = transition_offsets[transition_index];
          }

          file_offset = l1_offset.offset - total_offset_from_l1;
        } else {
          file_offset = offset.offset;
        }
      }

      auto& bd_buf = this->m_storage.template get<MetadataRole, 0>();
      IOStatus status = m_bd_stream.read_at(bd_buf.ptr(), file_offset, read_size);

      if (status != IOStatus::Success) {
        return status;
      }

      std::size_t read_count = m_bd_stream.read_count();
      if (read_count == 0) {
        return IOStatus::ZeroBytesRead;
      }

      return IOStatus::Success;
    }

    IOStatus setup() {
      if (m_smd_stream.connect(this->m_config.smd_path) != IOStatus::Success) {
        return IOStatus::OpenFailed;
      }

      if (m_bd_stream.connect(this->m_config.xtc_path) != IOStatus::Success) {
        return IOStatus::OpenFailed;
      }

      return IOStatus::Success;
    }

    IOStatus read_xtc_metadata() {
      auto& smd_buf = this->m_storage.template get<MetadataRole>();
      IOStatus status = m_smd_stream.read_one(smd_buf.ptr(), smd_buf.size());

      if (status != IOStatus::Success) {
        return status;
      }

      auto* dg = reinterpret_cast<XTC2::Dgram*>(smd_buf.ptr());

      if (dg->service() == XTC2::TransitionId::Configure) {
        XTC2Traits::discover_metadata(dg, this->m_metadata_inv, 0);
      }

      return IOStatus::Success;
    }

  private:
    Stream<IO, XTC2Traits> m_smd_stream;       ///< The 'small data' stream (.smd.xtc2)
    Stream<IO, XTC2Traits> m_bd_stream;        ///< The 'big data' stream (.xtc2)

    std::size_t m_num_events { 0 };            ///< Current number of events read
    std::size_t m_num_transitions { 0 };       ///< Current number of transitions read
    std::size_t m_curr_transition_index { 0 }; ///< Current transitions offset
    std::size_t m_curr_smd_offset { 0 };       ///< Current offset in small data stream

  };
} // namespace sbio

#endif // SBIO_CORE_XTC2_BROKER_HH
