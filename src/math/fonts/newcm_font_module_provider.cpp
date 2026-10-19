module folia.math.fonts.newcm_font_module_provider;
import folia.math.output.scene.types;
import std;

namespace folia::math::fonts {
NewcmFontDataBundle makeNewcmFontData() {
    output::common::FontRepositoryBuilder builder;
    output::svg::SvgFontResourcesBuilder svgBuilder;
    installNewcmFontData(builder, svgBuilder);
    return {std::move(builder).freeze(), std::move(svgBuilder).freeze()};
}

NewcmProviderCoverage newcmProviderCoverage(output::common::FontRepository const& repository) {
    const auto& face = repository.face("MathJaxNewcm");
    std::size_t direct=0, remaps=0;
    std::set<output::scene::GlyphResource const*> resources;
    for(auto const& [_,variant]:face.variants){
        direct+=variant.glyphs.size(); remaps+=variant.remap.size();
        for(auto const& [__,glyph]:variant.glyphs)if(glyph.resource)resources.insert(glyph.resource.get());
    }
    return {.source_dynamic_module_count=face.source_dynamic_module_count,
            .source_provider_trace_events=face.source_provider_trace_events,
            .native_actual_direct_glyph_records=direct,
            .native_actual_remap_records=remaps,
            .native_actual_glyph_resource_objects=resources.size()};
}
}
