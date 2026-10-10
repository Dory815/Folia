// Root-owned source defaults/static primitive bindings.
module folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.leaf_defaults;
import folia.math.core.mml_tree.leaf_host;
import std;
namespace folia::math::mml {
PropertyList& MmlMtext::defaults(){return mtextDefaults();}
std::vector<std::string>& MmlMtext::NONSPACELIKE(){static std::vector<std::string> v={"style","mathbackground","background"};return v;}
}
