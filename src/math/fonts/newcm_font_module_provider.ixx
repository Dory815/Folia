export module folia.math.fonts.newcm_font_module_provider;
export import folia.math.fonts.newcm_font_data;
import std;

export namespace folia::math::fonts {
struct NewcmProviderCoverage {
    std::size_t source_dynamic_module_count{};
    std::size_t source_provider_trace_events{};
    std::size_t native_actual_direct_glyph_records{};
    std::size_t native_actual_remap_records{};
    std::size_t native_actual_glyph_resource_objects{};
};
struct NewcmFontDataBundle {
    output::common::FontRepository fonts;
    output::svg::SvgFontResources svg_resources;
};
// Builds all pinned shards before publication. module_count and request_count
// Source trace fields are provenance metadata. Native direct glyph, remap and
// resource counts are measured from the constructed immutable face.
NewcmFontDataBundle makeNewcmFontData();
NewcmProviderCoverage newcmProviderCoverage(output::common::FontRepository const& repository);
}
