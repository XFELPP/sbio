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

  ///--- SourceSet Implementations (Zipping multiple DataSources) ---///

  /**
   * A set of DataSources iterated together, in lockstep by step index.
   *
   * The SourceSet allows grouping multiple DataSources, even those that are viewing
   * different underlying data. Iteration over a SourceSet means iterating over each
   * component DataSource simultaneously - ie.e when a SoruceSet returns a step `n`,
   * it corresponds to step `n` for each DataSource. No remapping/alignment occurs.
   *
   * @note If using a SourceSet for iteration, the individual DataSources must NOT be
   *       iterated independently!
   *
   * @note As the DataSources are iterated together without alignment the capacity is set
   *       to the minimum capacity of all the component DataSources. If reindexing can
   *       occur, though, only the DataSources who reached the capacity are reindexed to
   *       avoid overwriting the step offset data.
   *
   * @note The SourceSet is restricted to DataSources with the same EPolicy and StepIdxType.
   *
   * @note The SourceSet must be constructed from DataSources after the metadata
   *       discovery process.
   *
   * The SourceSet holds its own EPolicy IterationState and therefore behaves like an
   * independent DataSource.
   *
   * Each parallel executing unit (e.g. a thread or rank, etc.) iterating will setup
   * its own range-based for loop over the zipped set. First create the set, then zip
   * the BrokerGroups of intereset.
   *
   * @code{.cpp}
   *
   * auto set = source_set(ds_a, ds_b);
   * for (auto step : zip(set, bg1, bg2)) {
   *   auto hdl1 = step.get(bg1);
   *   auto hdl2 = step.get(bg2);
   * }
   *
   * @endcode
   *
   * @note Early exit from a loop invokes the EPolicy end iteration implementation, the
   *       same as using `DataSource::next()` (or `steps()`, etc.)
   *
   * @tparam Sources The DataSource types. They must use the same EPolicy and StepIdxType.
   */
  template <class... Sources>
  class SourceSet {
    static_assert(sizeof...(Sources) > 0, "A SourceSet needs at least one DataSource.");

    using First = hd_std::remove_const_t<hd_std::tuple_element_t<0, hd_std::tuple<Sources...>>>;

  public:
    using DataFormat = typename First::DataFormat;
    using ExecutionPolicy = typename First::ExecutionPolicy;
    using StepIdxType = typename First::StepIdxType;

    static_assert((hd_std::is_same_v<typename hd_std::remove_const_t<Sources>::ExecutionPolicy,
                                     ExecutionPolicy> && ...),
                  "The DataSources of a SourceSet must use the same EPolicy.");
    static_assert((hd_std::is_same_v<typename hd_std::remove_const_t<Sources>::StepIdxType,
                                     StepIdxType> && ...),
                  "The DataSources of a SourceSet must use the same StepIdxType.");

    /**
     * Construct a set over DataSources which have already discovered their metadata.
     */
    SBIO_HD explicit SourceSet(Sources&... sources)
      : m_sources(&sources...)
    {
      m_capacity = min_capacity();
    }

    SourceSet(const SourceSet&) = delete;
    SourceSet& operator=(const SourceSet&) = delete;

    /**
     * Request the next index for a step to read data for, common to all DataSources.
     *
     * @returns The next step index to fetch and query data for, or the ExhaustedSentinel if
     *          no more data is available.
     */
    SBIO_HD StepIdxType next() const {
      auto trigger = [&] () { return reindex_trigger(); };

      return ExecutionPolicy::template next<DataFormat>(m_iteration_state,
                                                        m_capacity,
                                                        trigger);
    }

    /**
     * Request the next contiguous batch of indices for steps, common to all DataSources.
     *
     * @note A batch is providfed with a maximum size - it may be smaller, as it will
     *       not extend beyond the current maximum capacity.
     *
     * @note If `batch_size` is left to 0, then the default size will be the smallest
     *       `max_batch_size` of the component DataSources in the set.
     *
     * @param[in] batch_size The maximum size of the batch of step indcies to read.
     * @returns The next back of step indices [first, last) to fetch data for. The
     *          Exhausted batch is returned as first and last equal to ExhaustedSentinel
     */
    SBIO_HD StepBatch<StepIdxType> next_batch(hd_std::size_t batch_size = 0) const {
      if (batch_size == 0) {
        batch_size = min_batch_size();
      }

      auto trigger = [&] () { return reindex_trigger(); };

      return ExecutionPolicy::template next_batch<DataFormat>(m_iteration_state,
                                                              batch_size,
                                                              m_capacity,
                                                              trigger);
    }

    /**
     * An iterator implementation to allow generating step indices from the set.
     *
     * @tparam Set The SourceSet to iterate over (const/non-const etc.)
     * @tparam Batched Whether the iterator generates batches of steps.
     */
    template <class Set, bool Batched>
    class IteratorImpl {
    public:
      using iterator_category = hd_std::input_iterator_tag;
      using difference_type = hd_std::ptrdiff_t;
      using value_type = hd_std::conditional_t<Batched, StepBatch<StepIdxType>, StepIdxType>;
      using pointer = void;
      using reference = value_type;

      SBIO_HD IteratorImpl(Set& set, value_type value, hd_std::size_t batch_size = 0)
        : m_set(set)
        , m_value(value)
        , m_batch_size(batch_size)
      {}

      SBIO_HD reference operator*() const { return m_value; }

      SBIO_HD IteratorImpl& operator++() {
        if constexpr (Batched) {
          m_value = m_set.next_batch(m_batch_size);
        } else {
          m_value = m_set.next();
        }
        return *this;
      }

      SBIO_HD void operator++(int) { ++(*this); }

      SBIO_HD friend bool operator==(const IteratorImpl& a, const IteratorImpl& b) {
        if constexpr (Batched) {
          return a.m_value.first == b.m_value.first;
        } else {
          return a.m_value == b.m_value;
        }
      }

      SBIO_HD friend bool operator!=(const IteratorImpl& a, const IteratorImpl& b) {
        return !(a == b);
      }

    private:
      Set& m_set;
      value_type m_value;
      hd_std::size_t m_batch_size;
    };

    using Iterator = IteratorImpl<SourceSet, false>;
    using ConstIterator = IteratorImpl<const SourceSet, false>;

    using BatchIterator = IteratorImpl<SourceSet, true>;
    using ConstBatchIterator = IteratorImpl<const SourceSet, true>;

    /**
     * A range over the steps (or batches) of the set, for the calling unit.
     *
     * Leaving the loop, by exhaustion or by `break`, ends the unit's iteration
     *with the Execution policy, as for a DataSource.
     *
     * @tparam Set The SourceSet to iterate over (const/non-const etc.)
     * @tparam Batched Whether the range is over batches of steps.
     */
    template <class Set, bool Batched>
    class RangeImpl {
    public:
      using It = IteratorImpl<Set, Batched>;

      SBIO_HD RangeImpl(Set& set, hd_std::size_t batch_size = 0)
        : m_set(set)
        , m_batch_size(batch_size)
      {}

      SBIO_HD ~RangeImpl() { ExecutionPolicy::end_iteration(m_set.m_iteration_state); }

      RangeImpl(const RangeImpl&) = delete;
      RangeImpl& operator=(const RangeImpl&) = delete;

      SBIO_HD It begin() const {
        if constexpr (Batched) {
          return It(m_set, m_set.next_batch(m_batch_size), m_batch_size);
        } else {
          return It(m_set, m_set.next());
        }
      }

      SBIO_HD It end() const {
        if constexpr (Batched) {
          return It(m_set,
                    StepBatch<StepIdxType> { DataFormat::ExhaustedSentinel, DataFormat::ExhaustedSentinel },
                    m_batch_size);
        } else {
          return It(m_set, DataFormat::ExhaustedSentinel);
        }
      }

      SBIO_HD It cbegin() const { return begin(); }
      SBIO_HD It cend() const { return end(); }

    private:
      Set& m_set;
      hd_std::size_t m_batch_size;
    };

    using StepRange = RangeImpl<SourceSet, false>;
    using ConstStepRange = RangeImpl<const SourceSet, false>;

    using BatchRange = RangeImpl<SourceSet, true>;
    using ConstBatchRange = RangeImpl<const SourceSet, true>;

    /**
     * Return a range over the common steps of the set for the calling unit.
     *
     * @returns A range to iterate over in a range-based for loop.
     */
    SBIO_HD StepRange steps() { return StepRange(*this); }
    SBIO_HD ConstStepRange steps() const { return ConstStepRange(*this); }
    SBIO_HD ConstStepRange csteps() const { return ConstStepRange(*this); }

    /**
     * Return a range over contiguous batches of steps, common to the DataSources.
     *
     * @param[in] batch_size The maximum number of steps per batch. The default is the
     *            smallest configured `max_batch_size`.
     * @returns A range to iterate over in a range-based for loop.
     */
    SBIO_HD BatchRange batches(hd_std::size_t batch_size = 0) {
      return BatchRange(*this, batch_size);
    }
    SBIO_HD ConstBatchRange batches(hd_std::size_t batch_size = 0) const {
      return ConstBatchRange(*this, batch_size);
    }
    SBIO_HD ConstBatchRange cbatches(hd_std::size_t batch_size = 0) const {
      return BatchRange(*this, batch_size);
    }

  private:
    /**
     * Reindex the DataSources which reached the common capacity.
     *
     * @returns Whether more common steps are available (false once any is exhausted).
     */
    SBIO_HD bool reindex_trigger() const {
      bool more { true };

      auto reindex = [&](auto* source) {
        if (static_cast<StepIdxType>(source->m_steps_capacity) <= m_capacity) {
          more = source->reindex_trigger() && more;
        }
      };

      hd_std::apply([&](auto*... sources) { (reindex(sources), ...); }, m_sources);

      m_capacity = min_capacity();

      return more;
    }

    SBIO_HD StepIdxType min_capacity() const {
      StepIdxType capacity {
        static_cast<StepIdxType>(hd_std::get<0>(m_sources)->m_steps_capacity)
      };

      auto take_min = [&](auto* source) {
        const auto c { static_cast<StepIdxType>(source->m_steps_capacity) };
        capacity = (c < capacity) ? c : capacity;
      };

      hd_std::apply([&](auto*... sources) { (take_min(sources), ...); }, m_sources);

      return capacity;
    }

    SBIO_HD hd_std::size_t min_batch_size() const {
      hd_std::size_t size {
        hd_std::get<0>(m_sources)->data_stream(0).config().max_batch_size
      };

      auto take_min = [&](auto* source) {
        const hd_std::size_t s { source->data_stream(0).config().max_batch_size };
        size = (s < size) ? s : size;
      };

      hd_std::apply([&](auto*... sources) { (take_min(sources), ...); }, m_sources);

      return size;
    }

    hd_std::tuple<Sources*...> m_sources;
    mutable typename ExecutionPolicy::IterationState m_iteration_state {};
    mutable StepIdxType m_capacity { 0 };
  };

  /**
   * Create a SourceSet to iterate several DataSources together, in lockstep.
   *
   * @param[in] sources The DataSources (after metadata discovery).
   * @returns The SourceSet.
   */
  template <class... Sources>
  SBIO_HD SourceSet<Sources...> source_set(Sources&... sources) {
    return SourceSet<Sources...>(sources...);
  }
} // namespace sbio

#endif // SBIO_CORE_ITERATION_HH
