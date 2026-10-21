#include "pch.h"

import std;
#include "media/MathJaxRenderer.State.h"
#include "media/SvgNormalizer.h"

namespace winrt::Folia
{
    void MathJaxRenderer::State::StartWorker()
    {
        auto self = shared_from_this();
        std::thread([self = std::move(self)]
        {
            try { self->Run(); }
            catch (...) { self->FailWorker(); }
        }).detach();
    }

    std::string MathJaxRenderer::State::MakeKey(std::string_view tex, bool display,
        float em, float width, std::string_view color, MathRenderPurpose purpose)
    {
        std::string key = std::to_string(tex.size()) + ':' + std::string(tex);
        key.push_back(display ? '1' : '0');
        key.append(reinterpret_cast<char const*>(&em), sizeof(em));
        key.append(reinterpret_cast<char const*>(&width), sizeof(width));
        key.push_back(purpose == MathRenderPurpose::Preview ? 'P' : 'D');
        key.append(color);
        return key;
    }

    std::shared_ptr<MathJaxRenderer::State::Entry> MathJaxRenderer::State::Request(
        std::string_view tex, bool display, float em, float width,
        std::string_view color, std::stop_token stop, MathRenderPurpose purpose)
    {
        if (stop.stop_requested()) throw winrt::hresult_canceled();
        if (purpose != MathRenderPurpose::Document && purpose != MathRenderPurpose::Preview)
            throw std::invalid_argument("Unknown math render purpose");
        std::unique_lock lock(mutex);
        if (stopping || workerFinished) throw winrt::hresult_canceled();
        if (!stop.stop_possible()) stop = preparationStop;
        if (stop.stop_requested()) throw winrt::hresult_canceled();
        auto key = MakeKey(tex, display, em, width, color, purpose);
        if (stop.stop_requested()) throw winrt::hresult_canceled();

        if (auto found = index.find(key); found != index.end())
        {
            auto entry = found->second;
            if (activeOwner) blockEntries.insert(entry->id);
            // Viewport materialization also warms SVG bytes in the background.
            // Geometry remains available immediately, including on a cold hit.
            if (purpose == MathRenderPurpose::Document && entry->result)
                for (auto id : entry->fragmentIds)
                    if (auto ref = svgRefs.find(id); ref != svgRefs.end() && !ref->second.resident)
                        QueueReadLocked(id, ref->second);
            return entry;
        }

        // Acquire the runtime lease on the requester's thread before the
        // detached native worker can outlive application teardown.
        if (!normalizerLease) normalizerLease = std::make_shared<SvgNormalizer>();

        auto entry = std::make_shared<Entry>();
        entry->id = nextEntryId++;
        if (entry->id == 0) entry->id = nextEntryId++;
        entry->key = key;
        entry->purpose = purpose;
        entry->unowned = !activeOwner.has_value();
        bool inIndex = false;
        bool byId = false;
        bool inBlock = false;
        try
        {
            inIndex = index.emplace(key, entry).second;
            if (!inIndex)
            {
                auto existing = index.at(key);
                if (activeOwner) blockEntries.insert(existing->id);
                return existing;
            }
            byId = entriesById.emplace(entry->id, entry).second;
            if (!byId) throw std::logic_error("Duplicate MathJax cache entry id");
            if (activeOwner) inBlock = blockEntries.insert(entry->id).second;
            Job job;
            job.kind = Job::Kind::Render;
            job.entry = entry;
            job.tex.assign(tex);
            job.color.assign(color);
            job.display = display;
            job.em = em;
            job.containerWidth = width;
            job.callerStop = stop;
            jobs.push_back(std::move(job));
        }
        catch (...)
        {
            if (inBlock) blockEntries.erase(entry->id);
            if (byId) entriesById.erase(entry->id);
            if (inIndex) index.erase(key);
            throw;
        }
        ++pending;
        bool startWorker = !workerStarted;
        if (startWorker) workerStarted = true;
        wake.notify_one();
        lock.unlock();
        if (startWorker)
        {
            try { StartWorker(); }
            catch (...)
            {
                FailWorker();
                throw;
            }
        }
        return entry;
    }

    MathJaxSvg MathJaxRenderer::State::InfrastructureFailure(std::string message, bool display)
    {
        MathJaxSvg result;
        result.display = display;
        result.error = std::move(message);
        result.errorKind = MathJaxErrorKind::Infrastructure;
        return result;
    }

    void MathJaxRenderer::State::AddSaturated(std::uint64_t& value, std::uint64_t add) noexcept
    {
        auto const maximum = (std::numeric_limits<std::uint64_t>::max)();
        value = add > maximum - value ? maximum : value + add;
    }

    void MathJaxRenderer::State::RemoveSvgRefLocked(std::uint64_t id, std::uint64_t ownerEntry)
    {
        auto found = svgRefs.find(id);
        if (found == svgRefs.end() || found->second.entryId != ownerEntry) return;
        auto& ref = found->second;
        if (ref.range.size != 0)
        {
            AddSaturated(deadBytes, ref.range.size);
            compactionRequested = true;
        }
        if (ref.inLru) readyLru.erase(ref.lruPosition);
        if (ref.resident) residentSvgBytes -= ref.resident->size();
        if (ref.preview) previewSvgBytes -= ref.preview->size();
        svgRefs.erase(found);
    }

    void MathJaxRenderer::State::RemoveEntryLocked(std::shared_ptr<Entry> const& entry,
        std::vector<std::shared_ptr<Entry>>& cancelAfterUnlock)
    {
        cancelAfterUnlock.push_back(entry);
        entry->removed = true;
        auto found = index.find(entry->key);
        if (found != index.end() && found->second->id == entry->id) index.erase(found);
        entriesById.erase(entry->id);
        for (auto id : entry->fragmentIds) RemoveSvgRefLocked(id, entry->id);
        entry->fragmentIds.clear();
        entry->result.reset();

        for (auto it = jobs.begin(); it != jobs.end();)
        {
            if (it->entry && it->entry->id == entry->id)
            {
                it = jobs.erase(it);
                if (pending) --pending;
            }
            else ++it;
        }
        // Active jobs stay counted until their worker completion path runs.
        entry->pending = false;
        ready.notify_all();
        idle.notify_all();
        wake.notify_one();
    }

    void MathJaxRenderer::State::RemoveIfOrphanLocked(std::uint64_t id,
        std::vector<std::shared_ptr<Entry>>& cancelAfterUnlock)
    {
        if (blockEntries.contains(id)) return;
        auto found = entriesById.find(id);
        if (found == entriesById.end()) return;
        auto entry = found->second;
        if (!entry->unowned && entry->owners.empty()) RemoveEntryLocked(entry, cancelAfterUnlock);
    }

    void MathJaxRenderer::State::FinishBlock(std::vector<std::shared_ptr<Entry>>& cancelAfterUnlock)
    {
        if (!activeOwner) throw std::logic_error("EndBlock called without BeginBlock");
        auto owner = *activeOwner;
        std::erase_if(blockEntries, [&](std::uint64_t id) { return !entriesById.contains(id); });
        auto previous = ownerEntries[owner];
        for (auto id : previous)
        {
            if (blockEntries.contains(id)) continue;
            if (auto found = entriesById.find(id); found != entriesById.end())
                found->second->owners.erase(owner);
        }
        for (auto id : blockEntries)
            if (auto found = entriesById.find(id); found != entriesById.end())
                found->second->owners.insert(owner);
        ownerEntries[owner] = blockEntries;
        if (ownerEntries[owner].empty()) ownerEntries.erase(owner);
        auto affected = std::move(previous);
        affected.insert(blockEntries.begin(), blockEntries.end());
        blockEntries.clear();
        activeOwner.reset();
        for (auto id : affected) RemoveIfOrphanLocked(id, cancelAfterUnlock);
    }

    void MathJaxRenderer::State::Retain(std::span<std::uint64_t const> owners,
        std::vector<std::shared_ptr<Entry>>& cancelAfterUnlock)
    {
        std::unordered_set<std::uint64_t> retained(owners.begin(), owners.end());
        std::unordered_set<std::uint64_t> affected;
        for (auto it = ownerEntries.begin(); it != ownerEntries.end();)
        {
            if (retained.contains(it->first))
            {
                ++it;
                continue;
            }
            for (auto id : it->second)
            {
                affected.insert(id);
                if (auto found = entriesById.find(id); found != entriesById.end())
                    found->second->owners.erase(it->first);
            }
            it = ownerEntries.erase(it);
        }
        for (auto id : affected) RemoveIfOrphanLocked(id, cancelAfterUnlock);
    }

    void MathJaxRenderer::State::QueueReadLocked(std::uint64_t id, SvgRef& ref)
    {
        if (ref.readQueued || ref.failed || stopping) return;
        auto entry = entriesById.find(ref.entryId);
        if (entry == entriesById.end()) return;
        Job job;
        job.kind = Job::Kind::Read;
        job.entry = entry->second;
        job.renderId = id;
        job.generation = ref.generation;
        jobs.push_back(std::move(job));
        ref.readQueued = true;
        ++pending;
        wake.notify_one();
    }

    std::shared_ptr<std::string const> MathJaxRenderer::State::GetSvg(std::uint64_t id)
    {
        std::scoped_lock lock(mutex);
        auto found = svgRefs.find(id);
        if (found == svgRefs.end() || stopping) return {};
        auto& ref = found->second;
        if (ref.preview) return ref.preview;
        if (ref.resident)
        {
            auto value = ref.resident;
            if (ref.oneShotOversize)
            {
                residentSvgBytes -= ref.resident->size();
                ref.resident.reset();
                ref.oneShotOversize = false;
                if (ref.inLru) readyLru.erase(ref.lruPosition);
                ref.inLru = false;
            }
            else if (ref.inLru)
            {
                ref.lastUse = ++useClock;
                readyLru.splice(readyLru.end(), readyLru, ref.lruPosition);
            }
            return value;
        }
        QueueReadLocked(id, ref);
        return {};
    }

    bool MathJaxRenderer::State::HasReadyChangeLocked(Entry const& entry,
        std::vector<std::shared_ptr<std::string const>> const& payloads) const
    {
        for (std::size_t i = 0; i < entry.fragmentIds.size() && i < payloads.size(); ++i)
        {
            if (payloads[i]) continue;
            auto found = svgRefs.find(entry.fragmentIds[i]);
            if (found == svgRefs.end() || found->second.failed || found->second.resident)
                return true;
        }
        return false;
    }

    std::shared_ptr<MathJaxSvg const> MathJaxRenderer::State::AwaitFull(
        std::shared_ptr<Entry> const& entry, std::stop_token stop)
    {
        std::unique_lock lock(mutex);
        std::vector<std::shared_ptr<std::string const>> payloads;
        bool initialized = false;
        for (;;)
        {
            if (stop.stop_requested() || stopping || entry->removed)
                throw winrt::hresult_canceled();
            if (entry->pending)
            {
                if (!ready.wait(lock, stop, [&]
                    { return !entry->pending || stopping || entry->removed || workerFinished; }))
                    throw winrt::hresult_canceled();
                continue;
            }
            if (!entry->result) throw winrt::hresult_canceled();
            if (entry->result->errorKind == MathJaxErrorKind::Infrastructure
                || entry->purpose == MathRenderPurpose::Preview
                || entry->result->fragments.empty())
                return entry->result;
            if (!initialized)
            {
                payloads.resize(entry->result->fragments.size());
                initialized = true;
            }

            bool waiting = false;
            bool failed = false;
            for (std::size_t i = 0; i < entry->result->fragments.size(); ++i)
            {
                if (payloads[i]) continue;
                auto id = entry->result->fragments[i].renderId;
                auto ref = svgRefs.find(id);
                if (ref == svgRefs.end() || ref->second.entryId != entry->id || ref->second.failed)
                {
                    failed = true;
                    break;
                }
                if (ref->second.resident)
                {
                    payloads[i] = ref->second.resident;
                    ref->second.lastUse = ++useClock;
                    if (ref->second.oneShotOversize)
                    {
                        residentSvgBytes -= ref->second.resident->size();
                        ref->second.resident.reset();
                        ref->second.oneShotOversize = false;
                    }
                }
                else
                {
                    QueueReadLocked(id, ref->second);
                    waiting = true;
                }
            }
            if (failed)
            {
                auto failure = std::make_shared<MathJaxSvg const>(
                    InfrastructureFailure("A cached math SVG could not be read", entry->result->display));
                entry->result = std::move(failure);
                return entry->result;
            }
            bool complete = std::ranges::all_of(payloads, [](auto const& value) { return static_cast<bool>(value); });
            if (complete)
            {
                auto full = std::make_shared<MathJaxSvg>(*entry->result);
                for (std::size_t i = 0; i < payloads.size(); ++i)
                    full->fragments[i].svg = std::move(payloads[i]);
                return full;
            }
            if (waiting && !ready.wait(lock, stop, [&]
                { return stopping || entry->removed || HasReadyChangeLocked(*entry, payloads); }))
                throw winrt::hresult_canceled();
        }
    }

    void MathJaxRenderer::State::Stop() noexcept
    {
        std::unordered_map<std::string, std::shared_ptr<Entry>> oldEntries;
        {
            std::scoped_lock lock(mutex);
            if (stopping) return;
            stopping = true;
            completion = {};
            for (auto& [key, entry] : index)
            {
                entry->removed = true;
                entry->pending = false;
            }
            oldEntries.swap(index);
            entriesById.clear();
            svgRefs.clear();
            readyLru.clear();
            jobs.clear();
            pending = 0;
            residentSvgBytes = 0;
            previewSvgBytes = 0;
        }
        for (auto& [key, entry] : oldEntries) entry->cancel.request_stop();
        wake.notify_all();
        ready.notify_all();
        idle.notify_all();
    }
}
