#include "support/folia_test.hpp"
#include "editor/rendering/EmbeddedCompletionDispatch.h"

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace {
using namespace boost::ut;
using namespace winrt::Folia::detail;

suite embedded_completion_dispatch = [] {
    "production dispatcher closure is weak, active-checked, and coalesced on the UI pump thread"_test = [] {
        auto state = std::make_shared<EmbeddedInvalidationState>();
        std::weak_ptr<EmbeddedInvalidationState> weak = state;
        std::vector<std::function<void()>> queued;
        std::atomic_size_t callbacks = 0;
        std::thread::id callbackThread;
        state->callback = [&] { ++callbacks; callbackThread = std::this_thread::get_id(); };
        auto enqueue = [&](std::function<void()> callback) {
            queued.push_back(std::move(callback));
            return true;
        };
        QueueEmbeddedCompletion(weak, enqueue);
        QueueEmbeddedCompletion(weak, enqueue);
        expect(queued.size() == 1) << "duplicate worker notifications coalesce";
        expect(state->completionPending.load()) << "queued work owns the pending bit";
        expect(state->generation == 0 && callbacks.load() == 0) << "generation changes only on dispatch";
        auto uiThread = std::this_thread::get_id();
        queued.front()();
        expect(state->generation == 1 && callbacks.load() == 1) << "active dispatch advances shared generation once";
        expect(callbackThread == uiThread) << "callback executes on the dispatch queue thread";
        expect(!state->completionPending.load()) << "dispatch clears coalescing state";

        queued.clear();
        QueueEmbeddedCompletion(weak, enqueue);
        auto pending = std::move(queued.front());
        DeactivateEmbeddedInvalidation(state);
        pending();
        expect(state->generation == 1 && callbacks.load() == 1) << "deactivated owner ignores already queued work";
        expect(!state->active && !state->completionPending.load()) << "teardown deactivates and drains shared state";

        state.reset();
        expect(weak.expired()) << "dispatcher callback does not retain invalidation owner state";
    };

    "an already queued production callback cannot retain or touch a destroyed owner"_test = [] {
        auto state = std::make_shared<EmbeddedInvalidationState>();
        std::weak_ptr<EmbeddedInvalidationState> weak = state;
        std::vector<std::function<void()>> queued;
        std::atomic_size_t callbacks = 0;
        state->callback = [&] { ++callbacks; };
        QueueEmbeddedCompletion(weak, [&](std::function<void()> callback) {
            queued.push_back(std::move(callback));
            return true;
        });
        state.reset();
        expect(weak.expired()) << "queued production closure holds only a weak shared-state reference";
        queued.front()();
        expect(callbacks.load() == 0) << "late dispatcher callback cannot access a destroyed owner";
    };

    "rejected platform enqueue releases the actual shared-state coalescing bit"_test = [] {
        auto state = std::make_shared<EmbeddedInvalidationState>();
        QueueEmbeddedCompletion(std::weak_ptr(state), [](std::function<void()>) { return false; });
        expect(!state->completionPending.load()) << "failed TryEnqueue can be retried";
        expect(state->generation == 0) << "failed dispatch does not invalidate a document";
    };
};
}
