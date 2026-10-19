import folia.math.fonts.newcm_font_module_provider;
import folia.math.output.common.font_data;
import folia.math.output.common.font_path;
import folia.math.output.scene.types;
import std;

namespace folia::math::fonts {
namespace {
void stringJson(std::ostream& out, std::string_view value) {
    out << '"';
    for (const unsigned char c : value) {
        switch (c) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (c < 0x20) out << "\\u00" << std::hex << std::setw(2) << std::setfill('0') << int(c) << std::dec;
            else out << char(c);
        }
    }
    out << '"';
}
void pointJson(std::ostream& out, output::scene::Point p) { out << '[' << p.x << ',' << p.y << ']'; }
void outlineErrorJson(std::ostream& out,
        std::optional<output::common::FontOutlineError> const& error) {
    if (!error) { out << "null"; return; }
    auto reason = [](output::common::FontOutlineErrorReason value) -> std::string_view {
        using R = output::common::FontOutlineErrorReason;
        switch (value) {
        case R::missing_command: return "missing_command";
        case R::unsupported_command: return "unsupported_command";
        case R::invalid_number: return "invalid_number";
        case R::incomplete_segment: return "incomplete_segment";
        }
        return "invalid_reason";
    };
    out << "{\"byte_offset\":" << error->byte_offset << ",\"command\":";
    if (error->command == '\0') out << "null";
    else { const char text[] = {error->command}; stringJson(out, std::string_view{text, 1}); }
    out << ",\"reason\":"; stringJson(out, reason(error->reason)); out << '}';
}
void outlineJson(std::ostream& out, output::scene::GlyphResource const* resource) {
    if (!resource) { out << "null"; return; }
    out << "{\"font_id\":"; stringJson(out, resource->font_id()); out << ",\"segments\":[";
    bool first = true;
    for (auto const& segment : resource->outline().outline.segments) {
        if (!first) out << ','; first = false;
        std::visit([&](auto const& s) {
            using T = std::decay_t<decltype(s)>;
            if constexpr (std::is_same_v<T, output::scene::MoveTo> || std::is_same_v<T, output::scene::LineTo>) {
                out << "{\"type\":\"" << (std::is_same_v<T, output::scene::MoveTo> ? "M" : "L") << "\",\"to\":"; pointJson(out,s.to); out << '}';
            } else if constexpr (std::is_same_v<T, output::scene::QuadTo>) {
                out << "{\"type\":\"Q\",\"control\":"; pointJson(out,s.control); out << ",\"to\":"; pointJson(out,s.to); out << '}';
            } else if constexpr (std::is_same_v<T, output::scene::CubicTo>) {
                out << "{\"type\":\"C\",\"control1\":"; pointJson(out,s.control1); out << ",\"control2\":"; pointJson(out,s.control2); out << ",\"to\":"; pointJson(out,s.to); out << '}';
            } else if constexpr (std::is_same_v<T, output::scene::ArcTo>) {
                out << "{\"type\":\"A\",\"rx\":" << s.radius_x << ",\"ry\":" << s.radius_y << ",\"rotation\":" << s.rotation << ",\"large_arc\":" << (s.large_arc?"true":"false") << ",\"sweep\":" << (s.sweep?"true":"false") << ",\"to\":"; pointJson(out,s.to); out << '}';
            } else out << "{\"type\":\"Z\"}";
        }, segment);
    }
    out << "]}";
}
template<class Range, class Emit> void arrayJson(std::ostream& out, Range const& range, Emit emit) {
    out << '['; bool first = true;
    for (auto const& value : range) { if (!first) out << ','; first = false; emit(value); }
    out << ']';
}
}

// Root may bind this bounded writer to folia_console_test_runtime when adding
// a native target. It emits a complete deterministic comparison document.
void dumpNativeRaiiFontData(std::ostream& out) {
    const auto bundle = makeNewcmFontData();
    auto const& face = bundle.fonts.face("MathJaxNewcm");
    std::size_t directGlyphRecords=0, remapRecords=0, outlineSegments=0;
    std::set<output::scene::GlyphResource const*> resources;
    for(auto const& [_,variant]:face.variants){
        directGlyphRecords+=variant.glyphs.size(); remapRecords+=variant.remap.size();
        for(auto const& [__,glyph]:variant.glyphs)if(glyph.resource){
            resources.insert(glyph.resource.get());
            outlineSegments+=glyph.resource->outline().outline.segments.size();
        }
    }
    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    out << "{\"face\":\"MathJaxNewcm\",\"native_actual_counts\":{";
    out << "\"variant_records\":" << face.variants.size() << ",\"direct_glyph_records\":" << directGlyphRecords
        << ",\"remap_records\":" << remapRecords << ",\"distinct_glyph_resource_objects\":" << resources.size()
        << ",\"outline_segment_records\":" << outlineSegments << "},\"source_profile\":{";
    out << "\"source_profile_glyph_count\":" << face.sourceProfileGlyphCount()
        << ",\"source_dynamic_module_count\":" << face.source_dynamic_module_count
        << ",\"source_provider_trace_events\":" << face.source_provider_trace_events << "}";
    out << ",\"size_variants\":"; arrayJson(out,face.size_variants,[&](auto const& x){stringJson(out,x);});
    out << ",\"stretch_variants\":"; arrayJson(out,face.stretch_variants,[&](auto const& x){stringJson(out,x);});
    out << ",\"parameters\":{"; bool first=true; for(auto const& [k,v]:face.parameters.values){if(!first)out<<',';first=false;stringJson(out,k);out<<':'<<v;} out<<'}';
    out << ",\"extension_glyph_counts\":{"; first=true; for(auto const& [k,v]:face.extension_glyph_counts){if(!first)out<<',';first=false;stringJson(out,k);out<<':'<<v;} out<<'}';
    auto stringMap=[&](char const* label,auto const& map){out<<",\""<<label<<"\":{";bool fm=true;for(auto const&[cp,text]:map){if(!fm)out<<',';fm=false;stringJson(out,std::to_string(cp));out<<':';arrayJson(out,text,[&](auto c){out<<unsigned(c);});}out<<'}';};
    stringMap("operator_remap",face.operatorRemap);stringMap("accent_remap",face.accentRemap);stringMap("number_remap",face.numberRemap);
    out << ",\"variants\":{"; first=true;
    for (auto const& [name,variant] : face.variants) {
        if(!first)out<<','; first=false; stringJson(out,name); out<<":{";
        out << "\"cache_id\":"; stringJson(out,variant.cache_id);
        out << ",\"inherit\":"; if(variant.inherit_variant)stringJson(out,*variant.inherit_variant);else out<<"null";
        out << ",\"link_overlay\":"; if(variant.link_overlay_variant)stringJson(out,*variant.link_overlay_variant);else out<<"null";
        out << ",\"glyphs\":{"; bool fg=true;
        for(auto const& [cp,g] : variant.glyphs){if(!fg)out<<',';fg=false;stringJson(out,std::to_string(cp));out<<":{";
            out<<"\"metrics\":["<<g.metrics.h<<','<<g.metrics.d<<','<<g.metrics.w<<','<<g.metrics.ic<<','<<g.metrics.sk<<','<<g.metrics.dx<<','<<g.metrics.oc<<']';
            out<<",\"path_defined\":"<<(g.path_defined?"true":"false")<<",\"composition\":";arrayJson(out,g.composition,[&](auto x){out<<x;});
            out<<",\"outline\":";outlineJson(out,g.resource.get());out<<",\"outline_error\":";outlineErrorJson(out,g.outline_error);out<<'}';}
        out << "},\"remap\":{"; bool fr=true; for(auto const&[cp,r]:variant.remap){if(!fr)out<<',';fr=false;stringJson(out,std::to_string(cp));out<<":{\"variant\":";stringJson(out,r.variant);out<<",\"codepoint\":"<<r.codepoint<<'}';} out<<"}}";
    }
    out << "},\"effective_glyphs\":{"; first=true;
    auto collectOwn=[](output::common::FontVariant const& v,std::set<std::uint32_t>& keys){
        for(auto const& [cp,_]:v.glyphs)keys.insert(cp);
        for(auto const& [cp,_]:v.remap)keys.insert(cp);
    };
    auto collectEffective=[&](auto&& self,output::common::FontFaceData const& f,
            std::string const& variantName,std::set<std::uint32_t>& keys,
            std::set<std::string,std::less<>>& visited)->void{
        if(!visited.insert(variantName).second)return;
        auto const& v=f.variants.at(variantName);
        collectOwn(v,keys);
        if(v.link_overlay_variant){auto it=f.variants.find(*v.link_overlay_variant);if(it!=f.variants.end())collectOwn(it->second,keys);}
        if(v.inherit_variant){auto it=f.variants.find(*v.inherit_variant);if(it!=f.variants.end())self(self,f,it->first,keys,visited);}
    };
    for(auto const& [name,variant]:face.variants){
        if(!first)out<<',';first=false;stringJson(out,name);out<<":{";std::set<std::uint32_t> keys;std::set<std::string,std::less<>> visited;collectEffective(collectEffective,face,name,keys,visited);bool fg=true;
        for(auto cp:keys){auto resolved=bundle.fonts.resolveGlyph(face.name,name,cp);if(!resolved)continue;if(!fg)out<<',';fg=false;stringJson(out,std::to_string(cp));out<<":{";
            auto const& g=*resolved->data;out<<"\"metrics\":["<<g.metrics.h<<','<<g.metrics.d<<','<<g.metrics.w<<','<<g.metrics.ic<<','<<g.metrics.sk<<','<<g.metrics.dx<<','<<g.metrics.oc<<']';
            out<<",\"path_defined\":"<<(g.path_defined?"true":"false")<<",\"composition\":";arrayJson(out,g.composition,[&](auto x){out<<x;});
            out<<",\"glyph_key\":[";stringJson(out,resolved->key.variant);out<<',';stringJson(out,resolved->key.variant_cache_id);out<<','<<resolved->key.codepoint<<']';
            out<<",\"outline_font_id\":";if(g.resource)stringJson(out,g.resource->font_id());else out<<"null";
            out<<",\"outline_error\":";outlineErrorJson(out,g.outline_error);
            auto const* svg=bundle.svg_resources.find({resolved->key.variant,resolved->key.codepoint});
            out<<",\"svg_resource\":";if(!svg)out<<"null";else {out<<"{\"path_defined\":"<<(svg->path_defined?"true":"false")<<",\"path_data\":";stringJson(out,svg->path_data);out<<",\"content_data\":";if(svg->content_data)stringJson(out,*svg->content_data);else out<<"null";out<<'}';}out<<'}';
        }out<<'}';
    }
    out << "},\"delimiters\":{"; first=true;
    for(auto const&[cp,d]:face.delimiters){if(!first)out<<',';first=false;stringJson(out,std::to_string(cp));out<<":{";
        out<<"\"direction\":"<<int(d.direction)<<",\"sizes\":";arrayJson(out,d.sizes,[&](auto x){out<<x;});
        auto ids=[&](char const* key,auto const& arr){out<<",\""<<key<<"\":";arrayJson(out,arr,[&](auto x){out<<x;});};
        ids("variants",d.variants);ids("schar",d.schar);ids("stretch",d.stretch);ids("stretchv",d.stretchv);
        auto opt=[&](char const* key,auto const& value){out<<",\""<<key<<"\":";if(value)out<<*value;else out<<"null";};
        opt("ext",d.ext);opt("min",d.min);opt("c",d.c);
        auto optarr=[&](char const* key,auto const& value){out<<",\""<<key<<"\":";if(value)arrayJson(out,*value,[&](auto x){out<<x;});else out<<"null";};
        optarr("hd",d.heightDepth);optarr("HDW",d.heightDepthWidth);optarr("fullExt",d.fullExtent);out<<'}';}
    out << "},\"svg_resources\":{"; first=true;
    for(auto const&[key,value]:bundle.svg_resources.entries()){if(!first)out<<',';first=false;std::ostringstream encodedKey;encodedKey<<'[';stringJson(encodedKey,key.variant);encodedKey<<','<<key.codepoint<<']';stringJson(out,encodedKey.str());out<<":{\"path_defined\":"<<(value.path_defined?"true":"false")<<",\"path_data\":";stringJson(out,value.path_data);out<<",\"content_data\":";if(value.content_data)stringJson(out,*value.content_data);else out<<"null";out<<'}';}
    out << "},\"svg_aliases\":["; first=true;for(auto const&[key,target]:bundle.svg_resources.aliases()){if(!first)out<<',';first=false;out<<"[[";stringJson(out,key.variant);out<<','<<key.codepoint<<"],[";stringJson(out,target.variant);out<<','<<target.codepoint<<"]]";}out<<"]}\n";
}
}

// Standalone verification tool, linked with folia_console_test_runtime.
int main() {
    try {
        folia::math::fonts::dumpNativeRaiiFontData(std::cout);
        return std::cout ? 0 : 1;
    } catch (std::exception const& error) {
        std::cerr << "Font data dump failed: " << error.what() << '\n';
        return 1;
    }
}
