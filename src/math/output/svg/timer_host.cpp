/*************************************************************
 * Same-thread timer queue for Source SVG action callbacks.
 *************************************************************/
module folia.math.output.svg.timer_host;
import std;

namespace folia::math::output::svg {
using namespace options;
struct TimerHost::State {
    struct Entry { std::uint64_t id; TimePoint due; ObjectRef callback; };
    Context& context;
    Now now;
    std::uint64_t next=1;
    std::vector<Entry> entries;
    explicit State(Context& cx,Now clock):context(cx),now(clock?std::move(clock):Now([]{return Clock::now();})){}
};
TimerHost::TimerHost(Context& context,Now now):state_(std::make_shared<State>(context,std::move(now))){}
TimerHost::~TimerHost()=default;
std::uint64_t TimerHost::schedule(double delayMilliseconds,ObjectRef callback){
    if(state_->context.kind(callback)!=ObjectKind::function)
        throw Exception("TypeError",u"setTimeout callback is not callable");
    if(std::isnan(delayMilliseconds)||delayMilliseconds<0)delayMilliseconds=0;
    const auto now=state_->now();
    const auto remaining=TimePoint::max()-now;
    const auto maxMilliseconds=std::chrono::duration<double,std::milli>(remaining).count();
    const auto delay=delayMilliseconds>=maxMilliseconds
        ?remaining
        :std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double,std::milli>(delayMilliseconds));
    // Handles cross the Context boundary as JavaScript Numbers, so stop before
    // integer identity can be lost to binary64 rounding.
    constexpr std::uint64_t maxSafeInteger=9007199254740991ULL;
    if(state_->next>maxSafeInteger)
        throw Exception("RangeError",u"timer handle space is exhausted");
    const auto id=state_->next++;
    state_->entries.push_back({id,now+delay,callback});
    return id;
}
void TimerHost::cancel(const Value& handle){
    const double number=state_->context.toNumber(handle);
    // double(UINT64_MAX) rounds to 2^64, which is already outside the range
    // accepted by the integer conversion below.
    if(!std::isfinite(number)||number<0||std::floor(number)!=number||
       number>=static_cast<double>(std::numeric_limits<std::uint64_t>::max()))return;
    const auto id=static_cast<std::uint64_t>(number);
    std::erase_if(state_->entries,[&](const auto& entry){return entry.id==id;});
}
std::size_t TimerHost::dispatchDue(){
    // A timer created by a callback belongs to the next event-loop turn even
    // when its delay is zero. Capture the sequence ceiling before dispatch.
    const auto turnCeiling=state_->next-1;
    std::size_t count=0;
    for(;;){
        const auto now=state_->now();
        // Ignore timers queued by a callback in this dispatch turn. They do
        // not block older due timers even if their requested delay is zero.
        auto it=std::min_element(state_->entries.begin(),state_->entries.end(),
            [turnCeiling](const auto& a,const auto& b){
                const bool aEligible=a.id<=turnCeiling,bEligible=b.id<=turnCeiling;
                if(aEligible!=bEligible)return aEligible;
                return a.due<b.due||(a.due==b.due&&a.id<b.id);
            });
        if(it==state_->entries.end()||it->id>turnCeiling||it->due>now)break;
        const auto callback=it->callback;
        state_->entries.erase(it);
        (void)state_->context.call(callback,Value(Undefined{}));
        ++count;
    }
    return count;
}
std::size_t TimerHost::pending()const noexcept{return state_->entries.size();}
}
