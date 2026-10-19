/*************************************************************
 * Copyright (c) 2022-2026 The MathJax Consortium.
 * Native implementation of MmlNodes/HtmlNode.ts (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.core.mml_tree.mml_nodes.html_node;
export import folia.math.core.mml_tree.html_adaptor;
import std;

export namespace folia::math::mml {
class HtmlNode : public XMLNode {
  public:
    explicit HtmlNode(MmlDocument &);
    NodeKind kind() const noexcept override;
    XmlRef getHTML() const;
    XMLNode *setHTML(XmlRef, std::shared_ptr<XmlAdaptor const> adaptor = {});
    std::string getSerializedHTML() const;
    std::u16string textContent() const;
    std::string toString() const override;
    void verifyTree(std::optional<VerifyOptions> const &) override;
};
} // namespace folia::math::mml
