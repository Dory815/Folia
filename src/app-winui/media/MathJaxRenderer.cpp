#include "pch.h"

import std;
#include "media/MathJaxRenderer.h"
#include "media/MathJaxRenderer.State.h"

namespace winrt::Folia
{
    MathJaxRenderer::MathJaxRenderer() : state(std::make_shared<State>()) {}

    MathJaxRenderer::~MathJaxRenderer()
    {
        auto old = CurrentState();
        if (old) old->Stop();
    }

    MathJaxRenderer::MathJaxRenderer(MathJaxRenderer&& other) noexcept
    {
        std::scoped_lock lock(other.stateMutex);
        state = std::move(other.state);
    }

    MathJaxRenderer& MathJaxRenderer::operator=(MathJaxRenderer&& other) noexcept
    {
        if (this != &other)
        {
            std::shared_ptr<State> old;
            {
                std::scoped_lock lock(stateMutex, other.stateMutex);
                old = std::move(state);
                state = std::move(other.state);
            }
            if (old) old->Stop();
        }
        return *this;
    }

    std::shared_ptr<MathJaxRenderer::State> MathJaxRenderer::CurrentState() const
    {
        std::scoped_lock lock(stateMutex);
        return state;
    }

    std::shared_ptr<MathJaxSvg const> MathJaxRenderer::Render(
        std::string_view tex, bool display, float em, float containerWidth,
        std::string_view color, std::stop_token stop, MathRenderPurpose purpose)
    {
        auto current = CurrentState();
        if (!current) throw winrt::hresult_canceled();
        auto entry = current->Request(tex, display, em, containerWidth, color, stop, purpose);
        std::scoped_lock lock(current->mutex);
        return entry->result;
    }

    std::shared_ptr<MathJaxSvg const> MathJaxRenderer::Prepare(
        std::string_view tex, bool display, float em, float containerWidth,
        std::string_view color, std::stop_token stop, MathRenderPurpose purpose)
    {
        auto current = CurrentState();
        if (!current) throw winrt::hresult_canceled();
        if (!stop.stop_possible())
        {
            std::scoped_lock lock(current->mutex);
            stop = current->preparationStop;
        }
        auto entry = current->Request(tex, display, em, containerWidth, color, stop, purpose);
        {
            std::unique_lock lock(current->mutex);
            if (!current->ready.wait(lock, stop, [&]
                { return !entry->pending || current->stopping || entry->removed || current->workerFinished; }))
                throw winrt::hresult_canceled();
            if (stop.stop_requested() || entry->removed || current->stopping)
                throw winrt::hresult_canceled();
            if (!entry->result) throw winrt::hresult_canceled();
            if (entry->result->errorKind == MathJaxErrorKind::Infrastructure
                || purpose == MathRenderPurpose::Preview)
                return entry->result;
        }
        return current->AwaitFull(entry, stop);
    }

    std::shared_ptr<std::string const> MathJaxRenderer::GetSvg(std::uint64_t renderId)
    {
        auto current = CurrentState();
        return current ? current->GetSvg(renderId) : std::shared_ptr<std::string const>{};
    }

    void MathJaxRenderer::WaitForIdle(std::stop_token stop)
    {
        auto current = CurrentState();
        if (!current) return;
        std::unique_lock lock(current->mutex);
        if (!current->idle.wait(lock, stop, [&]
            { return (current->pending == 0 && !current->workerBusy) || current->workerFinished; }))
            throw winrt::hresult_canceled();
        if (stop.stop_requested()) throw winrt::hresult_canceled();
        if (current->workerFailure) std::rethrow_exception(current->workerFailure);
    }

    void MathJaxRenderer::SetCompletionCallback(std::function<void()> callback)
    {
        auto current = CurrentState();
        if (!current) return;
        std::scoped_lock lock(current->mutex);
        if (!current->stopping && !current->workerFinished)
            current->completion = std::move(callback);
    }

    MathJaxRenderer::Statistics MathJaxRenderer::Snapshot() const
    {
        Statistics result;
        auto current = CurrentState();
        if (!current) return result;
        std::scoped_lock lock(current->mutex);
        result.entries = current->index.size();
        result.svgBytes = current->residentSvgBytes + current->previewSvgBytes;
        result.nativeRenders = current->nativeRenders;
        result.pending = current->pending;
        result.fileReads = current->fileReads;
        result.fileWrites = current->fileWrites;
        result.residentSvgBytes = current->residentSvgBytes;
        result.previewSvgBytes = current->previewSvgBytes;
        result.backingPath = current->backingPath;
        result.lastIoThread = current->lastIoThread;
        return result;
    }

    void MathJaxRenderer::SetPreparationStopToken(std::stop_token stop)
    {
        auto current = CurrentState();
        if (!current) return;
        std::scoped_lock lock(current->mutex);
        current->preparationStop = stop;
    }

    void MathJaxRenderer::BeginBlock(std::uint64_t owner)
    {
        auto current = CurrentState();
        if (!current) return;
        std::vector<std::shared_ptr<State::Entry>> cancelAfterUnlock;
        {
            std::scoped_lock lock(current->mutex);
            auto previous = std::move(current->blockEntries);
            current->blockEntries.clear();
            for (auto id : previous) current->RemoveIfOrphanLocked(id, cancelAfterUnlock);
            current->activeOwner = owner;
        }
        for (auto const& entry : cancelAfterUnlock) entry->cancel.request_stop();
    }

    void MathJaxRenderer::EndBlock()
    {
        auto current = CurrentState();
        if (!current) return;
        std::vector<std::shared_ptr<State::Entry>> cancelAfterUnlock;
        {
            std::scoped_lock lock(current->mutex);
            current->FinishBlock(cancelAfterUnlock);
        }
        for (auto const& entry : cancelAfterUnlock) entry->cancel.request_stop();
    }

    void MathJaxRenderer::RetainBlocks(std::span<std::uint64_t const> owners)
    {
        auto current = CurrentState();
        if (!current) return;
        std::vector<std::shared_ptr<State::Entry>> cancelAfterUnlock;
        {
            std::scoped_lock lock(current->mutex);
            current->Retain(owners, cancelAfterUnlock);
        }
        for (auto const& entry : cancelAfterUnlock) entry->cancel.request_stop();
    }

    void MathJaxRenderer::Clear()
    {
        auto replacement = std::make_shared<State>();
        std::shared_ptr<State> old;
        {
            std::scoped_lock lock(stateMutex);
            old = std::move(state);
            state = std::move(replacement);
        }
        if (old) old->Stop();
    }
}
