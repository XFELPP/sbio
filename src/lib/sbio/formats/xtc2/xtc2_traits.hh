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

#ifndef SBIO_FORMATS_XTC2_XTC2_TRAITS_HH
#define SBIO_FORMATS_XTC2_XTC2_TRAITS_HH

#include "sbio/core/storage.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/formats/xtc2/traversal.hh"
#include "sbio/formats/xtc2/xtc2.hh"

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  struct XTC2Traits : public BaseTraits {
    // Sizeof dgram header
    // 4 bytes + 4 bytes + 4 bytes + sizeof(Xtc)
    // Xtc: 4 bytes (src) + 2 bytes (damage) + 2 (TypeId) + 4 bytes extent
    // Total: 12 + 12 = 24
    static constexpr std::size_t HeaderSize { 24 };
    static constexpr std::uint16_t MaxRank { 10 };
    static constexpr std::uint16_t MaxNameSize { 256 };

    // Using method 1 - data from a detector is sub-divided into portions, i.e.,
    // "segments", that are distributed across streams. To read the entire data at
    // any time point, you must coallesce the data from all streams.
    static constexpr StreamPartitioningStrategy PartitioningStrategy {
        StreamPartitioningStrategy::SubDivide
    };

    // For the types of streams/files
    enum Roles { SMD, BD };
    static constexpr std::size_t RoleCount { 2 };

    // Ways of looking up data inside of an XTC2 file for different detector kinds
    enum class DataAccessPtn : std::uint8_t {
      L1Accept = 0, // "Normal detectors"     - Look for data in L1Accept dgrams
      SlowUpdate,   // "EPICS Arch detectors" - Look for data in SlowUpdate dgrams
      BeginStep     // "Scan detectors"       - Look for data in BeginStep dgrams
    };
    static constexpr std::size_t DataAccessPtnCount { 3 };

    using DataUnit = XTC2::Dgram;
    using StepIdxType = std::size_t; // Unit type for indexing and selecting data units ("events")
    static constexpr StepIdxType ExhaustedSentinel { static_cast<StepIdxType>(-1) }; // Indicator all units read

    struct StreamParameters {
      char smd_path[MaxNameSize];
      char xtc_path[MaxNameSize];
      std::size_t max_dgram_size;
      std::size_t events_per_read;
    };

    struct EventOffset {
      std::uint64_t offset;
      std::uint64_t size;
    };

    struct TransitionOffset {
      std::uint64_t offset;
      std::uint64_t size;
      std::int64_t previous_l1_index;
      XTC2::TransitionId transition_id;
    };

    /**
     * Specify all the buffers that are required to implement a reader of XTC2.
     * We need to hold:
     * 1. Transition datagrams
     * 2. Event datagrams
     * 3. Event offsets
     * 4. Transition offsets
     *
     * The execution policy will determine how to fulfill allocation and synchronization
     * of these requested buffers.
     */
    using BrokerBufferRequirements = TypeList <
      BufferDescriptor<MetadataRole, 0, sizeof(XTC2::Dgram)>,  /* Buffer for transition */
      BufferDescriptor<MetadataRole, 1, sizeof(XTC2::Dgram)>,  /* Scratch buffer for SMD */
      BufferDescriptor<DataRole, 0, sizeof(XTC2::Dgram)>,      /* Buffer for events */
      BufferDescriptor<IndexRole, 0, sizeof(EventOffset)>,     /* EventOffsets buffer */
      BufferDescriptor<IndexRole, 1, sizeof(TransitionOffset)> /* TransitionOffsets buffer */
    >;

    using GroupBufferRequirements = TypeList<
      BufferDescriptor<GroupRole, 0, sizeof(XTC2::Dgram), Shareable> /* Buffer for constants */
    >;

    /**
     * A convenience struct to keep track of indices into offset buffers
     * and the total number of events and transitions seen.
     */
    struct DiscoveryState {
      ssize_t last_l1_idx_seen { -1 };
      std::uint64_t next_bd_offset { 0 };
      bool seen_end_run { false };
      std::size_t l1_offset_idx { 0 };
      std::size_t trans_offset_idx { 0 };
      std::size_t events_per_read { 0 };
    };

    struct DataRequest {
      DataRequest() = default;

      DataRequest(const DataRequest& other) = default;
      DataRequest& operator=(const DataRequest& other) = default;
      DataRequest(DataRequest&& other) noexcept = default;
      DataRequest& operator=(DataRequest&& other) noexcept = default;

      DataRequest(const char* name,
                  const char* type,
                  const char* alg,
                  const char* field) {
        std::strncpy(detector_name, name, MaxNameSize);
        std::strncpy(detector_type, type, MaxNameSize);
        std::strncpy(alg_name, alg, MaxNameSize);
        std::strncpy(field_name, field, MaxNameSize);
      }

      DataRequest(const char* name, const char* type) {
        std::strncpy(detector_name, name, MaxNameSize);
        std::strncpy(detector_type, type, MaxNameSize);
        std::strncpy(alg_name, "raw", MaxNameSize);
        std::strncpy(field_name, "raw", MaxNameSize);
      }

      char detector_type[MaxNameSize];
      char detector_name[MaxNameSize];
      std::uint32_t segment_number { 0 };
      char alg_name[MaxNameSize];
      char field_name[MaxNameSize];
    };

    // Defined below (larger)
    /**
     * The struct to hold maps for fast data lookup.
     * This should be flat and allow for O(1) retrieval of offsets into a
     * a datagram for a combination of (Detector, Algorithm, Field)
     */
    struct MetadataInventory;

    /**
     * The struct to be used to return results when data is requested.
     * This is defined in the XTC2 namespace and aliased here.
     * It has the structure:
     * struct DataResult {
     *   const void* data;           ///< Pointer to the raw data
     *   std::size_t size;           ///< Total bytes of payload
     *   std::uint16_t rank;         ///< Rank of data (num dimensions)
     *   const std::uint32_t* shape; ///< Shape of the data
     *   XTC2::DType dtype;          ///< Data type
     * };
     */
    using DataResult = typename XTC2::DataResult;

    // Defined at end of the header, after MetadataInventory is completed
    template <class DataBrokerType, class SegmentRef>
    SBIO_HD static std::size_t find_detector_segments(const MetadataInventory& inv,
                                                      const char* name,
                                                      SegmentRef* ref_out,
                                                      std::size_t max_out,
                                                      DataBrokerType* broker,
                                                      char* dettype = nullptr,
                                                      DataAccessPtn ptn = DataAccessPtn::L1Accept);

    SBIO_HD static inline std::size_t get_payload_size(void* buf) {
      return reinterpret_cast<XTC2::Dgram*>(buf)->xtc.sizeofPayload();
    }

    SBIO_HD static void discover_metadata(DataUnit* buffer,
                                          MetadataInventory& inv,
                                          std::size_t offset);

    SBIO_HD static DataResult resolve_data(void* buffer,
                                           const MetadataInventory& inv,
                                           const DataRequest& req);

    /**
     * Traverse a datagram to populate transition and event offsets.
     * The datagram should come from a .smd.xtc2 file. This should be called in a loop
     * if you want to load many offsets.
     * @param[in] dg The datagram to traverse
     * @param[in] state A reference to a state tracking variable to maintain a history
     *            of offsets and indices seen.
     * @param[in] beggining_offset A starting point.
     * @param[in] l1_offsets_buf The L1Accept offsets buffer - the state will tell us
     *            where to index into this buffer to record the new offset from dg
     * @param[in] transition_offsets_buf The transition offsets buffer - the state will
     *            tell us where to index into this buffer to record the new offset from dg
     * @param[in] access_ofsfet The access offset to be incremented.
     */
    SBIO_HD static std::size_t populate_offsets(DataUnit* dg,
                                                DiscoveryState& state,
                                                std::size_t beginning_offset,
                                                EventOffset* l1_offsets_buf,
                                                TransitionOffset* transition_offsets_buf,
                                                std::size_t access_offset = 0);
  };

  struct XTC2Traits::MetadataInventory {
    // Resolve a (Detector, Segment, Algorithm) into a NamesId for lookup
    struct DetAlgKey {
      char dettype[MaxNameSize];
      char detname[MaxNameSize];
      std::uint32_t segment;
      char algname[MaxNameSize];
      char detId[MaxNameSize];

      SBIO_HD inline bool operator<(const DetAlgKey& other) const {
        int cmp = std::strcmp(dettype, other.dettype);
        if (cmp != 0) {
          return cmp < 0;
        }

        cmp = std::strcmp(detname, other.detname);

        if (cmp != 0) {
          return cmp < 0;
        }

        if (segment != other.segment) {
          return segment < other.segment;
        }

        return std::strcmp(algname, other.algname) < 0;
      }
    };

    struct NamesIdMap {
      DetAlgKey key;
      std::uint32_t names_id;

      SBIO_HD inline bool operator<(const NamesIdMap& other) const {
        return key < other.key;
      }

      SBIO_HD inline bool operator<(const DetAlgKey& other) const {
        return key < other;
      }
    };

    // Resolve a (NamesId, FieldName) into a field index
    struct FieldKey {
      std::uint32_t names_id;
      char fieldname[XTC2Traits::MaxNameSize];

      SBIO_HD inline bool operator<(const FieldKey& other) const {
        if (names_id != other.names_id) {
          return names_id < other.names_id;
        }
        return std::strcmp(fieldname, other.fieldname) < 0;
      }
    };

    struct FieldMap {
      FieldKey key;
      std::uint32_t field_idx;

      SBIO_HD inline bool operator<(const FieldMap& other) const {
        return key < other.key;
      }

      SBIO_HD inline bool operator<(const FieldKey& other) const {
        return key < other;
      }
    };

    struct SdOffsetMap {
      std::uint32_t names_id;
      std::uint32_t offset;

      SBIO_HD SdOffsetMap(std::uint32_t nid, std::uint32_t off)
        : names_id(nid)
        , offset(off)
      {}

      SdOffsetMap() = default;

      SBIO_HD inline bool operator<(const SdOffsetMap& other) const {
        return names_id < other.names_id;
      }

      SBIO_HD inline bool operator<(std::uint32_t other_nid) const {
        return names_id < other_nid;
      }
    };

    struct SchemaOffset {
      std::uint32_t names_id;
      std::uint32_t start_idx; // Index into the flattened m_schemas array

      SBIO_HD inline bool operator<(const SchemaOffset& other) const {
        return names_id < other.names_id;
      }

      SBIO_HD inline bool operator<(std::uint32_t other_nid) const {
        return names_id < other_nid;
      }
    };

    // Storage: Usually allocated as a single memory block
    // during the Configure transition.
    NamesIdMap* m_names_id_table { nullptr };
    FieldMap* m_field_table { nullptr };
    SdOffsetMap* m_sd_offsets { nullptr };
    SchemaOffset* m_schema_offsets { nullptr };
    XTC2::Name* m_schemas { nullptr };

    std::size_t m_names_id_count { 0 };
    std::size_t m_field_count { 0 };
    std::size_t m_schema_offset_count { 0 };
    std::size_t m_sd_offset_count { 0 };

    // Lookup methods
    SBIO_HD std::uint32_t get_sd_offset(std::uint32_t nid) const;

    SBIO_HD const XTC2::Name* get_schema(std::uint32_t nid) const;

    SBIO_HD std::uint32_t resolve_names_id(const DataRequest& req) const;

    SBIO_HD std::uint32_t resolve_field_idx(std::uint32_t nid, const char* field) const;
  };

  template <class DataBrokerType, class SegmentRef>
  SBIO_HD inline std::size_t XTC2Traits::find_detector_segments(const XTC2Traits::MetadataInventory& inv,
                                                                const char* name,
                                                                SegmentRef* ref_out,
                                                                std::size_t max_out,
                                                                DataBrokerType* broker,
                                                                char* dettype,
                                                                XTC2Traits::DataAccessPtn ptn) {
    std::size_t n_found { 0 };

    if (ptn == DataAccessPtn::L1Accept) {
      for (std::size_t i = 0; i < inv.m_names_id_count && n_found < max_out; ++i) {
        if (std::strcmp(inv.m_names_id_table[i].key.detname, name) == 0) {
          std::uint32_t segment_no = inv.m_names_id_table[i].key.segment;

          // If the detector has multiple algorithms it will appear various times
          // So don't record it again
          bool duplicate { false };
          for (std::size_t j = 0; j < n_found; ++j) {
            if (ref_out[j].segment_no == segment_no) {
              duplicate = true;
              break;
            }
          }

          if (!duplicate) {
            ref_out[n_found++] = {
              broker,
              segment_no,
              ptn
            };

            // If provided, populate the detector type as well
            const char* dettype_ = inv.m_names_id_table[i].key.dettype;
            std::size_t k { 0 };

            for (; k < XTC2Traits::MaxNameSize - 1 &&  dettype_[k] != '\0'; ++k) {
              dettype[k] = dettype_[k];
            }
            dettype[k] = '\0';
          }
        }
      }

      if (n_found > 0) {
        return n_found;
      }
    } else if (ptn == DataAccessPtn::SlowUpdate) {
      // Continue on to check for EPICS detectors
      // All EPICS (ie EPICSArch) detectors are under the `epics` name
      for (std::size_t i = 0; i < inv.m_names_id_count && n_found < max_out; ++i) {
        if (std::strcmp(inv.m_names_id_table[i].key.detname, "epics") == 0) {
          std::uint32_t nid = inv.m_names_id_table[i].names_id;
          if (inv.resolve_field_idx(nid, name) != 0xFFFFFFFF) {
            ref_out[n_found++] = {
              broker,
              inv.m_names_id_table[i].key.segment,
              DataAccessPtn::SlowUpdate
            };
          }
        }
      }

      if (n_found > 0) {
        return n_found;
      }
    } else {
      // NOTE: The `scan` detector behaves much like the normal detectors, but
      // has the data in BeginStep buffers, instead of L1Accept buffers.
      // It can always be access via `scan` detector above. However, for a syntactic
      // sugar, like with EPICS above, we'll allow detectors to be created based on
      // the scan variable name directly.
      // Normally, the scan will have a single algorithm, with these fields:
      // - `step_value`     : INT64
      // - `step_docstring` : CHARSTR, optional (but usually present)
      // - `scan_var_xxx`   : ANY (the actual scanned variable - may have multiple)
      // So we'll match the scan_var_names as we did above with EPICS
      for (std::size_t i = 0; i < inv.m_names_id_count && n_found < max_out; ++i) {
        if (std::strcmp(inv.m_names_id_table[i].key.detname, "scan") == 0) {
          std::uint32_t nid = inv.m_names_id_table[i].names_id;
          if (inv.resolve_field_idx(nid, name) != 0xFFFFFFFF) {
            ref_out[n_found++] = {
              broker,
              inv.m_names_id_table[i].key.segment,
              DataAccessPtn::BeginStep
            };
          }
        }
      }
    }
    return n_found;
  }
} // namespace sbio

#endif // SBIO_FORMATS_XTC2_XTC2_TRAITS_HH
