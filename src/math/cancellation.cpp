module folia.math.cancellation;
import std;

namespace folia::math::cancellation {
namespace {
thread_local Request const* activeRequest=nullptr;
}
Scope::Scope(Request const& request) noexcept : previous_(activeRequest) { activeRequest=&request; }
Scope::~Scope() { activeRequest=previous_; }
void checkpoint(CheckpointStage) {
    if(activeRequest)activeRequest->checkpoint();
}
}
