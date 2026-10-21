#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <stop_token>
#include <vector>

namespace winrt::Folia
{
    enum class MathJaxErrorKind
    {
        None,
        Formula,
        Infrastructure,
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

    class SvgNormalizer;
    class NativeMathSession;

    // A document-owned index. Live layout metrics are not evicted for cache
    // capacity; SVG bytes live in its delete-on-close temporary store.
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
            SvgNormalizer& normalizer,
            std::string_view color,
            std::stop_token stop = {});
        void SetPreparationStopToken(std::stop_token stop);
        void BeginBlock(std::uint64_t owner);
        void EndBlock();
        void RetainBlocks(std::span<std::uint64_t const> owners);
        void Clear();

    private:
        struct State;

        std::unique_ptr<State> state;
        // Thread-confined execution state is never moved with the document
        // cache. The preparation host destroys it on its creating thread.
        std::unique_ptr<NativeMathSession> session;
    };
}
