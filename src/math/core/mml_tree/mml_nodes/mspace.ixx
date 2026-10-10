// MathJax 4.1.3 MmlNodes/mspace.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mspace;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMspace:public AbstractMmlTokenNode{public:using AbstractMmlTokenNode::AbstractMmlTokenNode;static PropertyList& defaults();static std::vector<std::string>& NONSPACELIKE();void initializeClassFields()override;std::string kind()const override;double arity()const override;bool isSpacelike()const override;virtual bool hasNewline()const;virtual bool canBreak()const;MmlNode* setTeXclass(MmlNode*)override;};
}
