#include "pch.h"

import std;
#include "editor/rendering/EditorSurfaceRenderer.h"

namespace winrt::Folia
{
    EditorDocumentPreparationContext EditorSurfaceRenderer::CapturePreparationContext() const
    {
        return {themeProfile, themeRevision, resources.surfaceWidthDip, resources.surfaceHeightDip, resources.Ready()};
    }

    bool EditorSurfaceRenderer::MatchesPreparationContext(
        EditorDocumentPreparationContext const& context) const
    {
        return context.ready && resources.Ready() && context.themeRevision == themeRevision
            && context.surfaceWidth == resources.surfaceWidthDip;
    }

    std::unique_ptr<EditorDocumentPreparation> EditorSurfaceRenderer::PrepareDocument(
        detail::EditorRenderFrame const& frame,
        EditorDocumentPreparationContext const& context,
        std::stop_token stop,
        std::function<void(std::size_t, std::size_t)> progress)
    {
        if (stop.stop_requested()) throw winrt::hresult_canceled();
        EditorSurfaceRenderer worker;
        worker.SetTheme(context.theme);
        worker.themeRevision = context.themeRevision;
        worker.resources.InitializeOffscreen(
            worker.styleSheet, context.surfaceWidth, context.surfaceHeight);
        worker.SetMathPreparationStopToken(stop);
        worker.preparingDocument = true;
        worker.preparationProgress = std::move(progress);
        worker.DrawDocument(frame);
        if (stop.stop_requested()) throw winrt::hresult_canceled();
        auto result = std::make_unique<EditorDocumentPreparation>();
        result->context = context;
        worker.mathJax.SetPreparationStopToken({});
        result->math = std::move(worker.mathJax);
        result->layout = std::move(worker.preparedDocument);
        return result;
    }

    void EditorSurfaceRenderer::AdoptDocumentPreparation(
        std::unique_ptr<EditorDocumentPreparation> preparation)
    {
        if (!preparation || !MatchesPreparationContext(preparation->context))
            throw std::logic_error("Document preparation context changed before publication");
        ResetDocumentCaches();
        mathJax = std::move(preparation->math);
        preparedDocument = std::move(preparation->layout);
        totalDocumentHeight = preparedDocument ? preparedDocument->totalHeight : 0.0f;
    }

    void EditorSurfaceRenderer::SetMathPreparationStopToken(std::stop_token stop)
    {
        preparationStop = stop;
        mathJax.SetPreparationStopToken(stop);
    }
}
