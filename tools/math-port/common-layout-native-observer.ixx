// Diagnostic-only module for observing the real Common layout job.
export module folia.math.port.common_layout_native_observer;
import std;
export namespace folia::math::port::common_layout {
class WrapperIds {
    std::unordered_map<const void*,std::size_t> ids_;
    std::size_t next_=0;
public:
    std::size_t idOf(const void* wrapper);
};
struct WrapperSnapshot {
    std::size_t rootId=0;
    std::string wrappersJson;
};
WrapperSnapshot snapshotWrappers(const void* root,const void* job,WrapperIds& ids);
class LayoutTrace {
    struct State;
    std::shared_ptr<State> state_;
    std::shared_ptr<void> observer_;
public:
    LayoutTrace();
    const std::shared_ptr<void>& observerOpaque()const noexcept{return observer_;}
    WrapperIds& ids()const noexcept;
    std::string callsJson()const;
    std::string paintJson()const;
};
}
