#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace winrt::Folia::detail
{
    struct EmbeddedInvalidationState
    {
        std::mutex mutex;
        std::function<void()> callback;
        std::atomic_bool completionPending = false;
        std::uint64_t generation = 0;
        bool active = true;
    };

    template<class Enqueue>
    void QueueEmbeddedCompletion(
        std::weak_ptr<EmbeddedInvalidationState> weakState, Enqueue&& enqueue)
    {
        auto state = weakState.lock();
        if (!state || state->completionPending.exchange(true)) return;
        auto accepted = std::forward<Enqueue>(enqueue)([weakState]
        {
            auto state = weakState.lock();
            if (!state) return;
            std::function<void()> callback;
            {
                std::scoped_lock lock(state->mutex);
                state->completionPending = false;
                if (!state->active) return;
                ++state->generation;
                callback = state->callback;
            }
            if (callback) callback();
        });
        if (!accepted)
        {
            if (auto state = weakState.lock()) state->completionPending = false;
        }
    }

    inline void DeactivateEmbeddedInvalidation(
        std::shared_ptr<EmbeddedInvalidationState> const& state)
    {
        std::scoped_lock lock(state->mutex);
        state->active = false;
        state->callback = {};
        state->completionPending = false;
    }
}
