// Root-owned static source bindings; Munder inherits ACCENTS, Mover owns it.
module folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.core.mml_tree.script_defaults;
import std;
namespace folia::math::mml {
PropertyList& MmlMunderover::defaults(){return munderoverDefaults();}
PropertyList& MmlMunder::defaults(){return munderDefaults();}
PropertyList& MmlMover::defaults(){return moverDefaults();}
std::shared_ptr<std::vector<Property>>& MmlMunderover::accents(){return underAccents();}
std::shared_ptr<std::vector<Property>>& MmlMover::accents(){return overAccents();}
}
