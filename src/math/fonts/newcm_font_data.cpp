module folia.math.fonts.newcm_font_data;
import folia.math.output.common.font_path;
import folia.math.output.svg.font_data;
import std;

namespace folia::math::fonts {
using namespace output::common;
using namespace output::svg;

struct NewcmFontRequest::State {
    mutable std::mutex mutex;
    std::vector<std::span<NewcmFontGlyphRow const>> tables;
    std::vector<std::span<NewcmSvgAliasRow const>> aliases;
    mutable std::map<SvgFontGlyphKey, NewcmGlyphData> loaded;
    std::size_t recordCount = 0;

    NewcmFontGlyphRow const* row(SvgFontGlyphKey const& key) const {
        for (auto table = tables.rbegin(); table != tables.rend(); ++table) {
            auto found = std::lower_bound(table->begin(), table->end(), key,
                [](auto const& row, auto const& target) {
                    const auto name = std::string_view(row.variant);
                    return name < target.variant || (name == target.variant && row.code < target.codepoint);
                });
            if (found != table->end() && found->variant == key.variant && found->code == key.codepoint)
                return &*found;
        }
        return nullptr;
    }
    NewcmFontGlyphRow const* svgRow(SvgFontGlyphKey const& key) const {
        for (auto table = aliases.rbegin(); table != aliases.rend(); ++table) {
            auto found = std::lower_bound(table->begin(), table->end(), key,
                [](auto const& row, auto const& target) {
                    const auto name = std::string_view(row.from_variant);
                    return name < target.variant || (name == target.variant && row.from_codepoint < target.codepoint);
                });
            if (found != table->end() && found->from_variant == key.variant && found->from_codepoint == key.codepoint)
                return row({found->to_variant, found->to_codepoint});
        }
        return row(key);
    }
};

NewcmFontRequest::NewcmFontRequest() : state_(std::make_unique<State>()) {}
NewcmFontRequest::~NewcmFontRequest() = default;
void NewcmFontRequest::addRows(std::span<NewcmFontGlyphRow const> rows) {
    state_->tables.push_back(rows);
    state_->recordCount += rows.size();
}
void NewcmFontRequest::addAliases(std::span<NewcmSvgAliasRow const> rows) {
    state_->aliases.push_back(rows);
}
void NewcmFontRequest::setGlyphCount(std::size_t count) { state_->recordCount = count; }
std::size_t NewcmFontRequest::glyphCount() const noexcept { return state_->recordCount; }
std::size_t NewcmFontRequest::loadedGlyphCount() const noexcept {
    std::scoped_lock lock(state_->mutex);
    return state_->loaded.size();
}
std::size_t NewcmFontRequest::loadedResourceCount() const noexcept {
    std::scoped_lock lock(state_->mutex);
    return static_cast<std::size_t>(std::ranges::count_if(state_->loaded, [](auto const& entry) {
        return entry.second.glyph && entry.second.glyph->resource;
    }));
}

std::vector<NewcmGlyphData const*> NewcmFontRequest::read(std::span<SvgFontGlyphKey const> keys) const {
    std::scoped_lock lock(state_->mutex);
    std::vector<NewcmGlyphData const*> result;
    result.reserve(keys.size());
    for (auto const& key : keys) {
        auto found = state_->loaded.find(key);
        if (found == state_->loaded.end()) {
            NewcmGlyphData data;
            if (auto row = state_->row(key)) {
                FontGlyph glyph;
                glyph.metrics = {row->metrics[0], row->metrics[1], row->metrics[2], row->metrics[3],
                    row->metrics[4], row->metrics[5], row->metrics[6]};
                glyph.path_defined = row->path_defined;
                glyph.composition.assign(row->composition, row->composition + row->composition_size);
                if (row->path_defined) {
                    auto decoded = *row->path ? decodeFontOutlinePrefix(std::string("M") + row->path + "Z", 1000.0)
                        : FontOutlineDecodeResult{output::scene::GlyphOutline{}, std::nullopt};
                    glyph.outline_error = decoded.error;
                    glyph.resource = output::scene::GlyphResource::make(
                        std::string("MathJaxNewcm/") + row->variant, std::move(decoded.outline));
                }
                data.glyph = std::move(glyph);
            }
            if (auto row = state_->svgRow(key)) {
                SvgFontGlyphSource source{row->path_defined, row->serialized_path, {}};
                if (row->content_defined) source.content_data = row->content;
                data.svg = std::move(source);
            }
            found = state_->loaded.emplace(key, std::move(data)).first;
        }
        result.push_back(&found->second);
    }
    return result;
}

FontGlyph const* NewcmFontRequest::glyph(std::string_view variant, std::uint32_t codepoint) const {
    const SvgFontGlyphKey key{std::string(variant), codepoint};
    auto values = read(std::span{&key, std::size_t{1}});
    return values.front()->glyph ? &*values.front()->glyph : nullptr;
}
SvgFontGlyphSource const* NewcmFontRequest::find(SvgFontGlyphKey const& key) const {
    auto values = read(std::span{&key, std::size_t{1}});
    return values.front()->svg ? &*values.front()->svg : nullptr;
}
std::vector<SvgFontGlyphKey> NewcmFontRequest::keys() const {
    std::set<SvgFontGlyphKey> keys;
    for (auto table : state_->tables)
        for (auto const& row : table) keys.insert({row.variant, row.code});
    return {keys.begin(), keys.end()};
}
std::vector<std::pair<SvgFontGlyphKey, SvgFontGlyphKey>> NewcmFontRequest::aliases() const {
    std::map<SvgFontGlyphKey, SvgFontGlyphKey> aliases;
    for (auto table : state_->aliases)
        for (auto const& row : table)
            aliases.insert_or_assign(SvgFontGlyphKey{row.from_variant, row.from_codepoint},
                SvgFontGlyphKey{row.to_variant, row.to_codepoint});
    return {aliases.begin(), aliases.end()};
}

#include "newcm_font_data_typed.inc"

void installNewcmFontData(output::common::FontRepositoryBuilder& builder,
                          output::svg::SvgFontResourcesBuilder& svgBuilder) {
    auto request = std::make_shared<NewcmFontRequest>();
    auto face = makeNewcmFace(*request);
    face.glyphSource = request;
    svgBuilder.setSource(request);
    builder.addFace(std::move(face));
}
}
