// MathJax 4.1.3 MmlNodes/mrow.ts; Apache-2.0 (LICENSE-MathJax.txt).
export module folia.math.core.mml_tree.mml_nodes.mrow;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMrow:public AbstractMmlNode{
protected:mutable Property core_=PropertyNull{};
public:using AbstractMmlNode::AbstractMmlNode;static PropertyList& defaults();void initializeClassFields()override;
Property sourceCoreField()const{return core_;}void setSourceCoreField(Property value){core_=std::move(value);}
std::string kind()const override;bool isSpacelike()const override;bool isEmbellished()const override;
NodeValue core()override;NodeValue coreMO()override;
virtual std::size_t nonSpaceLength();virtual MmlNode* firstNonSpace();virtual MmlNode* lastNonSpace();NodeValue setTeXclass(NodeValue)override;
};
class MmlInferredMrow:public MmlMrow{public:using MmlMrow::MmlMrow;static PropertyList& defaults();std::string kind()const override;bool isInferred()const override;Property notParent()const override;std::string toString()const override;};
}
