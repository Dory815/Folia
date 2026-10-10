// Root-owned source defaults/static primitive bindings.
module folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.leaf_defaults;
import folia.math.core.mml_tree.leaf_host;
import std;
namespace folia::math::mml {
PropertyList& MmlMi::defaults(){return miDefaults();}
Pattern& MmlMi::operatorName(){static Pattern p=host::operatorNamePattern;return p;}
Pattern& MmlMi::singleCharacter(){static Pattern p=host::singleCharacterPattern;return p;}
}
