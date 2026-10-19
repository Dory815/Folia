/*************************************************************
 * Copyright (c) 2022-2026 The MathJax Consortium.
 * Native implementation of MmlNodes/HtmlNode.ts (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.html_node;
import std;

namespace folia::math::mml {
namespace {
HtmlAdaptor const &requireHtmlAdaptor(std::shared_ptr<XmlAdaptor const> const &base) {
    auto result = std::dynamic_pointer_cast<HtmlAdaptor const>(base);
    if (!result)
        throw std::logic_error("HTML adaptor is unavailable");
    return *result;
}
} // namespace

HtmlNode::HtmlNode(MmlDocument &d) : XMLNode(d) {
    setXML({}, {});
}

NodeKind HtmlNode::kind() const noexcept {
    return NodeKind::html;
}

XmlRef HtmlNode::getHTML() const {
    return getXML();
}

XMLNode *HtmlNode::setHTML(XmlRef html, std::shared_ptr<XmlAdaptor const> base) {
    auto adaptor = std::dynamic_pointer_cast<HtmlAdaptor const>(base);
    if (!adaptor)
        throw std::logic_error("HTML adaptor is unavailable");
    try {
        (void)adaptor->getAttribute(html, "data-mjx-hdw");
    } catch (...) {
        html = adaptor->node("span", {}, std::vector<XmlRef>{std::move(html)});
    }
    return setXML(std::move(html), std::move(base));
}

std::string HtmlNode::getSerializedHTML() const {
    return requireHtmlAdaptor(adaptor_).outerHTML(xml_);
}

std::u16string HtmlNode::textContent() const {
    return requireHtmlAdaptor(adaptor_).textContent(xml_);
}

std::string HtmlNode::toString() const {
    auto const &a = requireHtmlAdaptor(adaptor_);
    auto k = a.kind(xml_);
    return "HTML=<" + k + ">...</" + k + ">";
}

void HtmlNode::verifyTree(std::optional<VerifyOptions> const &options) {
    if (parent() && !parent()->isToken()) {
        VerifyOptions empty{};
        (void)mError(u"HTML can only be a child of a token element", options ? *options : empty,
                     true);
    }
}
} // namespace folia::math::mml
