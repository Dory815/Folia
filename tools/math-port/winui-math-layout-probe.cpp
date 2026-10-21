#include "pch.h"

import std;
import folia.core.render_model;

#include "editor/session/EditorSession.h"
#include "editor/rendering/EditorPreparedDocument.h"
#include "editor/rendering/EditorSurfaceRenderer.h"
#include "media/SvgNormalizer.h"
#include "editor/rendering/EditorDocumentBlockPreparer.h"
#include "editor/rendering/EditorSvgPainter.h"

namespace
{
    using namespace winrt::Folia;

    void Require(bool condition, std::string_view message)
    {
        if (!condition) throw std::runtime_error(std::string(message));
    }

    struct BlockStamp
    {
        std::uint64_t id = 0;
        std::uint64_t sourceKey = 0;
        std::uint64_t presentationKey = 0;
        folia::RenderBlockKind kind{};

        friend bool operator==(BlockStamp const&, BlockStamp const&) = default;
    };

    std::vector<BlockStamp> Stamp(detail::EditorRenderFrame const& frame)
    {
        std::vector<BlockStamp> result;
        result.reserve(frame.renderModel.blocks.size());
        for (auto const& block : frame.renderModel.blocks)
            result.push_back({ block.id.v, block.source_key, block.presentation_key, block.kind });
        return result;
    }

    void CheckLayout(EditorDocumentPreparation const& preparation, std::string_view label)
    {
        Require(preparation.layout != nullptr, std::string(label) + ": layout exists");
        Require(!preparation.layout->blocks.empty(), std::string(label) + ": blocks were prepared");
        Require(std::isfinite(preparation.layout->totalHeight) && preparation.layout->totalHeight > 0.0f,
            std::string(label) + ": total height is finite and positive");
        Require(preparation.layout->geometry.Initialized()
                && preparation.layout->geometry.Size() == preparation.layout->blocks.size(),
            std::string(label) + ": geometry index covers every prepared block");

        bool containsMath = false;
        for (std::size_t index = 0; index < preparation.layout->blocks.size(); ++index)
        {
            auto const& block = preparation.layout->blocks[index];
            Require(std::isfinite(block.height) && block.height > 0.0f,
                std::string(label) + ": every block has measured height");
            auto placement = preparation.layout->geometry.At(index);
            Require(std::isfinite(placement.top) && std::isfinite(placement.bottom)
                    && std::abs((placement.bottom - placement.top) - block.height) < 0.01f,
                std::string(label) + ": geometry placement retains each measured height");
            Require(!block.layout && block.images.empty() && block.mathPreviews.empty()
                    && block.mermaids.empty() && block.mermaidPreviews.empty() && !block.table,
                std::string(label) + ": visual resources do not cross preparation boundary");
            Require(block.display.text.empty() && block.display.mathOverlays.empty()
                    && block.display.imageOverlays.empty() && block.display.mermaidOverlays.empty(),
                std::string(label) + ": display projections are released after measurement");
            containsMath = containsMath || block.containsMath;
        }
        Require(containsMath, std::string(label) + ": math blocks reached the layout preparer");
    }

    float DocumentWidth(EditorDocumentPreparationContext const& context)
    {
        auto padding = (std::min)(context.theme.layout.document_horizontal_padding,
            (std::max)(12.0f, context.surfaceWidth * 0.06f));
        auto left = padding;
        auto right = (std::max)(left + 1.0f, context.surfaceWidth - padding - 14.0f);
        return right - left;
    }

    std::string TextColor(folia::ThemeProfile const& theme)
    {
        auto const& color = theme.colors.fg;
        char buffer[8]{};
        std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X",
            static_cast<unsigned>(color.r), static_cast<unsigned>(color.g), static_cast<unsigned>(color.b));
        return buffer;
    }

    std::size_t MathOnlyBlockIndex(detail::EditorRenderFrame const& frame)
    {
        for (std::size_t blockIndex = 0; blockIndex < frame.renderModel.blocks.size(); ++blockIndex)
        {
            auto const& block = frame.renderModel.blocks[blockIndex];
            for (auto const& item : block.inline_items)
            {
                if (item.text.find(U"MATH_WRAP_SENTINEL") != std::u32string::npos)
                    return blockIndex;
            }
        }
        throw std::runtime_error("math-only wrapping paragraph was not projected as an inline block");
    }

    bool SameMetrics(MathJaxSvg const& left, MathJaxSvg const& right)
    {
        if (left.width != right.width || left.height != right.height
            || left.verticalAlign != right.verticalAlign
            || left.fragments.size() != right.fragments.size()) return false;
        for (std::size_t index = 0; index < left.fragments.size(); ++index)
        {
            auto const& a = left.fragments[index];
            auto const& b = right.fragments[index];
            if (a.renderId != b.renderId || a.width != b.width || a.height != b.height
                || a.verticalAlign != b.verticalAlign || a.breakSpace != b.breakSpace
                || a.breakBefore != b.breakBefore) return false;
        }
        return true;
    }

    void CheckReadingLayout(detail::EditorRenderFrame const& frame,
        EditorDocumentPreparation& preparation, std::size_t index)
    {
        auto const& context = preparation.context;
        auto sheet = CreateEditorStyleSheet(context.theme);
        EditorRenderResources resources;
        resources.InitializeOffscreen(sheet, context.surfaceWidth, context.surfaceHeight);
        EditorRenderCache cache;
        SvgNormalizer normalizer;
        MermaidRenderer mermaid;
        TreeSitterHighlighter highlighter;
        EditorSvgPainter svgPainter(resources, cache);
        EditorInlineImageRenderer images(resources, cache, sheet, normalizer, svgPainter, frame.baseDirectory, false);
        EditorTextLayoutEngine text(resources, sheet);
        EditorInteractionMap interaction;
        auto width = DocumentWidth(context);
        EditorDocumentPainter painter(resources, sheet, interaction, highlighter, {}, false,
            width, frame.selection, {}, frame.renderModel.editable_index);
        EditorDocumentBlockPreparer preparer(frame, resources, cache, sheet, text, images,
            painter, preparation.math, mermaid, normalizer, highlighter, frame.selection.active,
            width, svgPainter.Supported(), svgPainter.Supported(),
            context.theme.variant != folia::Theme::Light, 0, 0);
        auto makeLines = [](IDWriteTextLayout* layout) {
            UINT32 count = 0;
            layout->GetLineMetrics(nullptr, 0, &count);
            std::vector<DWRITE_LINE_METRICS> lines(count);
            winrt::check_hresult(layout->GetLineMetrics(lines.data(), count, &count));
            return lines;
        };
        auto first = preparer.Prepare(frame.renderModel.blocks[index], true, true);
        Require(first.layout && !first.display.mathOverlays.empty(), "reading uses math inline objects");
        Require(first.height == preparation.layout->blocks[index].height,
            "reading height exactly matches the complete preparation height");
        auto firstLines = makeLines(first.layout.Get());
        Require(firstLines.size() > 1, "long inline formula wraps in DirectWrite");
        auto firstPositions = EditorDocumentPainter::PositionMath(first.layout.Get(), first.display.mathOverlays, width);
        for (int pass = 0; pass < 6; ++pass)
        {
            // Re-materialize with embedded content outside/inside the viewport
            // band, exactly as scrolling does, without retaining visual state.
            auto reread = preparer.Prepare(frame.renderModel.blocks[index], pass % 2 != 0, false);
            Require(reread.height == first.height, "scroll rematerialization preserves measured height");
            auto lines = makeLines(reread.layout.Get());
            Require(lines.size() == firstLines.size(), "scroll rematerialization preserves line count");
            for (std::size_t line = 0; line < lines.size(); ++line)
                Require(lines[line].length == firstLines[line].length
                    && lines[line].height == firstLines[line].height
                    && lines[line].baseline == firstLines[line].baseline,
                    "scroll rematerialization preserves line breaks and baselines");
            auto positions = EditorDocumentPainter::PositionMath(reread.layout.Get(), reread.display.mathOverlays, width);
            Require(positions.size() == firstPositions.size(), "scroll rematerialization preserves math fragments");
            for (std::size_t part = 0; part < positions.size(); ++part)
                Require(positions[part].localX == firstPositions[part].localX
                    && positions[part].localTop == firstPositions[part].localTop,
                    "scroll rematerialization preserves formula positions");
        }
        std::cout << "reading lines=" << firstLines.size() << " repeated-placements=stable\n";
    }

    void RunProbe()
    {
        std::string longFormula = "x";
        for (std::size_t index = 0; index < 80; ++index)
            longFormula += "+x";

        winrt::Folia::EditorSession session;
        std::wstring source =
            L"# Layout preparation\n\n"
            L"A long paragraph exercises DirectWrite wrapping at more than one surface width. "
            L"It carries an inline formula $\\frac{1}{2}$ between words, then continues with enough "
            L"ordinary text to occupy several lines when the document is narrow. The source must stay "
            L"exactly as entered after every preparation pass.\n\n"
            L"MATH_WRAP_SENTINEL $";
        for (char character : longFormula)
            source.push_back(static_cast<wchar_t>(character));
        source +=
            L"$\n\n"
            L"- First list item with $x^2$\n"
            L"  - Nested list item\n\n"
            L"> Quoted paragraph with $\\mathbb{R}$ and wrapped explanatory text.\n\n"
            L"| Value | Formula |\n"
            L"| --- | --- |\n"
            L"| half | $\\frac{1}{2}$ |\n\n"
            L"$$\n\\int_0^1 x^2 \\, dx\n$$\n";
        session.SetText(winrt::hstring(source));
        const auto originalText = session.TextUtf8();
        auto frame = session.RenderFrame();
        const auto originalStamps = Stamp(frame);
        const auto mathOnlyBlock = MathOnlyBlockIndex(frame);
        Require(frame.renderModel.blocks.size() >= 6, "rich sample produced multiple render blocks");
        Require(std::ranges::any_of(frame.renderModel.blocks, [](auto const& block) {
            return block.kind == folia::RenderBlockKind::Quote;
        }), "sample includes a quote block");
        Require(std::ranges::any_of(frame.renderModel.blocks, [](auto const& block) {
            return block.kind == folia::RenderBlockKind::Table;
        }), "sample includes a table block");
        Require(std::ranges::any_of(frame.renderModel.blocks, [](auto const& block) {
            return block.kind == folia::RenderBlockKind::Math;
        }), "sample includes display math");
        auto hasIndentedFlow = [](auto const& self, auto const& block) -> bool {
            if (block.flow_local_indent_columns > 0) return true;
            return std::ranges::any_of(block.child_blocks, [&](auto const& child) {
                return self(self, child);
            });
        };
        Require(std::ranges::any_of(frame.renderModel.blocks, [&](auto const& block) {
            return hasIndentedFlow(hasIndentedFlow, block);
        }), "sample includes an indented list flow");

        EditorSurfaceRenderer renderer;
        renderer.InitializeForPdf();
        const auto baseContext = renderer.CapturePreparationContext();
        auto narrowContext = baseContext;
        narrowContext.surfaceWidth = 300.0f;
        narrowContext.surfaceHeight = 1000.0f;
        auto wideContext = narrowContext;
        wideContext.surfaceWidth = 1100.0f;

        std::size_t progressCurrent = 0;
        std::size_t progressTotal = 0;
        auto narrow = EditorSurfaceRenderer::PrepareDocument(frame, narrowContext, {},
            [&](std::size_t current, std::size_t total) {
                progressCurrent = current;
                progressTotal = total;
            });
        CheckLayout(*narrow, "narrow");
        Require(progressCurrent == frame.renderModel.blocks.size()
                && progressTotal == frame.renderModel.blocks.size(),
            "preparation progress reaches the full block count");
        Require(session.TextUtf8() == originalText && Stamp(frame) == originalStamps,
            "narrow preparation preserves exact source and render model identities");

        auto wide = EditorSurfaceRenderer::PrepareDocument(frame, wideContext, {}, {});
        CheckLayout(*wide, "wide");
        Require(narrow->layout->totalHeight > wide->layout->totalHeight,
            "narrow DirectWrite layout wraps to a greater document height");
        Require(narrow->layout->blocks[mathOnlyBlock].height > wide->layout->blocks[mathOnlyBlock].height,
            "narrow MathJax fragments increase the math-only paragraph height");
        Require(session.TextUtf8() == originalText && Stamp(frame) == originalStamps,
            "wide preparation preserves exact source and render model identities");

        SvgNormalizer normalizer;
        const auto em = baseContext.theme.typography.body.size;
        const auto width = DocumentWidth(narrowContext);
        const auto color = TextColor(narrowContext.theme);
        auto preparedMath = narrow->math.Render(longFormula, false, em, width, normalizer, color);
        auto repeatedMath = narrow->math.Render(longFormula, false, em, width, normalizer, color);
        auto wideMath = wide->math.Render(longFormula, false, em, DocumentWidth(wideContext), normalizer, color);
        Require(preparedMath && repeatedMath && wideMath && static_cast<bool>(*preparedMath)
                && static_cast<bool>(*wideMath),
            "prepared document retains usable inline math metrics");
        Require(preparedMath->fragments.size() > 1
                && std::ranges::any_of(preparedMath->fragments, [](auto const& fragment) {
                    return fragment.breakBefore;
                }) && wideMath->fragments.size() > 1,
            "MathJax supplies optional inline fragment breaks for DirectWrite at both widths");
        Require(SameMetrics(*preparedMath, *repeatedMath),
            "repeated prepared math preserves metrics and SVG render ids");

        CheckReadingLayout(frame, *narrow, mathOnlyBlock);

        std::stop_source cancellation;
        cancellation.request_stop();
        bool cancelled = false;
        try
        {
            (void)EditorSurfaceRenderer::PrepareDocument(frame, narrowContext,
                cancellation.get_token(), {});
        }
        catch (winrt::hresult_canceled const&)
        {
            cancelled = true;
        }
        Require(cancelled, "pre-cancelled preparation throws and yields no result");

        std::stop_source midway;
        bool midwayCancelled = false;
        std::unique_ptr<EditorDocumentPreparation> incomplete;
        try
        {
            incomplete = EditorSurfaceRenderer::PrepareDocument(frame, narrowContext, midway.get_token(),
                [&](std::size_t current, std::size_t) { if (current == 2) midway.request_stop(); });
        }
        catch (winrt::hresult_canceled const&) { midwayCancelled = true; }
        Require(midwayCancelled && !incomplete, "midway cancellation never returns partial preparation");

        // Force the existing virtualized model path and prepare every batch,
        // including the last block, without changing document source.
        EditorSession largeSession;
        std::wstring largeSource;
        for (int block = 0; block < 4100; ++block) largeSource += L"Paragraph $1$\n\n";
        largeSession.SetText(winrt::hstring(largeSource));
        auto largeOriginalText = largeSession.TextUtf8();
        auto largeFrame = largeSession.RenderFrame();
        Require(largeFrame.renderModel.virtualized, "large fixture uses existing render-model virtualization");
        auto large = EditorSurfaceRenderer::PrepareDocument(largeFrame, narrowContext, {}, {});
        CheckLayout(*large, "virtualized");
        auto mathBlocks = std::ranges::count_if(large->layout->blocks,
            [](auto const& block) { return block.containsMath; });
        Require(large->layout->blocks.size() == largeFrame.renderModel.blocks.size()
            && mathBlocks == 4100,
            "complete preparation measures every virtualized math paragraph, including the final one");
        Require(largeSession.TextUtf8() == largeOriginalText,
            "virtualized preparation preserves exact source");
        std::cout << "virtualized blocks=" << large->layout->blocks.size() << " math-paragraphs=" << mathBlocks
                  << " cancellation-midway=success\n";

        auto matching = EditorSurfaceRenderer::PrepareDocument(frame, baseContext, {}, {});
        renderer.AdoptDocumentPreparation(std::move(matching));
        Require(renderer.MatchesPreparationContext(baseContext),
            "renderer adopts a preparation with matching context");
        auto changedContext = baseContext;
        changedContext.surfaceWidth += 1.0f;
        auto mismatched = EditorSurfaceRenderer::PrepareDocument(frame, changedContext, {}, {});
        bool rejected = false;
        try
        {
            renderer.AdoptDocumentPreparation(std::move(mismatched));
        }
        catch (std::logic_error const&)
        {
            rejected = true;
        }
        Require(rejected, "renderer rejects preparation measured at another width");
        Require(session.TextUtf8() == originalText && Stamp(frame) == originalStamps,
            "all preparation paths preserve exact source and render model identities");

        std::cout << "layout narrow-height=" << narrow->layout->totalHeight
                  << " wide-height=" << wide->layout->totalHeight
                  << " blocks=" << originalStamps.size()
                  << " progress=" << progressCurrent << '/' << progressTotal
                  << " math-fragments=" << preparedMath->fragments.size()
                  << " wide-math-fragments=" << wideMath->fragments.size()
                  << " cancellation=success publication-context=success\n";
        std::cout << "native MathJax document layout probe: PASS\n";
    }
}

int wmain()
{
    try
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        RunProbe();
        winrt::uninit_apartment();
        return 0;
    }
    catch (winrt::hresult_error const& error)
    {
        std::cerr << "native MathJax document layout probe: FAIL: HRESULT 0x"
                  << std::hex << static_cast<std::uint32_t>(error.code().value)
                  << ": " << winrt::to_string(error.message()) << '\n';
        return 1;
    }
    catch (std::exception const& error)
    {
        std::cerr << "native MathJax document layout probe: FAIL: " << error.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "native MathJax document layout probe: FAIL: unknown exception\n";
        return 1;
    }
}
