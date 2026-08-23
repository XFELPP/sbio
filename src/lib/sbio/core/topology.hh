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

#ifndef SBIO_CORE_TOPOLOGY_HH
#define SBIO_CORE_TOPOLOGY_HH

#include "sbio/core/broker.hh"
#include "sbio/core/types.hh"
#include "sbio/util/string.hh"

#ifdef __CUDACC__

#include <cuda/std/array>
#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/cstring>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

namespace sbio {
  /**
   * Map a StreamBroker to a logical position within a BrokerGroup.
   */
  template <IsStreamBroker BrokerType>
  struct SegmentRef {
    /**
     * The type of data being read.
     */
    using DataFormat = typename BrokerType::DataFormat;

    /**
     * The type of the enumerator used to specify access patterns used for the format.
     */
    using DataAccessPtn = typename DataFormat::DataAccessPtn;

    /**
     * The pointer to the StreamBroker where you find this logical segment.
     */
    BrokerType* broker { nullptr };
    /**
     * The identifier provided by the actual data format specification.
     *
     * The format identifier does not necessarily equal the logical_slot. In some cases,
     * the format may not even provide an identifier.
     */
    hd_std::uint32_t format_segment_id { 0 };
    /**
     * The format-specific access strategy used to access this logical segment.
     *
     * Some formats expose multiple mechanisms to traverse data. The access pattern
     * controls which mechanism to use.
     */
    DataAccessPtn access_ptn;
    /**
     * The logical order of the segment within a BrokerGroup.
     */
    hd_std::uint32_t logical_slot { 0 };
  };

  /**
   * The layout, post any sorting, of all segment references in a BrokerGroup.
   *
   * The topology organization is established once during the DISCOVERY state
   * transition when metadata is parsed.
   */
  template <IsStreamBroker BrokerType, hd_std::size_t MaxSegments = 128>
  struct GroupTopology {
    /**
     * The type of data being read.
     */
    using DataFormat = typename BrokerType::DataFormat;

    /**
     * The type of the enumerator used to specify access patterns used for the format.
     */
    using DataAccessPtn = typename DataFormat::DataAccessPtn;

    /**
     * The type used to request a specific step from the Stream.
     *
     * This type is required and guaranteed to be convertible std::size_t; however,
     * different data format's may use different underlying types.
     */
    using StepIdxType = typename DataFormat::StepIdxType;

    static constexpr StreamPartitioningStrategy strategy {
      DataFormat::PartitioningStrategy
    };

    char group_name[256] { 0 };
    char group_type[256] { 0 };

    hd_std::array<BrokerType*, MaxSegments> stream_brokers { nullptr };
    hd_std::array<DataAccessPtn, MaxSegments> broker_access_ptns {};
    hd_std::size_t num_stream_brokers { 0 };

    hd_std::array<SegmentRef<BrokerType>, MaxSegments> segments {};
    hd_std::size_t num_segments { 0 };

    SBIO_HD bool empty() const { return num_segments == 0; }

    // --- Construction --- //

    SBIO_HD GroupTopology() = default;

    SBIO_HD GroupTopology(const char* name,
                          const char* type,
                          hd_std::array<SegmentRef<BrokerType>, MaxSegments> segs,
                          hd_std::size_t n_segs)
      : segments(segs)
      , num_segments(n_segs)
    {
      safe_strncpy(group_name, name, 256);
      safe_strncpy(group_type, type, 256);

      for (hd_std::size_t n = 0; n < num_segments; ++n) {
        auto* broker { broker_for_segment(n) };
        const auto& ptn { pattern_for_segment(n) };

        hd_std::size_t b { 0 };
        for (; b < num_stream_brokers; ++b) {
          if (stream_brokers[b] == broker) {
            break;
          }
        }
        if (b == num_stream_brokers) {
          set_stream_broker(b, broker, ptn);
        }
      }
    }

    /**
     * Build the GroupTopology for the requested name.
     *
     * The available StreamBrokers will have their metadata queried to test if they
     * belong to the requested named group. (There is a wildcard placeholder which
     * will incorporate all of the broker/segments into the group, but this is currently
     * fragile as the various segments must actually have the same shape.)
     *
     * @param[in] name The name to build the topology for.
     * @param[in] brokers The available StreamBrokers to organize into the topology.
     * @param[in] num_brokers The number of available StreamBrokers.
     * @returns The constructed GroupTopology which can be used to create a BrokerGroup.
     */
    SBIO_HD static GroupTopology<BrokerType, MaxSegments>
    build_topology(const char* name, BrokerType* brokers, hd_std::size_t num_brokers) {
      char final_type[256] { "unknown" };

      hd_std::array<SegmentRef<BrokerType>, MaxSegments> sorted_segments{};
      hd_std::array<SegmentRef<BrokerType>, MaxSegments> tmp_segments {};
      hd_std::array<hd_std::uint32_t, MaxSegments> stream_indices {};

      hd_std::size_t n_segments_found { 0 };
      for (hd_std::size_t idx = 0; idx < DataFormat::DataAccessPtnCount; ++idx) {
        auto ptn { static_cast<DataAccessPtn>(idx) };

        for (hd_std::size_t b = 0; b < num_brokers && n_segments_found < MaxSegments; ++b) {
          auto* broker { &brokers[b] };
          const auto& inv { broker->metadata() };

          for (hd_std::size_t e = 0; e < inv.num_entries() && n_segments_found < MaxSegments; ++e) {
            // Check if a specific entry from the inventory matches a provided name
            // query - The logic for this is format-specific, and may also depend on
            // the specific access pattern.
            // Also allow a "Wildcard" to match anything.
            // - TODO: This wildcard will break if the shapes are different...
            //         SO will need to update and improve later!
            if (inv.entry_matches(e, name, ptn) || hd_std::strcmp(name, "*") == 0) {
              // For a match, construct a segment
              // Inventory must provide accessor that gives <type, seg_no> for a name
              auto [type, seg_no] = inv.metadata_for(e);

              // Check if this was previously registered
              bool duplicate { false };
              for (hd_std::size_t k = 0; k < n_segments_found; ++k) {
                if (tmp_segments[k].broker == broker && tmp_segments[k].format_segment_id == seg_no) {
                  duplicate = true;
                  break;
                }
              }

              if (!duplicate) {
                // Logical slot will be done next, to deal with sorting.
                stream_indices[n_segments_found] = broker->stream_idx();
                tmp_segments[n_segments_found++] = { broker, seg_no, ptn };

                if (hd_std::strcmp(final_type, "unknown") == 0) {
                  safe_strncpy(final_type, type, 256);
                }
              }
            }
          }
        }
      }

      if (n_segments_found > 0) {
        hd_std::array<hd_std::uint32_t, MaxSegments> final_segment_indices {};

        hd_std::uint32_t smallest { 0 };
        if constexpr (strategy == StreamPartitioningStrategy::Chronological) {
          smallest = stream_indices[0];
        } else {
          smallest = tmp_segments[0].format_segment_id;
        }

        for (hd_std::size_t j = 0; j < n_segments_found; ++j) {
          final_segment_indices[j] = j;

          if constexpr (strategy == StreamPartitioningStrategy::Chronological) {
            if (stream_indices[j] < smallest) {
              smallest = stream_indices[j];
            }
          } else {
            if (tmp_segments[j].format_segment_id < smallest) {
              smallest = tmp_segments[j].format_segment_id;
            }
          }
        }

        for (hd_std::size_t i = 0; i < n_segments_found - 1; ++i) {
          hd_std::uint32_t best { static_cast<hd_std::uint32_t>(i) };
          for (hd_std::size_t j = i + 1; j < n_segments_found; ++j) {
            hd_std::uint32_t diff_best { 0 };
            hd_std::uint32_t diff_j { 0 };

            if constexpr (strategy == StreamPartitioningStrategy::Chronological) {
              diff_best = stream_indices[final_segment_indices[best]] - smallest;
              diff_j = stream_indices[final_segment_indices[j]] - smallest;
            } else {
              diff_best =
                tmp_segments[final_segment_indices[best]].format_segment_id - smallest;
              diff_j =
                tmp_segments[final_segment_indices[j]].format_segment_id - smallest;
            }

            if (diff_j < diff_best) {
              best = j;
            }
          }

          if (best != i) {
            hd_std::uint32_t tmp { final_segment_indices[i] };
            final_segment_indices[i] = final_segment_indices[best];
            final_segment_indices[best] = tmp;
          }
        }

        for (hd_std::size_t j = 0; j < n_segments_found; ++j) {
          sorted_segments[j] = tmp_segments[final_segment_indices[j]];

          sorted_segments[j].logical_slot = j;
          if constexpr (strategy == StreamPartitioningStrategy::Chronological) {
            sorted_segments[j].format_segment_id = 0;
          }
        }
      }

      return GroupTopology<BrokerType, MaxSegments>(name,
                                                    final_type,
                                                    sorted_segments,
                                                    n_segments_found);
    }


    // --- Strategy and traits aware getters --- //
    /**
     * Retrieve the correct, active, StreamBroker for a specified step.
     *
     * Depending on the data format and various traits and strategies, not all
     * StreamBrokers may be active/available for every step. This function translates
     * a request for a specific broker, for a specific step, into the correct active
     * StreamBroker.
     *
     * @param[in] step_idx The step for which the active broker is needed.
     * @param[in] broker_no The uncorrected index for the StreamBroker.
     * @returns The active StreamBroker - this may more may not correspond to the
     *          StreamBroker indicated by `broker_no`.
     */
    SBIO_HD BrokerType* active_stream_broker(hd_std::size_t step_idx,
                                             hd_std::size_t broker_no) {
      if constexpr (strategy == StreamPartitioningStrategy::Chronological) {
        auto active_broker_idx { step_idx % num_segments };

        return stream_brokers[active_broker_idx];
      } else {
        auto active_broker_idx { broker_no };

        return stream_brokers[active_broker_idx];
      }
    }

    /**
     * Retrieve the correct DataAccessPtn for the currently active StreamBroker.
     *
     * Depending on the data format and various traits and strategies, not all
     * StreamBrokers may be active/available for every step. This function translates
     * a request for a specific broker, for a specific step, into the correct active
     * StreamBroker and returns its associated DataAccessPtn.
     *
     * @param[in] step_idx The step for which the active broker is needed.
     * @param[in] broker_no The uncorrected index for the StreamBroker.
     * @returns The DataAccessPtn for the active StreamBroker.
     */
    SBIO_HD const DataAccessPtn& active_access_ptn(hd_std::size_t step_idx,
                                                   hd_std::size_t broker_no) {
      if constexpr (strategy == StreamPartitioningStrategy::Chronological) {
        auto active_broker_idx { step_idx % num_segments };

        return broker_access_ptns[active_broker_idx];
      } else {
        auto active_broker_idx { broker_no };

        return broker_access_ptns[active_broker_idx];
      }
    }

    /**
     * Remap a specific step index to account for StreamBroker inactivity and types.
     *
     * Depending on the data format and various traits and strategies, not all
     * StreamBrokers may be active/available for every step. As a result, the step
     * indices may increment faster than the logical count of steps (because it is
     * simultaneously counting steps, and cycling between active brokers). This
     * function will appropriately remap an uncorrected step index to account for
     * this.
     *
     * Furthermore, all data formats are required to use a type (StepIdxType) that can
     * be converted into a simple integral value (size_t). How this conversion is done
     * is not explicitly specified - this function will convert the index to a size_t.
     * Anything downstream of this can use size_t instead of the data format specific
     * types.
     *
     * @param[in] step_idx The unadjusted step index.
     * @returns The 'corrected' step index, accounting for any StreamBroker activity
     *          cycling, and in a size_t integral format.
     */
    SBIO_HD const hd_std::size_t remap_step_idx(hd_std::size_t step_idx) {
      // All formats currently just use size_t directly, but can add in a conversion
      // function to the FormatTraits specifications if it becomes necessary.
      // That call/interface would then be hidden here.
      if constexpr (strategy == StreamPartitioningStrategy::Chronological) {
        return static_cast<hd_std::size_t>(step_idx / num_segments);
      } else {
        // No remapping required, so this is a no-op for the SubDivide strategy
        return static_cast<hd_std::size_t>(step_idx);
      }
    }


    // --- Direct getters/setters --- //
    SBIO_HD const SegmentRef<BrokerType>& segment(hd_std::size_t seg_no) const {
      return segments[seg_no];
    }
    SBIO_HD void set_segment(hd_std::size_t seg_no, SegmentRef<BrokerType>& seg) {
      segments[seg_no] = seg;
    }
    SBIO_HD void set_stream_broker(hd_std::size_t broker_no,
                                   BrokerType* broker,
                                   DataAccessPtn ptn) {
      stream_brokers[broker_no] = broker;
      broker_access_ptns[broker_no] = ptn;
      num_stream_brokers++;
    }

    SBIO_HD BrokerType* broker_for_segment(hd_std::size_t seg_no) {
      return segments[seg_no].broker;
    }
    SBIO_HD DataAccessPtn& pattern_for_segment(hd_std::size_t seg_no) {
      return segments[seg_no].access_ptn;
    }

    SBIO_HD BrokerType* stream_broker(hd_std::size_t broker_no) {
      return stream_brokers[broker_no];
    }
    SBIO_HD const DataAccessPtn& access_ptn(hd_std::size_t broker_no) const {
      return broker_access_ptns[broker_no];
    }
  };
} // namespace sbio

#endif // SBIO_CORE_TOPOLOGY_HH
