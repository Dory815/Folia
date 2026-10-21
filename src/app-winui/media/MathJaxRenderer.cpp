#include "pch.h"
#include "media/MathJaxRenderer.h"
#include "media/MathJaxSvgParser.h"
#include "folia/math/native_renderer_bridge.h"

import folia.platform.editor_priority_work_queue;

namespace winrt::Folia
{
    using folia::platform::editor::EditorPriorityWorkQueue;
    using folia::platform::editor::EditorQueueChange;
    using folia::platform::editor::EditorWorkPriority;

    struct MathJaxRenderer::State
    {
        struct Request
        {
            std::string key;
            std::string tex;
            bool display = false;
            float em = 0.0f;
            float containerWidth = 0.0f;
            std::uint64_t generation = 0;
        };

        struct RenderOutcome
        {
            MathJaxSvg result;
            bool retryable = false;
        };

        using RendererHandle = std::unique_ptr<FoliaNativeMathRenderer,
            decltype(&FoliaNativeMathRendererDestroy)>;
        using CancellationHandle = std::unique_ptr<FoliaNativeMathCancellation,
            decltype(&FoliaNativeMathCancellationDestroy)>;
        using ResultHandle = std::unique_ptr<FoliaNativeMathResult,
            decltype(&FoliaNativeMathResultDestroy)>;

        std::mutex mutex;
        std::condition_variable_any ready;
        EditorPriorityWorkQueue<Request> requestQueue;
        std::unordered_map<std::string, std::shared_ptr<MathJaxSvg const>> cache;
        std::unordered_map<std::string, std::uint64_t> completionRevisions;
        std::unordered_map<std::string, std::size_t> transientFailures;
        std::deque<std::string> cacheOrder;
        std::size_t cacheBytes = 0;
        std::uint64_t generation = 0;
        std::uint64_t nextCompletionRevision = 1;
        std::function<void()> completion;
        bool enabled = false;
        std::jthread worker;
        RendererHandle nativeRenderer{nullptr, &FoliaNativeMathRendererDestroy};

        RenderOutcome RenderNow(Request const& request, std::stop_token stop)
        {
            RenderOutcome outcome;
            outcome.result.display = request.display;
            if (stop.stop_requested())
            {
                outcome.result.error = "Math rendering cancelled";
                outcome.result.errorKind = MathJaxErrorKind::Infrastructure;
                outcome.retryable = true;
                return outcome;
            }
            try
            {

                if (!nativeRenderer)
                {
                    FoliaNativeMathRenderer *created = nullptr;
                    auto status = FoliaNativeMathRendererCreate(&created);
                    nativeRenderer.reset(created);
                    if (status != FOLIA_NATIVE_MATH_OK || !nativeRenderer)
                    {
                        nativeRenderer.reset();
                        outcome.result.error = "Native math renderer could not be created";
                        outcome.result.errorKind = MathJaxErrorKind::Infrastructure;
                        outcome.retryable = status == FOLIA_NATIVE_MATH_OUT_OF_MEMORY;
                        return outcome;
                    }
                }

                FoliaNativeMathCancellation *cancellationRaw = nullptr;
                const auto cancellationStatus = FoliaNativeMathCancellationCreate(&cancellationRaw);
                CancellationHandle cancellation{cancellationRaw, &FoliaNativeMathCancellationDestroy};
                if (cancellationStatus != FOLIA_NATIVE_MATH_OK || !cancellation)
                {
                    outcome.result.error = "Native math cancellation handle could not be created";
                    outcome.result.errorKind = MathJaxErrorKind::Infrastructure;
                    outcome.retryable = cancellationStatus == FOLIA_NATIVE_MATH_OUT_OF_MEMORY;
                    if (outcome.retryable)
                        nativeRenderer.reset();
                    return outcome;
                }

                FoliaNativeMathResult *resultRaw = nullptr;
                FoliaNativeMathStatus status = FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
                {
                    std::stop_callback stopCallback(
                        stop, [handle = cancellation.get()] { FoliaNativeMathCancellationRequestStop(handle); });
                    const auto text = winrt::to_hstring(std::string_view(request.tex.data(), request.tex.size()));
                    std::vector<std::uint16_t> utf16Units;
                    utf16Units.reserve(text.size());
                    for (wchar_t codeUnit : text)
                        utf16Units.push_back(static_cast<std::uint16_t>(codeUnit));

                    status = FoliaNativeMathRenderSvg(nativeRenderer.get(), utf16Units.data(), utf16Units.size(),
                                                      request.display ? 1 : 0, request.em, request.containerWidth,
                                                      cancellation.get(), 2000, &resultRaw);
                    ResultHandle nativeResult{resultRaw, &FoliaNativeMathResultDestroy};

                    if (status == FOLIA_NATIVE_MATH_OK)
                    {
                        std::size_t svgSize = 0;
                        auto svgBytes = FoliaNativeMathResultSvg(nativeResult.get(), &svgSize);
                        if (svgBytes || svgSize == 0)
                        {
                            std::string ownedSvg(svgBytes ? svgBytes : "", svgSize);
                            nativeResult.reset();
                            outcome.result = ParseMathJaxSvgOutput(ownedSvg, request.display, request.em);
                        }
                        else
                        {
                            outcome.result.error = "Native renderer returned an invalid SVG result";
                            outcome.result.errorKind = MathJaxErrorKind::Infrastructure;
                        }
                    }
                    else
                    {
                        std::size_t errorSize = 0;
                        auto errorBytes = FoliaNativeMathResultError(nativeResult.get(), &errorSize);
                        outcome.result.error =
                            errorBytes ? std::string(errorBytes, errorSize) : "Native math renderer failed";
                        outcome.result.errorKind = MathJaxErrorKind::Infrastructure;
                        outcome.retryable = status == FOLIA_NATIVE_MATH_CANCELLED ||
                                            status == FOLIA_NATIVE_MATH_SESSION_UNUSABLE ||
                                            status == FOLIA_NATIVE_MATH_OUT_OF_MEMORY;
                        if (outcome.retryable)
                            nativeRenderer.reset();
                    }
                }
            }
            catch (std::bad_alloc const &)
            {
                nativeRenderer.reset();
                outcome.result.error = "Native math renderer ran out of memory";
                outcome.result.errorKind = MathJaxErrorKind::Infrastructure;
                outcome.retryable = true;
            }
            catch (std::exception const &error)
            {
                outcome.result.error = error.what();
                outcome.result.errorKind = MathJaxErrorKind::Infrastructure;
            }
            catch (...)
            {
                outcome.result.error = "Unknown native math renderer failure";
                outcome.result.errorKind = MathJaxErrorKind::Infrastructure;
            }
            return outcome;
        }

        static std::size_t ResultBytes(MathJaxSvg const &result)
        {
            std::size_t bytes = sizeof(result);
            for (auto const &fragment : result.fragments)
                bytes += sizeof(fragment) + (fragment.svg ? fragment.svg->size() : 0);
            return bytes + result.error.size();
        }

        void Store(Request const &request, MathJaxSvg result)
        {
            constexpr std::size_t budget = 16 * 1024 * 1024;
            auto shared = std::make_shared<MathJaxSvg const>(std::move(result));
            auto bytes = request.key.size() + ResultBytes(*shared);
            while ((!cacheOrder.empty()) && (cacheBytes + bytes > budget || cache.size() >= 4096))
            {
                auto oldest = std::move(cacheOrder.front());
                cacheOrder.pop_front();
                auto found = cache.find(oldest);
                if (found == cache.end()) continue;
                cacheBytes -= found->first.size() + ResultBytes(*found->second);
                cache.erase(found);
                completionRevisions.erase(oldest);
            }
            if (bytes <= budget)
            {
                cacheBytes += bytes;
                cacheOrder.push_back(request.key);
                cache.emplace(request.key, std::move(shared));
            }
        }

        void Run(std::stop_token stop)
        {
            while (!stop.stop_requested())
            {
                Request request;
                std::uint64_t ticket = 0;
                {
                    std::unique_lock lock(mutex);
                    if (!ready.wait(lock, stop, [&] {
                            return requestQueue.Ready(true);
                        })) break;
                    auto queued = requestQueue.Pop(true);
                    if (!queued) continue;
                    ticket = queued->ticket;
                    request = std::move(queued->work);
                    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
                }
                auto outcome = RenderNow(request, stop);
                std::function<void()> notify;
                {
                    std::scoped_lock lock(mutex);
                    requestQueue.Complete(request.key, ticket);
                    if (request.generation == generation)
                    {
                        auto retryable = outcome.retryable;
                        bool store = true;
                        if (retryable)
                        {
                            auto [failure, first] = transientFailures.try_emplace(request.key, 1);
                            if (first) store = false;
                            else transientFailures.erase(failure);
                        }
                        else
                        {
                            transientFailures.erase(request.key);
                        }
                        if (store) Store(request, std::move(outcome.result));
                        completionRevisions[request.key] = nextCompletionRevision++;
                        notify = completion;
                    }
                }
                if (notify) notify();
            }
            nativeRenderer.reset();
        }
    };

    MathJaxRenderer::MathJaxRenderer() : state(std::make_unique<State>()) {}

    MathJaxRenderer::~MathJaxRenderer()
    {
        if (!state) return;
        SetCompletionCallback({});
        SetEnabled(false);
    }

    void MathJaxRenderer::SetEnabled(bool enabled)
    {
        if (!state) return;
        if (enabled)
        {
            std::scoped_lock lock(state->mutex);
            if (state->enabled) return;
            state->enabled = true;
            auto current = state.get();
            state->worker = std::jthread([current](std::stop_token stop) { current->Run(stop); });
            return;
        }

        std::jthread worker;
        {
            std::scoped_lock lock(state->mutex);
            if (!state->enabled && !state->worker.joinable()) return;
            state->enabled = false;
            ++state->generation;
            state->requestQueue.Clear();
            state->cache.clear();
            state->completionRevisions.clear();
            state->cacheOrder.clear();
            state->transientFailures.clear();
            state->cacheBytes = 0;
            state->worker.request_stop();
            state->ready.notify_all();
            worker = std::move(state->worker);
        }
        if (worker.joinable()) worker.join();
    }

    bool MathJaxRenderer::Enabled() const
    {
        if (!state) return false;
        std::scoped_lock lock(state->mutex);
        return state->enabled;
    }

    void MathJaxRenderer::Clear()
    {
        if (!state) return;
        std::scoped_lock lock(state->mutex);
        ++state->generation;
        state->requestQueue.Clear();
        state->cache.clear();
        state->completionRevisions.clear();
        state->cacheOrder.clear();
        state->transientFailures.clear();
        state->cacheBytes = 0;
    }

    void MathJaxRenderer::SetCompletionCallback(std::function<void()> callback)
    {
        std::scoped_lock lock(state->mutex);
        state->completion = std::move(callback);
    }

    std::shared_ptr<MathJaxSvg const> MathJaxRenderer::GetOrQueue(
        std::string_view tex,
        bool display,
        float em,
        float containerWidth,
        bool allowQueue,
        bool highPriority,
        AsyncWorkDependency* pendingDependency)
    {
        auto key = std::string(tex) + '\x1f' + (display ? "1" : "0") + '\x1f' + std::to_string(em) + '\x1f' + std::to_string(containerWidth);
        std::scoped_lock lock(state->mutex);
        if (!state->enabled) return {};
        if (auto found = state->cache.find(key); found != state->cache.end()) return found->second;
        if (pendingDependency)
        {
            pendingDependency->key = key;
            if (auto completed = state->completionRevisions.find(key);
                completed != state->completionRevisions.end())
                pendingDependency->observedCompletion = completed->second;
        }
        if (!allowQueue) return {};
        auto change = state->requestQueue.Enqueue(
            State::Request{
                key,
                std::string(tex),
                display,
                em,
                containerWidth,
                state->generation,
            },
            highPriority
                ? EditorWorkPriority::Visible
                : EditorWorkPriority::Background,
            256);
        if (change != EditorQueueChange::Unchanged) state->ready.notify_one();
        return {};
    }

    bool MathJaxRenderer::AnyCompletedAfter(
        std::span<AsyncWorkDependency const> dependencies) const
    {
        if (!state || dependencies.empty()) return false;
        std::scoped_lock lock(state->mutex);
        return std::ranges::any_of(
            dependencies,
            [&](AsyncWorkDependency const& dependency)
            {
                if (dependency.key.empty()) return false;
                auto completed = state->completionRevisions.find(dependency.key);
                return completed != state->completionRevisions.end()
                    && completed->second > dependency.observedCompletion;
            });
    }
}
