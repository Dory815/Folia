// Root-frozen MathJax 4.1.3 MmlNodes/mfenced.ts; Apache-2.0.
// SHA256 64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397
export module folia.math.core.mml_tree.mml_nodes.mfenced;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::mml {
class MmlMfenced : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    void initializeClassFields() override;
    std::string kind() const override;
    Children separators;
    MmlNode* open = nullptr;
    MmlNode* close = nullptr;
    NodeValue setTeXclass(NodeValue) override;
protected:
    void setChildInheritedAttributes(AttributeList,Property,Property,Property) override;
    virtual void addFakeNodes();
    virtual MmlNode* fakeNode(std::u16string,Property=Missing{},Property=Missing{});
};
}
