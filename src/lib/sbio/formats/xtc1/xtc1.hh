#ifndef SBIO_FORMATS_XTC1_XTC1_HH
#define SBIO_FORMATS_XTC1_XTC1_HH

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
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  class XTC1Traits; // Forward declare for metadata inventory access

  namespace XTC1 {
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

    class SBIO_API TransitionId {
    public:
      enum Value {
        Unknown,
        Reset,
        Map,
        Unmap,
        Configure,
        Unconfigure,
        BeginRun,
        EndRun,
        BeginCalibCycle,
        EndCalibCycle,
        Enable,
        Disable,
        L1Accept,
        NumberOf
      };

      SBIO_HD TransitionId()
        : m_val(Unknown)
      {}
      SBIO_HD TransitionId(Value v)
        : m_val(v)
      {}
      SBIO_HD inline operator Value() const { return m_val; }

      SBIO_HD inline const char* name(Value id) {
        static const char* names[] = {
          "Unknown",
          "Reset",
          "Map",
          "Unmap",
          "Configure",
          "Unconfigure",
          "BeginRun",
          "EndRun",
          "BeginCalibCycle",
          "EndCalibCycle",
          "Enable",
          "Disable",
          "L1Accept"
        };
        auto i_val { static_cast<int>(id) };
        auto n_val { static_cast<int>(NumberOf) };

        return (i_val < n_val ? names[i_val] : "-Invalid-");
      };

    private:
      Value m_val;
    };

    class SBIO_API TypeId {
    public:
      /*
       * Notice: New enum values should be appended to the end of the enum list, since
       *   the old values have already been recorded in the existing xtc files.
       */
      enum Type {
        Any,
        Id_Xtc,          // generic hierarchical container
        Id_Frame,        // raw image
        Id_AcqWaveform,
        Id_AcqConfig,
        Id_TwoDGaussian, // 2-D Gaussian + covariances
        Id_Opal1kConfig,
        Id_FrameFexConfig,
        Id_EvrConfig,
        Id_TM6740Config,
        Id_ControlConfig,
        Id_pnCCDframe,
        Id_pnCCDconfig,
        Id_Epics,        // Epics Data Type
        Id_FEEGasDetEnergy,
        Id_EBeam,
        Id_PhaseCavity,
        Id_PrincetonFrame,
        Id_PrincetonConfig,
        Id_EvrData,
        Id_FrameFccdConfig,
        Id_FccdConfig,
        Id_IpimbData,
        Id_IpimbConfig,
        Id_EncoderData,
        Id_EncoderConfig,
        Id_EvrIOConfig,
        Id_PrincetonInfo,
        Id_CspadElement,
        Id_CspadConfig,
        Id_IpmFexConfig,  // LUSI Diagnostics
        Id_IpmFex,
        Id_DiodeFexConfig,
        Id_DiodeFex,
        Id_PimImageConfig,
        Id_SharedIpimb,
        Id_AcqTdcConfig,
        Id_AcqTdcData,
        Id_Index,
        Id_XampsConfig,
        Id_XampsElement,
        Id_Cspad2x2Element,
        Id_SharedPim,
        Id_Cspad2x2Config,
        Id_FexampConfig,
        Id_FexampElement,
        Id_Gsc16aiConfig,
        Id_Gsc16aiData,
        Id_PhasicsConfig,
        Id_TimepixConfig,
        Id_TimepixData,
        Id_CspadCompressedElement,
        Id_OceanOpticsConfig,
        Id_OceanOpticsData,
        Id_EpicsConfig,
        Id_FliConfig,
        Id_FliFrame,
        Id_QuartzConfig,
        Reserved1,        // previously Id_CompressedFrame        : no corresponding class
        Reserved2,        // previously Id_CompressedTimePixFrame : no corresponding class
        Id_AndorConfig,
        Id_AndorFrame,
        Id_UsdUsbData,
        Id_UsdUsbConfig,
        Id_GMD,
        Id_SharedAcqADC,
        Id_OrcaConfig,
        Id_ImpData,
        Id_ImpConfig,
        Id_AliasConfig,
        Id_L3TConfig,
        Id_L3TData,
        Id_Spectrometer,
        Id_RayonixConfig,
        Id_EpixConfig,
        Id_EpixElement,
        Id_EpixSamplerConfig,
        Id_EpixSamplerElement,
        Id_EvsConfig,
        Id_PartitionConfig,
        Id_PimaxConfig,
        Id_PimaxFrame,
        Id_Arraychar,
        Id_Epix10kConfig,
        Id_Epix100aConfig,
        Id_GenericPgpConfig,
        Id_TimeToolConfig,
        Id_TimeToolData,
        Id_EpixSConfig,
        Id_SmlDataConfig,
        Id_SmlDataOrigDgramOffset,
        Id_SmlDataProxy,
        Id_ArrayUInt16,
        Id_GotthardConfig,
        Id_AnalogInput,
        Id_SmlData,
        Id_Andor3dConfig,
        Id_Andor3dFrame,
        Id_BeamMonitorBldData,
        Id_Generic1DConfig,
        Id_Generic1DData,
        Id_UsdUsbFexConfig,
        Id_UsdUsbFexData,
        Id_EOrbits,
        Id_SharedUsdUsb,
        Id_ControlsCameraConfig,
        Id_ArchonConfig,
        Id_JungfrauConfig,
        Id_JungfrauElement,
        Id_QuadAdcConfig,
        Id_ZylaConfig,
        Id_ZylaFrame,
        Id_Epix10kaConfig,
        Id_PixisConfig,
        Id_PixisFrame,
        Id_UxiConfig,
        Id_UxiFrame,
        Id_Epix10ka2MConfig,
        Id_Epix10kaArray,
        Id_StreakConfig,
        Id_Epix10kaQuadConfig,
        Id_iStarConfig,
        Id_AlviumConfig,
        Id_VimbaFrame,
        NumberOf
      };

      enum { VCompressed = 0x8000 };

      SBIO_HD TypeId() {}
      SBIO_HD TypeId(const TypeId& v)
        : m_value(v.m_value)
      {}

      SBIO_HD TypeId(Type type, std::uint32_t version, bool compressed = false)
        : m_value((version << 16) | static_cast<std::uint32_t>(type) | (compressed ? 0x80000000 : 0))
      {}

      SBIO_HD TypeId(const char* s)
        : m_value(NumberOf)
      {
        const char* token { strrchr(s, '_') };
        if (!(token && *(token + 1) == 'v')) {
          return;
        }

        char* e;
        unsigned vsn { static_cast<unsigned>(strtoul(token + 2, &e, 10)) };
        if (e == token + 2 || *e != 0) {
          return;
        }

        char* p { strndup(s, token - s) };
        for (unsigned i = 0; i < static_cast<unsigned>(NumberOf); ++i) {
          if (strcmp(p, name(static_cast<Type>(i))) == 0) {
            m_value = (vsn << 16) | i;
          }
        }
        free(p);
      }

      SBIO_HD inline Type id() const { return static_cast<Type>(m_value & 0xFFFF); }
      SBIO_HD inline std::uint32_t version() const {
        return (m_value & 0xFFFF0000) >> 16;
      }
      SBIO_HD inline std::uint32_t value() const { return m_value; }

      SBIO_HD inline bool compressed() const { return m_value & 0x80000000; }
      SBIO_HD inline unsigned compressed_version() const {
        return static_cast<unsigned>((m_value & 0x7FFF0000) >> 16);
      }

      SBIO_HD inline bool is_configuration() const {
        static Type _configuration_types[] = {
          Id_AcqConfig,
          Id_Opal1kConfig,
          Id_FrameFexConfig,
          Id_TM6740Config,
          Id_ControlConfig,
          Id_pnCCDconfig,
          Id_PrincetonConfig,
          Id_FrameFccdConfig,
          Id_FccdConfig,
          Id_IpimbConfig,
          Id_EncoderConfig,
          Id_EvrIOConfig,
          Id_CspadConfig,
          Id_IpmFexConfig,  // LUSI Diagnostics
          Id_DiodeFexConfig,
          Id_PimImageConfig,
          Id_AcqTdcConfig,
          Id_XampsConfig,
          Id_Cspad2x2Config,
          Id_FexampConfig,
          Id_Gsc16aiConfig,
          Id_PhasicsConfig,
          Id_TimepixConfig,
          Id_OceanOpticsConfig,
          Id_FliConfig,
          Id_QuartzConfig,
          Id_AndorConfig,
          Id_UsdUsbConfig,
          Id_OrcaConfig,
          Id_ImpConfig,
          Id_AliasConfig,
          Id_L3TConfig,
          Id_RayonixConfig,
          Id_EpixConfig,
          Id_EpixSamplerConfig,
          Id_Epix10kConfig,
          Id_Epix100aConfig,
          Id_EvsConfig,
          Id_PartitionConfig,
          Id_PimaxConfig,
          Id_GenericPgpConfig,
          Id_TimeToolConfig,
          Id_EpixSConfig,
          Id_GotthardConfig,
          Id_Andor3dConfig,
          Id_Generic1DConfig,
          Id_UsdUsbFexConfig,
          Id_ControlsCameraConfig,
          Id_ArchonConfig,
          Id_JungfrauConfig,
          Id_QuadAdcConfig,
          Id_ZylaConfig,
          Id_Epix10kaConfig,
          Id_PixisConfig,
          Id_UxiConfig,
          Id_Epix10ka2MConfig,
          Id_StreakConfig,
          Id_Epix10kaQuadConfig,
          Id_iStarConfig,
          Id_AlviumConfig,
        };

        const unsigned nconfigtypes { sizeof(_configuration_types) / sizeof(Type) };
        Type t { id() };
        for (unsigned i = 0; i < nconfigtypes; ++i) {
          if (t == _configuration_types[i]) {
            return true;
          }
        }

        return false;
      }
      SBIO_HD static const char* name(Type type) {
        static const char* names[NumberOf] = {
          "Any",                     // 0
          "Xtc",                     // 1
          "Frame",                   // 2
          "AcqWaveform",             // 3
          "AcqConfig",               // 4
          "TwoDGaussian",            // 5
          "Opal1kConfig",            // 6
          "FrameFexConfig",          // 7
          "EvrConfig",               // 8
          "TM6740Config",            // 9
          "RunControlConfig",        // 10
          "pnCCDframe",              // 11
          "pnCCDconfig",             // 12
          "Epics",                   // 13
          "FEEGasDetEnergy",         // 14
          "EBeamBld",                // 15
          "PhaseCavity",             // 16
          "PrincetonFrame",          // 17
          "PrincetonConfig",         // 18
          "EvrData",                 // 19
          "FrameFccdConfig",         // 20
          "FccdConfig",              // 21
          "IpimbData",               // 22
          "IpimbConfig",             // 23
          "EncoderData",             // 24
          "EncoderConfig",           // 25
          "EvrIOConfig",             // 26
          "PrincetonInfo",           // 27
          "CspadElement",            // 28
          "CspadConfig",             // 29
          "IpmFexConfig",            // 30
          "IpmFex",                  // 31
          "DiodeFexConfig",          // 32
          "DiodeFex",                // 33
          "PimImageConfig",          // 34
          "SharedIpimb",             // 35
          "AcqTDCConfig",            // 36
          "AcqTDCData",              // 37
          "Index",                   // 38
          "XampsConfig",             // 39
          "XampsElement",            // 40
          "Cspad2x2Element",         // 41
          "SharedPIM",               // 42
          "Cspad2x2Config",          // 43
          "FexampConfig",            // 44
          "FexampElement",           // 45
          "Gsc16aiConfig",           // 46
          "Gsc16aiData",             // 47
          "PhasicsConfig",           // 48
          "TimepixConfig",           // 49
          "TimepixData",             // 50
          "CspadCompressedElement",  // 51
          "OceanOpticsConfig",       // 52
          "OceanOpticsData",         // 53
          "EpicsConfig",             // 54
          "FliConfig",               // 55
          "FliFrame",                // 56
          "QuartzConfig",            // 57
          "Reserved1",               // 58
          "Reserved2",               // 59
          "AndorConfig",             // 60
          "AndorFrame",              // 61
          "UsdUsbData",              // 62
          "UsdUsbConfig",            // 63
          "GMD",                     // 64
          "SharedAcqADC",            // 65
          "OrcaConfig",              // 66
          "ImpData",                 // 67
          "ImpConfig",               // 68
          "AliasConfig",             // 69
          "L3TConfig",               // 70
          "L3TData",                 // 71
          "Spectrometer",            // 72
          "RayonixConfig",           // 73
          "EpixConfig",              // 74
          "EpixElement",             // 75
          "EpixSamplerConfig",       // 76
          "EpixSamplerElement",      // 77
          "EvsConfig",               // 78
          "PartitionConfig",         // 79
          "PimaxConfig",             // 80
          "PimaxFrame",              // 81
          "Arraychar",               // 82
          "Epix10kConfig",           // 83
          "Epix100aConfig",          // 84
          "GenericPgpConfig",        // 85
          "TimeToolConfig",          // 86
          "TimeToolData",            // 87
          "EpixSConfig",             // 88
          "SmlDataConfig",           // 89
          "SmlDataOrigDgramOffset",  // 90
          "SmlDataProxy",            // 91
          "ArrayUInt16",             // 92
          "GotthardConfig",          // 93
          "AnalogInput",             // 94
          "SmlData",                 // 95
          "Andor3dConfig",           // 96
          "Andor3dFrame",            // 97
          "BeamMonitorBldData",      // 98
          "Generic1DConfig",         // 99
          "Generic1DData",           // 100
          "UsdUsbFexConfig",         // 101
          "UsdUsbFexData",           // 102
          "EOrbits",                 // 103
          "SharedUsdUsb",            // 104
          "ControlsCameraConfig",    // 105
          "ArchonConfig",            // 106
          "JungfrauConfig",          // 107
          "JungfrauElement",         // 108
          "QuadAdcConfig",           // 109
          "ZylaConfig",              // 110
          "ZylaFrame",               // 111
          "Epix10kaConfig",          // 112
          "PixisConfig",             // 113
          "PixisFrame",              // 114
          "UxiConfig",               // 115
          "UxiFrame",                // 116
          "Epix10ka2MConfig",        // 117
          "Epix10kaArray",           // 118
          "StreakConfig",            // 119
          "Epix10kaQuadConfig",      // 120
          "iStarConfig",             // 121
          "AlviumConfig",            // 122
          "VimbaFrame",              // 123
        };

        const char* p { (type < NumberOf ? names[type] : "-Invalid-") };
        if (!p) {
          p = "-Unnamed-";
        }

        return p;
      }
      SBIO_HD static std::uint32_t _sizeof() { return sizeof(TypeId); }

    private:
      std::uint32_t m_value;
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

    // --- Struct wrappers for XTC1 --- //
    struct SBIO_API Level {
    public:
      enum Type {
        Control,
        Source,
        Segment,
        Event,
        Recorder,
        Observer,
        Reporter,
        NumberOfLevels
      };

      SBIO_HD inline const char* name(Type type) {
        static const char* names[] = {
          "Control",
          "Source",
          "Segment",
          "Event",
          "Recorder",
          "Observer",
          "Reporter"
        };
        auto t_val { static_cast<int>(type) };
        auto n_val { static_cast<int>(NumberOfLevels) };
        return (t_val < n_val ? names[type] : "-Invalid-" );
      }
    };

    struct SBIO_API Src {
    public:
      SBIO_HD Src()
        : m_log(std::numeric_limits<std::uint32_t>::max())
        , m_phy(std::numeric_limits<std::uint32_t>::max())
      {}

      SBIO_HD Src(Level::Type level)
        : m_log((static_cast<std::uint32_t>(level) & 0xFF) << 24)
      {}

      SBIO_HD inline std::uint32_t log() const { return m_log; }
      SBIO_HD inline std::uint32_t phy() const { return m_phy; }

      SBIO_HD Level::Type level() const {
        return static_cast<Level::Type>((m_log >> 24) & 0xFF);
      }

      SBIO_HD inline bool operator==(const Src& s) const {
        return m_phy == s.m_phy && m_log == s.m_log;
      }
      SBIO_HD inline bool operator<(const Src& s) const {
        return (m_phy < s.m_phy) || ((m_phy == s.m_phy) && (m_log < s.m_log));
      }

      SBIO_HD inline std::uint32_t _sizeof() const { return sizeof(Src); }

    protected:
      std::uint32_t m_log; // Logical identifier
      std::uint32_t m_phy; // Physical identifier
    };

    struct SBIO_API Damage {
    public:
      enum Value {
        DroppedContribution    = 1,
        Uninitialized          = 11,
        OutOfOrder             = 12,
        OutOfSynch             = 13,
        UserDefined            = 14,
        IncompleteContribution = 15,
        ContainsIncomplete     = 16
      };

      // reserve the top byte to augment user defined errors
      enum { NotUserBitsMask = 0x00FFFFFF, UserBitsShift = 24 };

      SBIO_HD Damage() {}
      SBIO_HD Damage(std::uint32_t v)
        : m_damage(v)
      {}

      SBIO_HD inline std::uint32_t value() const { return m_damage; }

      SBIO_HD inline void increase(Value v) {
        m_damage |= ((1 << v) & NotUserBitsMask);
      }
      SBIO_HD inline void increase(std::uint32_t v) {
        m_damage |= v & NotUserBitsMask;
      }

      SBIO_HD inline std::uint32_t bits() const { return m_damage & NotUserBitsMask; }
      SBIO_HD inline std::uint32_t userBits() const { return m_damage >> UserBitsShift; }
      SBIO_HD inline void userBits(std::uint32_t v) {
        m_damage &= NotUserBitsMask;
        m_damage |= (v << UserBitsShift);
      }

    private:
      std::uint32_t m_damage;
    };

#pragma pack(push, 2)
    struct SBIO_API Xtc {
      Damage damage;
      Src src;
      TypeId contains;
      std::uint32_t extent;

      SBIO_HD Xtc()
        : damage(0)
        , extent(0)
      {}
      SBIO_HD Xtc(const Xtc& xtc)
        : damage(xtc.damage)
        , src(xtc.src)
        , contains(xtc.contains)
        , extent(sizeof(Xtc))
      {}
      SBIO_HD Xtc(const TypeId& type)
        : damage(0)
        , contains(type)
        , extent(sizeof(Xtc))
      {}
      SBIO_HD Xtc(const TypeId& type, const Src& _src)
        : damage(0)
        , src(_src)
        , contains(type)
        , extent(sizeof(Xtc))
      {}
      SBIO_HD Xtc(const TypeId& _tag, const Src& _src, unsigned _damage)
        : damage(_damage)
        , src(_src)
        , contains(_tag)
        , extent(sizeof(Xtc))
      {}
      SBIO_HD Xtc(const TypeId& _tag, const Src& _src, const Damage& _damage)
        : damage(_damage)
        , src(_src)
        , contains(_tag)
        , extent(sizeof(Xtc))
      {}

      SBIO_HD inline void* operator new(std::size_t size, char* p) {
        return reinterpret_cast<void*>(p);
      }
      SBIO_HD inline void* operator new(std::size_t size, Xtc* p) {
        return p->alloc(size);
      }

      SBIO_HD inline const char* payload() const { return reinterpret_cast<const char*>(this + 1); }

      SBIO_HD inline char* payload() { return reinterpret_cast<char*>(this + 1); }

      SBIO_HD inline int sizeofPayload() const { return extent - sizeof(Xtc); }

      SBIO_HD inline Xtc* next() {
        return reinterpret_cast<Xtc*>(reinterpret_cast<char*>(this) + extent);
      }

      SBIO_HD inline const Xtc* next() const {
        return reinterpret_cast<const Xtc*>(reinterpret_cast<const char*>(this) + extent);
      }

      SBIO_HD inline void* alloc(std::uint32_t size) {
        void* buffer = next();
        extent += size;
        return buffer;
      }
    };
#pragma pack(pop)
    class SBIO_API ClockTime {
    public:
      SBIO_HD ClockTime() {}
      SBIO_HD ClockTime(const ClockTime& t)
        : m_low(t.m_low)
        , m_high(t.m_high)
      {}

      SBIO_HD ClockTime(const ::timespec& ts)
        : m_low(ts.tv_nsec)
        , m_high(ts.tv_sec)
      {}

      SBIO_HD ClockTime(const double sec) {
        double intpart;
        double fracpart = std::modf(sec, &intpart);
        m_high = static_cast<unsigned>(intpart);
        m_low = static_cast<unsigned>(1.e9 * fracpart + 0.5);
      }

      SBIO_HD ClockTime(unsigned sec, unsigned nsec)
        : m_low(nsec)
        , m_high(sec)
      {}

      SBIO_HD inline unsigned seconds() const { return m_high; }
      SBIO_HD inline unsigned nanoseconds() const { return m_low; }

      SBIO_HD inline double asDouble() const {
        return static_cast<double>(m_high + (static_cast<double>(m_low) / 1.e9));
      }

      SBIO_HD inline bool isZero() const { return m_low == 0 && m_high == 0; }

      SBIO_HD inline ClockTime& operator=(const ClockTime& input) {
        // Why did they never check if the object was the same??
        m_low = input.m_low;
        m_high = input.m_high;
        return *this;
      }
      SBIO_HD inline bool operator>(const ClockTime& t) const {
        return (m_high > t.m_high) || (m_high || t.m_high && m_low > t.m_low);
      }
      SBIO_HD inline bool operator==(const ClockTime& t) const {
        return (m_high == t.m_high) && (m_low == t.m_low);
      }

    private:
      uint32_t m_low;
      uint32_t m_high;
    };

    class SBIO_API TimeStamp {
    public:
      enum { NumFiducialBits = 17 };
      enum { MaxFiducials = (1<<17)-32 };
      enum { ErrFiducial = (1<<17)-1 };

      enum { v_ticks = 0, k_ticks = 24 };
      enum { v_cntrl = 24, k_cntrl = 8 };
      enum { m_ticks = ((1 << k_ticks) - 1), s_ticks = (m_ticks << v_ticks) };
      enum { m_cntrl = ((1 << k_cntrl) - 1), s_cntrl = (m_cntrl << v_cntrl) };
      enum { v_fiduc = 0, k_fiduc = TimeStamp::NumFiducialBits };
      enum { v_vecto = TimeStamp::NumFiducialBits, k_vecto = 32 - TimeStamp::NumFiducialBits };
      enum { m_fiduc = ((1 << k_fiduc) - 1), s_fiduc = (m_fiduc << v_fiduc) };
      enum { m_vecto = ((1 << k_vecto) - 1), s_vecto = (m_vecto << v_vecto) };

      SBIO_HD TimeStamp()
        : m_low(0)
        , m_high(0)
      {}

      SBIO_HD TimeStamp(const TimeStamp& input)
        : m_low(input.m_low)
        , m_high(input.m_high)
      {}

      SBIO_HD TimeStamp(const TimeStamp& input, unsigned control)
        : m_low((input.m_low & s_ticks) | ((control & m_cntrl) << v_cntrl))
        , m_high(input.m_high)
      {}

      SBIO_HD TimeStamp(unsigned ticks,
                        unsigned fiducials,
                        unsigned vector,
                        unsigned control = 0)
        : m_low((ticks & s_ticks) | ((control & m_cntrl) << v_cntrl))
        , m_high((fiducials & s_fiduc) | ((vector & m_vecto) << v_vecto))
      {}

      SBIO_HD inline TimeStamp& operator=(const TimeStamp& input) {
        // Why did they never check if same object???
        m_low = input.m_low;
        m_high = input.m_high;
        return *this;
      }

      /**
       * 119 MHz counter within the fiducial for eventcode which initiated readout.
       */
      SBIO_HD inline unsigned ticks() const { return (m_low & s_ticks) >> v_ticks; }

      /**
       * 360 Hz pulse ID
       */
      SBIO_HD inline unsigned fiducials() const { return (m_high & s_fiduc) >> v_fiduc; }

      /**
       * Internal bits for alternate interpretation of XTC header fields.
       */
      SBIO_HD inline unsigned control() const { return (m_low & s_cntrl) >> v_cntrl; }

      /**
       * 15-bit seed for event-level distribution (events since configure).
       */
      SBIO_HD inline unsigned vector() const { return (m_high & s_vecto) >> v_vecto; }

      SBIO_HD inline bool operator==(const TimeStamp& ref) const {
        return fiducials() == ref.fiducials();
      }
      SBIO_HD inline bool operator>=(const TimeStamp& ref) const {
        return fiducials() >= ref.fiducials();
      }
      SBIO_HD inline bool operator<=(const TimeStamp& ref) const {
        return fiducials() <= ref.fiducials();
      }
      SBIO_HD inline bool operator<(const TimeStamp& ref) const {
        return fiducials() < ref.fiducials();
      }
      SBIO_HD inline bool operator>(const TimeStamp& ref) const {
        return fiducials() > ref.fiducials();
      }

    private:
      std::uint32_t m_low;
      std::uint32_t m_high;
    };

    class SBIO_API Sequence {
    public:
      enum Type { Event = 0, Occurrence = 1, Marker = 2 };
      enum { NumberOfTypes = 3 };

      enum { v_cntrl = 0, k_cntrl = 8 };
      enum { v_service = 0, k_service = 4 };
      enum { v_seqtype = 4, k_seqtype = 2 };
      enum { v_extend = 7, k_extend = 1 };

      enum { m_cntrl = ((1 << k_cntrl) - 1), s_cntrl = (m_cntrl << v_cntrl) };
      enum { m_service = ((1 << k_service) - 1), s_service = (m_service << v_service) };
      enum { m_seqtype = ((1 << k_seqtype) - 1), s_seqtype = (m_seqtype << v_seqtype) };
      enum { m_extend = ((1 << k_extend) - 1), s_extend = (m_extend << v_extend) };

      SBIO_HD Sequence() {}
      SBIO_HD Sequence(const Sequence& input)
        : m_clock(input.m_clock)
        , m_stamp(input.m_stamp)
      {}

      SBIO_HD Sequence(const ClockTime& clock, const TimeStamp& stamp)
        : m_clock(clock)
        , m_stamp(stamp)
      {}

      SBIO_HD Sequence(Type type,
                       TransitionId::Value service,
                       const ClockTime& clock,
                       const TimeStamp& stamp)
        : m_clock(clock)
        , m_stamp(stamp, ((type & m_seqtype) << v_seqtype) | ((service & m_service) << v_service))
      {}

      SBIO_HD Sequence& operator=(const Sequence& input) {
        // Why did they never check if it was the same object???
        m_clock = input.m_clock;
        m_stamp = input.m_stamp;

        return *this;
      }

      SBIO_HD inline Type type() const {
        return Type((m_stamp.control() >> v_seqtype) & m_seqtype);
      }

      SBIO_HD inline TransitionId::Value service() const {
        return TransitionId::Value((m_stamp.control() >> v_service) & m_service);
      }

      SBIO_HD inline bool isExtended() const { return (m_stamp.control() & s_extend) != 0; }
      SBIO_HD inline bool isEvent() const { return m_stamp.control() & s_extend; }

      SBIO_HD inline const ClockTime& clock() const { return m_clock; }
      SBIO_HD inline const TimeStamp& stamp() const { return m_stamp; }

    private:
      ClockTime m_clock;
      TimeStamp m_stamp;
    };

    class SBIO_API Env {
    public:
      SBIO_HD Env() {}
      SBIO_HD Env(const Env& in)
        : m_env(in.m_env)
      {}
      SBIO_HD Env(std::uint32_t env)
        : m_env(env)
      {}
      SBIO_HD inline const Env& operator=(const Env& other) {
        // Why did they never check if it was the same object???
        m_env = other.m_env;

        return *this;
      }

      SBIO_HD inline std::uint32_t value() const { return m_env; }

    private:
      std::uint32_t m_env;
    };

    namespace SmlData {
      struct SBIO_API OrigDgramOffsetV1 {
        std::int64_t m_fileOffset;
        std::uint32_t m_extent;

        SBIO_HD inline std::int64_t fileOffset() const { return m_fileOffset; }
        SBIO_HD inline std::uint32_t extent() const { return m_extent; }
      };
    };

    struct SBIO_API Dgram {
    public:
      Sequence seq;
      Env env;
      Xtc xtc;

      SBIO_HD inline TransitionId::Value service() const { return seq.service(); }
    };

    struct DataDescr {
    public:
      std::uint32_t rank;
      std::uint32_t shape[MaxRank];
    };

    struct XtcDetectorDescr {
      char name[MaxNameSize];
      char type[MaxNameSize];
      unsigned segment;
    };

    struct SBIO_API XtcFieldDescr {
      XtcFieldDescr(const char* name_,
                    std::uint32_t rank_,
                    XTC1::DType dtype_,
                    std::uint32_t field_idx_ = 0,
                    bool is_config_ = false,
                    bool not_is_bld_ = false,
                    std::uint32_t evt_type_id_ = 0)
        : rank(rank_)
        , dtype(dtype_)
        , field_idx(field_idx_)
        , is_config(is_config_)
        , not_is_bld(not_is_bld_)
        , evt_type_id(evt_type_id_)
      {
        std::strcpy(name, name_);
      }
      std::uint32_t rank;
      XTC1::DType dtype;
      std::uint32_t field_idx;
      bool is_config;
      bool not_is_bld; /// ALREADY FORGOT WHAT I WAS GOING TO DO HERE!!! TODO: FIX THIS
      std::uint32_t evt_type_id { 0 };
      char name[MaxNameSize] {};
    };

    struct SBIO_API Name {
    public:
      SBIO_HD Name()
        : m_name {}
        , m_type { static_cast<std::uint32_t>(DType::UINT8) }
        , m_rank { 1 }
      {}

      SBIO_HD Name(const char* name, DType type, std::uint32_t rank) {
        m_type = static_cast<std::uint32_t>(type);
        m_rank = rank;
        safe_strncpy(m_name, name, MaxNameSize);
      }
      SBIO_HD inline const char* name() const {
        return m_name;
      }
      SBIO_HD inline DType type() const {
        return static_cast<DType>(m_type);
      }
      SBIO_HD inline std::uint32_t rank() const {
        return m_rank;
      }

    private:
      char m_name[MaxNameSize];
      std::uint32_t m_type;
      std::uint32_t m_rank;
    };

    SBIO_HD SBIO_API DataResult resolve_xtc_pointer(void* buffer,
                                                    std::uint32_t sd_offset,
                                                    const Name* field_schema,
                                                    std::uint32_t nid,
                                                    std::uint32_t f_idx,
                                                    std::uint32_t evt_type_id = 0,
                                                    const void* inv = nullptr,
                                                    std::uint32_t src_log = 0,
                                                    std::uint32_t src_phy = 0);

    SBIO_HD SBIO_API const Xtc* find_xtc_by_type(const Xtc* xtc,
                                                 TypeId::Type target,
                                                 std::uint32_t src_log = 0,
                                                 std::uint32_t src_phy = 0);
  } // namespace XTC1
} // namespace sbio

#endif // SBIO_FORMATS_XTC1_XTC1_HH
