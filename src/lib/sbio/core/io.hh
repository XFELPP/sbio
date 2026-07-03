#ifndef SBIO_CORE_IO_HH
#define SBIO_CORE_IO_HH

#include <fcntl.h>
#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif
#include <unistd.h>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace fs = std::filesystem;

namespace sbio {
  enum class IOStatus : std::uint32_t {
    Success = 0,
    Pending,    // IO in flight
    WouldBlock, // Resource busy (NIC/Disk queue full e.g.)
    OpenFailed,
    GeneralIOError,
    ZeroBytesRead,
    TruncatedRead,
    FunctionUnavailable,
    NoOffsetInData,
    HeaderReadError,
    PayloadTruncatedError,
    AllRequestedRead
  };

  template <typename Derived>
  struct IOPolicy {
  public:
    IOPolicy()
      : m_file_size(0)
      , m_read_count(0)
      , m_total_bytes_read(0)
    {}

    template <typename... Args>
    SBIO_HD inline IOStatus connect(Args&&... args) {
      return static_cast<Derived*>(this)->connect(std::forward<Args>(args)...);
    }

    SBIO_HD inline IOStatus read(std::uint64_t offset, std::size_t size, void* dest) {
      return static_cast<Derived*>(this)->read(offset, size, dest);
    }

    SBIO_HD inline IOStatus iread(std::uint64_t offset, std::size_t size, void* dest) {
      if constexpr (requires { static_cast<Derived*>(this)->iread(offset, size, dest); }) {
        return static_cast<Derived*>(this)->iread(offset, size, dest);
      }
      m_cached_offset = offset;
      m_cached_size = size;
      m_cached_dest = dest;
      return IOStatus::FunctionUnavailable;
    }

    SBIO_HD inline IOStatus wait() {
      if constexpr (requires { static_cast<Derived*>(this)->wait(); }) {
        return static_cast<Derived*>(this)->wait();
      }
      // If the IO policy does not have iread/wait functionality, fall back and
      // use a normal synchronous read when caller uses wait.
      return static_cast<Derived*>(this)->read(m_cached_offset,
                                               m_cached_size,
                                               m_cached_dest);
    }

    SBIO_HD inline std::size_t file_size() const { return m_file_size; }

    SBIO_HD inline std::size_t read_count() const { return m_read_count; }

    SBIO_HD inline std::size_t total_bytes_read() const { return m_total_bytes_read; }

  protected:
    std::size_t m_file_size;
    std::size_t m_read_count; ///< Bytes read on the last read
    std::size_t m_total_bytes_read; ///< Cummulative bytes read over all reads

  private:
    std::uint64_t m_cached_offset;
    std::size_t m_cached_size;
    void* m_cached_dest;
  };

  template <typename T>
  concept HasRead = requires(T io, std::uint64_t offset, std::size_t size, void* dest) {
    { io.read(offset, size, dest) } -> std::convertible_to<IOStatus>;
    { io.read_count() } -> std::convertible_to<std::size_t>;
    { io.file_size() } -> std::convertible_to<std::size_t>;
  };

  template <typename T>
  concept IOTraits = HasRead<T>;
} // namespace sbio
#endif // SBIO_CORE_IO_HH
