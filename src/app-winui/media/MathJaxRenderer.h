#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <functional>
#include <condition_variable>
#include <exception>
#include <filesystem>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <stop_token>
#include <thread>
#include <vector>

namespace winrt::Folia
{
    enum class MathJaxErrorKind
    {
        None,
        Formula,
        Infrastructure,
    };

    enum class MathRenderPurpose
    {
        Document,
        Preview,
    };

    struct MathJaxSvgFragment
    {
        std::uint64_t renderId = 0;
        std::shared_ptr<std::string const> svg;
        float width = 0.0f;
        float height = 0.0f;
        float verticalAlign = 0.0f;
        float breakSpace = 0.0f;
        bool breakBefore = false;
    };

    struct MathJaxSvg
    {
        std::vector<MathJaxSvgFragment> fragments;
        std::string error;
        MathJaxErrorKind errorKind = MathJaxErrorKind::None;
        float width = 0.0f;
        float height = 0.0f;
        float verticalAlign = 0.0f;
        bool display = false;

        explicit operator bool() const
        {
            return errorKind != MathJaxErrorKind::Infrastructure
                && !fragments.empty() && width > 0.0f && height > 0.0f;
        }
    };

    // Document-owned, asynchronous native math cache. All native rendering
    // and SVG normalization run on its private worker.
    class MathJaxRenderer
    {
    public:
        MathJaxRenderer();
        ~MathJaxRenderer();
        MathJaxRenderer(MathJaxRenderer const&) = delete;
        MathJaxRenderer& operator=(MathJaxRenderer const&) = delete;

        MathJaxRenderer(MathJaxRenderer&&) noexcept;
        MathJaxRenderer& operator=(MathJaxRenderer&&) noexcept;

        std::shared_ptr<MathJaxSvg const> Render(
            std::string_view tex,
            bool display,
            float em,
            float containerWidth,
            std::string_view color,
            std::stop_token stop = {},
            MathRenderPurpose purpose = MathRenderPurpose::Document);
        std::shared_ptr<MathJaxSvg const> Prepare(
            std::string_view tex,
            bool display,
            float em,
            float containerWidth,
            std::string_view color,
            std::stop_token stop = {},
            MathRenderPurpose purpose = MathRenderPurpose::Document);
        std::shared_ptr<std::string const> GetSvg(std::uint64_t renderId);
        void WaitForIdle(std::stop_token stop = {});
        void SetCompletionCallback(std::function<void()> callback);

        struct Statistics
        {
            std::size_t entries = 0;
            std::size_t svgBytes = 0;
            std::size_t nativeRenders = 0;
            std::size_t pending = 0;
            std::uint64_t fileReads = 0;
            std::uint64_t fileWrites = 0;
            std::size_t residentSvgBytes = 0;
            std::size_t previewSvgBytes = 0;
            std::filesystem::path backingPath;
            std::thread::id lastIoThread;
        };
        Statistics Snapshot() const;

        void SetPreparationStopToken(std::stop_token stop);
        void BeginBlock(std::uint64_t owner);
        void EndBlock();
        void RetainBlocks(std::span<std::uint64_t const> owners);
        void Clear();

    private:
        struct State;
        std::shared_ptr<State> CurrentState() const;

        mutable std::mutex stateMutex;
        std::shared_ptr<State> state;
    };
}
