module folia.math.output.common.font_data;
import std;

namespace folia::math::output::common {
namespace {
struct GlyphLookup { FontGlyph const* data{}; std::uint32_t codepoint{}; };
std::optional<GlyphLookup> resolveSourceGlyph(FontFaceData const& face,
        FontVariant const& start, std::uint32_t codepoint) {
    auto direct = [&](FontVariant const& variant, std::uint32_t cp) -> FontGlyph const* {
        if (auto found = variant.glyphs.find(cp); found != variant.glyphs.end()) return &found->second;
        return face.glyphSource ? face.glyphSource->glyph(variant.name, cp) : nullptr;
    };
    auto resolve = [&](auto&& self, FontVariant const& current, std::uint32_t cp,
                       unsigned depth) -> std::optional<GlyphLookup> {
        if (depth > face.variants.size()) return {};
        if (auto r = current.remap.find(cp); r != current.remap.end()) {
            auto target = face.variants.find(r->second.variant);
            if (target == face.variants.end()) return {};
            return self(self, target->second, r->second.codepoint, depth + 1);
        }
        if (auto found = direct(current, cp)) return GlyphLookup{found, cp};
        if (current.link_overlay_variant) {
            auto link = face.variants.find(*current.link_overlay_variant);
            if (link != face.variants.end()) {
                if (auto r = link->second.remap.find(cp); r != link->second.remap.end()) {
                    auto target = face.variants.find(r->second.variant);
                    if (target == face.variants.end()) return {};
                    return self(self, target->second, r->second.codepoint, depth + 1);
                }
                if (auto found = direct(link->second, cp)) return GlyphLookup{found, cp};
            }
        }
        if (current.inherit_variant) {
            auto parent = face.variants.find(*current.inherit_variant);
            if (parent != face.variants.end()) return self(self, parent->second, cp, depth + 1);
        }
        return {};
    };
    return resolve(resolve,start,codepoint,0);
}
std::optional<GlyphReference> resolveSourceRemap(FontFaceData const &face, FontVariant const &start,
                                                 std::uint32_t codepoint) {
    auto hasDirect = [&](FontVariant const& variant, std::uint32_t cp) {
        return variant.glyphs.contains(cp)
            || (face.glyphSource && face.glyphSource->contains(variant.name, cp));
    };
    auto resolve = [&](auto &&self, FontVariant const &current, std::uint32_t cp,
                       unsigned depth) -> std::optional<GlyphReference> {
        if (depth > face.variants.size())
            return {};
        if (auto found = current.remap.find(cp); found != current.remap.end())
            return found->second;
        // Source reads the resolved character entry, not just its SMP marker.
        // An explicit glyph at this level owns the codepoint and therefore
        // prevents a remap inherited from a lower-priority variant.
        if (hasDirect(current, cp))
            return {};
        if (current.link_overlay_variant) {
            auto link = face.variants.find(*current.link_overlay_variant);
            if (link != face.variants.end()) {
                if (auto found = link->second.remap.find(cp); found != link->second.remap.end())
                    return found->second;
                // The linked variant is Source's shallow character overlay. Its
                // glyph entry owns the slot before the requested variant's inherit.
                if (hasDirect(link->second, cp))
                    return {};
            }
        }
        if (current.inherit_variant) {
            auto parent = face.variants.find(*current.inherit_variant);
            if (parent != face.variants.end())
                return self(self, parent->second, cp, depth + 1);
        }
        return {};
    };
    return resolve(resolve, start, codepoint, 0);
}
} // namespace

std::size_t FontFaceData::glyphCount() const noexcept {
    std::size_t count = glyphSource ? glyphSource->glyphCount() : 0;
    for (const auto &[name, variant] : variants) {
        count += variant.glyphs.size();
        count += variant.remap.size();
    }
    return count;
}

std::string FontFaceData::getFamily(std::string_view family) const {
    if (cssFamilyPrefix.empty()) return std::string(family);
    std::string result;
    result.reserve(cssFamilyPrefix.size() + 2 + family.size());
    result.append(cssFamilyPrefix).append(", ").append(family);
    return result;
}

CssFontData FontFaceData::getCssFont(std::string_view variant) const {
    const auto it = cssFontMap.find(variant);
    return it == cssFontMap.end() ? CssFontData{"serif", false, false} : it->second;
}

FontFaceData const& FontRepository::face(std::string_view name) const {
    const auto it = faces_.find(name);
    if (it == faces_.end()) throw std::out_of_range("unknown font face");
    return *it->second;
}

FontGlyph const* FontRepository::glyph(std::string_view faceName,
        std::string_view variantName, std::uint32_t codepoint) const {
    const auto f = faces_.find(faceName);
    if (f == faces_.end()) return nullptr;
    const auto v = f->second->variants.find(variantName);
    if (v == f->second->variants.end()) return nullptr;
    const auto resolved = resolveSourceGlyph(*f->second,v->second,codepoint);
    return resolved ? resolved->data : nullptr;
}

std::optional<ResolvedFontGlyph> FontRepository::resolveGlyph(std::string_view faceName,
        std::string_view variantName, std::uint32_t codepoint) const {
    const auto f = faces_.find(faceName);
    if (f == faces_.end()) return {};
    const auto v = f->second->variants.find(variantName);
    if (v == f->second->variants.end()) return {};
    const auto resolved = resolveSourceGlyph(*f->second,v->second,codepoint);
    if (!resolved) return {};
    return ResolvedFontGlyph{resolved->data,
        output::scene::GlyphKey{std::string(variantName), v->second.cache_id, resolved->codepoint}};
}

std::optional<GlyphReference> FontRepository::remap(std::string_view faceName,
        std::string_view variantName, std::uint32_t codepoint) const {
    const auto f = faces_.find(faceName);
    if (f == faces_.end()) return {};
    const auto v = f->second->variants.find(variantName);
    if (v == f->second->variants.end()) return {};
    return resolveSourceRemap(*f->second,v->second,codepoint);
}

DelimiterRecipe const* FontRepository::delimiter(std::string_view faceName,
        std::uint32_t codepoint) const noexcept {
    const auto f = faces_.find(faceName);
    if (f == faces_.end()) return nullptr;
    const auto d = f->second->delimiters.find(codepoint);
    return d == f->second->delimiters.end() ? nullptr : &d->second;
}

std::size_t FontRepository::glyphCount() const noexcept {
    std::size_t count = 0;
    for (const auto& [_, face] : faces_)
        count += face->glyphCount();
    return count;
}

void FontRepositoryBuilder::addFace(FontFaceData face) {
    if (face.name.empty()) throw std::invalid_argument("font face name is empty");
    auto name = face.name;
    auto [_, inserted] = faces_.emplace(std::move(name),
        std::make_shared<FontFaceData>(std::move(face)));
    if (!inserted) throw std::invalid_argument("duplicate font face");
}

void FontRepositoryBuilder::addGlyph(std::string_view faceName, std::string_view variantName,
        std::uint32_t codepoint, FontGlyph glyph) {
    const auto it = faces_.find(faceName);
    if (it == faces_.end()) throw std::out_of_range("unknown font face");
    auto& variant = it->second->variants[std::string(variantName)];
    variant.name = variantName;
    if (it->second->glyphSource && it->second->glyphSource->contains(variantName, codepoint))
        throw std::invalid_argument("duplicate glyph in generated font data");
    if (!variant.glyphs.emplace(codepoint, std::move(glyph)).second)
        throw std::invalid_argument("duplicate glyph in generated font data");
}

void FontRepositoryBuilder::setVariantInheritance(std::string_view faceName,
        std::string_view variantName, std::optional<std::string> inherit,
        std::optional<std::string> linkOverlay) {
    const auto it = faces_.find(faceName);
    if (it == faces_.end()) throw std::out_of_range("unknown font face");
    auto& variant = it->second->variants[std::string(variantName)];
    variant.name = variantName;
    variant.inherit_variant = std::move(inherit);
    variant.link_overlay_variant = std::move(linkOverlay);
}

void FontRepositoryBuilder::addRemap(std::string_view faceName, std::string_view variantName,
        std::uint32_t codepoint, GlyphReference target) {
    const auto it = faces_.find(faceName);
    if (it == faces_.end()) throw std::out_of_range("unknown font face");
    auto& variant = it->second->variants[std::string(variantName)];
    variant.name = variantName;
    if (!variant.remap.emplace(codepoint, std::move(target)).second)
        throw std::invalid_argument("duplicate remap in generated font data");
}

void FontRepositoryBuilder::addParameter(std::string_view faceName, std::string key, double value) {
    const auto it = faces_.find(faceName);
    if (it == faces_.end()) throw std::out_of_range("unknown font face");
    if (!it->second->parameters.values.emplace(std::move(key), value).second)
        throw std::invalid_argument("duplicate font parameter");
}

void FontRepositoryBuilder::addDelimiter(std::string_view faceName, std::uint32_t codepoint,
        DelimiterRecipe recipe) {
    const auto it = faces_.find(faceName);
    if (it == faces_.end()) throw std::out_of_range("unknown font face");
    if (!it->second->delimiters.emplace(codepoint, std::move(recipe)).second)
        throw std::invalid_argument("duplicate delimiter recipe");
}

FontRepository FontRepositoryBuilder::freeze() && {
    decltype(FontRepository::faces_) frozen;
    for (auto& [name, face] : faces_)
        // Snapshot values so no mutable builder alias can alter publication.
        frozen.emplace(name, std::make_shared<FontFaceData const>(*face));
    faces_.clear();
    return FontRepository(std::move(frozen));
}
}
