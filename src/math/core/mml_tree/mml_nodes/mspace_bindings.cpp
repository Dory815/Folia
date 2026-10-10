// Root-owned source defaults/static primitive bindings.
module folia.math.core.mml_tree.mml_nodes.mspace;
import folia.math.core.mml_tree.leaf_defaults;
import folia.math.core.mml_tree.leaf_host;
import std;
namespace folia::math::mml {
PropertyList& MmlMspace::defaults(){return mspaceDefaults();}
std::vector<std::string>& MmlMspace::NONSPACELIKE(){static std::vector<std::string> v={"height","depth","style","mathbackground","background"};return v;}
}
