// MathJax MmlNode.ts Empty/Text/XML semantic branch (4.1.3).
// Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
module folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.mml_node_host;
import std;

namespace folia::math::mml {

bool AbstractMmlEmptyNode::isToken() const { return false; }
bool AbstractMmlEmptyNode::isEmbellished() const { return false; }
bool AbstractMmlEmptyNode::isSpacelike() const { return false; }
bool AbstractMmlEmptyNode::linebreakContainer() const { return false; }
std::string AbstractMmlEmptyNode::linebreakAlign() const { return {}; }
bool AbstractMmlEmptyNode::isEmpty() const { return true; }
double AbstractMmlEmptyNode::arity() const { return 0; }
bool AbstractMmlEmptyNode::isInferred() const { return false; }
Property AbstractMmlEmptyNode::notParent() const { return false; }
NodeValue AbstractMmlEmptyNode::Parent() const { return getParent(); }

Property AbstractMmlEmptyNode::texClass() { return TEXCLASS.NONE; }
void AbstractMmlEmptyNode::setTexClass(Property) {
    host::typeError("Cannot set read only texClass");
}
Property AbstractMmlEmptyNode::prevClass() { return TEXCLASS.NONE; }
void AbstractMmlEmptyNode::setPrevClass(Property) {
    host::typeError("Cannot set read only prevClass");
}
Property AbstractMmlEmptyNode::prevLevel() const { return 0.0; }
void AbstractMmlEmptyNode::setPrevLevel(Property) {
    host::typeError("Cannot set read only prevLevel");
}
std::shared_ptr<Attributes> AbstractMmlEmptyNode::attributes() const { return nullptr; }
void AbstractMmlEmptyNode::setAttributes(std::shared_ptr<Attributes>) {
    host::typeError("Cannot set read only attributes");
}

NodeValue AbstractMmlEmptyNode::core() { return this; }
NodeValue AbstractMmlEmptyNode::coreMO() { return this; }
std::size_t AbstractMmlEmptyNode::coreIndex() { return 0; }
std::optional<std::size_t> AbstractMmlEmptyNode::childPosition() { return 0; }
NodeValue AbstractMmlEmptyNode::setTeXclass(NodeValue previous) { return previous; }
Property AbstractMmlEmptyNode::texSpacing() { return ""; }
bool AbstractMmlEmptyNode::hasSpacingAttributes() { return false; }
void AbstractMmlEmptyNode::inheritAttributes(AttributeList, Property, Property, Property) {}
void AbstractMmlEmptyNode::inheritAttributesFrom(NodeValue) {
    // MathJax AbstractMmlEmptyNode deliberately ignores this operation.
}
void AbstractMmlEmptyNode::verifyTree(Property) {}
MmlNode* AbstractMmlEmptyNode::mError(std::string, Property, bool) { return nullptr; }

std::string TextNode::kind() const { return "text"; }
std::u16string TextNode::getText() const { return text_; }
TextNode* TextNode::setText(std::u16string text) {
    text_ = std::move(text);
    return this;
}
MmlNode* TextNode::copy(bool keepIds) {
    tree::TreeCallGuard<MmlNode> guard;
    (void)keepIds;

    // Preserve the source order: create by current kind, then call the source
    // virtual getter and the result's virtual setter.
    MmlNode* const created = factory.create(kind(), {});
    std::u16string const text = getText();
    auto* const result = dynamic_cast<TextNode*>(created);
    if (!result) host::typeError("Factory product has no TextNode setText method");
    return result->setText(text);
}
std::string TextNode::toString() const { return host::debugText(text_); }

std::string XMLNode::kind() const { return "XML"; }
XmlRef XMLNode::getXML() const { return xml_; }
XMLNode* XMLNode::setXML(XmlRef xml, XmlAdaptor* adaptor) {
    xml_ = std::move(xml);
    adaptor_ = adaptor;
    return this;
}
std::string XMLNode::getSerializedXML() const {
    if (!adaptor_) host::typeError("Cannot read properties of null (reading 'serializeXML')");
    return adaptor_->serializeXML(xml_);
}
MmlNode* XMLNode::copy(bool keepIds) {
    tree::TreeCallGuard<MmlNode> guard;
    (void)keepIds;

    // The factory result is allocated first. Source adaptor.clone runs next;
    // setXML receives only the clone, so the result's adaptor remains null.
    MmlNode* const created = factory.create(kind(), {});
    if (!adaptor_) host::typeError("Cannot read properties of null (reading 'clone')");
    XmlRef const clone = adaptor_->clone(xml_);
    auto* const result = dynamic_cast<XMLNode*>(created);
    if (!result) host::typeError("Factory product has no XMLNode setXML method");
    return result->setXML(clone);
}
std::string XMLNode::toString() const { return "XML data"; }

} // namespace folia::math::mml
