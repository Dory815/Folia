#include "pch.h"
#include "editor/rendering/EditorDocumentPreparationPass.h"

namespace winrt::Folia
{
    using folia::platform::editor::BuildEditorViewportPlan;

    EditorDocumentPreparationPass::EditorDocumentPreparationPass(
        detail::EditorRenderFrame const& valueFrame,
        EditorRenderResources& valueResources,
        EditorStyleSheet const& valueStyleSheet,
        EditorInlineImageRenderer& valueInlineImages,
        EditorDocumentBlockPreparer& valueBlockPreparer,
        folia::platform::editor::EditorScrollState& valueScrollState,
        std::unique_ptr<EditorPreparedDocument>& valuePreparedDocument,
        folia::TextSelection valueSelection,
        float valueDocumentWidth,
        std::uint64_t valueThemeRevision,
        std::uint64_t valueEmbeddedGeneration,
        std::uint64_t valueRemoteImageGeneration,
        float valueScrollOffset,
        bool valuePrintMode)
        : cache(
              valueFrame,
              valueStyleSheet,
              valueInlineImages,
              valueBlockPreparer,
              valuePreparedDocument),
          viewportMaterializer(
              valueFrame,
              valueResources,
              valueBlockPreparer,
              valueScrollState,
              valuePreparedDocument),
          embeddedContentUpdater(
              valueFrame,
              valueResources,
              valueInlineImages,
              valueBlockPreparer,
              valuePreparedDocument),
          retention(
              valueFrame,
              valueResources,
              valueInlineImages,
              valuePreparedDocument),
          resources(valueResources),
          preparedDocument(valuePreparedDocument),
          selection(valueSelection),
          documentWidth(valueDocumentWidth),
          themeRevision(valueThemeRevision),
          embeddedGeneration(valueEmbeddedGeneration),
          remoteImageGeneration(valueRemoteImageGeneration),
          scrollOffset(valueScrollOffset),
          printMode(valuePrintMode)
    {
    }

    EditorDocumentPreparationResult EditorDocumentPreparationPass::Prepare()
    {
        cache.Reconcile(selection, documentWidth, themeRevision);
        cache.EnsureGeometry();
        auto viewport = viewportMaterializer.Materialize(scrollOffset, printMode);
        scrollOffset = viewport.scrollOffset;
        invalidateRequested = invalidateRequested || viewport.invalidateRequested;
        invalidateRequested = embeddedContentUpdater.Update(
                scrollOffset,
                printMode,
                viewport.scrollingForward,
                viewport.viewportActive,
                embeddedGeneration,
                remoteImageGeneration)
            || invalidateRequested;
        retention.ReleaseOutside(scrollOffset, printMode);

        auto paintPlan = BuildEditorViewportPlan(
            preparedDocument->geometry,
            scrollOffset,
            resources.surfaceHeightDip,
            printMode,
            true,
            viewportPolicy);
        return {
            .scrollOffset = scrollOffset,
            .visible = paintPlan.visible,
            .invalidateRequested = invalidateRequested,
        };
    }

    void EditorDocumentPreparationPass::PrepareAll(
        detail::EditorRenderFrame const& frame,
        EditorDocumentBlockPreparer& blockPreparer,
        std::stop_token stop,
        std::function<void(std::size_t, std::size_t)> const& progress)
    {
        cache.Reconcile(selection, documentWidth, themeRevision);
        cache.EnsureGeometry();
        auto const count = frame.renderModel.blocks.size();
        constexpr std::size_t batchSize = 16;
        if (progress) progress(0, count);
        for (std::size_t begin = 0; begin < count; begin += batchSize)
        {
            if (stop.stop_requested()) throw winrt::hresult_canceled();
            auto end = (std::min)(count, begin + batchSize);
            if (frame.materializeBlocks) frame.materializeBlocks(begin, end);
            for (auto index = begin; index < end; ++index)
            {
                if (stop.stop_requested()) throw winrt::hresult_canceled();
                auto const& source = frame.renderModel.blocks[index];
                if (source.kind != folia::RenderBlockKind::ThematicBreak)
                {
                    auto measured = blockPreparer.Prepare(source, true, true);
                    // No DirectWrite layout, D2D image, or worker device crosses
                    // the publication boundary. Exact heights remain resident.
                    measured.ReleaseVisualContent();
                    preparedDocument->blocks[index] = std::move(measured);
                    preparedDocument->geometry.UpdateHeight(
                        index, preparedDocument->blocks[index].height);
                }
                if (progress) progress(index + 1, count);
            }
            if (frame.releaseBlocksOutside) frame.releaseBlocksOutside(end, end);
        }
        preparedDocument->totalHeight = preparedDocument->geometry.TotalHeight();
        preparedDocument->layoutBlocks.clear();
        preparedDocument->embeddedBlocks.clear();
    }
}
