// Root-frozen portable registry interface for complete ts/core/MmlTree/MML.ts.
export module folia.math.core.mml_tree.mml;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
// Source const MML binding retains one mutable registry object and class refs.
const std::shared_ptr<tree::FactoryClassMap<MmlNode>>& MML();
}
