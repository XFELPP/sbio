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
   * A batch of steps handed out by a zipped iteration, whose data has already been fetched.
   *
   * The handle converts to the `StepBatch`. Data for any of the zipped BrokerGroups is
   * resolved from the already filled buffers with `get`, without further IO.
   *
   * As with `BrokerGroup::get_multi_data`, results are valid until the buffers are
   * refetched, i.e. until the zipped iteration advances.
   *
   * @note Results from batched calls always include a batch axis in the returned arrays.
   *
   * @tparam StepIdxType The step index type of the data format.
   */
  template <typename StepIdxType>
  class BatchHandle {
  public:
    SBIO_HD BatchHandle(StepBatch<StepIdxType> batch, IOStatus status)
      : m_batch(batch)
      , m_status(status)
    {}

    SBIO_HD operator StepBatch<StepIdxType>() const { return m_batch; }

    /**
     * The batch of steps [first, last).
     */
    SBIO_HD StepBatch<StepIdxType> batch() const { return m_batch; }
    SBIO_HD StepIdxType first() const { return m_batch.first; }
    SBIO_HD StepIdxType last() const { return m_batch.last; }
    SBIO_HD StepIdxType count() const { return m_batch.count(); }

    /**
     * The IOStatus of fetching the batch for all zipped BrokerGroups.
     */
    SBIO_HD IOStatus status() const { return m_status; }

    /**
     * Get data for the batch from one of the zipped BrokerGroups, without IO.
     *
     * @param[in] group One of the BrokerGroups the iteration was zipped over.
     * @param[in] args The arguments for the group's DataRequest constructor.
     * @returns The requested data (as from `BrokerGroup::get_multi_data`).
     */
    template <class Group, typename... Args>
    SBIO_HD auto get(const Group& group, Args&&... args) const {
      return group.resolve_multi_data(m_batch.first,
                                      m_batch.last,
                                      hd_std::forward<Args>(args)...);
    }

  private:
    StepBatch<StepIdxType> m_batch;
    IOStatus m_status;
  };

  namespace impl {
    /**
     * The planned IO for a fixed set of BrokerGroups.
     *
     * Every distinct fetch (StreamBroker + access pattern + step mapping) across all the
     * groups is recorded once, so a step (or batch) is fetched exactly once per distinct
     * fetch, through the Execution policy's `get_data` / `get_data_steps` hooks.
     *
     * @tparam ExecutionPolicy The Execution policy.
     * @tparam FTraits The data format.
     * @tparam Groups The BrokerGroup types.
     */
    template <class ExecutionPolicy, class FTraits, class... Groups>
    class ZipFetchPlan {
    public:
      using StepIdxType = typename FTraits::StepIdxType;

      /**
       * The maximum number of distinct fetches over every StreamBroker of every group.
       */
      static constexpr hd_std::size_t MaxFetches {
        ( Groups::SegmentCapacity + ... + 0 )
      };

      SBIO_HD explicit ZipFetchPlan(const Groups&... groups)
        : m_groups(&groups...)
      {
        plan_fetches(hd_std::index_sequence_for<Groups...> {});
      }

      /**
       * The number of distinct fetches performed per step (or batch).
       */
      SBIO_HD hd_std::size_t num_fetches() const { return m_fetches.size(); }

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

      /**
       * Perform every planned fetch for the batch [first, last), through the policy.
       */
      SBIO_HD IOStatus fetch(StepBatch<StepIdxType> batch) const {
        auto unit_fetcher = [&](hd_std::size_t i, auto&&...) {
          return fetch_range_one(m_fetches[i], batch, hd_std::index_sequence_for<Groups...> {});
        };
        auto no_access = [](hd_std::size_t, hd_std::size_t) {}; // Data is resolved on demand via BatchHandle::get

        return ExecutionPolicy::template get_data_steps<FTraits>({ batch.first, batch.last },
                                                                 unit_fetcher,
                                                                 m_fetches.size(),
                                                                 no_access,
                                                                 0);
      }

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
                                 StepIdxType step,
                                 hd_std::index_sequence<Is...>) const {
        IOStatus status { IOStatus::Success };
        auto do_fetch = [&](const auto* group, hd_std::size_t idx) {
          if (idx == fetch_.group) {
            StepIdxType target_idx { step };
            status = group->fetch_next_for(target_idx, fetch_.broker_no);
          }
        };

        ( do_fetch(hd_std::get<Is>(m_groups), Is), ... );

        return status;
      }

      template <hd_std::size_t... Is>
      SBIO_HD IOStatus fetch_range_one(const Fetch& fetch_,
                                       StepBatch<StepIdxType> batch,
                                       hd_std::index_sequence<Is...>) const {
        IOStatus status { IOStatus::Success };
        auto do_fetch = [&](const auto* group, hd_std::size_t idx) {
          if (idx == fetch_.group) {
            status = group->fetch_steps_for({ batch.first, batch.last },
                                            fetch_.broker_no);
          }
        };

        ( do_fetch(hd_std::get<Is>(m_groups), Is), ... );

        return status;
      }

      hd_std::tuple<const Groups*...> m_groups;
      BoundedList<Fetch, MaxFetches> m_fetches;
    };
  } // namespace impl

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
    using FetchPlan = impl::ZipFetchPlan<ExecutionPolicy, FTraits, Groups...>;

    SBIO_HD ZippedGroupRange(DS& ds, const Groups&... groups)
      : m_steps(ds.steps())
      , m_plan(groups...)
    {}

    ZippedGroupRange(const ZippedGroupRange&) = delete;
    ZippedGroupRange& operator=(const ZippedGroupRange&) = delete;

    // TODO: This Iterator and the batched one are nearly identical - try to factor out
    //       Only need to generalize the exhausted checks and fetch a bit...
    template <class FP>
    class IteratorImpl {
    public:
      using iterator_category = hd_std::input_iterator_tag;
      using difference_type = hd_std::ptrdiff_t;
      using value_type = StepHandle<StepIdxType>;
      using pointer = void;
      using reference = value_type;

      SBIO_HD IteratorImpl(FP* plan, StepIt it)
        : m_plan(plan)
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
          m_status = m_plan->fetch(step);
        }
      }

      FP* m_plan;
      StepIt m_it;
      IOStatus m_status { IOStatus::Success };
    };

    using Iterator = IteratorImpl<FetchPlan>;
    using ConstIterator = IteratorImpl<const FetchPlan>;

    /**
     * Return an iterator at the first step for this unit, with its data fetched.
     *
     * @note This invokes `next` (once) to get the first step.
     */
    SBIO_HD ConstIterator begin() const {
      return ConstIterator(&m_plan, m_steps.begin());
    }
    SBIO_HD ConstIterator end() const {
      return ConstIterator(&m_plan, m_steps.end());
    }

    SBIO_HD ConstIterator cbegin() const { return begin(); }
    SBIO_HD ConstIterator cend() const { return end(); }

    /**
     * The number of distinct fetches performed per step.
     */
    SBIO_HD hd_std::size_t num_fetches() const { return m_plan.num_fetches(); }

  private:
    StepRange m_steps;
    FetchPlan m_plan;
  };

  /**
   * Iterate contiguous step batches of a DataSource with fetches over a set of BrokerGroups.
   *
   * Equivalent to the `ZippedGroupRange` but for the batched API calls.
   *
   * Each parallel executing unit (e.g. a thread or rank, etc.) iterating will setup
   * its own range-based for loop (`for (auto batch : zip_batches(ds, bg1, bg2)) {}`).
   *
   * @note The batch size is `max_batch_size` used to configure the StreamBrokers. This
   *       is currently not adjustable at the zipped layer.
   *
   * @tparam DS The DataSource type (const or non-const).
   * @tparam Groups The BrokerGroup types, all created from the DataSource.
   */
  template <class DS, class... Groups>
  class ZippedBatchRange {
  public:
    using StepIdxType = typename hd_std::remove_const_t<DS>::StepIdxType;
    using FTraits = typename hd_std::remove_const_t<DS>::DataFormat;
    using ExecutionPolicy = typename hd_std::remove_const_t<DS>::ExecutionPolicy;
    using BatchRange = decltype(hd_std::declval<DS&>().batches());
    using BatchIt = decltype(hd_std::declval<const BatchRange&>().begin());
    using FetchPlan = impl::ZipFetchPlan<ExecutionPolicy, FTraits, Groups...>;

    SBIO_HD ZippedBatchRange(DS& ds, const Groups&... groups)
      : m_batches(ds.batches())
      , m_plan(groups...)
    {}

    ZippedBatchRange(const ZippedBatchRange&) = delete;
    ZippedBatchRange& operator=(const ZippedBatchRange&) = delete;

    template <class FP>
    class IteratorImpl {
    public:
      using iterator_category = hd_std::input_iterator_tag;
      using difference_type = hd_std::ptrdiff_t;
      using value_type = BatchHandle<StepIdxType>;
      using pointer = void;
      using reference = value_type;

      SBIO_HD IteratorImpl(const FetchPlan* plan, BatchIt it)
        : m_plan(plan)
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
        const StepBatch<StepIdxType> batch { *m_it };
        if (batch.first != FTraits::ExhaustedSentinel) {
          m_status = m_plan->fetch(batch);
        }
      }

      FP* m_plan;
      BatchIt m_it;
      IOStatus m_status { IOStatus::Success };
    };

    using Iterator = IteratorImpl<FetchPlan>;
    using ConstIterator = IteratorImpl<const FetchPlan>;

    /**
     * Return an iterator at the first batch for this unit, with its data fetched.
     *
     * @note This invokes `next_batch` (once) to get the first batch.
     */
    SBIO_HD ConstIterator begin() const {
      return ConstIterator(&m_plan, m_batches.begin());
    }
    SBIO_HD ConstIterator end() const {
      return ConstIterator(&m_plan, m_batches.end());
    }

    SBIO_HD ConstIterator cbegin() const { return begin(); }
    SBIO_HD ConstIterator cend() const { return end(); }

    /**
     * The number of distinct fetches performed per batch.
     */
    SBIO_HD hd_std::size_t num_fetches() const { return m_plan.num_fetches(); }

  private:
    BatchRange m_batches;
    FetchPlan m_plan;
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

  /**
   * Zip a fixed set of BrokerGroups from a DataSource for iteration over its step batches.
   *
   * @param[in] ds The DataSource the groups were created from.
   * @param[in] groups The BrokerGroups to fetch for at each batch.
   * @returns A range to iterate over in a range-based for loop.
   */
  template <class DS, class... Groups>
  SBIO_HD ZippedBatchRange<DS, Groups...> zip_batches(DS& ds,
                                                      const Groups&... groups) {
    return ZippedBatchRange<DS, Groups...>(ds, groups...);
  }
} // namespace sbio

#endif // SBIO_CORE_ITERATION_HH
