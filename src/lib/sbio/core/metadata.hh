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

#ifndef SBIO_CORE_METADATA_HH
#define SBIO_CORE_METADATA_HH

#include "sbio/core/request.hh"
#include "sbio/util/parameters.hh"

#include <ncarray/dtype.hh>

#ifdef __CUDACC__

#include <cuda/std/array>
#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/cstring>
#include <cuda/std/inplace_vector> // Use inplace_vector if CUDA available
#include <cuda/std/utility>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>                  // Use vector for host-only (inplace_vector is C++26)

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

#ifndef SBIO_MAX_NDIM
#define SBIO_MAX_NDIM NCARRAY_MAX_NDIM // Currently, 10
#endif

namespace sbio {
  namespace impl {
    /**
     * @brief A simplified lower_bound for use with the flat sbio inventory vectors.
     *
     * @note This is provided as a host/device compatible lower_bound as a CCCL
     *       version is not available. Use std::lower_bound wherever possible. This
     *       implementation only makes sense for the very simple use-case of the
     *       MetadataInventory containers.
     *
     * @note As with the std::lower_bound, this function is undefined if the input
     *       is not sorted.
     *
     * @tparam Container The container type - usually cuda::std::inplace_vector or std::vector.
     * @tparam Key The Key type for lower_bound comparison.
     * @param[in] vec The vector to find the lower_bound for.
     * @param[in] key The key to use to determine the lower_bound.
     * @returns The iterator at the lower bound (or vec.end()).
     */
    template <typename Container, typename Key>
    SBIO_HD inline auto hd_lower_bound(const Container& vec, const Key& key) {
      hd_std::size_t low { 0 };
      hd_std::size_t high { vec.size() };
      while (low < high) {
        hd_std::size_t mid { low + (high - low) / 2 };
        if (vec[mid] < key) {
          low = mid + 1;
        } else {
          high = mid;
        }
      }

      return vec.begin() + low;
    }

    /**
     * @brief A simplified insertion sort for use with the flat sbio inventory vectors.
     *
     * @note This is provided as a host/device compatible sort as a CCCL version
     *       is not available. Use std::sort wherever possible. This
     *       implementation only makes sense for the very simple use-case of the
     *       MetadataInventory containers.
     *
     * @tparam Container The container type - usually cuda::std::inplace_vector or std::vector.
     * @param[in/out] vec The vector to inplace sort.
     */
    template <typename Container>
    SBIO_HD inline void hd_sort(Container& vec) {
      hd_std::size_t n { vec.size() };
      for (hd_std::size_t i = 1; i < n; ++i) {
        auto temp { vec[i] };

        hd_std::size_t j { i };
        while (j > 0 && temp < vec[j - 1]) {
          vec[j] = vec[j - 1];
          --j;
        }

        vec[j] = temp;
      }
    }
  } // namespace impl

  /**
   * @brief The MetadataInventory provides a lookup of a StreamBroker's fields.
   *
   * The inventory stores the names, types, and segment identifiers of discovered
   * groups. It additionally associates these group segments to any data fields
   * that they may have. The field lookup and association is data-format-specific
   * and uses the same mechanisms to generalize this as the DataRequest - the
   * DataRequest is in fact used to query the lookup tables.
   *
   * The inventory has a two-stage life-cycle. During the discovery stage of the
   * StreamBroker's state machine, it is filled dynamically - i.e., there is an
   * upper-bound on the capacity, but the size grows during discovery.
   * Once the discovery phase completes, the inventory is "baked", or finalized,
   * which consists of sorting the constructed lookups for fast access.
   *
   * The inventory maintains epoch counters to help establish if the stored data
   * is changing over time - e.g., if fields associated to a group segment may
   * return variably sized data, then offsets cannot be used directly if stale.
   * These counters can help inform decisions about crossing memory space boundaries
   * (e.g. performing GPU<->CPU copies) and similar activities.
   *
   * @tparam FTraits The data format to create the inventory for.
   * @tparam MaxGroups The maximum capacity for the number of groups (BrokerGroups).
   * @tparam MaxFields The maximum capacity for the number of fields.
   */
  template <typename FTraits, hd_std::size_t MaxGroups = 128, hd_std::size_t MaxFields = 1024>
  struct SBIO_API MetadataInventory {
    using RequestSchema = typename FTraits::RequestSchema;
    using DataRequest = sbio::DataRequest<RequestSchema>;

    static constexpr hd_std::size_t KeyCount { RequestSchema::count };

    using DataAccessPtn = typename FTraits::DataAccessPtn;

    using GroupKeys = typename FTraits::GroupKeys;

    // Since we cannot anticipate how the data-format needs to perform traversal
    // in addition to the general/generic lookup table, formats are allowed to
    // define some additional pieces of data to attach to the inventory
    using FormatMetadata = typename FTraits::FieldMetadata;

    /**
     * @brief A sortable group segment descriptor key.
     */
    struct GroupKey {
      FixedName<> group_name;
      FixedName<> group_type;
      hd_std::uint32_t segment { 0 };

      hd_std::array<FixedName<>, GroupKeys::count> extra_metadata {};

      template <FixedName Key>
      SBIO_HD inline const FixedName<>& get() const {
        constexpr hd_std::size_t idx { GroupKeys::template index_of<Key>() };
        static_assert(idx < GroupKeys::count, "Key not found!");

        return extra_metadata[idx];
      }

      template <FixedName Key>
      SBIO_HD inline void set(const char* val) {
        constexpr hd_std::size_t idx { GroupKeys::template index_of<Key>() };
        static_assert(idx < GroupKeys::count, "Key not found!");
        extra_metadata[idx] = val;
      }

      SBIO_HD inline FixedName<>& operator[](const char* key_name) {
        for (hd_std::size_t i = 0; i < GroupKeys::count; ++i) {
          if (GroupKeys::keys[i] == key_name) {
            return extra_metadata[i];
          }
        }
        static FixedName<> fallback{};
        return fallback; // Fallback option?
      }

      SBIO_HD inline const FixedName<>& operator[](const char* key_name) const {
        for (hd_std::size_t i = 0; i < GroupKeys::count; ++i) {
          if (GroupKeys::keys[i] == key_name) {
            return extra_metadata[i];
          }
        }
        static FixedName<> fallback{};
        return fallback; // Fallback option?
      }

      SBIO_HD inline bool operator<(const GroupKey& other) const {
        if (group_type != other.group_type) {
          return group_type < other.group_type;
        }

        if (group_name != other.group_name) {
          return group_name < other.group_name;
        }

        return segment < other.segment;
      }
    };

    /**
     * @brief The GroupEntry associates a key (via an index) to a group segment.
     */
    struct GroupEntry {
      GroupKey key;                     ///< Lookup key
      hd_std::uint32_t group_id { 0 };  ///< Group index
      // TODO: Try to get rid of access_ptn here if possible... bit of a hack...
      hd_std::uint8_t access_ptn { 0 }; ///< Format's data access pattern (cast to uint)
      hd_std::uint64_t epoch { 0 };     ///< Epoch counter if the group's field descriptors change

      SBIO_HD inline bool operator<(const GroupEntry& other) const { return key < other.key; }
      SBIO_HD inline bool operator<(const GroupKey& k) const { return key < k; }
    };

    struct FieldKey {
      hd_std::uint32_t group_id { 0 };
      hd_std::array<FixedName<>, KeyCount> keys {};

      SBIO_HD inline bool operator<(const FieldKey& other) const {
        if (group_id != other.group_id) {
          return group_id < other.group_id;
        }

        for (hd_std::size_t i = 0; i < KeyCount; ++i) {
          if (keys[i] != other.keys[i]) {
            return keys[i] < other.keys[i];
          }
        }

        return false;
      }
    };

    /**
     * @brief The FieldDescriptor describes how to retrieve and interpret a field.
     */
    struct FieldDescriptor {
      mutable hd_std::uint32_t payload_offset { 0 };   ///< The byte offset into a `DataUnit` to find the field
      ncarray::DType dtype;                            ///< The datatype of the field's data.
      hd_std::uint16_t rank { 0 };                     ///< The number of dimensions of the field's data.
      mutable hd_std::uint32_t shape[SBIO_MAX_NDIM]{}; ///< The full shape of the field's dimensions.
      hd_std::uint32_t field_version { 0 };            ///< The version counter (if the descriptor changes)
      bool is_config { false };                        ///< If the descriptor is static at configuration

      // TODO: Try to get rid of this if possible... bit of a hack...
      FormatMetadata format_meta {};                   ///< If needed, any format-specific metadata for the field
    };

    /**
     * @brief The FieldEntry associates an index to the descriptor of the field's data.
     */
    struct FieldEntry {
      FieldKey key;               ///< Lookup key/index
      FieldDescriptor descriptor; ///< Descriptor of the field's data.

      SBIO_HD inline bool operator<(const FieldEntry& other) const { return key < other.key; }
      SBIO_HD inline bool operator<(const FieldKey& k) const { return key < k; }
    };

#ifdef __CUDACC__
    hd_std::inplace_vector<GroupEntry, MaxGroups> m_groups;
    hd_std::inplace_vector<FieldEntry, MaxFields> m_fields;
#else
    // For host-only code, inplace_vector is unfortunately not available
    // until C++26 so use normal vectors
    std::vector<GroupEntry> m_groups;
    std::vector<FieldEntry> m_fields;
#endif

    /**
     * @brief Access the registered groups directly.
     *
     * @returns All registered groups.
     */
    SBIO_HD const auto& groups() const { return m_groups; }

    /**
     * @brief Access the registered fields directly.
     *
     * @returns All registered fields.
     */
    SBIO_HD const auto& fields() const { return m_fields; }

    /**
     * @brief Add a new group segment into the inventory.
     *
     * This function only adds the group segment's entry. It does not associate any
     * fields to that segment. If that is needed, see `add_field` below as well.
     *
     * @tparam GroupKeyStringArgs The grouping of metadata key names.
     * @param[in] name The name of the group.
     * @param[in] type The group type.
     * @param[in] segment The segment number associated to the entry.
     * @param[in] ptn The DataAccessPtn for the lookup strategy for the group segment.
     * @param[in] grp_meta The values for any additional metadata keys defined by the format.
     * @returns The entry of the group segment in the inventory. This can be used to
     *          further inventory associated fields as needed.
     */
    template <typename... GroupKeyStringArgs>
    requires (sizeof...(GroupKeyStringArgs) == GroupKeys::count)
    SBIO_HD hd_std::uint32_t register_group(const char* name,
                                            const char* type,
                                            hd_std::uint32_t segment,
                                            DataAccessPtn ptn,
                                            GroupKeyStringArgs... grp_meta) {
      for (hd_std::size_t i = 0; i < m_groups.size(); ++i) {
        if (m_groups[i].key.segment == segment &&
            m_groups[i].key.group_name == name &&
            m_groups[i].key.group_type == type) {
          return m_groups[i].group_id;
        }
      }

      hd_std::uint32_t id { static_cast<hd_std::uint32_t>(m_groups.size()) };

      GroupEntry entry;
      entry.key.group_name = name;
      entry.key.group_type = type;
      const char* grp_ptrs[] = { static_cast<const char*>(grp_meta)... };
      for (hd_std::size_t k = 0; k < GroupKeys::count; ++k) {
        entry.key.extra_metadata[k] = grp_ptrs[k];
      }

      entry.key.segment = segment;
      entry.group_id = id;
      entry.access_ptn = static_cast<hd_std::uint8_t>(ptn);
      entry.epoch = 0;
      m_groups.push_back(entry);

      return id;
    }

    /**
     * @brief Add a new group segment into the inventory.
     *
     * This function only adds the group segment's entry. It does not associate any
     * fields to that segment. If that is needed, see `add_field` below as well.
     *
     * @tparam GroupKeyStringArgs The grouping of metadata key names.
     * @param[in] name The name of the group.
     * @param[in] type The group type.
     * @param[in] segment The segment number associated to the entry.
     * @param[in] ptn The DataAccessPtn for the lookup strategy for the group segment.
     * @param[in] grp_meta The values for any additional metadata keys defined by the format.
     * @returns The entry of the group segment in the inventory. This can be used to
     *          further inventory associated fields as needed.
     */
    template <typename... GroupKeyStringArgs>
    requires (sizeof...(GroupKeyStringArgs) == GroupKeys::count)
    SBIO_HD hd_std::uint32_t register_group_alias(const char* alias_name,
                                                  const char* parent_type,
                                                  hd_std::uint32_t segment,
                                                  DataAccessPtn ptn,
                                                  GroupKeyStringArgs... grp_meta) {
      return register_group(alias_name,
                            parent_type,
                            segment,
                            ptn,
                            hd_std::forward<GroupKeyStringArgs>(grp_meta)...);
    }

    /**
     * @brief Add a new field into the inventory.
     *
     * @tparam KeyStringArgs... The format's schema values.
     * @param[in] group_id The group index to which the field belongs.
     * @param[in] offset The byte offset to use to read this field.
     * @param[in] dtype The datatype of the field's data.
     * @param[in] rank The total number of dimensions (rank) of the field's data.
     * @param[in] shape The full shape of the field's data.
     * @param[in] fmt_meta Any format-specific metadata required. This may be an empty
     *            struct.
     * @param[in] key_values... The format-specific keys associated to the field.
     *            Different formats have a different schema for lookup. There must
     *            be 1 key proivded for each of the entries in the format's schema.
     */
    template <typename... KeyStringArgs>
    requires (sizeof...(KeyStringArgs) == KeyCount)
    SBIO_HD void add_field(hd_std::uint32_t group_id,
                           hd_std::uint32_t offset,
                           ncarray::DType dtype,
                           hd_std::uint16_t rank,
                           const hd_std::uint32_t* shape,
                           FormatMetadata fmt_meta,
                           KeyStringArgs... key_values) {
      FieldEntry entry;
      entry.key.group_id = group_id;
      entry.descriptor.payload_offset = offset;
      entry.descriptor.dtype = dtype;
      entry.descriptor.rank = rank;
      entry.descriptor.format_meta = fmt_meta;

      for (hd_std::uint16_t r = 0; r < rank; ++r) {
        entry.descriptor.shape[r] = shape[r];
      }

      const char* key_ptrs[] = { static_cast<const char*>(key_values)... };
      for (hd_std::size_t k = 0; k < KeyCount; ++k) {
        entry.key.keys[k] = key_ptrs[k];
      }

      m_fields.push_back(entry);
    }

    /**
     * @brief Sort and finalize metadata after the discovery phase is completed.
     *
     * Containers are sorted after the discovery is complete to facilitate faster
     * traversal and lookup.
     */
    SBIO_HD void finalize() {
      impl::hd_sort(m_groups);
      impl::hd_sort(m_fields);
    }

    /**
     * @brief Return a descriptor for the requested data.
     *
     * For a given request, the associated descriptor points to a location in a
     * `DataUnit` read, and the appropriate size, shape, and type information to
     * interpret the pointed to payload.
     *
     * @param[in] req The DataRequest with any format-specific fields included.
     * @returns The entry with key (for indices), and data descriptor to find
     *          the requested data.
     */
    SBIO_HD inline const FieldEntry* lookup(const DataRequest& req) const {
      GroupKey gkey;
      gkey.group_name = req.group_name;
      gkey.group_type = req.group_type;
      gkey.segment = req.segment_number;

      auto g_it { impl::hd_lower_bound(m_groups, gkey) };
      if (g_it == m_groups.end() || (gkey < g_it->key)) {
        gkey.segment = 0;
        g_it = impl::hd_lower_bound(m_groups, gkey);

        if (g_it == m_groups.end() || (gkey < g_it->key)) {
          return nullptr;
        }
      }

      FieldKey fkey;
      fkey.group_id = g_it->group_id;
      for (hd_std::size_t i = 0; i < KeyCount; ++i) {
        fkey.keys[i] = req.field_values[i];
      }

      auto f_it { impl::hd_lower_bound(m_fields, fkey) };
      if (f_it != m_fields.end() && !(fkey < f_it->key)) {
        auto& entry { *f_it };

        return &entry;
      }

      return nullptr;
    }

    /**
     * @brief For the specified group, retrieve the epoch counter for its descriptors.
     *
     * The epoch is incremented when its determined that the descriptor data has
     * changed. If the data format does not change over the course of the read
     * steps, then the value will always be 0 for the epoch. If it does, for instance
     * if the returned payloads may change in size, then the epoch will be incremented
     * accordingly.
     *
     * @param[in] group_id The index for the group to retrieve the epoch for.
     * @returns The epoch counter.
     */
    SBIO_HD inline hd_std::uint64_t get_epoch(hd_std::uint32_t group_id) const {
      for (hd_std::size_t i = 0; i < m_groups.size(); ++i) {
        if (m_groups[i].group_id == group_id) {
          return m_groups[i].epoch;
        }
      }

      return 0;
    }

    /**
     * @brief Return the total number of groups inventoried.
     *
     * @returns The total number of groups inventoried.
     */
    SBIO_HD hd_std::size_t num_entries() const { return m_groups.size(); }

    /**
     * @brief Query whether a specific entry index matches a name/access pattern.
     *
     * @param[in] entry_no The entry index.
     * @param[in] name_query The group name to test for.
     * @param[in] ptn The access pattern to test for.
     * @returns Whether the query matches.
     */
    SBIO_HD bool entry_matches(hd_std::size_t entry_no,
                               const char* name_query,
                               DataAccessPtn ptn) const {
      if constexpr (requires { FTraits::entry_matches(*this, entry_no, name_query, ptn); }) {
        // Will try to set it up so that the generic inventory doesn't require ptn
        // But... leave this escape hatch in case we cant cover all cases/oddities
        return FTraits::entry_matches(*this, entry_no, name_query, ptn);
      } else {
        if (entry_no >= m_groups.size()) {
          return false;
        }

        return
          (hd_std::strcmp(name_query, "*") == 0) ||
          m_groups[entry_no].key.group_name == name_query;
      }
    }

    /**
     * @brief Retrieve the group name and segment number for an entry index.
     *
     * @param[in] entry_no The entry index.
     * @returns The group name, segment number pair for the provided index.
     */
    SBIO_HD auto metadata_for(hd_std::size_t entry_no) const {
      if (entry_no >= m_groups.size()) {
        return hd_std::make_pair(FixedName<>{}, static_cast<hd_std::uint32_t>(0));
      }

      return hd_std::make_pair(m_groups[entry_no].key.group_type,
                               m_groups[entry_no].key.segment);
    }
  };
} // namespace sbio

#endif // SBIO_CORE_METADATA_HH
