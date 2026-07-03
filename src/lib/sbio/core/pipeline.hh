#ifndef SBIO_CORE_PIPELINE_HH
#define SBIO_CORE_PIPELINE_HH

#include <tuple>
#include <type_traits>
#include <utility>

namespace sbio {

  /**
   * A PipelineTask represents a deferred operation in the execution graph.
   * It uses a functional approach to chain operations together.
   */
  template <typename Func>
  struct PipelineTask {
    Func transform;

    /**
     * Chaining operator (>>). Takes the result of the current task
     * and passes it as the input to the next functional operation.
     */
    template <typename NextOp>
    auto operator>>(NextOp&& next_op) const {
      auto chained = [f = transform,
                      next = std::forward<NextOp>(next_op)](auto&& input) {
        return next(f(std::forward<decltype(input)>(input)));
      };

      return PipelineTask<decltype(chained)> { chained };
    }

    /**
     * Support for combining tasks (e.g., for accumulation)
     */
    template <typename OtherFunc>
    auto operator+(const PipelineTask<OtherFunc>& other) const {
      auto combined_func = [t1 = *this,
                            t2 = other] (auto&&... args) {
        return
          t1.transform(std::forward<decltype(args)>(args)...) +
          t2.transform(std::forward<decltype(args)>(args)...);
      };

      return PipelineTask<decltype(combined_func)> { combined_func };
    }
  };

  /**
   * A source node that knows how to pull data from a specific detector
   * using the current state of the execution context.
   */
  template <typename DetType, typename... Args>
  struct DetectorSource {
    DetType detector;
    std::tuple<Args...> req_args;

    DetectorSource(DetType det, Args&&... args)
      : detector(std::move(det))
      , req_args(std::forward<Args>(args)...)
    {}

    template <typename Op>
    auto operator>>(Op&& op) const {
      auto start = [det = detector, args = req_args] (auto& ctx) {
        auto getter = [&] (auto&&... unpacked_args) {
          return det.get_data(ctx.current_step(), unpacked_args...);
        };
        return std::apply(getter, args);
      };

      return PipelineTask<decltype(start)> { start } >> std::forward<Op>(op);
    }
  };

  // Helper for class template argument deduction
  template <typename DetType, typename... Args>
  DetectorSource(DetType, Args...) -> DetectorSource<DetType, Args...>;

  /**
   * Helper to create a source task (e.g., reading from a detector)
   */
  template <typename SourceFunc>
  auto make_source(SourceFunc&& src) {
    return PipelineTask<std::decay_t<SourceFunc>> { std::forward<SourceFunc>(src) };
  }

} // namespace sbio

#endif // SBIO_CORE_PIPELINE_HH
