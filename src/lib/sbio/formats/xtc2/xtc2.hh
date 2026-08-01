#ifndef SBIO_FORMATS_XTC2_XTC2_HH
#define SBIO_FORMATS_XTC2_XTC2_HH

#include "sbio/export_macro.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/util/string.hh"

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  namespace XTC2 {
    constexpr auto MaxRank = 10;
    constexpr auto MaxNameSize = 256;

    enum class DType {
      UINT8,
      UINT16,
      UINT32,
      UINT64,
      INT8,
      INT16,
      INT32,
      INT64,
      FLOAT,
      DOUBLE,
      CHARSTR,
      ENUMVAL,
      ENUMDICT
    };

    /**
     * A small struct that returns all necessary information to access a payload
     * in a datagram. When a detector/segment/algorithm/field combination is
     * specified, the return type will be this struct which provides the pointer
     * to the data, the shape and size of the payload, and the data type.
     */
    struct DataResult {
      const void* data;                ///< Pointer to the raw data
      std::size_t size { 0 };          ///< Total bytes of payload
      std::uint16_t rank { 0 };        ///< Rank of data (num dimensions)
      std::uint32_t shape[MaxRank] {}; ///< Shape of the data
      DType dtype;                     ///< Data type
    };

    enum class TransitionId {
      ClearReadout,
      Reset,
      Configure,
      Unconfigure,
      BeginRun,
      EndRun,
      BeginStep,
      EndStep,
      Enable,
      Disable,
      SlowUpdate,
      Unused_11,
      L1Accept = 12, // Must be 12 to agree with firmware
      NumberOf
    };

    struct SBIO_API TypeId {
      enum Type {
        Parent,
        ShapesData,
        Shapes,
        Data,
        Names,
        NumberOf
      };

      TypeId() {}
      TypeId(Type type, unsigned version)
        : m_value(((version << VersionBitShift) & VersionBitMask) | type)
      {}

      SBIO_HD inline unsigned value() const { return m_value; }

      SBIO_HD inline unsigned version() const {
        return (m_value & VersionBitMask) >> VersionBitShift;
      }

      SBIO_HD inline Type id() const {
        return static_cast<Type>(m_value & TypeBitMask);
      }

    private:
      enum { TypeBitMask = 0x0FFF };
      enum { VersionBitMask = 0xF000, VersionBitShift = 12 };

      std::uint16_t m_value;
    };

    // --- Helper arrays to to convert enums to strings --- //
    static constexpr std::array<char[10], 13> DTypeString {
      "UINT8",
      "UINT16",
      "UINT32",
      "UINT64",
      "INT8",
      "INT16",
      "INT32",
      "INT64",
      "FLOAT",
      "DOUBLE",
      "CHARSTR",
      "ENUMVAL",
      "ENUMDICT"
    };

    static constexpr std::array<std::uint8_t, 13> DTypeSize {
        sizeof(std::uint8_t),
        sizeof(std::uint16_t),
        sizeof(std::uint32_t),
        sizeof(std::uint64_t),
        sizeof(std::int8_t),
        sizeof(std::int16_t),
        sizeof(std::int32_t),
        sizeof(std::int64_t),
        sizeof(float),
        sizeof(double),
        sizeof(std::int32_t), // ENUMVAL
        sizeof(std::int32_t)  // ENUMDICT
    };

    static constexpr std::array<char[13], 14> TransitionIdString {
        "ClearReadout",
        "Reset",
        "Configure",
        "Unconfigure",
        "BeginRun",
        "EndRun",
        "BeginStep",
        "EndStep",
        "Enable",
        "Disable",
        "SlowUpdate",
        "Unused_11",
        "L1Accept",
        "NumberOf"
    };

    static constexpr std::array<char[11], 6> TypeIdString {
      "Parent",
      "ShapesData",
      "Shapes",
      "Data",
      "Names",
      "NumberOf"
    };

    // --- Struct wrappers for XTC2 --- //
    struct SBIO_API Src {
    public:
      SBIO_HD inline unsigned value() const { return m_value & ValueBitMask; }

    protected:
      std::uint32_t m_value;

    private:
      enum { LevelBitMask = 0xF0000000, LevelBitShift = 28 };
      enum { ValueBitMask = 0x0FFFFFFF };
    };

    struct SBIO_API Damage {
    public:
      enum Value {
        Truncate            = 0,
        OutOfOrder          = 1,
        OutOfSynch          = 2,
        Corrupted           = 3,
        DroppedContribution = 4,
        MissingData         = 5,
        TimedOut            = 6,
        UserDefined         = 12
      };

      enum { UserBitMask = 0xF000, UserBitShift = 12 };
      enum { ValueBitMask = 0x0FFF };

      SBIO_HD inline std::uint16_t value() const { return m_damage; }

      SBIO_HD inline void increase(Value v) {
        m_damage |= ((1 << v) & ValueBitMask);
      }

      SBIO_HD inline void increase(std::uint16_t v) {
        m_damage |= v & ValueBitMask;
      }

      SBIO_HD inline std::uint16_t bits() const { return m_damage & ValueBitMask; }

      SBIO_HD inline std::uint16_t userBits() const { return m_damage >> UserBitShift; }

      SBIO_HD inline void userBits(std::uint16_t v) {
        m_damage &= ValueBitMask;
        m_damage |= (v << UserBitShift);
      }

    private:
      std::uint16_t m_damage;
    };

#pragma pack(push, 2)
    struct SBIO_API Xtc {
      Src src;              // 4 bytes
      Damage damage;        // 2 bytes
      TypeId contains;      // 2 bytes
      std::uint32_t extent; // 4 bytes

      SBIO_HD inline const char* payload() const { return reinterpret_cast<const char*>(this + 1); }

      SBIO_HD inline char* payload() { return reinterpret_cast<char*>(this + 1); }

      SBIO_HD inline int sizeofPayload() const { return extent - sizeof(Xtc); }

      SBIO_HD inline Xtc* next() {
        return reinterpret_cast<Xtc*>(reinterpret_cast<char*>(this) + extent);
      }

      SBIO_HD inline const Xtc* next() const {
        return reinterpret_cast<const Xtc*>(reinterpret_cast<const char*>(this) + extent);
      }
    };
#pragma pack(pop)

#pragma pack(push, 4)
    // This Dgram wrapper collapses the more complex class hierarchy from xtcdata
    // Comments indicate where the actual functions and data members are defined
    // in the original XtcData classes
    struct SBIO_API Dgram {
    public: // Added in TransitionBase
      enum Type { Event = 0, Occurrence = 1, Marker = 2 };
      enum { NumberOfTypes = 3 };

      SBIO_HD inline std::uint16_t readoutGroups() const { return env & 0xFFFF; }

    public: // Added in Transition
      SBIO_HD inline unsigned control() const { return (env >> 24) & 0xFF; }
      SBIO_HD inline TransitionId service() const {
        return static_cast<TransitionId>(control() & 0xF);
      }
      SBIO_HD inline Type type() const {
        return static_cast<Type>((control() >> 4) & 0x3);
      }
      SBIO_HD inline bool isEvent() const { return service() == TransitionId::L1Accept; }

    public: // Added in L1Dgram
      // 8 bits are reserved
      //    0 - L1Accept
      // 5..1 - L0Tag
      //    6 - L0Raw
      //    7 - L0Reject
      SBIO_HD inline std::uint16_t reserved() const { return (env >> 16) & 0xFF; }
      SBIO_HD inline bool keepRaw() const { return (env >> 22) & 1; }

    public: // Added in TransitionBase
      std::uint32_t nanoseconds;
      std::uint32_t seconds;

    public: // Added in Transition
      std::uint32_t env;

    public: // Added in Dgram
      Xtc xtc;
    };
#pragma pack(pop)

    struct DataDescr {
    public:
      std::uint32_t rank;
      std::uint32_t shape[MaxRank];
    };

    struct XtcDetectorDescr {
      char name[MaxNameSize];
      char type[MaxNameSize];
      char alg[MaxNameSize];
      char detId[MaxNameSize];
      unsigned segment;
    };

    struct SBIO_API XtcFieldDescr {
    public:
      XtcFieldDescr(const char* name_, std::uint32_t rank_, XTC2::DType dtype_)
        : rank(rank_)
        , dtype(dtype_)
      {
        std::strcpy(name, name_);
      }
      std::uint32_t rank;
      XTC2::DType dtype;
      char name[MaxNameSize]{};
    };

    struct SBIO_API AlgVersion {
    public:
      SBIO_HD AlgVersion(std::uint8_t major, std::uint8_t minor, std::uint8_t micro) {
        m_version = major << 16 | minor << 8 | micro;
      }

      SBIO_HD inline unsigned major() const { return (m_version >> 16) & 0xFF; }
      SBIO_HD inline unsigned minor() const { return (m_version >> 8) & 0xFF; }
      SBIO_HD inline unsigned micro() const { return m_version & 0xFF; }
      SBIO_HD inline unsigned version() const { return m_version; }

    private:
      std::uint32_t m_version;
    };

    struct SBIO_API Alg {
      SBIO_HD Alg(const char* alg, std::uint8_t major, std::uint8_t minor, std::uint8_t micro)
        : m_version(major, minor, micro)
      {
        safe_strncpy(m_alg, alg, MaxNameSize);
      }
      SBIO_HD inline std::uint32_t version() const { return m_version.version(); }
      SBIO_HD inline const char* name() const { return m_alg; }
    private:
      char m_alg[MaxNameSize];
      AlgVersion m_version;
    };

    struct SBIO_API Name {
    public:
      SBIO_HD Name()
        : m_alg("", 0, 0, 0)
        , m_name {}
        , m_type { static_cast<std::uint32_t>(DType::UINT8) }
        , m_rank { 1 }
      {}

      SBIO_HD Name(const char* name, DType type, std::uint32_t rank)
        : m_alg("", 0, 0, 0)
      {
        m_type = static_cast<std::uint32_t>(type);
        m_rank = rank;
        safe_strncpy(m_name, name, MaxNameSize);
      }

      SBIO_HD Name(const char* name, DType type, Alg& alg)
        : m_alg(alg)
      {
        m_type = static_cast<std::uint32_t>(DType::UINT8);
        m_rank = 1;
        safe_strncpy(m_name, name, MaxNameSize);
      }

      SBIO_HD Name(const char* name, Alg& alg)
        : m_alg(alg)
      {
        safe_strncpy(m_name, name, MaxNameSize);
        m_type = static_cast<std::uint32_t>(DType::UINT8);
        m_rank = 1;
      }
      SBIO_HD inline const char* name() const { return m_name; }
      SBIO_HD inline DType type() const { return static_cast<DType>(m_type); }
      SBIO_HD inline std::uint32_t rank() const { return m_rank; }
      SBIO_HD inline const Alg& alg() const { return m_alg; }
      SBIO_HD inline Alg& alg() { return m_alg; }

    private:
      Alg m_alg;
      char m_name[MaxNameSize];
      std::uint32_t m_type;
      std::uint32_t m_rank;
    };

    struct SBIO_API Shape {
    public:
      SBIO_HD inline unsigned size(Name& name) {
        unsigned size { 1 };
        for (unsigned i = 0; i < name.rank(); ++i) {
          size *= m_shape[i];
        }
        return size * DTypeSize[static_cast<std::uint8_t>(name.type())];
      }

      SBIO_HD inline unsigned size(const Name& name) const {
        unsigned size { 1 };
        for (unsigned i = 0; i < name.rank(); ++i) {
          size *= m_shape[i];
        }
        return size * DTypeSize[static_cast<std::uint8_t>(name.type())];
      }

      SBIO_HD inline const std::uint32_t* shape() const { return m_shape; }
      SBIO_HD inline std::uint32_t* shape() { return m_shape; }
    private:
      std::uint32_t m_shape[MaxRank];
    };

    struct NameInfo {
    public:
      std::uint32_t numArrays;
      char detType[MaxNameSize];
      char detName[MaxNameSize];
      char detId[MaxNameSize];
      Alg alg;
      std::uint32_t segment;
    };

    struct SBIO_API NamesId : public Src {
    public:
      enum { NumberOf = 1 << 20 };

      SBIO_HD inline operator unsigned() const { return value(); }
      SBIO_HD inline unsigned nodeId() const { return (m_value >> 8) & 0xFFF; }
      SBIO_HD inline unsigned namesId() const { return m_value & 0xFF; }
    };

    // NOTE: `Names`, `Data`, and `Shapes` actually inherit from AutoParentAlloc.
    //       That class isn't needed for reading though.

    struct SBIO_API Names : public Xtc {
    public:
      SBIO_HD inline std::uint32_t numArrays() const { return m_name_info.numArrays; }
      SBIO_HD inline const char* detName() const { return m_name_info.detName; }
      SBIO_HD inline const char* detType() const { return m_name_info.detType; }
      SBIO_HD inline const char* detId() const { return m_name_info.detId; }
      SBIO_HD inline unsigned segment() const { return m_name_info.segment; }
      SBIO_HD inline const Alg& alg() const { return m_name_info.alg; }
      SBIO_HD inline Alg& alg() { return m_name_info.alg; }

      SBIO_HD inline const Name& get(unsigned index) {
        return reinterpret_cast<Name*>(this + 1)[index];
      }

      SBIO_HD inline unsigned num() const {
        unsigned my_size =
          reinterpret_cast<const char*>(next()) - reinterpret_cast<const char*>(this + 1);
        if (my_size % sizeof(Name) != 0) {
          // Warning? Abort?
          std::abort();
        }
        return my_size / sizeof(Name);
      }

      SBIO_HD inline NamesId& namesId() { return reinterpret_cast<NamesId&>(src); }

    private:
      NameInfo m_name_info;
    };

    struct SBIO_API Data : public Xtc {};

    struct SBIO_API Shapes : public Xtc {
    public:
      SBIO_HD inline const Shape& get(unsigned index) const {
        return reinterpret_cast<const Shape*>(this + 1)[index];
      }
      SBIO_HD inline Shape& get(unsigned index) {
        return reinterpret_cast<Shape*>(this + 1)[index];
      }
    };

    struct SBIO_API ShapesData : public Xtc {
    public:
      SBIO_HD inline const NamesId& namesId() const {
        return reinterpret_cast<const NamesId&>(src);
      }
      SBIO_HD inline NamesId& namesId() { return reinterpret_cast<NamesId&>(src); }
      SBIO_HD inline const Data& data() const {
        const Xtc& item = first_item();
        if (item.contains.id() == TypeId::Type::Shapes) {
          const Data& d = reinterpret_cast<const Data&>(second_item());
          if (d.contains.id() != TypeId::Type::Data) {
            std::abort();
          }
          return d;
        }
        const Data& d = reinterpret_cast<const Data&>(item);
        if (d.contains.id() != TypeId::Type::Data) {
          std::abort();
        }

        return d;
      }

      SBIO_HD inline const Shapes& shapes() const {
        const Xtc& item = first_item();
        if (item.contains.id() == TypeId::Type::Shapes) {
          return reinterpret_cast<const Shapes&>(item);
        }
        const Shapes& s = reinterpret_cast<const Shapes&>(second_item());
        if (s.contains.id() != TypeId::Type::Shapes) {
          std::abort();
        }

        return s;
      }

    private:
      SBIO_HD inline const Xtc& first_item() const  {
        return *reinterpret_cast<const Xtc*>(payload());
      }
      SBIO_HD inline const Xtc& second_item() const {
        return *(first_item().next());
      }
    };

    SBIO_HD SBIO_API DataResult resolve_xtc2_pointer(void* buffer,
                                                     std::uint32_t sd_offset,
                                                     const Name* field_schema,
                                                     std::uint32_t nid,
                                                     std::uint32_t f_idx);
  } // namespace XTC2
} // namespace sbio

#endif // SBIO_FORMATS_XTC2_XTC2_HH
