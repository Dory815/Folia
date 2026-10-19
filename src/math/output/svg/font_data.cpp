module folia.math.output.svg.font_data;
import std;

namespace folia::math::output::svg {
SvgFontGlyphSource const* SvgFontResources::find(SvgFontGlyphKey const& key) const noexcept {
    auto current = glyphs_.find(key);
    if (current != glyphs_.end()) return &current->second;
    auto alias = aliases_.find(key);
    if (alias == aliases_.end()) return nullptr;
    current = glyphs_.find(alias->second);
    return current == glyphs_.end() ? nullptr : &current->second;
}

std::size_t SvgFontResources::size() const noexcept { return glyphs_.size(); }

void SvgFontResourcesBuilder::add(SvgFontGlyphKey key, SvgFontGlyphSource source) {
    if (!source.path_defined && !source.path_data.empty())
        throw std::invalid_argument("SVG font path bytes without Source p option");
    glyphs_.insert_or_assign(std::move(key), std::move(source));
}

void SvgFontResourcesBuilder::addAlias(SvgFontGlyphKey key, SvgFontGlyphKey target) {
    if (key == target) return;
    aliases_.insert_or_assign(std::move(key), std::move(target));
}

SvgFontResources SvgFontResourcesBuilder::freeze() && {
    for (auto const& [_, target] : aliases_)
        if (!glyphs_.contains(target))
            throw std::invalid_argument("SVG font alias target is absent");
    return SvgFontResources(std::move(glyphs_), std::move(aliases_));
}
}
