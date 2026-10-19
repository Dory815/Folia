/*************************************************************
 * Source FontCache.ts ids, local ids and insertion behavior.
 *************************************************************/
module folia.math.output.svg.font_cache;
import folia.math.util.property_host;
import folia.math.util.string;
import std;

namespace folia::math::output::svg {
using namespace folia::math;
using namespace folia::math::output::scene;

NodeRef FontCache::clearCache(Builder& builder){
    cache_.clear();
    defs_=builder.node(u"defs");
    return defs_;
}
void FontCache::useLocalID(const Property& id){
    const bool nullish=std::holds_alternative<Missing>(id.value)||
                       std::holds_alternative<PropertyNull>(id.value);
    const std::u16string value=nullish
        ?property_host::string(Property(static_cast<double>(++nextID_)))
        :property_host::string(id);
    localID_=value;
    if(!nullish&&id!=Property(std::string("")))localID_.push_back(u'-');
}
std::string FontCache::cachePath(std::string_view variant,std::string_view cacheID,
                                 std::string_view code,std::string path,Builder& builder){
    const auto id="MJX-"+property_host::debugText(localID_)+std::string(cacheID)+"-"+std::string(code);
    if(!cache_.contains(id)){
        cache_.emplace(id,path);
        auto glyph=builder.node(u"path",PropertyList{{"id",Property(id)}});
        glyph->geometry=PathGeometry{decodeGlyphOutline(std::move(path))};
        builder.retain(std::get<PathGeometry>(glyph->geometry).outline);
        if(!defs_)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'append')");
        builder.append(defs_,glyph);
    }
    (void)variant;
    return id;
}
}
