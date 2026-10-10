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

#ifndef SBIO_CORE_CONTEXT_HH
#define SBIO_CORE_CONTEXT_HH

#include "sbio/core/broker.hh"
#include "sbio/core/types.hh"
#include "sbio/util/string.hh"
#include "sbio/core/datasource.hh"
#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/formats/format_traits.hh"


#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <map>
#include <string>
#include <string_view>

namespace sbio {
  namespace impl {
    /**
     * Check if a valid Context already exists for an Execution Policy.
     *
     * @note Because important EPolicy state is static, only one Context should be
     *       employed per EPolicy at a time.
     */
    template <class EPolicy>
    struct ContextRegistry {
      static inline std::atomic<bool> live { false };
    };
  } // namespace impl

  /**
   * The Context constructs DataSources from IO and Execution policies within an environment.
   *
   * The Context provides a mechanism to check both the compatiblity of the various
   * policies that are being composed (in large part at compile), as well as a
   * central place to handle interactions with the environment.
   *
   * The Context can record and interface directly with environment variables, which
   * may be needed for locating streams e.g. (not yet implemented with Locator support),
   * as well as various configuration of execution policies.
   *
   * Compile time checks look for:
   * - IOCapability
   * - ParallelSupport
   *
   * At construction runtime checks include:
   * - IO::validate_environment for the IO policy (if provided).
   * - Execution::validate_environment for the execution policy
   *   - E.g. to check MPI thread support level.
   *
   * Context will only configure the EPolicy once. From the Context DataSources
   * can be created (using a separate constructor that avoids reconfiguration).
   *
   * @note The Context only provides an alternative entry-point and a DataSource can
   *       still be created directly.
   *
   * @code{.cpp}
   * sbio::Context<sbio::SyncPOSIXIO, sbio::ThreadedExecution> ctx(ecfg);
   * if (!ctx.ok()) {
   *   std::cerr << ctx.error() << std::endl;
   * }
   * auto ds = ctx.data_source<sbio::XTC2Traits>();
   * @endcode
   *
   * @tparam IO The IO policy.
   * @tparam EPolicy The Execution policy.
   */
  template <IOTraits IO, class EPolicy>
  class Context {
  public:
    using IOPolicy = IO;
    using ExecutionPolicy = EPolicy;
    using EPolicyConfig = typename EPolicy::Config;

    static_assert(has_capabilities(io_capabilities<IO>(),
                                   EPolicy::template required_io_capabilities<IO>()),
                  "This IOPolicy lacks capabilities required by the Execution Policy. "
                  "(Refer to IOCapability, IO::Capabilities, required_io_capabilities)");

    /**
     * Validate the proposed policies within the environment and then configure.
     *
     * @param[in] config The Execution policy configuration.
     */
    explicit Context(const EPolicyConfig& config = {}) {
      bool expected { false };
      if (!impl::ContextRegistry<EPolicy>::live.compare_exchange_strong(expected, true)) {
        // Because of static state, can only have 1 Context per EPolicy
        m_error = "A Context for this Execution policy already exists.";
        return;
      }

      m_registered = true;

      if (!validate_policy(config, m_error)) {
        return;
      }

      if constexpr (requires (const char*& reason) { IO::open_environment(reason); }) {
        const char* reason { nullptr };
        if (!IO::open_environment(reason)) {
          m_error = reason ? reason : "The IO policy environment could not be opened.";
          return;
        }

        m_io_environment = true;
      }

      EPolicy::configure(config);
      m_ok = true;
    }

    ~Context() {
      if constexpr (requires { IO::close_environment(); }) {
        if (m_io_environment) {
          IO::close_environment();
        }
      }

      if (m_registered) {
        impl::ContextRegistry<EPolicy>::live.store(false);
      }
    }

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;

    /**
     * @returns Whether the Context is valid (EPolicy has been configured.)
     */
    bool ok() const { return m_ok; }

    /**
     * @returns Why the Context is not valid or empty string.
     */
    const std::string& error() const { return m_error; }

    /**
     * Check if a particular parallelization method is supported (threads, MPI, etc.)
     *
     * @param[in] method The parallelization method
     * @returns `true` if supported else `false`
     */
    static constexpr bool supports(ParallelizationMethods method) {
      switch (method) {
      case ParallelizationMethods::THREADS: {
        return true;
      }
      case ParallelizationMethods::MPI: {
#ifdef SBIO_HAS_MPI
        return true;
#else
        return false;
#endif
      }
      case ParallelizationMethods::CUDA: {
#ifdef SBIO_HAS_CUDA
        return true;
#else
        return false;
#endif
      }
      default: {
        return false;
      }
      }
    }

    /**
     * Check an EPolicy can be used with `config`, and IOPolicy, in the current env.
     *
     * @param[in] config The EPolicy configuration parameters
     * @param[out] error A description of the problem if not a usable compostion.
     * @returns `true` if the policy can be configured else false.
     */
    static bool validate_policy(const EPolicyConfig& config, std::string& error) {
      constexpr auto required { EPolicy::ParallelSupport };
      for (std::size_t method = 0; method < required.size(); ++method) {
        if (required[method] && !supports(static_cast<ParallelizationMethods>(method))) {
          error =
            std::string("The EPolicy uses an unsupported parallelization method (")
            + method_name(static_cast<ParallelizationMethods>(method)) + std::string(")");

          return false;
        }
      }

      const char* reason { nullptr };
      if (!EPolicy::validate_environment(config, reason)) {
        error = reason ? reason : "The environment does not support the Execution policy.";

        return false;
      }

      if constexpr (requires { IO::validate_environment(reason); }) {
        if (!IO::validate_environment(reason)) {
          error = reason ? reason : "The environment does not support the IO policy.";

          return false;
        }
      }

      return true;
    }

    /**
     * Construct a DataSource from the Context.
     *
     * @note The DataSource will use the Context's (configured) EPolicy and IOPolicy.
     *
     * @pre `ok()`.
     * @tparam FTraits The data format type
     * @returns The DataSource, ready for stream locating.
     */
    template <class FTraits>
    requires FormatTraits<FTraits, IO, EPolicy>
    DataSource<IO, EPolicy, FTraits> data_source() const {
      assert(m_ok && "Creating a DataSource from an invalid Context!");

      return DataSource<IO, EPolicy, FTraits>(ExecutionPolicyConfigured {});
    }

    /**
     * Retrieve an environment variable, with optional fallback.
     *
     * @note The Context may also set overrides itself. These will be looked up
     *       before checking the actual process environment.
     *
     * @param[in] key The environment variable name.
     * @param[in] fallback The value if the variable is not set.
     * @returns The value.
     */
    std::string env(std::string_view key, std::string_view fallback = {}) const {
      if (auto it { m_env.find(std::string(key)) }; it != m_env.end()) {
        return it->second;
      }

      if (const char* value { std::getenv(std::string(key).c_str()) }) {
        return value;
      }

      return std::string(fallback);
    }

    /**
     * @returns Whether `key` is set in the Context overrides or process environment.
     */
    bool has_env(std::string_view key) const {
      return
        m_env.count(std::string(key)) > 0 ||
        std::getenv(std::string(key).c_str()) != nullptr;
    }

    /**
     * Provide a specific override for an environment variable that only the Context uses.
     *
     * @note Overrides will be searched first when the Context is queried for env
     *       vars, before then searching the actual process environment.
     *
     * @note There is no synchronization with overrides - should be set before moving
     *       Contexts between threads, for example.
     */
    void set_env(std::string_view key, std::string_view value) {
     m_env[std::string(key)] = std::string(value);
    }

  private:
    static constexpr const char* method_name(ParallelizationMethods method) {
      switch (method) {
      case ParallelizationMethods::THREADS:       { return "THREADS"; }
      case ParallelizationMethods::MPI:           { return "MPI"; }
      case ParallelizationMethods::CUDA:          { return "CUDA"; }
      case ParallelizationMethods::CUDA_MULTIGPU: { return "CUDA_MULTIGPU"; }
      case ParallelizationMethods::CUDA_FROMDEV:  { return "CUDA_FROMDEV"; }
      default:                                    { return "unknown"; }
      }
    }

    bool m_ok { false };
    bool m_registered { false };
    bool m_io_environment { false }; ///< Whether this Context holds the IO environment
    std::string m_error {};
    std::map<std::string, std::string> m_env {};
  };
} // namespace sbio

#endif // SBIO_CORE_CONTEXT_HH
