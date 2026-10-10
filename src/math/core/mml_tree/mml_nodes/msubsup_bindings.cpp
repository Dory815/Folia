// Root-owned static source bindings.
module folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.script_defaults;
namespace folia::math::mml {
PropertyList& MmlMsubsup::defaults(){return msubsupDefaults();}
PropertyList& MmlMsub::defaults(){return msubDefaults();}
PropertyList& MmlMsup::defaults(){return msupDefaults();}
}
