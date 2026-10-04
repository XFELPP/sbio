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

#ifndef SBIO_CORE_ITERATION_HH
#define SBIO_CORE_ITERATION_HH

#include "sbio/core/io.hh"
#include "sbio/core/types.hh"

#ifdef __CUDACC__

#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/inplace_vector>
#include <cuda/std/iterator>
#include <cuda/std/tuple>
#include <cuda/std/type_traits>
#include <cuda/std/utility>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

namespace sbio {
  namespace impl {
    /**
     * A list of at most N elements: fixed capacity on device, a vector on host.
     */
#ifdef __CUDACC__
    template <class T, hd_std::size_t N>
    using BoundedList = cuda::std::inplace_vector<T, N>;
#else
    template <class T, hd_std::size_t N>
    using BoundedList = std::vector<T>;
#endif // __CUDACC__
  } // namespace impl

  /**
   * A step handed out by a zipped iteration, whose data has already been fetched.
   *
   * The handle converts to the step index. Data for any of the zipped BrokerGroups is
   * resolved from the already filled buffers with `get`, without further IO.
   *
   * As with `BrokerGroup::get_data`, results are valid until the buffers are refetched,
   * i.e. until the zipped iteration advances.
   *
   * @tparam StepIdxType The step index type of the data format.
   */
  template <typename StepIdxType>
  class StepHandle {
  public:
    SBIO_HD StepHandle(StepIdxType step, IOStatus status)
      : m_step(step)
      , m_status(status)
    {}

    SBIO_HD operator StepIdxType() const { return m_step; }

    /**
     * The step index.
     */
    SBIO_HD StepIdxType step() const { return m_step; }

    /**
     * The IOStatus of fetching the step for all zipped BrokerGroups.
     */
    SBIO_HD IOStatus status() const { return m_status; }

    /**
     * Get data from one of the zipped BrokerGroups, without IO.
     *
     * @param[in] group One of the BrokerGroups the iteration was zipped over.
     * @param[in] args The arguments for the group's DataRequest constructor.
     * @returns The requested data (as from `BrokerGroup::get_data`).
     */
    template <class Group, typename... Args>
    SBIO_HD auto get(const Group& group, Args&&... args) const {
      return group.resolve_data(m_step, hd_std::forward<Args>(args)...);
    }

  private:
    StepIdxType m_step;
    IOStatus m_status;
  };

  /**
   * Iterate the steps of a DataSource with fetches over a set of BrokerGroups.
   *
   * Comapred to the straight iteration/ranges on the DataSource direclty, zipping
   * provides the opportunity to optimize the IO fetches. As the set of BrokerGroups,
   * and therefore, the underlying StreamBrokers corresponding to their SegmentRefs,
   * is known up front, fetches can be performed once per zipped-group. I.e., when
   * BrokerGroup's share StreamBrokers, the fetches on the shared StreamBrokers is only
   * performed once, as opposed to iterating separately with the two BrokerGroups
   * independently.
   *
   * Each parallel executing unit (e.g. a thread or rank, etc.) iterating will setup
   * its own range-based for loop (`for (auto step : zip(ds, bg1, bg2)) {}`).
   *
   * @note Early exit from a loop invokes the EPolicy end iteration implementation, the
   *       same as using `DataSource::steps()`.
   *
   * @tparam DS The DataSource type (const or non-const).
   * @tparam Groups The BrokerGroup types, all created from the DataSource.
   */
  template <class DS, class... Groups>
  class ZippedGroupRange {
  public:
    using StepIdxType = typename hd_std::remove_const_t<DS>::StepIdxType;
    using FTraits = typename hd_std::remove_const_t<DS>::DataFormat;
    using ExecutionPolicy = typename hd_std::remove_const_t<DS>::ExecutionPolicy;
    using StepRange = decltype(hd_std::declval<DS&>().steps());
    using StepIt = decltype(hd_std::declval<const StepRange&>().begin());

    /**
     * The maximum number of distinct fetches over every StreamBroker of every group.
     */
    static constexpr hd_std::size_t MaxFetches {
      ( Groups::SegmentCapacity + ... + 0 )
    };

    SBIO_HD ZippedGroupRange(DS& ds, const Groups&... groups)
      : m_steps(ds.steps())
      , m_groups(&groups...)
    {
      plan_fetches(hd_std::index_sequence_for<Groups...> {});
    }

    ZippedGroupRange(const ZippedGroupRange&) = delete;
    ZippedGroupRange& operator=(const ZippedGroupRange&) = delete;

    template <class ZGR>
    class IteratorImpl {
    public:
      using iterator_category = hd_std::input_iterator_tag;
      using difference_type = hd_std::ptrdiff_t;
      using value_type = StepHandle<StepIdxType>;
      using pointer = void;
      using reference = value_type;

      SBIO_HD IteratorImpl(ZGR* range, StepIt it)
        : m_range(range)
        , m_it(it)
      {
        fetch();
      }

      SBIO_HD reference operator*() const { return value_type(*m_it, m_status); }

      SBIO_HD IteratorImpl& operator++() {
        ++m_it;
        fetch();

        return *this;
      }

      SBIO_HD void operator++(int) { ++(*this); }

      SBIO_HD friend bool operator==(const IteratorImpl& a, const IteratorImpl& b) {
        return a.m_it == b.m_it;
      }
      SBIO_HD friend bool operator!=(const IteratorImpl& a, const IteratorImpl& b) {
        return !(a == b);
      }

    private:
      SBIO_HD void fetch() {
        const StepIdxType step { *m_it };

        if (step != FTraits::ExhaustedSentinel) {
          m_status = m_range->fetch(step);
        }
      }

      ZGR* m_range;
      StepIt m_it;
      IOStatus m_status { IOStatus::Success };
    };

    using Iterator = IteratorImpl<ZippedGroupRange>;
    using ConstIterator = IteratorImpl<const ZippedGroupRange>;

    /**
     * Return an iterator at the first step for this unit, with its data fetched.
     *
     * @note This invokes `next` (once) to get the first step.
     */
    SBIO_HD ConstIterator begin() const {
      return ConstIterator(this, m_steps.begin());
    }
    SBIO_HD ConstIterator end() const {
      return ConstIterator(this, m_steps.end());
    }

    SBIO_HD ConstIterator cbegin() const { return begin(); }
    SBIO_HD ConstIterator cend() const { return end(); }

    /**
     * The number of distinct fetches performed per step.
     */
    SBIO_HD hd_std::size_t num_fetches() const { return m_fetches.size(); }

  private:
    /**
     * Identifier for a fetch from the zipped groups.
     *
     * Indicates a fetch of StreamBroker `broker_no` in BrokerGroup `group`.
     */
    struct Fetch {
      hd_std::size_t group;
      hd_std::size_t broker_no;
    };

    template <hd_std::size_t... Is>
    SBIO_HD void plan_fetches(hd_std::index_sequence<Is...>) {
      ( plan_group<Is>(), ... );
    }

    template <hd_std::size_t G>
    SBIO_HD void plan_group() {
      const auto& group { *hd_std::get<G>(m_groups) };

      using Group = hd_std::remove_cvref_t<decltype(group)>;
      constexpr bool Chronological {
        Group::DataFormat::PartitioningStrategy == StreamPartitioningStrategy::Chronological
      };
      // Chronological groups fetch a single, step-dependent, StreamBroker.
      const hd_std::size_t num_brokers { Chronological ? 1 : group.num_stream_brokers() };

      for (hd_std::size_t b = 0; b < num_brokers; ++b) {
        bool shared { false };
        for (const auto& f : m_fetches) {
          shared = shared || shares_fetch(group, b, f, hd_std::index_sequence_for<Groups...> {});
        }

        if (!shared) {
          m_fetches.push_back({ G, b });
        }
      }
    }

    template <class Group, hd_std::size_t... Is>
    SBIO_HD bool shares_fetch(const Group& group,
                              hd_std::size_t broker_no,
                              const Fetch& fetch_,
                              hd_std::index_sequence<Is...>) const {
      bool shared { false };
      auto check = [&](const auto* other, hd_std::size_t idx) {
        if constexpr (hd_std::is_same_v<hd_std::remove_cvref_t<decltype(*other)>, Group>) {
          if (idx == fetch_.group) {
            shared = group.shares_fetch(broker_no, *other, fetch_.broker_no);
          }
        }
      };

      ( check(hd_std::get<Is>(m_groups), Is), ... );

      return shared;
    }

    template <hd_std::size_t... Is>
    SBIO_HD IOStatus fetch_one(const Fetch& fetch_,
                               StepIdxType step_idx,
                               hd_std::index_sequence<Is...>) const {
      IOStatus status { IOStatus::Success };
      auto do_fetch = [&](const auto* group, hd_std::size_t idx) {
        if (idx == fetch_.group) {
          StepIdxType target_idx { step_idx };
          status = group->fetch_next_for(target_idx, fetch_.broker_no);
        }
      };

      ( do_fetch(hd_std::get<Is>(m_groups), Is), ... );

      return status;
    }

    /**
     * Perform every planned fetch for the step, through the Execution policy.
     *
     * @param[in] step_idx The index of the step to fetch.
     */
    SBIO_HD IOStatus fetch(StepIdxType step_idx) const {
      auto unit_fetcher = [&](hd_std::size_t i) {
        return fetch_one(m_fetches[i], step_idx, hd_std::index_sequence_for<Groups...> {});
      };
      auto no_access = [](hd_std::size_t) {}; // Data is resolved on demand via StepHandle::get

      return ExecutionPolicy::template get_data<FTraits>(step_idx,
                                                         unit_fetcher,
                                                         m_fetches.size(),
                                                         no_access,
                                                         0);
    }

    StepRange m_steps;
    hd_std::tuple<const Groups*...> m_groups;
    impl::BoundedList<Fetch, MaxFetches> m_fetches;
  };

  /**
   * Zip a fixed set of BrokerGroups from a DataSource for iteration over its steps.
   *
   * @param[in] ds The DataSource the groups were created from.
   * @param[in] groups The BrokerGroups to fetch for at each step.
   * @returns A range to iterate over in a range-based for loop.
   */
  template <class DS, class... Groups>
  SBIO_HD ZippedGroupRange<DS, Groups...> zip(DS& ds, const Groups&... groups) {
    return ZippedGroupRange<DS, Groups...>(ds, groups...);
  }
} // namespace sbio

#endif // SBIO_CORE_ITERATION_HH
