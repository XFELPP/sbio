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

#ifndef SBIO_CORE_DATASOURCE_HH
#define SBIO_CORE_DATASOURCE_HH

#include "sbio/core/broker.hh"
#include "sbio/core/broker_group.hh"
#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/util/string.hh"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <utility> // std::forward

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace fs = std::filesystem;

namespace sbio {
  template <
    IOTraits IO,
    class EPolicy,
    FormatTraits FTraits,
    class BrokerType,
    std::size_t MaxDataStreams = 128
  >
  class DataSource {
  public:
    using DSTraits = FTraits;
    using Config = typename FTraits::StreamParameters;

    DataSource() = default;

    SBIO_HD inline bool add_data_stream(const Config& cfg) {
      if (m_num_data_streams >= MaxDataStreams) {
        return false;
      }

      m_data_streams[m_num_data_streams].configure_broker(cfg);
      m_num_data_streams++;

      return true;
    }

    /**
     * load_run: Scans the standard hutch directory and automatically registers
     * all streams for a given experiment and run number.
     */
    bool load_run(std::string_view experiment,
                  unsigned run,
                  Config base_cfg) {
      std::string SIT_PSDM_DATA = std::getenv("SIT_PSDM_DATA");
      if (SIT_PSDM_DATA.empty()) {
        SIT_PSDM_DATA = "/sdf/data/lcls/ds";
      }
      std::string hutch { experiment.substr(0,3) };
      std::ostringstream dir_oss;
      dir_oss << SIT_PSDM_DATA << "/" << hutch << "/" << experiment << "/xtc";
      std::string xtc_dir = dir_oss.str();

      std::ostringstream oss;
      oss << experiment << "-r" << std::setw(4) << std::setfill('0') << run;
      std::string file_base_ptn = oss.str();
      for (auto const& dir_entry : fs::directory_iterator(xtc_dir)) {
        std::string xtc_path = dir_entry.path();
        if (xtc_path.find(file_base_ptn) != std::string::npos) {
          std::string xtc_stem = dir_entry.path().stem();
          std::string ext = dir_entry.path().extension().string();

          std::string smd_path;
          if (ext == ".xtc") {
            smd_path = xtc_dir + "/smalldata/" + xtc_stem + ".smd.xtc";
          } else {
            smd_path = xtc_dir + "/smalldata/" + xtc_stem + ".smd.xtc2";
          }

          Config stream_cfg = base_cfg;
          safe_strncpy(stream_cfg.smd_path, smd_path.c_str(), smd_path.size() + 1);
          safe_strncpy(stream_cfg.xtc_path, xtc_path.c_str(), xtc_path.size() + 1);

          // NOTE: add_data_stream increments m_num_data_streams
          add_data_stream(stream_cfg);
        }
      }

      return m_num_data_streams > 0;
    }

    // TODO: RECONSIDER WHERE INDEX_STREAM IS CALLED!!
    SBIO_HD inline IOStatus discover_metadata() {
      std::size_t steps_capacity { std::numeric_limits<std::size_t>::lowest() };
      for (std::size_t n_stream = 0; n_stream < m_num_data_streams; ++n_stream) {
        IOStatus status = m_data_streams[n_stream].prepare();

        // Abort if any stream fails
        if (status != IOStatus::Success) {
          return status;
        }

        status = m_data_streams[n_stream].index_stream();
        if (status != IOStatus::Success) {
          return status;
        }
        // Update our total steps capacity as the data stream brokers tell us
        std::size_t stream_steps = m_data_streams[n_stream].capacity();
        if (stream_steps > steps_capacity) {
          steps_capacity = stream_steps;
        }
      }

      m_steps_capacity = steps_capacity;

      return IOStatus::Success;
    }

    SBIO_HD inline typename FTraits::StepIdxType next() {
      using StepIdx = typename FTraits::StepIdxType;

      auto trigger_reindexing = [&] () {
        StepIdx total_capacity { std::numeric_limits<StepIdx>::lowest() };

        for (std::size_t n_stream = 0; n_stream < m_num_data_streams; ++n_stream) {
          IOStatus status = m_data_streams[n_stream].index_stream();

          if (status != IOStatus::Success) {
            return false;
          }

          StepIdx stream_capacity = m_data_streams[n_stream].capacity();

          if constexpr (FTraits::PartitioningStrategy ==
                        StreamPartitioningStrategy::Chronological) {
            total_capacity += stream_capacity;
          } else {
            if (stream_capacity > total_capacity) {
              total_capacity = stream_capacity;
            }
          }
        }

        m_steps_capacity += total_capacity;

        return total_capacity > 0;
      };

      return EPolicy::template next<FTraits>(m_steps_capacity, trigger_reindexing);
    }

    template <std::size_t MaxSegments = 128>
    SBIO_HD inline BrokerGroup<BrokerType, FTraits, MaxSegments>
    get_stream_group(const char* name) {
      using BrokerGroupType = BrokerGroup<BrokerType, FTraits, MaxSegments>;
      typename BrokerGroupType::DataSegmentRef segments[MaxSegments] {};

      std::uint32_t stream_indices[MaxSegments] {};

      std::size_t n_streams_found { 0 };

      // File formats may have various logical groupings of the data streams, which
      // in turn may require different lookup procedures. The pass loop tests them
      // in turn, as just given a name, it cannot be known which access pattern is needed.
      char final_type[FTraits::MaxNameSize] = "unknown";
      for (std::size_t pass = 0; pass < FTraits::DataAccessPtnCount; ++pass) {
        auto ptn = static_cast<typename FTraits::DataAccessPtn>(pass);
        for (std::size_t i = 0; i < m_num_data_streams && n_streams_found < MaxSegments; ++i) {
          char dettype[FTraits::MaxNameSize] = "unknown";
          n_streams_found += FTraits::find_detector_segments(m_data_streams[i].metadata(),
                                                             name,
                                                             &segments[n_streams_found],
                                                             MaxSegments - n_streams_found,
                                                             &m_data_streams[i],
                                                             dettype,
                                                             ptn);

          if constexpr (FTraits::PartitioningStrategy ==
                        StreamPartitioningStrategy::Chronological) {
            // In this case there will be 1 per stream, so on each iteration
            // collect the appropriate stream_idx to sort.
            stream_indices[n_streams_found - 1] =
              segments[n_streams_found - 1].broker->stream_idx();
          }

          if (std::strcmp(dettype, "unknown") != 0) {
            std::strcpy(final_type, dettype);
          }
        }
        // TODO: FIX SEARCH TO AVOID NAME COLLISIONS (E.G. "jungfrau" and PVs..)
        if (n_streams_found >= MaxSegments) {
          break;
        }
      }

      std::size_t num_segments { n_streams_found };

      // Some file formats will divide the data chronologically across streams
      // instead of sub-dividing a single logical unit for each time point. (e.g. XTC1)
      // The exposed stream_idx() returns a value that can be used to order the
      // the streams then. Use this to attach the correct ordering to the segment_no.
      // stream(s). The traits indicate whether they should forcibly be sequenced.
      if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
        if (num_segments > 0) {
          std::uint32_t final_stream_indices[MaxSegments] {};
          std::uint32_t smallest { stream_indices[0] };
          for (std::size_t j = 0; j < num_segments; ++j) {
            final_stream_indices[j] = j;
            if (stream_indices[j] < smallest) {
              smallest = stream_indices[j];
            }
          }

          // Sort with a circular wrapping accounted for in the fiducial.
          for (std::size_t i = 0; i < num_segments - 1; ++i) {
            std::uint32_t best { i };
            for (std::size_t j = i + 1; j < num_segments; ++j) {
              std::uint32_t diff_best { stream_indices[final_stream_indices[best]] - smallest };
              std::uint32_t diff_j { stream_indices[final_stream_indices[j]] - smallest };

              if (diff_j < diff_best) {
                best = j;
              }
            }

            if (best != i) {
              std::uint32_t tmp { final_stream_indices[i] };
              final_stream_indices[i] = final_stream_indices[best];
              final_stream_indices[best] = tmp;
            }
          }

          // Use the sorted stream indices as the "segment"
          // The broker will know that Chronological partitioning requires a different
          // interpretation of the segment numbering
          typename BrokerGroupType::DataSegmentRef sorted_segments[MaxSegments] {};
          for (std::size_t j = 0; j < num_segments; ++j) {
            sorted_segments[j] = segments[final_stream_indices[j]];
            sorted_segments[j].segment_no = 0;
            // segments[j].segment_no = final_stream_indices[j];
          }

          return BrokerGroupType(name, final_type, num_segments, sorted_segments);
        } else {
          return BrokerGroupType(name, final_type, num_segments, segments);
        }
      } else {
        return BrokerGroupType(name, final_type, num_segments, segments);
      }
    }

    SBIO_HD inline std::size_t num_data_streams() const { return m_num_data_streams; }

    SBIO_HD inline const BrokerType* data_streams() const { return m_data_streams[0]; }
    SBIO_HD inline BrokerType* data_streams() { return m_data_streams[0]; }
    SBIO_HD inline const BrokerType* data_stream(std::size_t i) const { return m_data_streams[i]; }
    SBIO_HD inline BrokerType* data_stream(std::size_t i) { return m_data_streams[i]; }

    template <class DS>
    class IteratorImpl {
    public:
      // Values generated on the fly so reference type is really value type
      using iterator_category = std::forward_iterator_tag; // One-direction
      using difference_type = std::ptrdiff_t;
      using value_type = typename FTraits::StepIdxType;
      // using pointer = value_type*;
      using pointer = void;
      using reference = value_type;

      IteratorImpl(DS& ds, value_type idx)
        : m_ds(ds)
        , m_idx(idx)
      {}

      // We're going to just generate indices and return by value for now.
      // The normal reference/pointer iterator semantics don't match perfectly
      // because there's nothing to "point to" or "reference".
      reference operator*() const { return m_idx; }

      // pointer operator->() { } // Not immediately sure how to do this

      // Incrementing the iterator calls the associated datasource's next function
      // This will trigger stream reindexing and so on.
      // You'll either get a new index. Or the ExhaustedSentinel if done.
      IteratorImpl& operator++() {
        m_idx = m_ds.next();
        return *this;
      }
      IteratorImpl operator++(int) {
        IteratorImpl tmp = *this;
        ++(*this);
        return tmp;
      }

      friend bool operator==(const IteratorImpl& a, const IteratorImpl& b) {
        // Need to figure out best way to compare DataSource
        // For now, just punt and return comparison of indices...
        return (a.m_idx == b.m_idx);
      }

      friend bool operator!=(const IteratorImpl& a, const IteratorImpl& b) {
        return !(a == b);
      }

    private:
      DS& m_ds;
      typename FTraits::StepIdxType m_idx;
    };

    using Iterator = IteratorImpl<DataSource>;
    using ConstIterator = IteratorImpl<const DataSource>;

    Iterator begin() { return Iterator(*this, 0); }
    Iterator end() { return Iterator(*this, FTraits::ExhaustedSentinel); }

    // ConstIterator needs some rework on the DataSource::next function to properly work
    // ConstIterator begin() const { return ConstIterator(*this, 0); }
    // ConstIterator end() const { return ConstIterator(*this, FTraits::ExhaustedSentinel); }

  private:
    BrokerType m_data_streams[MaxDataStreams];
    std::size_t m_num_data_streams { 0 };
    std::size_t m_steps_capacity { 0 };
  };
} // namespace sbio

#endif // SBIO_CORE_DATASOURCE_HH
