// Root-owned shared operation corresponding to MmlMo.adjustTeXclass.
// TeXAtom installs that same upstream function as its final method.
export module folia.math.core.mml_tree.operator_class;
export import folia.math.core.mml_tree.mml_node;
export namespace folia::math::mml {
MmlNode* adjustOperatorTeXclass(MmlNode& receiver, MmlNode* previous);
}
