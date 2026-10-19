/*************************************************************
 * Context-thread timer queue used by Source SVG action callbacks.
 * The owner drives dispatchDue() from its normal event loop; no worker thread
 * enters the Context.
 *************************************************************/
export module folia.math.output.svg.timer_host;
export import folia.math.util.option_types;
import std;

export namespace folia::math::output::svg {
class TimerHost {
public:
    using Clock=std::chrono::steady_clock;
    using TimePoint=Clock::time_point;
    using Now=std::function<TimePoint()>;
private:
    struct State;
    std::shared_ptr<State> state_;
public:
    explicit TimerHost(options::Context&,Now now={});
    ~TimerHost();
    TimerHost(const TimerHost&)=delete;
    TimerHost& operator=(const TimerHost&)=delete;
    std::uint64_t schedule(double delayMilliseconds,options::ObjectRef callback);
    void cancel(const options::Value& handle);
    std::size_t dispatchDue();
    std::size_t pending()const noexcept;
};
}
