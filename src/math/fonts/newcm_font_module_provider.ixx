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
// Publishes one font request over pinned read-only records. Source trace fields
// are provenance metadata. Available glyph/remap records are separate from
// resource objects actually materialized by this request.
NewcmFontDataBundle makeNewcmFontData();
NewcmProviderCoverage newcmProviderCoverage(output::common::FontRepository const& repository);
}
