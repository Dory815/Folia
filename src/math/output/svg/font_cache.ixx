/*************************************************************
 * MathJax 4.1.3 output/svg/FontCache.ts scene-backed cache owner.
 *************************************************************/
export module folia.math.output.svg.font_cache;
export import folia.math.output.scene;
import std;

export namespace folia::math::output::svg {
class FontCache {
    std::unordered_map<std::string,std::string> cache_;
    std::u16string localID_;
    std::uint64_t nextID_=0;
    folia::math::output::scene::NodeRef defs_;
public:
    folia::math::output::scene::NodeRef clearCache(folia::math::output::scene::Builder&);
    folia::math::output::scene::NodeRef getCache()const{return defs_;}
    void clearLocalID(){localID_.clear();}
    void useLocalID(const folia::math::Property& id=folia::math::Property(folia::math::PropertyNull{}));
    std::string cachePath(std::string_view variant,std::string_view cacheID,
                          std::string_view code,std::string path,
                          folia::math::output::scene::Builder&);
    std::size_t size()const noexcept{return cache_.size();}
};
}
