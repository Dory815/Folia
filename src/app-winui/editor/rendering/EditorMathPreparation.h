#pragma once

#include "media/MathJaxRenderer.h"
#include "editor/session/EditorRenderFrame.h"

namespace winrt::Folia
{
    struct EditorMathPreparationContext
    {
        folia::ThemeProfile theme;
        std::uint64_t themeRevision = 0;
        float surfaceWidth = 1.0f;
        float surfaceHeight = 1.0f;
        bool ready = false;
    };

    // Only completed immutable math results leave background preparation.
    // Ordinary text layout and visual resources remain viewport-local.
    struct EditorMathPreparation
    {
        EditorMathPreparationContext context;
        MathJaxRenderer math;
        std::size_t formulaCount = 0;
    };
}
