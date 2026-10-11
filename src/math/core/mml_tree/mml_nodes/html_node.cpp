/*************************************************************
 * Copyright (c) 2022-2026 The MathJax Consortium.
 * Native implementation of MmlNodes/HtmlNode.ts (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA-256: 1bf0ba981fc72b63a525424fb51e8bc45231fcf768d618fb8c75517bd72ae665
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.html_node;
import folia.math.core.mml_tree.mml_node_host;
import std;

namespace folia::math::mml {
namespace {
HtmlAdaptor& htmlAdaptor(XmlAdaptor* adaptor) {
    auto* const result = dynamic_cast<HtmlAdaptor*>(adaptor);
    if (!result) host::typeError("HTML adaptor method is unavailable");
    return *result;
}

ErrorOptions htmlErrorOptions(const std::optional<PropertyList>& options) {
    return options ? ErrorOptions(*options) : ErrorOptions(nullptr);
}
}

XmlRef HtmlNode::getHTML() const {
    return getXML();
}

XMLNode* HtmlNode::setHTML(XmlRef html, XmlAdaptor* adaptor) {
    try {
        (void)htmlAdaptor(adaptor).getAttribute(html, "data-mjx-hdw");
    } catch (...) {
        HtmlAdaptor& fallbackAdaptor = htmlAdaptor(adaptor);
        std::vector<XmlRef> children{html};
        html = fallbackAdaptor.node("span", PropertyList{}, std::move(children));
    }
    return setXML(std::move(html), adaptor);
}

std::string HtmlNode::getSerializedHTML() const {
    return htmlAdaptor(adaptor_).outerHTML(xml_);
}

std::u16string HtmlNode::textContent() const {
    return htmlAdaptor(adaptor_).textContent(xml_);
}

std::string HtmlNode::toString() const {
    const std::string htmlKind = htmlAdaptor(adaptor_).kind(xml_);
    return "HTML=<" + htmlKind + ">...</" + htmlKind + ">";
}

void HtmlNode::verifyTree(std::optional<PropertyList> options) {
    // P23f Root stub: HtmlNode::verifyTree (additional parent consumer).
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

} // namespace folia::math::mml
