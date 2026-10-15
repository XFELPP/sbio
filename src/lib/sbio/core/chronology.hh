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

#ifndef SBIO_CORE_CHRONOLOGY_HH
#define SBIO_CORE_CHRONOLOGY_HH

#include "sbio/core/types.hh"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <utility>
#include <vector>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  /**
   * The order of steps across the StreamBrokers of a Chronological data format.
   *
   * With Chronological partitioning, each step is complete in one stream, and the
   * streams must be merged to reconstruct the time axis. Each StreamBroker's steps are
   * already in time order, so the merge only needs each step's time (its order key).
   *
   * Steps are merged as the StreamBrokers are indexed. A step can only be placed once
   * every StreamBroker that is not exhausted has an indexed step after it, so a
   * StreamBroker's steps may wait (pending) for other StreamBrokers to be indexed
   * further.
   *
   * @note Only the steps of the format's distributed kind (`FTraits::ChronologicalKind`)
   *       are merged. Other kinds (e.g. transitions) are in every stream.
   */
  class ChronologicalOrder {
  public:
    /**
     * Where a step is, defined by the StreamBroker, and the step's ordinal in it.
     */
    struct Location {
      std::uint32_t broker { 0 };
      std::uint64_t ordinal { 0 };
    };

    ChronologicalOrder() = default;

    explicit ChronologicalOrder(std::size_t num_brokers)
     : m_pending(num_brokers)
      , m_next_ordinal(num_brokers, 0)
      , m_exhausted(num_brokers, false)
    {}

    /**
     * The number of merged steps.
     */
    std::size_t size() const { return m_order.size(); }

    /**
     * The number of StreamBrokers.
     */
    std::size_t num_brokers() const { return m_pending.size(); }

    /**
     * Find a step.
     *
     * @param[in] step The (merged) step.
     * @param[out] loc Its StreamBroker and ordinal.
     * @returns Whether the step has been merged.
     */
    bool locate(std::size_t step, Location& loc) const {
      if (step >= m_order.size()) {
        return false;
      }

      loc = m_order[step];

      return true;
    }

    /**
     * Whether a StreamBroker can be indexed further without losing steps.
     *
     * @note A StreamBroker's index only holds its current batch. Steps waiting to
     *       be merged must not be replaced.
     */
    bool may_index(std::size_t broker) const {
      return !m_exhausted[broker] && m_pending[broker].empty();
    }

    /**
     * Indicate a StreamBroker has no more steps to index.
     */
    void set_exhausted(std::size_t broker) { m_exhausted[broker] = true; }

    bool exhausted(std::size_t broker) const { return m_exhausted[broker]; }

    /**
     * Add the steps of a StreamBroker's current index batch.
     *
     * @note Steps already added (e.g. carried over between index batches) are skipped.
     *
     * @param[in] broker The StreamBroker.
     * @param[in] ordinal The step's ordinal in the StreamBroker.
     * @param[in] key The step's order key (time).
     */
    void add(std::size_t broker, std::uint64_t ordinal, std::uint64_t key) {
      if (ordinal < m_next_ordinal[broker]) {
        return;
      }

      m_pending[broker].emplace_back(ordinal, key);
      m_next_ordinal[broker] = ordinal + 1;
    }

    /**
     * Merge pending steps, while their order is certain.
     *
     * @returns The number of steps merged.
     */
    std::size_t merge() {
      std::size_t merged { 0 };

      while (true) {
        std::size_t best { m_pending.size() };
        for (std::size_t b = 0; b < m_pending.size(); ++b) {
          if (m_pending[b].empty()) {
            if (!m_exhausted[b]) {
              return merged; // Its next step could be earlier than any pending one
            }
            continue;
          }

          // Ties are broken by the StreamBroker's position
          if (best == m_pending.size()                                    ||
              m_pending[b].front().second < m_pending[best].front().second) {
            best = b;
          }
        }

        if (best == m_pending.size()) {
          return merged; // All exhausted, nothing pending
        }

        m_order.push_back({ static_cast<std::uint32_t>(best), m_pending[best].front().first });
        m_pending[best].pop_front();
        ++merged;
      }
    }

  private:
    std::vector<Location> m_order {};
    std::vector<std::deque<std::pair<std::uint64_t, std::uint64_t>>> m_pending {};
    std::vector<std::uint64_t> m_next_ordinal {};
    std::vector<bool> m_exhausted {};
  };
} // namespace sbio

#endif // SBIO_CORE_CHRONOLOGY_HH
