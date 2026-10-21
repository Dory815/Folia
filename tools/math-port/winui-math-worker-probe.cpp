#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "media/MathJaxRenderer.h"

using namespace std::chrono_literals;
using winrt::Folia::AsyncWorkDependency;
using winrt::Folia::MathJaxErrorKind;
using winrt::Folia::MathJaxRenderer;
using winrt::Folia::MathJaxSvg;

namespace {
void require(bool condition, std::string_view message) {
    if (!condition)
        throw std::runtime_error(std::string(message));
}

class WorkerProbe {
public:
    WorkerProbe() {
        renderer_.SetCompletionCallback([this] {
            {
                std::scoped_lock lock(mutex_);
                ++completions_;
            }
            ready_.notify_all();
        });
        renderer_.SetEnabled(true);
        require(renderer_.Enabled(), "worker enabled");
    }
    ~WorkerProbe() {
        renderer_.SetCompletionCallback({});
        renderer_.SetEnabled(false);
    }

    std::shared_ptr<MathJaxSvg const> request(std::string_view tex, bool display = false,
                                              float em = 16.0f, float width = 1280.0f,
                                              bool highPriority = false,
                                              AsyncWorkDependency *observed = nullptr) {
        std::uint64_t before;
        {
            std::scoped_lock lock(mutex_);
            before = completions_;
        }
        AsyncWorkDependency dependency;
        auto value = renderer_.GetOrQueue(tex, display, em, width, true, highPriority,
                                          &dependency);
        if (!value) {
            require(static_cast<bool>(dependency), "cache miss publishes pending dependency");
            waitForCompletion(before);
            value = renderer_.GetOrQueue(tex, display, em, width, false);
            require(value != nullptr, "completed request is in cache");
            const std::array dependencies{dependency};
            require(renderer_.AnyCompletedAfter(dependencies), "completion revision advances");
        }
        if (observed)
            *observed = std::move(dependency);
        return value;
    }

    std::uint64_t completionCount() {
        std::scoped_lock lock(mutex_);
        return completions_;
    }
    bool staysPendingFor(std::uint64_t before, std::chrono::milliseconds duration) {
        std::unique_lock lock(mutex_);
        ready_.wait_for(lock, duration, [&] { return completions_ > before; });
        return completions_ == before;
    }

    MathJaxRenderer &renderer() noexcept { return renderer_; }

private:
    void waitForCompletion(std::uint64_t before) {
        const auto localEnd = std::chrono::steady_clock::now() + 6s;
        const auto end = (std::min)(localEnd, overallEnd_);
        std::unique_lock lock(mutex_);
        require(ready_.wait_until(lock, end, [&] { return completions_ > before; }),
                "worker completion callback within bounded phase deadline");
    }

    MathJaxRenderer renderer_;
    std::mutex mutex_;
    std::condition_variable ready_;
    std::uint64_t completions_{};
    const std::chrono::steady_clock::time_point overallEnd_ =
        std::chrono::steady_clock::now() + 60s;
};

void checkFragments(MathJaxSvg const &value, std::string_view name) {
    require(static_cast<bool>(value), std::string(name) + ": parsed result is usable");
    require(value.width > 0 && value.height > 0, std::string(name) + ": aggregate dimensions");
    require(!value.fragments.empty(), std::string(name) + ": at least one parsed SVG fragment");
    for (auto const &fragment : value.fragments) {
        require(fragment.svg && !fragment.svg->empty(), std::string(name) + ": fragment SVG bytes");
        require(fragment.width > 0 && fragment.height > 0,
                std::string(name) + ": fragment dimensions");
    }
}

void runProbe() {
    WorkerProbe probe;
    auto mathbb = probe.request("\\mathbb{R}");
    checkFragments(*mathbb, "mathbb");
    std::cout << "mathbb fragments=" << mathbb->fragments.size() << '\n';

    AsyncWorkDependency pending;
    constexpr std::string_view fraction = R"(\frac{1}{2})";
    auto frac = probe.request(fraction, true, 16.0f, 1280.0f, true, &pending);
    checkFragments(*frac, "fraction");
    require(static_cast<bool>(pending), "fraction records pending dependency");
    auto same = probe.renderer().GetOrQueue(fraction, true, 16.0f, 1280.0f, false);
    auto sameAgain = probe.renderer().GetOrQueue(fraction, true, 16.0f, 1280.0f, true);
    require(same && sameAgain && same == sameAgain && same == frac,
            "same cache key returns identical shared result");
    std::cout << "fraction cache=shared dependency=completed\n";

    constexpr std::string_view unicode = "\xCE\xB1" "+" "\xCE\xB2";
    auto rawUnicode = probe.request(unicode);
    checkFragments(*rawUnicode, "raw Unicode");
    std::cout << "raw-unicode fragments=" << rawUnicode->fragments.size() << '\n';

    auto undefined = probe.request(R"(\foliaNoSuchControlSequence{z})");
    checkFragments(*undefined, "noundefined package output");
    require(undefined->errorKind == MathJaxErrorKind::None,
            "noundefined renders unknown commands without an merror");
    std::cout << "undefined-command=noundefined-output\n";

    constexpr std::string_view malformed = R"(\frac{a}{)";
    auto formula = probe.request(malformed);
    require(formula->errorKind == MathJaxErrorKind::Formula,
            "malformed TeX is classified as a formula error");
    checkFragments(*formula, "malformed formula fragments");
    require(!formula->error.empty() && !formula->fragments.empty(),
            "formula error retains diagnostic and parsed fragments");
    std::cout << "malformed kind=Formula fragments=" << formula->fragments.size() << '\n';

    auto recovered = probe.request(R"(\sqrt{2})");
    checkFragments(*recovered, "request after formula error");
    std::cout << "after-error=success\n";

    const std::string stale = "\\underbrace{" + std::string(1200, 'x') + "}";
    AsyncWorkDependency staleDependency;
    auto queued = probe.renderer().GetOrQueue(stale, false, 16.0f, 1280.0f, true, false,
                                               &staleDependency);
    require(!queued && static_cast<bool>(staleDependency), "long old-generation request queued");
    probe.renderer().Clear();
    require(!probe.renderer().GetOrQueue(stale, false, 16.0f, 1280.0f, false),
            "Clear drops old key from cache");
    auto afterClear = probe.request(R"(\frac{c}{d})");
    checkFragments(*afterClear, "request after Clear");
    require(!probe.renderer().GetOrQueue(stale, false, 16.0f, 1280.0f, false),
            "old-generation completion does not repopulate cache");
    std::cout << "clear=old-generation-dropped\n";

    const std::string active = "\\underbrace{" + std::string(6000, 'x') + "}";
    AsyncWorkDependency activeDependency;
    const auto beforeStop = probe.completionCount();
    auto activeResult = probe.renderer().GetOrQueue(active, false, 16.0f, 1280.0f, true, true,
                                                     &activeDependency);
    require(!activeResult && static_cast<bool>(activeDependency), "long request queued before stop");
    require(probe.staysPendingFor(beforeStop, 20ms),
            "long legal request remains active through the bounded stop setup window");
    const auto stopStart = std::chrono::steady_clock::now();
    probe.renderer().SetEnabled(false);
    const auto stopTime = std::chrono::steady_clock::now() - stopStart;
    require(stopTime < 10s, "disable and worker join complete within ten seconds");
    require(!probe.renderer().Enabled(), "worker disabled after join");
    probe.renderer().SetEnabled(true);
    require(probe.renderer().Enabled(), "worker re-enabled");
    auto restarted = probe.request(R"(\frac{e}{f})");
    checkFragments(*restarted, "request after worker restart");
    std::cout << "disable-join-ms="
              << std::chrono::duration_cast<std::chrono::milliseconds>(stopTime).count()
              << " reenabled=success\n";
}
} // namespace

int main() {
    try {
        runProbe();
        std::cout << "worker integration probe: PASS\n";
        return 0;
    } catch (std::exception const &error) {
        std::cerr << "worker integration probe: FAIL: " << error.what() << '\n';
        return 1;
    }
}
