// Root-frozen captured constructor services; no semantic algorithms.
export module folia.math.core.mml_tree.mo_types;
export import folia.math.core.mml_tree.mo_patterns;
import std;
export namespace folia::math::mml {
using MoPatternHandle=std::shared_ptr<Pattern>;
struct MoClassServices {
    std::function<Property()> table,spacing,remap;
    std::function<MoPatternHandle()> pseudoScripts,primes,opPattern,mathaccents,mathaccentsWithWidth;
};
}
