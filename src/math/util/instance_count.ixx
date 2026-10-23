export module folia.math.util.instance_count;
import std;

export namespace folia::math::util {
template <class T> class InstanceCount final {
    inline static std::atomic<std::size_t> live_count_{};
    inline static std::atomic<std::size_t> constructed_count_{};
    inline static std::atomic<std::size_t> destroyed_count_{};

    static void construct() noexcept {
        constructed_count_.fetch_add(1, std::memory_order_relaxed);
        live_count_.fetch_add(1, std::memory_order_relaxed);
    }

  public:
    InstanceCount() noexcept {
        construct();
    }
    InstanceCount(InstanceCount const &) noexcept {
        construct();
    }
    InstanceCount(InstanceCount &&) noexcept {
        construct();
    }
    InstanceCount &operator=(InstanceCount const &) noexcept {
        return *this;
    }
    InstanceCount &operator=(InstanceCount &&) noexcept {
        return *this;
    }
    ~InstanceCount() {
        live_count_.fetch_sub(1, std::memory_order_relaxed);
        destroyed_count_.fetch_add(1, std::memory_order_relaxed);
    }

    static std::size_t live() noexcept {
        return live_count_.load(std::memory_order_relaxed);
    }
    static std::size_t constructed() noexcept {
        return constructed_count_.load(std::memory_order_relaxed);
    }
    static std::size_t destroyed() noexcept {
        return destroyed_count_.load(std::memory_order_relaxed);
    }
};
} // namespace folia::math::util
