#pragma once

import folia.core.render_model;

namespace winrt::Folia
{
    // Rich-text document and formula preparation use identical width inputs.
    inline float EditorDocumentContentWidth(float surfaceWidth, float horizontalPadding)
    {
        auto padding = (std::min)(horizontalPadding, (std::max)(12.0f, surfaceWidth * 0.06f));
        return (std::max)(padding + 1.0f, surfaceWidth - padding - 14.0f) - padding;
    }

    inline float EditorBlockContentWidth(folia::RenderBlock const& block, float documentWidth)
    {
        auto flow = !block.inline_items.empty()
            && (block.kind == folia::RenderBlockKind::Quote || block.kind == folia::RenderBlockKind::Callout);
        return (std::max)(1.0f, documentWidth
            - (flow ? 0.0f : block.block_style.padding_left)
            - (flow ? 0.0f : block.block_style.padding_right));
    }

    inline float EditorTableCellContentWidth(float tableWidth, std::size_t columns)
    {
        return (std::max)(1.0f, (std::max)(1.0f, tableWidth)
            / static_cast<float>((std::max)(std::size_t{1}, columns)) - 20.0f);
    }
}
