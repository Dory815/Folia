#include "support/folia_test.hpp"
#include "media/MathJaxRenderer.h"

import folia.math;

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <thread>
#include <vector>

namespace winrt::Folia {
class MathJaxRendererTestAccess {
public:
    static std::unique_ptr<MathJaxRenderer> create(MathJaxWorkerFactory factory) {
        return std::unique_ptr<MathJaxRenderer>(new MathJaxRenderer(std::move(factory)));
    }
};
}

namespace {
using namespace boost::ut;
using namespace winrt::Folia;
using namespace std::chrono_literals;

struct Probe {
    std::mutex mutex;
    std::condition_variable changed;
    std::vector<std::thread::id> factoryThreads;
    std::vector<std::thread::id> renderThreads;
    std::vector<std::chrono::steady_clock::time_point> deadlines;
    std::vector<std::chrono::milliseconds> deadlineBudgets;
    std::thread::id callbackThread;
    std::thread::id destroyedThread;
    std::size_t calls = 0;
    std::size_t callbacks = 0;
    bool firstEntered = false;
    bool releaseFirst = false;
    bool block = false;
    bool release = false;
    bool waitForStop = false;
    bool sawStop = false;
};

struct SessionLifetime {
    std::shared_ptr<Probe> probe;
    explicit SessionLifetime(std::shared_ptr<Probe> value) : probe(std::move(value)) {}
    ~SessionLifetime() {
        std::scoped_lock lock(probe->mutex);
        probe->destroyedThread = std::this_thread::get_id();
        probe->changed.notify_all();
    }
};

bool waitFor(Probe& probe, std::function<bool(Probe const&)> predicate) {
    std::unique_lock lock(probe.mutex);
    return probe.changed.wait_for(lock, 5s, [&] { return predicate(probe); });
}

MathJaxSvg success(std::string text) {
    MathJaxSvg result;
    result.width = 10;
    result.height = 10;
    result.fragments.push_back({0, std::make_shared<std::string const>(std::move(text)), 10, 10});
    return result;
}

MathJaxWorkerFactory factoryFor(std::shared_ptr<Probe> probe) {
    return [probe = std::move(probe)] {
        {
            std::scoped_lock lock(probe->mutex);
            probe->factoryThreads.push_back(std::this_thread::get_id());
            probe->changed.notify_all();
        }
        auto session = std::make_shared<SessionLifetime>(probe);
        return [probe, session](std::string_view tex, bool, float, float, std::stop_token stop,
                                std::chrono::steady_clock::time_point deadline) {
            std::unique_lock lock(probe->mutex);
            const auto call = ++probe->calls;
            probe->renderThreads.push_back(std::this_thread::get_id());
            probe->deadlines.push_back(deadline);
            probe->deadlineBudgets.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(
                deadline - std::chrono::steady_clock::now()));
            probe->firstEntered = true;
            probe->changed.notify_all();
            if (probe->waitForStop) {
                std::stop_callback wake(stop, [probe] { probe->changed.notify_all(); });
                probe->changed.wait_for(lock, 5s, [&] { return stop.stop_requested(); });
                probe->sawStop = stop.stop_requested();
                probe->changed.notify_all();
                lock.unlock();
                throw folia::math::RenderError("Cancelled", "worker stop requested");
            }
            std::stop_callback wake(stop, [probe] { probe->changed.notify_all(); });
            if (probe->block && call == 1)
                probe->changed.wait(lock, [&] { return probe->releaseFirst || stop.stop_requested(); });
            if (probe->block && call > 1)
                probe->changed.wait(lock, [&] { return probe->release || stop.stop_requested(); });
            lock.unlock();
            if (tex == "throw-native")
                throw folia::math::RenderError("NativeFailure", "test diagnostic");
            return success(std::string(tex) + "#" + std::to_string(call));
        };
    };
}

std::string_view cachedSvg(std::shared_ptr<MathJaxSvg const> const& result) {
    if (!result || result->fragments.empty() || !result->fragments.front().svg) return {};
    return *result->fragments.front().svg;
}

suite mathjax_worker_harness = [] {
    "clear drops an active generation and same-key retry completes with a fresh ticket"_test = [] {
        auto probe = std::make_shared<Probe>();
        probe->block = true;
        auto renderer = MathJaxRendererTestAccess::create(factoryFor(probe));
        renderer->SetCompletionCallback([probe] {
            std::scoped_lock lock(probe->mutex);
            ++probe->callbacks;
            probe->callbackThread = std::this_thread::get_id();
            probe->changed.notify_all();
        });
        renderer->SetEnabled(true);
        AsyncWorkDependency oldDependency;
        (void)renderer->GetOrQueue("same", false, 16, 400, true, false, &oldDependency);
        expect(waitFor(*probe, [](Probe const& p) { return p.firstEntered; }))
            << "first generation entered its injected renderer on the worker";
        renderer->Clear();
        expect(!renderer->AnyCompletedAfter(std::span(&oldDependency, 1)))
            << "clear removes the old completion revision";
        AsyncWorkDependency newDependency;
        (void)renderer->GetOrQueue("same", false, 16, 400, true, false, &newDependency);
        {
            std::scoped_lock lock(probe->mutex);
            probe->releaseFirst = true;
            probe->changed.notify_all();
        }
        expect(waitFor(*probe, [](Probe const& p) { return p.calls == 2 && p.callbacks == 1; }))
            << "stale in-flight output is dropped and queued same-key work completes";
        expect(renderer->AnyCompletedAfter(std::span(&newDependency, 1)))
            << "only fresh work advances the dependency revision";
        auto cached = renderer->GetOrQueue("same", false, 16, 400, false, false);
        expect(cachedSvg(cached) == "same#2")
            << "cache contains fresh output, not stale output";
        {
            std::scoped_lock lock(probe->mutex);
            expect(probe->callbacks == 1 && probe->callbackThread == probe->renderThreads.back())
                << "completion runs once on the rendering worker thread";
        }
        renderer->SetCompletionCallback({});
        renderer->SetEnabled(false);
    };

    "separate MathJaxRenderer owners create sessions on separate workers"_test = [] {
        auto a = std::make_shared<Probe>();
        auto b = std::make_shared<Probe>();
        a->block = b->block = true;
        auto first = MathJaxRendererTestAccess::create(factoryFor(a));
        auto second = MathJaxRendererTestAccess::create(factoryFor(b));
        first->SetEnabled(true);
        second->SetEnabled(true);
        (void)first->GetOrQueue("a", false, 16, 400);
        (void)second->GetOrQueue("b", false, 16, 400);
        expect(waitFor(*a, [](Probe const& p) { return p.firstEntered; })) << "first worker entered";
        expect(waitFor(*b, [](Probe const& p) { return p.firstEntered; })) << "second worker entered";
        {
            std::scoped_lock lock(a->mutex, b->mutex);
            expect(a->factoryThreads.front() == a->renderThreads.front()) << "first session stays on its worker";
            expect(b->factoryThreads.front() == b->renderThreads.front()) << "second session stays on its worker";
            expect(a->renderThreads.front() != b->renderThreads.front()) << "renderer owners have separate workers";
            a->release = b->release = true;
            a->changed.notify_all();
            b->changed.notify_all();
        }
        first->SetEnabled(false);
        second->SetEnabled(false);
    };

    "typed cancellation detects an unusable session and recovers on the same worker"_test = [] {
        auto probe = std::make_shared<Probe>();
        auto factory = [probe] {
            {
                std::scoped_lock lock(probe->mutex);
                probe->factoryThreads.push_back(std::this_thread::get_id());
                probe->changed.notify_all();
            }
            auto session = std::make_shared<SessionLifetime>(probe);
            return [probe, session](std::string_view tex, bool, float, float, std::stop_token,
                                    std::chrono::steady_clock::time_point deadline) {
                std::unique_lock lock(probe->mutex);
                const auto call = ++probe->calls;
                probe->renderThreads.push_back(std::this_thread::get_id());
                probe->deadlines.push_back(deadline);
                probe->deadlineBudgets.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(
                    deadline - std::chrono::steady_clock::now()));
                probe->changed.notify_all();
                lock.unlock();
                if (call == 1) throw folia::math::RenderError("Cancelled", "deadline reached");
                if (call == 2) throw folia::math::RenderError("SessionUnusable", "session invalidated after cancellation");
                return success(std::string(tex) + "#" + std::to_string(call));
            };
        };
        auto renderer = MathJaxRendererTestAccess::create(std::move(factory));
        renderer->SetCompletionCallback([probe] {
            std::scoped_lock lock(probe->mutex);
            ++probe->callbacks;
            probe->changed.notify_all();
        });
        renderer->SetEnabled(true);
        (void)renderer->GetOrQueue("recover", false, 16, 400);
        expect(waitFor(*probe, [](Probe const& p) { return p.calls == 1 && p.callbacks == 1; }))
            << "first native call is cancelled and published as transient completion";
        expect(!renderer->GetOrQueue("recover", false, 16, 400, false, false))
            << "first cancellation is transient and is not cached";
        (void)renderer->GetOrQueue("recover", false, 16, 400);
        expect(waitFor(*probe, [](Probe const& p) { return p.calls == 2 && p.callbacks == 2 && p.factoryThreads.size() == 1; }))
            << "follow-up reports SessionUnusable on the same renderer after in-flight cancellation";
        expect(!renderer->GetOrQueue("recover", false, 16, 400, false, false))
            << "SessionUnusable is also transient and not cached";
        (void)renderer->GetOrQueue("recover", false, 16, 400);
        expect(waitFor(*probe, [](Probe const& p) { return p.calls == 3 && p.callbacks == 3 && p.factoryThreads.size() == 2; }))
            << "worker constructs a fresh renderer before the next request";
        auto result = renderer->GetOrQueue("recover", false, 16, 400, false, false);
        expect(cachedSvg(result) == "recover#3") << "retry uses the replacement session";
        {
            std::scoped_lock lock(probe->mutex);
            expect(probe->factoryThreads.front() == probe->factoryThreads.back())
                << "replacement session remains on this renderer's worker";
            expect(probe->renderThreads.front() == probe->renderThreads.back())
                << "both session calls remain on the same worker";
            expect(probe->deadlines.size() == 3 && probe->deadlineBudgets.size() == 3
                && std::all_of(probe->deadlineBudgets.begin(), probe->deadlineBudgets.end(), [](auto budget) {
                    return budget >= 1800ms && budget <= 2s;
                })) << "every retry receives a fresh two-second per-request deadline";
        }
        renderer->SetEnabled(false);
    };

    "native RenderError becomes a cached infrastructure result"_test = [] {
        auto probe = std::make_shared<Probe>();
        auto renderer = MathJaxRendererTestAccess::create(factoryFor(probe));
        renderer->SetEnabled(true);
        AsyncWorkDependency dependency;
        (void)renderer->GetOrQueue("throw-native", false, 16, 400, true, false, &dependency);
        expect(waitFor(*probe, [](Probe const& p) { return p.callbacks == 1; })) << "exception completes request";
        auto result = renderer->GetOrQueue("throw-native", false, 16, 400, false, false);
        expect(result && result->errorKind == MathJaxErrorKind::Infrastructure) << "failure kind is retained";
        expect(result && result->error == "NativeFailure: test diagnostic") << "name and message are retained";
        expect(renderer->AnyCompletedAfter(std::span(&dependency, 1))) << "failure advances its revision";
        renderer->SetCompletionCallback({});
        renderer->SetEnabled(false);
    };

    "destruction cancels active work and suppresses stale completion"_test = [] {
        auto probe = std::make_shared<Probe>();
        probe->waitForStop = true;
        auto renderer = MathJaxRendererTestAccess::create(factoryFor(probe));
        std::atomic_size_t callbackCalls = 0;
        renderer->SetCompletionCallback([&] { ++callbackCalls; });
        renderer->SetEnabled(true);
        (void)renderer->GetOrQueue("active", false, 16, 400);
        expect(waitFor(*probe, [](Probe const& p) { return p.firstEntered; })) << "render is active";
        renderer.reset();
        {
            std::scoped_lock lock(probe->mutex);
            expect(probe->sawStop) << "stop token reaches active render callback";
            expect(probe->factoryThreads.front() == probe->renderThreads.front()) << "session owner stays on worker";
            expect(probe->destroyedThread == probe->factoryThreads.front()) << "session destructs on its worker";
        }
        expect(callbackCalls.load() == 0) << "stale shutdown completion does not touch the UI callback";
    };

    "queued dispatcher callback can outlive the renderer without retaining its UI owner"_test = [] {
        struct UiOwner { std::atomic_size_t touches = 0; };
        auto probe = std::make_shared<Probe>();
        auto owner = std::make_shared<UiOwner>();
        std::weak_ptr<UiOwner> weakOwner = owner;
        std::mutex mutex;
        std::condition_variable changed;
        std::vector<std::function<void()>> queue;
        auto renderer = MathJaxRendererTestAccess::create(factoryFor(probe));
        renderer->SetCompletionCallback([&, weakOwner] {
            {
                std::scoped_lock lock(probe->mutex);
                probe->callbackThread = std::this_thread::get_id();
                ++probe->callbacks;
                probe->changed.notify_all();
            }
            std::scoped_lock lock(mutex);
            queue.push_back([weakOwner] { if (auto target = weakOwner.lock()) ++target->touches; });
            changed.notify_all();
        });
        renderer->SetEnabled(true);
        (void)renderer->GetOrQueue("posted", false, 16, 400);
        expect(waitFor(*probe, [](Probe const& p) { return p.callbacks == 1; })) << "worker callback ran";
        {
            std::unique_lock lock(mutex);
            expect(changed.wait_for(lock, 5s, [&] { return !queue.empty(); })) << "dispatcher queued completion";
        }
        renderer->SetCompletionCallback({});
        renderer.reset();
        owner.reset();
        std::vector<std::function<void()>> queued;
        {
            std::scoped_lock lock(mutex);
            queued.swap(queue);
        }
        for (auto& callback : queued) callback();
        expect(weakOwner.expired()) << "posted callback does not retain the UI owner";
    };
};
}
