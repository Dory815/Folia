// Cooperative, request-scoped cancellation for native math rendering.
export module folia.math.cancellation;
import std;

export namespace folia::math::cancellation {
enum class CheckpointStage { TeX, Layout, Scene, Serialization };

class Cancelled final : public std::exception {
public:
    const char* what() const noexcept override { return "Math rendering cancelled"; }
};

class Request final {
    std::stop_token stop_;
    std::optional<std::chrono::steady_clock::time_point> deadline_;
public:
    explicit Request(std::stop_token stop = {},
        std::optional<std::chrono::steady_clock::time_point> deadline = {}) noexcept
        : stop_(stop), deadline_(deadline) {}
    bool requested() const {
        return stop_.stop_requested() ||
            (deadline_ && std::chrono::steady_clock::now() >= *deadline_);
    }
    void checkpoint() const { if (requested()) throw Cancelled{}; }
};

// A request scope is thread-local and restores the prior scope when nested.
class Scope final {
    Request const* previous_;
public:
    explicit Scope(Request const&) noexcept;
    ~Scope();
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
};
void checkpoint(CheckpointStage stage=CheckpointStage::TeX);

}
