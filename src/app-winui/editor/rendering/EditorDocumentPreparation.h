#pragma once

#include "media/MathJaxRenderer.h"
#include "editor/rendering/EditorPreparedDocument.h"

namespace winrt::Folia
{
    struct EditorDocumentPreparationContext
    {
        folia::ThemeProfile theme;
        std::uint64_t themeRevision = 0;
        float surfaceWidth = 1.0f;
        float surfaceHeight = 1.0f;
        bool ready = false;
    };

    // Only document metrics and owned SVG file ranges leave the background
    // preparation thread. Device resources remain on their creating thread.
    struct EditorDocumentPreparation
    {
        EditorDocumentPreparationContext context;
        MathJaxRenderer math;
        std::unique_ptr<EditorPreparedDocument> layout;
    };
}
