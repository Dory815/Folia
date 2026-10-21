#include "pch.h"

import std;
#include "media/MathJaxRenderer.h"
#include "media/MathSvgStore.h"
#include "media/NativeMathSession.h"
#include "media/SvgNormalizer.h"

namespace winrt::Folia
{
    struct MathJaxRenderer::State
    {
        struct Entry
        {
            std::uint64_t id = 0;
            std::string const* cacheKey = nullptr;
            MathJaxSvg geometry;
            std::vector<MathSvgStore::Range> fragments;
            std::unordered_set<std::uint64_t> owners;
            bool unowned = false;
        };

        MathSvgStore store;
        std::unordered_map<std::string, Entry> index;
        std::unordered_map<std::uint64_t, Entry*> entriesById;
        std::unordered_map<std::uint64_t, std::unordered_set<std::uint64_t>> ownerEntries;
        std::optional<std::uint64_t> activeOwner;
        std::unordered_set<std::uint64_t> blockEntries;
        std::stop_token preparationStop;
        std::uint64_t writtenBytes = 0;
        std::uint64_t deadBytes = 0;
        std::uint64_t nextEntryId = 1;

        std::shared_ptr<MathJaxSvg const> Read(Entry const& entry) const
        {
            auto result = std::make_shared<MathJaxSvg>(entry.geometry);
            for (std::size_t i = 0; i < result->fragments.size(); ++i)
                result->fragments[i].svg = store.Read(entry.fragments[i]);
            return result;
        }

        void AddDeadBytes(std::uint64_t bytes) noexcept
        {
            auto const maximum = (std::numeric_limits<std::uint64_t>::max)();
            deadBytes = bytes > maximum - deadBytes ? maximum : deadBytes + bytes;
        }

        void MaybeCompact()
        {
            constexpr std::uint64_t minimumDeadBytes = 8ull * 1024 * 1024;
            if (deadBytes < minimumDeadBytes || deadBytes <= writtenBytes / 2)
                return;

            MathSvgStore replacement;
            std::vector<std::pair<Entry*, std::vector<MathSvgStore::Range>>> replacementRanges;
            replacementRanges.reserve(index.size());
            std::uint64_t liveBytes = 0;
            for (auto& item : index)
            {
                auto& entry = item.second;
                std::vector<MathSvgStore::Range> ranges;
                ranges.reserve(entry.fragments.size());
                for (auto const& range : entry.fragments)
                {
                    auto svg = store.Read(range);
                    ranges.push_back(replacement.Append(*svg));
                    liveBytes += range.size;
                }
                replacementRanges.emplace_back(&entry, std::move(ranges));
            }

            // All reads and writes succeeded. Range vectors are committed by
            // no-throw swaps before replacing the old delete-on-close store.
            for (auto& [entry, ranges] : replacementRanges)
                entry->fragments.swap(ranges);
            store = std::move(replacement);
            writtenBytes = liveBytes;
            deadBytes = 0;
        }

        void CollectOrphans(std::unordered_set<std::uint64_t> const& candidates)
        {
            for (auto entryId : candidates)
            {
                auto found = entriesById.find(entryId);
                if (found == entriesById.end()) continue;
                auto& entry = *found->second;
                if (entry.unowned || !entry.owners.empty()) continue;
                for (auto const& range : entry.fragments) AddDeadBytes(range.size);
                auto cacheEntry = index.find(*entry.cacheKey);
                entriesById.erase(found);
                index.erase(cacheEntry);
            }
            MaybeCompact();
        }

        void FinishBlock()
        {
            if (!activeOwner)
                throw std::logic_error("EndBlock called without BeginBlock");

            auto const owner = *activeOwner;
            std::erase_if(blockEntries, [&](std::uint64_t entryId)
            {
                return !entriesById.contains(entryId);
            });
            auto ownerIt = ownerEntries.try_emplace(owner).first;
            auto affected = ownerIt->second;
            affected.insert(blockEntries.begin(), blockEntries.end());

            // Stage only new memberships, not copies of every other block
            // sharing this formula. A repeated formula in a large document
            // must take constant work per owner rather than quadratic work.
            std::vector<Entry*> added;
            added.reserve(blockEntries.size());
            try
            {
                for (auto entryId : blockEntries)
                {
                    auto entry = entriesById.at(entryId);
                    if (entry->owners.insert(owner).second) added.push_back(entry);
                }
            }
            catch (...)
            {
                for (auto entry : added) entry->owners.erase(owner);
                throw;
            }
            for (auto entryId : ownerIt->second)
                if (!blockEntries.contains(entryId))
                    if (auto entry = entriesById.find(entryId); entry != entriesById.end())
                        entry->second->owners.erase(owner);
            ownerIt->second.swap(blockEntries);
            blockEntries.clear();
            if (ownerIt->second.empty()) ownerEntries.erase(ownerIt);
            activeOwner.reset();
            CollectOrphans(affected);
        }

        void Retain(std::span<std::uint64_t const> owners)
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
                for (auto entryId : it->second)
                {
                    affected.insert(entryId);
                    if (auto entry = entriesById.find(entryId); entry != entriesById.end())
                        entry->second->owners.erase(it->first);
                }
                it = ownerEntries.erase(it);
            }
            CollectOrphans(affected);
        }
    };

    MathJaxRenderer::MathJaxRenderer() : state(std::make_unique<State>()) {}
    MathJaxRenderer::~MathJaxRenderer() = default;
    MathJaxRenderer::MathJaxRenderer(MathJaxRenderer&& other) noexcept
        : state(std::move(other.state)) {}

    MathJaxRenderer& MathJaxRenderer::operator=(MathJaxRenderer&& other) noexcept
    {
        if (this != &other)
        {
            session.reset();
            state = std::move(other.state);
        }
        return *this;
    }

    void MathJaxRenderer::Clear()
    {
        state = std::make_unique<State>();
        session.reset();
    }

    void MathJaxRenderer::SetPreparationStopToken(std::stop_token stop)
    {
        state->preparationStop = stop;
    }

    void MathJaxRenderer::BeginBlock(std::uint64_t owner)
    {
        state->CollectOrphans(state->blockEntries);
        state->activeOwner = owner;
        state->blockEntries.clear();
    }

    void MathJaxRenderer::EndBlock()
    {
        state->FinishBlock();
    }

    void MathJaxRenderer::RetainBlocks(std::span<std::uint64_t const> owners)
    {
        state->Retain(owners);
    }

    std::shared_ptr<MathJaxSvg const> MathJaxRenderer::Render(
        std::string_view tex, bool display, float em, float containerWidth,
        SvgNormalizer& normalizer, std::string_view color, std::stop_token stop)
    {
        if (!stop.stop_possible()) stop = state->preparationStop;
        if (stop.stop_requested()) throw winrt::hresult_canceled();
        // Exact float bytes avoid rounding two distinct layout contexts to
        // the same key. The length prefix also permits any TeX character.
        auto key = std::to_string(tex.size()) + ':' + std::string(tex);
        key.push_back(display ? '1' : '0');
        key.append(reinterpret_cast<char const*>(&em), sizeof(em));
        key.append(reinterpret_cast<char const*>(&containerWidth), sizeof(containerWidth));
        key.append(color);
        if (auto found = state->index.find(key); found != state->index.end())
        {
            if (state->activeOwner)
                state->blockEntries.insert(found->second.id);
            return state->Read(found->second);
        }

        // Reuse the native execution context on this host thread. Moves only
        // transfer document data; this session stays with its original host.
        if (!session) session = std::make_unique<NativeMathSession>();
        auto result = session->Render(tex, display, em, containerWidth, stop);
        for (auto& fragment : result.fragments)
        {
            if (!fragment.svg) throw std::runtime_error("Math fragment has no SVG payload");
            auto colored = *fragment.svg;
            constexpr std::string_view marker = "currentColor";
            for (std::size_t at = 0; (at = colored.find(marker, at)) != std::string::npos;)
            {
                colored.replace(at, marker.size(), color);
                at += color.size();
            }
            if (stop.stop_requested()) throw winrt::hresult_canceled();
            auto normalized = normalizer.NormalizeNow(colored, em);
            if (!normalized)
            {
                result.fragments.clear();
                result.error = normalized.error;
                result.errorKind = MathJaxErrorKind::Infrastructure;
                break;
            }
            fragment.renderId = normalized.id;
            fragment.svg = std::move(normalized.svg);
        }
        if (stop.stop_requested()) throw winrt::hresult_canceled();
        auto rendered = std::make_shared<MathJaxSvg const>(result);
        State::Entry entry;
        entry.id = state->nextEntryId++;
        auto const entryId = entry.id;
        entry.geometry = *rendered;
        entry.fragments.reserve(entry.geometry.fragments.size());
        entry.unowned = !state->activeOwner.has_value();
        std::uint64_t appendedBytes = 0;
        try
        {
            if (state->activeOwner)
                state->blockEntries.insert(entry.id);
            for (auto& fragment : entry.geometry.fragments)
            {
                auto range = state->store.Append(*fragment.svg);
                state->writtenBytes += range.size;
                appendedBytes += range.size;
                entry.fragments.push_back(range);
                fragment.svg.reset();
            }
            auto [inserted, success] = state->index.emplace(key, std::move(entry));
            if (!success)
            {
                state->AddDeadBytes(appendedBytes);
                appendedBytes = 0;
                if (state->activeOwner)
                {
                    state->blockEntries.erase(entryId);
                    state->blockEntries.insert(inserted->second.id);
                }
                state->MaybeCompact();
                if (!state->activeOwner) inserted->second.unowned = true;
                return state->Read(inserted->second);
            }
            inserted->second.cacheKey = &inserted->first;
            try
            {
                if (!state->entriesById.emplace(entryId, &inserted->second).second)
                    throw std::logic_error("Duplicate MathJax cache entry id");
            }
            catch (...)
            {
                state->index.erase(inserted);
                throw;
            }
            return rendered;
        }
        catch (...)
        {
            state->AddDeadBytes(appendedBytes);
            throw;
        }
    }
}
