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

#ifndef SBIO_FORMATS_HDF5_HDF5_FORMAT_HH
#define SBIO_FORMATS_HDF5_HDF5_FORMAT_HH

#include "sbio/export_macro.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/formats/hdf5/vfd_driver.hh"
#include "sbio/locators/single_file.hh"
#include "sbio/util/parameters.hh"

#include <ncarray/dtype.hh>

#include <hdf5.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace sbio {
  namespace literals {
    SBIO_HD constexpr auto operator ""_dataset(const char* str, std::size_t) {
      return NamedArg<"dataset"> { str };
    }
  } // namespace literals

  /**
   * Mutex held for every call into libhdf5.
   *
   * Need special build for a reentrant/thread-safe call. For the time being,
   * sbio calls that need libhdf5 are done under lock. This only applies to discovery
   * and indexing - the fetches afterwards are done by sbio internal machinery.
   *
   * @note The function is statically defined in `hdf5_format.cc` - 1 instance.
   */
  SBIO_API std::mutex& hdf5_mutex();

  namespace hdf5 {
    inline constexpr std::size_t MaxStepFields { 64 };
    inline constexpr std::size_t MaxPathSize { 256 };
    inline constexpr std::uint64_t FieldAlign { 64 };
    inline constexpr std::uint64_t NoAddr { ~std::uint64_t { 0 } };

    enum class FieldClass : std::uint8_t {
      Config  = 0,
      Frames  = 1,
      PerStep = 2
    };

    /// One step-indexed dataset. Stored in Metadata buffer 1 by discovery.
    struct StepField {
      FixedName<MaxPathSize> path {};
      std::uint64_t row_bytes { 0 };  ///< Bytes of one step (one index along axis 0)
      std::uint32_t buf_offset { 0 }; ///< Offset of this field within one step of its kind
      std::uint8_t kind { 0 };        ///< StepKind
    };

    struct FieldTable {
      std::uint32_t num_fields { 0 };
      std::array<std::uint64_t, 2> step_bytes {}; ///< Bytes of one step, per kind, in its Data buffer
      std::array<StepField, MaxStepFields> fields {};
    };

    constexpr std::uint64_t align_up(std::uint64_t v) {
      return (v + FieldAlign - 1) / FieldAlign * FieldAlign;
    }

    /// Owning hid_t.
    class Hid {
    public:
      using Closer = herr_t (*)(hid_t);

      Hid() = default;
      Hid(hid_t id, Closer close)
        : m_id(id)
        , m_close(close)
      {}

      // No copy
      Hid(const Hid&) = delete;
      Hid& operator=(const Hid&) = delete;

      Hid(Hid&& o) noexcept
        : m_id(o.m_id)
        , m_close(o.m_close)
      {
        o.m_id = H5I_INVALID_HID;
      }

      Hid& operator=(Hid&& o) noexcept {
        if (this != &o) {
          reset();
          m_id = o.m_id;
          m_close = o.m_close;
          o.m_id = H5I_INVALID_HID;
        }
        return *this;
      }
      ~Hid() { reset(); }

      void reset() {
        if (m_id >= 0 && m_close != nullptr) {
          m_close(m_id);
        }
        m_id = H5I_INVALID_HID;
      }

      bool ok() const { return m_id >= 0; }
      operator hid_t() const { return m_id; }

    private:
      hid_t m_id { H5I_INVALID_HID };
      Closer m_close { nullptr };
    };

    /// Open a file through the sbio VFD, borrowing `io`. Caller holds hdf5_mutex().
    template <IOTraits IO>
    inline Hid open_file(const IO& io) {
      Hid fapl {
        HDF5VFDDriver::make_fapl(io),
        H5Pclose
      };

      if (!fapl.ok()) {
        return {};
      }

      // The VFD never uses the name (the IO comes through the fapl).
      // It only has to be unique per IO.
      char name[48];
      std::snprintf(name, sizeof(name), "sbio-vfd:%p", static_cast<const void*>(&io));

      return {
        H5Fopen(name, H5F_ACC_RDONLY, fapl),
        H5Fclose
      };
    }

    inline std::optional<ncarray::DType> to_dtype(hid_t type) {
      const H5T_class_t cls { H5Tget_class(type) };
      if (cls != H5T_INTEGER && cls != H5T_FLOAT) {
        return std::nullopt;
      }
      if (H5Tget_order(type) != H5Tget_order(H5T_NATIVE_INT)) {
        return std::nullopt; // Byte swapping not supported (yet)
      }

      const std::size_t size { H5Tget_size(type) };
      if (cls == H5T_FLOAT) {
        if (size == 4) {
          return ncarray::DType::float32;
        }

        if (size == 8) {
          return ncarray::DType::float64;
        }

        return std::nullopt;
      }

      const bool sgn { H5Tget_sign(type) == H5T_SGN_2 };
      switch (size) {
      case 1: {
        return sgn ? ncarray::DType::int8  : ncarray::DType::uint8;
      }
      case 2: {
        return sgn ? ncarray::DType::int16 : ncarray::DType::uint16;
      }
      case 4: {
        return sgn ? ncarray::DType::int32 : ncarray::DType::uint32;
      }
      case 8: {
        return sgn ? ncarray::DType::int64 : ncarray::DType::uint64;
      }
      default: {
        return std::nullopt;
      }
      }
    }

    struct DatasetInfo {
      ncarray::DType dtype { ncarray::DType::uint8 };
      int rank { 0 };
      std::array<hsize_t, H5S_MAX_RANK> dims {};
      std::uint64_t row_bytes { 0 }; ///< Bytes of one index along axis 0 (element size if rank 0)
      H5D_layout_t layout { H5D_LAYOUT_ERROR };
      std::array<hsize_t, H5S_MAX_RANK> chunk {};
      bool addressable { false };    ///< Rows are plain bytes at computable addresses
      std::uint64_t str_len { 0 }; ///< Zero for non-strings
    };

    /**
     * Check if dataset can be described.
     *
     * @returns false if dataset cannot be described (e.g. due to unsupported type).
     */
    inline bool inspect(hid_t dset, DatasetInfo& info) {
      Hid type {
        H5Dget_type(dset),
        H5Tclose
      };
      Hid space {
        H5Dget_space(dset),
        H5Sclose
      };
      Hid dcpl {
        H5Dget_create_plist(dset),
        H5Pclose
      };
      if (!type.ok() || !space.ok() || !dcpl.ok()) {
        return false;
      }

      // Variable strings not handled atm (are in HDF5 heap)
      const bool fixed_str {
        H5Tget_class(type) == H5T_STRING && H5Tis_variable_str(type) == 0
      };
      if (fixed_str) {
        info.dtype = ncarray::DType::uint8;
        info.str_len = H5Tget_size(type);
      } else {
        const auto dt { to_dtype(type) };
        if (!dt) {
          return false;
        }
        info.dtype = *dt;
      }

      info.rank = H5Sget_simple_extent_ndims(space);
      if (info.rank < 0                                                  ||
          H5Sget_simple_extent_dims(space, info.dims.data(), nullptr) < 0) {
        return false;
      }

      std::uint64_t row { H5Tget_size(type) };
      for (int dim = 1; dim < info.rank; ++dim) {
        row *= info.dims[dim];
      }
      info.row_bytes = row;
      // For strings append the length to the shape
      if (fixed_str) {
        info.dims[info.rank + 1] = info.str_len;
        info.rank += 1;
      }

      info.layout = H5Pget_layout(dcpl);

      // NOTE: HOOK PLACEMENT HERE
      //       For later -> Spec::decode is intended to handled filetered chunks
      //       with other layouts going to Spec::read_step (H5Dread as fallback).
      //       This is to provide support for compression (H5 plugins) and so on
      //       but none of this is implemented atm.
      const bool plain {
        H5Pget_nfilters(dcpl) == 0 && H5Pget_external_count(dcpl) == 0
      };

      bool whole_rows { false };
      if (info.layout == H5D_CONTIGUOUS) {
        whole_rows = true;
      } else if (info.layout == H5D_CHUNKED) {
        const int crank { H5Pget_chunk(dcpl, H5S_MAX_RANK, info.chunk.data()) };
        whole_rows = (crank == info.rank);

        for (int d = 1; whole_rows && d < info.rank; ++d) {
          whole_rows = (info.chunk[d] == info.dims[d]);
        }
      }

      info.addressable = plain && whole_rows && info.rank >= 1;

      return true;
    }

    /**
     * Get byte addresses of rows.
     *
     * Rows [first, first+count) are written out to out[i * stride].
     *
     * @note Rows in unallocated storage get NoAddr.
     */
    inline bool row_addresses(hid_t dset,
                              const DatasetInfo& info,
                              std::uint64_t first,
                              std::uint64_t count,
                              std::uint64_t* out,
                              std::size_t stride) {
      for (std::uint64_t i = 0; i < count; ++i) {
        out[i * stride] = NoAddr;
      }

      if (info.layout == H5D_CONTIGUOUS) {
        const haddr_t base { H5Dget_offset(dset) };
        if (base != HADDR_UNDEF) {
          for (std::uint64_t i = 0; i < count; ++i) {
            out[i * stride] = base + (first + i) * info.row_bytes;
          }
        }

        return true;
      }

      struct Ctx {
        std::uint64_t first;
        std::uint64_t count;
        std::uint64_t chunk_rows;
        std::uint64_t row_bytes;
        std::uint64_t* out;
        std::size_t stride;

        // HDF5 1.14 H5D_chunk_iter_op_t
        static int visit(const hsize_t* offset,
                         unsigned,
                         haddr_t addr,
                         hsize_t,
                         void* ptr) {
          auto& ctx { *static_cast<Ctx*>(ptr) };

          const std::uint64_t chunk0 { offset[0] };
          const std::uint64_t low_row { std::max<std::uint64_t>(chunk0, ctx.first) };
          const std::uint64_t high_row {
            std::min<std::uint64_t>(chunk0 + ctx.chunk_rows, ctx.first + ctx.count)
          };

          for (std::uint64_t row = low_row; row < high_row; ++row) {
            ctx.out[(row - ctx.first) * ctx.stride] =
              addr + (row - chunk0) * ctx.row_bytes;
          }

          return H5_ITER_CONT;
        }
      };

      Ctx ctx {
        first,
        count,
        info.chunk[0],
        info.row_bytes,
        out,
        stride
      };

      return H5Dchunk_iter(dset, H5P_DEFAULT, &Ctx::visit, &ctx) >= 0;
    }
  } // namespace hdf5

  /**
   * @brief What a specific HDF5-based format must provide.
   *
   * The HDF5Format is a base class implementing FormatTraits when using a HDF5Spec.
   *
   * The base class implements APIs needed for sbio. The HDF5Spec provides the specifics
   * to support an actual file format that works on top of HDF5 (e.g. NeXus, or CXI).
   *
   * The HDF5Spec must provide:
   *   - Name: A group type string for the inventory (e.g. "cxi")
   *   - FramesPath: This is the main step-indexed (detector) dataset
   *   - PerStepRoots: The groups walked for further per-step datasets.
   *                   The step axis equals the frame count.
   *   - StaticPaths: The datasets read whole at discovery as config fields.
   *                  Any missing datasets are skipped.
   */
  template <typename H5SpecT>
  concept HDF5Spec = requires {
    { H5SpecT::Name } -> std::convertible_to<const char*>;
    { H5SpecT::FramesPath } -> std::convertible_to<const char*>;
    { H5SpecT::PerStepRoots.size() } -> std::convertible_to<std::size_t>;
    { H5SpecT::StaticPaths.size() } -> std::convertible_to<std::size_t>;
  };

  template <HDF5Spec Spec>
  struct HDF5Format : public BaseTraits {
    using Self = HDF5Format<Spec>;
    using DefaultLocator = SingleFileLocator;

    static constexpr std::size_t HeaderSize { 0 };
    static constexpr std::uint16_t MaxRank { SBIO_MAX_NDIM };
    static constexpr std::uint16_t MaxNameSize { hdf5::MaxPathSize };

    struct DataUnit {}; ///< HDF5 steps are not self-describing units in the byte stream
    using StepIdxType = std::size_t;
    static constexpr StepIdxType ExhaustedSentinel { static_cast<StepIdxType>(-1) };

    static constexpr StreamPartitioningStrategy PartitioningStrategy {
      StreamPartitioningStrategy::SubDivide
    };

    static constexpr bool RediscoverPerChunk { true };

    struct DataStream
      : public StreamVariant<roles::Metadata, roles::Index, roles::Data> {};
    using StreamTypes = StreamSet<DataStream>;

    enum class DataAccessPtn : std::uint8_t {
      Frames = 0,
      PerStep = 1
    };
    static constexpr std::size_t DataAccessPtnCount { 2 };

    enum class StepKind : std::uint8_t {
      Frames = 0,
      PerStep = 1
    };
    static constexpr std::size_t NumStepKinds { 2 };

    static constexpr bool ResolvesOnDevice { false };

    struct StreamParameters {
      std::size_t config_bytes { 0x100000 };   ///< Space for all static (config) field values
      std::size_t per_step_bytes { 0x100000 }; ///< One PerStep step (all per-step fields)
      std::uint32_t max_step_fields { 32 };    ///< Sizes the Index buffer (<= hdf5::MaxStepFields)
    };

    struct IndexState {
      std::uint64_t next_step { 0 };
    };

    using BrokerBufferRequirements = RequirementsList<
      BufferDescriptor<roles::Metadata, 0>,                           // Config field values
      BufferDescriptor<roles::Metadata, 1, sizeof(hdf5::FieldTable)>, // Step field table
      BufferDescriptor<roles::Index, 0>,                              // Row addresses [step][field]
      BufferDescriptor<roles::Data, 0>,                               // Frames steps
      BufferDescriptor<roles::Data, 1>                                // PerStep steps
    >;

    using RequestSchema = sbio::NamedKeys<"dataset">;
    using DataRequest = sbio::DataRequest<RequestSchema>;
    using GroupKeys = sbio::NamedKeys<>;

    struct FieldMetadata {
      hdf5::FieldClass field_class { hdf5::FieldClass::Config };
      std::uint8_t slot { 0 }; ///< Index into the FieldTable (step fields only)
    };

    static constexpr StepKind kind_for_ptn(DataAccessPtn ptn) {
      return static_cast<StepKind>(ptn);
    }

    static AllocationRequest<Self> get_allocation_request(GenericStreamConfig<Self>& cfg) {
      const auto& params { cfg.format_params };
      const std::size_t batch { cfg.max_batch_size > 0 ? cfg.max_batch_size : 1 };
      const std::size_t num_fields {
        std::min<std::size_t>(params.max_step_fields, hdf5::MaxStepFields)
      };

      AllocationRequest<Self> request;
      request.size_requests[0] = params.config_bytes;
      request.size_requests[1] = sizeof(hdf5::FieldTable);
      request.size_requests[2] = cfg.index_batch_size * num_fields * sizeof(std::uint64_t);
      request.size_requests[3] = cfg.max_buffer_size * batch;
      request.size_requests[4] = params.per_step_bytes * batch;

      return request;
    }

    /// --- Metadata Discovery --- ///

    template <IOTraits IO, class StorageViewT>
    static IOStatus discover_metadata(Stream<IO, Self>* streams,
                                      StorageViewT& storage,
                                      MetadataInventory<Self>& inv) {
      std::lock_guard<std::mutex> lock { hdf5_mutex() };

      const auto& stream { get_stream<DataStream>(streams) };
      hdf5::Hid file { hdf5::open_file(stream) };
      if (!file.ok()) {
        return IOStatus::OpenFailed;
      }

      auto* table {
        static_cast<hdf5::FieldTable*>(storage.template acquire<
                                         roles::Metadata,
                                         1,
                                         ncarray::HostTag
                                       >(AcquireIntent::CallerMemorySpace))
      };

      auto* cfg_buf {
        static_cast<char*>(storage.template acquire<
                             roles::Metadata,
                             0,
                             ncarray::HostTag
                           >(AcquireIntent::CallerMemorySpace))
      };

      const std::size_t cfg_capacity { storage.template size<roles::Metadata, 0>() };
      std::size_t cfg_used { 0 };

      *table = hdf5::FieldTable {};

      // Frames
      hdf5::DatasetInfo frames {};
      IOStatus status { IOStatus::Success };
      {
        hdf5::Hid dset {
          H5Dopen2(file, Spec::FramesPath, H5P_DEFAULT),
          H5Dclose
        };

        if (!dset.ok() || !hdf5::inspect(dset, frames)) {
          status = IOStatus::HeaderReadError;
        } else if (!frames.addressable) {
          status = IOStatus::FunctionUnavailable; // Filtered/compact/virtual -- see HOOK in inspect
        } else {
          status =
            add_step_field(Spec::FramesPath, StepKind::Frames, frames, *table, inv);
        }
      }

      // Per-step datasets (deduplicated by object, frames excluded)
      if (status == IOStatus::Success) {
        std::vector<std::pair<std::string, H5O_token_t>> found;

        H5O_info2_t finfo;
        if (H5Oget_info_by_name3(file,
                                 Spec::FramesPath,
                                 &finfo,
                                 H5O_INFO_BASIC,
                                 H5P_DEFAULT) >= 0) {
          found.emplace_back(Spec::FramesPath, finfo.token);
        }
        const std::size_t first_candidate { found.size() };

        for (const char* root : Spec::PerStepRoots) {
          collect_datasets(file, root, found);
        }

        for (std::size_t i = first_candidate; i < found.size() && status == IOStatus::Success; ++i) {
          if (is_duplicate(file, found, i)) {
            continue;
          }

          const char* path { found[i].first.c_str() };
          hdf5::DatasetInfo info {};
          hdf5::Hid dset {
            H5Dopen2(file, path, H5P_DEFAULT),
            H5Dclose
          };

          if (!dset.ok() || !hdf5::inspect(dset, info)) {
            continue; // Unsupported type (strings, compounds, ...)
          }
          if (!info.addressable || info.dims[0] != frames.dims[0]) {
            continue; // Not stepped with the frames, or not addressable (See HOOK)
          }

          status = add_step_field(path, StepKind::PerStep, info, *table, inv);
        }
      }

      // Static (config) fields
      for (const char* path : Spec::StaticPaths) {
        if (status != IOStatus::Success) {
          break;
        }

        status = add_config_field(file, path, cfg_buf, cfg_capacity, cfg_used, inv);
      }

      storage.template release_written<roles::Metadata, 0>(cfg_buf, cfg_used);
      storage.template release_written<roles::Metadata, 1>(table,
                                                           sizeof(hdf5::FieldTable));

      inv.finalize();

      return status;
    }

    /// --- Step Offset Indexing --- ///

    template <IOTraits IO, class StorageViewT>
    static IOStatus index_stream(Stream<IO, Self>* streams,
                                 StorageViewT& storage,
                                 StreamCatalog<Self>& catalog,
                                 IndexingCursor<Self>& cursor,
                                 const GenericStreamConfig<Self>& cfg) {
      std::lock_guard<std::mutex> lock { hdf5_mutex() };

      const auto& stream { get_stream<DataStream>(streams) };
      hdf5::Hid file { hdf5::open_file(stream) };
      if (!file.ok()) {
        return IOStatus::OpenFailed;
      }

      const auto* table {
        static_cast<const hdf5::FieldTable*>(storage.template acquire<
                                               roles::Metadata,
                                               1,
                                               ncarray::HostTag
                                             >(AcquireIntent::CallerMemorySpace))
      };
      auto* addrs {
        static_cast<std::uint64_t*>(storage.template acquire<
                                      roles::Index,
                                      0,
                                      ncarray::HostTag
                                    >(AcquireIntent::CallerMemorySpace))
      };

      const std::size_t num_fields { table->num_fields };
      const std::size_t cap {
        num_fields > 0 ? storage.template size<roles::Index, 0>() / (num_fields * sizeof(std::uint64_t)) : 0
      };

      const std::uint64_t first { cursor.format.next_step };
      std::uint64_t count { std::min<std::uint64_t>(cap, cfg.index_batch_size) };
      std::uint64_t extent { ~std::uint64_t { 0 } };

      IOStatus status {
        num_fields > 0 ? IOStatus::Success : IOStatus::NoOffsetInData
      };

      for (std::size_t field_idx = 0; field_idx < num_fields && status == IOStatus::Success; ++field_idx) {
        hdf5::DatasetInfo info {};
        hdf5::Hid dset {
          H5Dopen2(file, table->fields[field_idx].path.c_str(), H5P_DEFAULT),
          H5Dclose
        };

        if (!dset.ok() || !hdf5::inspect(dset, info)) {
          status = IOStatus::HeaderReadError;
          break;
        }

        // Use current extent (in case the file has grown since discovery)
        extent = std::min<std::uint64_t>(extent, info.dims[0]);
        count = std::min<std::uint64_t>(count,
                                        info.dims[0] > first ? info.dims[0] - first : 0);

        if (!hdf5::row_addresses(dset,
                                 info,
                                 first,
                                 count,
                                 addrs + field_idx,
                                 num_fields)) {
          status = IOStatus::HeaderReadError;
        }
      }

      // Steps are usable only while every field's row is allocated
      std::uint64_t valid { 0 };
      if (status == IOStatus::Success) {
        for (; valid < count; ++valid) {
          bool all { true };
          for (std::size_t field_idx = 0; field_idx < num_fields && all; ++field_idx) {
            all = (addrs[valid * num_fields + field_idx] != hdf5::NoAddr);
          }

          if (!all) {
            break;
          }
        }
      }

      std::array<bool, NumStepKinds> has_kind {};
      for (std::size_t field_idx = 0; field_idx < num_fields; ++field_idx) {
        has_kind[table->fields[field_idx].kind] = true;
      }

      for (std::size_t kind_idx = 0; kind_idx < NumStepKinds; ++kind_idx) {
        catalog.num_steps[kind_idx] = has_kind[kind_idx] ? valid : 0;
        catalog.cummulative_steps[kind_idx] += catalog.num_steps[kind_idx];
      }

      catalog.index_epoch++;

      if (status == IOStatus::Success && first + valid >= extent) {
        catalog.chunk_complete = true;
      } else {
        catalog.chunk_complete = false;
      }

      cursor.format.next_step = first + valid;

      storage.template release_written<roles::Index, 0>(addrs,
                                                        valid * num_fields * sizeof(std::uint64_t));
      storage.template release<roles::Metadata, 1>(const_cast<hdf5::FieldTable*>(table));

      if (valid == 0) {
        return IOStatus::ZeroBytesRead;
      }

      return status;
    }

    /// --- Step Fetching (Reads) --- ///

    template <IOTraits IO, class StorageViewT>
    static IOStatus fetch_step(Stream<IO, Self>* streams,
                               StorageViewT& storage,
                               const StreamCatalog<Self>& catalog,
                               SegmentCursor<Self>& cursor,
                               const GenericStreamConfig<Self>&,
                               StepIdxType step_idx,
                               DataAccessPtn ptn) {
      return fetch_range(streams, storage, catalog, cursor, step_idx, 1, ptn);
    }

    template <IOTraits IO, class StorageViewT>
    static IOStatus fetch_multi_steps(Stream<IO, Self>* streams,
                                      StorageViewT& storage,
                                      const StreamCatalog<Self>& catalog,
                                      SegmentCursor<Self>& cursor,
                                      const GenericStreamConfig<Self>&,
                                      StepIdxType first,
                                      StepIdxType count,
                                      DataAccessPtn ptn) {
      return fetch_range(streams, storage, catalog, cursor, first, count, ptn);
    }

    /// --- Data Resolution --- ///

    template <class StorageViewT>
    static DataResult get_data_in_buffer(StorageViewT& storage,
                                         const MetadataInventory<Self>& inv,
                                         const DataRequest& req,
                                         PayloadOffsetCache& cache,
                                         DataAccessPtn ptn,
                                         std::size_t batch_idx = 0) {
      const auto* entry { inv.lookup(req) };
      if (entry == nullptr) {
        return {};
      }

      const auto cls { entry->descriptor.format_meta.field_class };
      if (cls == hdf5::FieldClass::Config) {
        void* cfg_buf {
          storage.template acquire<
            roles::Metadata,
            0,
            ncarray::HostTag
          >(AcquireIntent::CallerMemorySpace)
        };

        DataResult res { resolve_data(cfg_buf, inv, req, cache) };
        res.data = storage.template release<roles::Metadata, 0>(cfg_buf, res.data);

        return res;
      }

      if (ptn_for(cls) != ptn) {
        return {}; // The field lives in the other kind's buffer
      }

      const auto* table {
        static_cast<const hdf5::FieldTable*>(storage.template acquire<
                                               roles::Metadata,
                                               1,
                                               ncarray::HostTag
                                             >(AcquireIntent::CallerMemorySpace))
      };

      const std::uint64_t step_bytes {
        table->step_bytes[static_cast<std::size_t>(kind_for_ptn(ptn))]
      };
      storage.template release<roles::Metadata, 1>(const_cast<hdf5::FieldTable*>(table));

      // Fill the result for the pattern
      DataResult res {};
      auto resolve_for_pattern = [&](auto ptn_) {
        constexpr std::size_t Id { decltype(ptn_)::value };
        void* ptn_buf {
          storage.template acquire<
            roles::Data,
            Id,
            ViewMemTag
          >(AcquireIntent::CallerMemorySpace)
        };

        res = resolve_data(static_cast<char*>(ptn_buf) + batch_idx * step_bytes,
                           inv,
                           req,
                           cache);
        res.data = storage.template release<roles::Data, Id>(ptn_buf, res.data);
      };
      visit_ptn<Self>(ptn, resolve_for_pattern);

      return res;
    }

    /// `buf` is one step of the field's kind (or the config buffer for config fields).
    static DataResult resolve_data(void* buf,
                                   const MetadataInventory<Self>& inv,
                                   const DataRequest& req,
                                   PayloadOffsetCache&) {
      const auto* entry { inv.lookup(req) };
      if (entry == nullptr || buf == nullptr) {
        return {};
      }

      const auto& desc { entry->descriptor };
      DataResult res {};
      res.data = static_cast<const char*>(buf) + desc.payload_offset;
      res.dtype = desc.dtype;
      res.rank = desc.rank;

      std::size_t nelem { 1 };
      for (std::uint16_t dim = 0; dim < desc.rank; ++dim) {
        res.shape[dim] = desc.shape[dim];
        nelem *= desc.shape[dim];
      }
      res.size = nelem;

      return res;
    }

    /// Steps are not self-describing because sizes come from the FieldTable.
    static std::size_t get_payload_size(void*) { return 0; }

  private:
    static constexpr DataAccessPtn ptn_for(hdf5::FieldClass cls) {
      if (cls == hdf5::FieldClass::PerStep) {
        return DataAccessPtn::PerStep;
      }

      return DataAccessPtn::Frames;
    }

    static void register_field(MetadataInventory<Self>& inv,
                               const char* path,
                               DataAccessPtn ptn,
                               hdf5::FieldClass cls,
                               std::uint8_t slot,
                               ncarray::DType dtype,
                               int rank,
                               const hsize_t* dims,
                               std::uint32_t offset) {
      const std::string path_str { path };
      const auto pos { path_str.rfind('/') };
      const std::string group {
        (pos == 0 || pos == std::string::npos) ? std::string { "/" } : path_str.substr(0, pos)
      };
      const std::string field {
        path_str.substr(pos == std::string::npos ? 0 : pos + 1)
      };

      // Existing group keeps its pattern (config fields join step groups where they exist)
      const auto gid { inv.register_group(group.c_str(), Spec::Name, 0, ptn) };

      std::array<std::uint32_t, SBIO_MAX_NDIM> shape {};
      for (int dim = 0; dim < rank; ++dim) {
        shape[dim] = static_cast<std::uint32_t>(dims[dim]);
      }

      inv.add_field(gid,
                    offset,
                    dtype,
                    static_cast<std::uint16_t>(rank),
                    shape.data(),
                    FieldMetadata { cls, slot },
                    field.c_str());
    }

    static IOStatus add_step_field(const char* path,
                                   StepKind kind,
                                   const hdf5::DatasetInfo& info,
                                   hdf5::FieldTable& table,
                                   MetadataInventory<Self>& inv) {
      if (info.rank - 1 > static_cast<int>(SBIO_MAX_NDIM)) {
        return IOStatus::FunctionUnavailable;
      }
      if (table.num_fields >= hdf5::MaxStepFields) {
        return IOStatus::PayloadTruncatedError;
      }

      const auto kind_idx { static_cast<std::size_t>(kind) };
      const auto slot { static_cast<std::uint8_t>(table.num_fields++) };

      auto& field { table.fields[slot] };
      field.path = path;
      field.row_bytes = info.row_bytes;
      field.kind = static_cast<std::uint8_t>(kind);
      field.buf_offset = static_cast<std::uint32_t>(table.step_bytes[kind_idx]);
      table.step_bytes[kind_idx] =
        hdf5::align_up(table.step_bytes[kind_idx] + info.row_bytes);

      const auto cls {
        kind == StepKind::Frames ? hdf5::FieldClass::Frames : hdf5::FieldClass::PerStep
      };
      register_field(inv,
                     path,
                     static_cast<DataAccessPtn>(kind),
                     cls,
                     slot,
                     info.dtype,
                     info.rank - 1,
                     info.dims.data() + 1,
                     field.buf_offset);

      return IOStatus::Success;
    }

    static IOStatus add_config_field(hid_t file,
                                     const char* path,
                                     char* cfg_buf,
                                     std::size_t cap,
                                     std::size_t& used,
                                     MetadataInventory<Self>& inv) {
      if (H5Lexists(file, path, H5P_DEFAULT) <= 0) {
        return IOStatus::Success; // Optional in the format
      }

      hdf5::DatasetInfo info {};
      hdf5::Hid dset {
        H5Dopen2(file, path, H5P_DEFAULT),
        H5Dclose
      };

      if (!dset.ok() || !hdf5::inspect(dset, info)) {
        return IOStatus::Success; // Unsupported type: skipped
      }

      const std::uint64_t total {
        info.rank == 0 ? info.row_bytes : info.row_bytes * info.dims[0]
      };
      const std::uint64_t off { hdf5::align_up(used) };
      if (off + total > cap) {
        return IOStatus::PayloadTruncatedError;
      }

      // Native order already checked
      hdf5::Hid mtype {
        H5Dget_type(dset),
        H5Tclose
      };
      if (H5Dread(dset, mtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, cfg_buf + off) < 0) {
        return IOStatus::HeaderReadError;
      }
      used = static_cast<std::size_t>(off + total);

      register_field(inv,
                     path,
                     DataAccessPtn::PerStep,
                     hdf5::FieldClass::Config,
                     0,
                     info.dtype,
                     info.rank,
                     info.dims.data(),
                     static_cast<std::uint32_t>(off));

      return IOStatus::Success;
    }

    static void collect_datasets(hid_t file,
                                 const char* root,
                                 std::vector<std::pair<std::string, H5O_token_t>>& out) {
      if (H5Lexists(file, root, H5P_DEFAULT) <= 0) {
        return;
      }

      struct Visit {
        std::string root;
        std::vector<std::pair<std::string, H5O_token_t>>* out;

        static herr_t cb(hid_t grp, const char* name, const H5L_info2_t*, void* ptr) {
          auto& dset_visitor { *static_cast<Visit*>(ptr) };

          H5O_info2_t oinfo;
          if (H5Oget_info_by_name3(grp, name, &oinfo, H5O_INFO_BASIC, H5P_DEFAULT) < 0) {
            return 0; // Dangling link
          }

          if (oinfo.type == H5O_TYPE_DATASET) {
            std::string full_path;
            if (dset_visitor.root == "/") {
              full_path = dset_visitor.root;
            } else {
              full_path = dset_visitor.root + "/";
            }
            full_path += name;

            dset_visitor.out->emplace_back(full_path, oinfo.token);
          }

          return 0;
        }
      };

      Visit dset_visitor {
        root,
        &out
      };
      H5Lvisit_by_name2(file,
                        root,
                        H5_INDEX_NAME,
                        H5_ITER_INC,
                        &Visit::cb,
                        &dset_visitor,
                        H5P_DEFAULT);
    }

    static bool is_duplicate(hid_t file,
                             const std::vector<std::pair<std::string, H5O_token_t>>& found,
                             std::size_t i) {
      for (std::size_t j = 0; j < i; ++j) {
        int cmp { 1 };
        if (H5Otoken_cmp(file, &found[j].second, &found[i].second, &cmp) >= 0 &&
            cmp == 0) {
          return true;
        }
      }

      return false;
    }

    template <IOTraits IO, class StorageViewT>
    static IOStatus fetch_range(Stream<IO, Self>* streams,
                                StorageViewT& storage,
                                const StreamCatalog<Self>& catalog,
                                SegmentCursor<Self>& cursor,
                                StepIdxType first,
                                StepIdxType count,
                                DataAccessPtn ptn) {
      // All kinds advance in lockstep; Frames always has a field
      constexpr auto FrameKind { static_cast<std::size_t>(StepKind::Frames) };
      const std::uint64_t window_end { catalog.cummulative_steps[FrameKind] };
      const std::uint64_t window_first { window_end - catalog.num_steps[FrameKind] };
      if (count == 0 || first < window_first || first + count > window_end) {
        return IOStatus::NoOffsetInData;
      }

      const auto* table {
        static_cast<const hdf5::FieldTable*>(
          storage.template acquire<roles::Metadata, 1, ncarray::HostTag>(AcquireIntent::CallerMemorySpace))
      };
      const auto* addrs {
        static_cast<const std::uint64_t*>(
          storage.template acquire<roles::Index, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace))
      };

      const auto kind_idx { static_cast<std::size_t>(kind_for_ptn(ptn)) };
      const std::uint64_t step_bytes { table->step_bytes[kind_idx] };
      const std::size_t num_fields { table->num_fields };
      const std::uint64_t base { first - window_first };
      const auto& stream { get_stream<DataStream>(streams) };

      IOStatus status { step_bytes > 0 ? IOStatus::Success : IOStatus::NoOffsetInData };
      std::size_t total { 0 };
      ByteRegion last {};

      if (status == IOStatus::Success) {

        auto fetch_range_for_pattern = [&](auto ptn_) {
          constexpr std::size_t Id { decltype(ptn_)::value };
          if (count * step_bytes > storage.template size<roles::Data, Id>()) {
            status = IOStatus::PayloadTruncatedError;
            return;
          }

          auto* buf {
            static_cast<char*>(storage.template acquire<
                                 roles::Data,
                                 Id,
                                 ncarray::HostTag
                               >(AcquireIntent::CallerMemorySpace))
          };

          for (std::size_t field_idx = 0; field_idx < num_fields && status == IOStatus::Success; ++field_idx) {
            const auto& field { table->fields[field_idx] };
            if (field.kind != kind_idx) {
              continue;
            }

            // A field filling the whole step lays out exactly as on disk: merge
            // adjacent rows
            const bool packed { field.buf_offset == 0 && field.row_bytes == step_bytes };

            for (std::uint64_t step = 0; step < count && status == IOStatus::Success;) {
              const std::uint64_t addr { addrs[(base + step) * num_fields + field_idx] };
              std::uint64_t run { 1 };
              while (packed                                                                              &&
                     step + run < count                                                                  &&
                     addrs[(base + step + run) * num_fields + field_idx] == addr + run * field.row_bytes) {
                ++run;
              }

              const std::size_t len { static_cast<std::size_t>(run * field.row_bytes) };
              const auto res { stream.read_at(buf + step * step_bytes + field.buf_offset, addr, len) };
              if (res.status != IOStatus::Success) {
                status = res.status;
              } else if (res.bytes != len) {
                status = IOStatus::TruncatedRead;
              }

              last = ByteRegion { addr, len, 0 };
              step += run;
            }
          }

          total = static_cast<std::size_t>(count * step_bytes);
          storage.template release_written<roles::Data, Id>(buf, total);
        };
        visit_ptn<Self>(ptn, fetch_range_for_pattern);

        /*
        visit_ptn<Self>(ptn, [&](auto P) {
          constexpr std::size_t Id { decltype(P)::value };
          if (count * step_bytes > storage.template size<roles::Data, Id>()) {
            status = IOStatus::PayloadTruncatedError;
            return;
          }

          auto* buf {
            static_cast<char*>(
              storage.template acquire<roles::Data, Id, ncarray::HostTag>(AcquireIntent::CallerMemorySpace))
          };

          for (std::size_t field_idx = 0; field_idx < num_fields && status == IOStatus::Success; ++field_idx) {
            const auto& field { table->fields[field_idx] };
            if (field.kind != kind_idx) {
              continue;
            }

            // A field filling the whole step lays out exactly as on disk: merge adjacent rows
            const bool packed { field.buf_offset == 0 && field.row_bytes == step_bytes };

            for (std::uint64_t s = 0; s < count && status == IOStatus::Success;) {
              const std::uint64_t addr { addrs[(base + s) * num_fields + field_idx] };
              std::uint64_t run { 1 };
              while (packed && s + run < count &&
                     addrs[(base + s + run) * num_fields + field_idx] == addr + run * field.row_bytes) {
                ++run;
              }

              const std::size_t len { static_cast<std::size_t>(run * field.row_bytes) };
              const auto res { stream.read_at(buf + s * step_bytes + field.buf_offset, addr, len) };
              if (res.status != IOStatus::Success) {
                status = res.status;
              } else if (res.bytes != len) {
                status = IOStatus::TruncatedRead;
              }

              last = ByteRegion { addr, len, 0 };
              s += run;
            }
          }

          total = static_cast<std::size_t>(count * step_bytes);
          storage.template release_written<roles::Data, Id>(buf, total);
        });
        */
      }

      storage.template release<roles::Index, 0>(const_cast<std::uint64_t*>(addrs));
      storage.template release<roles::Metadata, 1>(const_cast<hdf5::FieldTable*>(table));

      cursor.read_count = total;
      cursor.last_region = last;

      return status;
    }
  };
} // namespace sbio

#endif // SBIO_FORMATS_HDF5_HDF5_FORMAT_HH
