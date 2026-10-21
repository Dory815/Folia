// MathJax MmlNode.ts Empty/Text/XML semantic branch (4.1.3), Apache-2.0.
module folia.math.core.mml_tree.mml_document;
import folia.math.util.utf16;
import std;

namespace folia::math::mml {
AbstractMmlEmptyNode::AbstractMmlEmptyNode(MmlDocument &d, NodeKind k)
    : tree::AbstractEmptyNode<MmlNode, NodeKind, MmlNode>(d, k) {}

void AbstractMmlEmptyNode::inheritAttributes(AttributeList const &, bool, double, bool) {}

void AbstractMmlEmptyNode::initializeMml(AttributeSet, std::span<MmlNode *const>) {}

bool AbstractMmlEmptyNode::isToken() const {
    return false;
}

bool AbstractMmlEmptyNode::isEmbellished() const {
    return false;
}

bool AbstractMmlEmptyNode::isSpacelike() const {
    return false;
}

bool AbstractMmlEmptyNode::linebreakContainer() const {
    return false;
}

std::string AbstractMmlEmptyNode::linebreakAlign() const {
    return {};
}

bool AbstractMmlEmptyNode::isEmpty() const {
    return true;
}

double AbstractMmlEmptyNode::arity() const {
    return 0;
}

bool AbstractMmlEmptyNode::isInferred() const {
    return false;
}

bool AbstractMmlEmptyNode::notParent() const {
    return false;
}

MmlNode *AbstractMmlEmptyNode::Parent() const noexcept {
    return parent();
}

std::optional<TexClass> AbstractMmlEmptyNode::texClass() {
    return TexClass::none;
}

void AbstractMmlEmptyNode::setTexClass(std::optional<TexClass>) {
    throw std::logic_error("read-only texClass");
}

std::optional<TexClass> AbstractMmlEmptyNode::prevClass() const noexcept {
    return TexClass::none;
}

void AbstractMmlEmptyNode::setPrevClass(std::optional<TexClass>) {
    throw std::logic_error("read-only prevClass");
}

std::optional<double> AbstractMmlEmptyNode::prevLevel() const noexcept {
    return 0.0;
}

void AbstractMmlEmptyNode::setPrevLevel(std::optional<double>) {
    throw std::logic_error("read-only prevLevel");
}

MmlNode *AbstractMmlEmptyNode::core() {
    return this;
}

MmlNode *AbstractMmlEmptyNode::coreMO() {
    return this;
}

std::size_t AbstractMmlEmptyNode::coreIndex() {
    return 0;
}

std::optional<std::size_t> AbstractMmlEmptyNode::childPosition() {
    return 0;
}

MmlNode *AbstractMmlEmptyNode::setTeXclass(MmlNode *previous) {
    return previous;
}

std::string AbstractMmlEmptyNode::texSpacing() {
    return {};
}

bool AbstractMmlEmptyNode::hasSpacingAttributes() {
    return false;
}

void AbstractMmlEmptyNode::setInheritedAttributes(AttributeList const &, bool, double, bool) {}

void AbstractMmlEmptyNode::inheritAttributesFrom(MmlNode &) {}

void AbstractMmlEmptyNode::verifyTree(std::optional<VerifyOptions> const &) {}

MmlNode *AbstractMmlEmptyNode::mError(std::u16string, VerifyOptions const &, bool) {
    return nullptr;
}

MmlNode &AbstractMmlEmptyNode::copy(bool) const {
    auto &result = document().make(kind());
    result.properties() = properties();
    return result;
}

TextNode::TextNode(MmlDocument &d) : AbstractMmlEmptyNode(d, NodeKind::text) {}

NodeKind TextNode::kind() const noexcept {
    return NodeKind::text;
}

std::u16string TextNode::getText() const {
    return std::u16string(text());
}

TextNode &TextNode::setText(std::u16string value) {
    MmlNode::setText(std::move(value));
    return *this;
}

MmlNode &TextNode::copy(bool) const {
    auto &raw = document().make(kind());
    auto *result = dynamic_cast<TextNode *>(&raw);
    if (!result)
        throw std::logic_error("text factory kind mismatch");
    return result->setText(getText());
}

std::string TextNode::toString() const {
    return folia::math::utf16::encode(text());
}

XMLNode::XMLNode(MmlDocument &d) : AbstractMmlEmptyNode(d, NodeKind::xml) {}

NodeKind XMLNode::kind() const noexcept {
    return NodeKind::xml;
}

XmlRef XMLNode::getXML() const {
    return xml_;
}

std::shared_ptr<XmlAdaptor const> XMLNode::xmlAdaptor() const noexcept {
    return adaptor_;
}

XMLNode *XMLNode::setXML(XmlRef value, std::shared_ptr<XmlAdaptor const> adaptor) {
    xml_ = std::move(value);
    adaptor_ = std::move(adaptor);
    return this;
}

std::string XMLNode::getSerializedXML() const {
    if (!adaptor_)
        throw std::logic_error("XML adaptor is absent");
    return adaptor_->serializeXML(xml_);
}

MmlNode &XMLNode::copy(bool) const {
    auto &raw = document().make(kind());
    auto *result = dynamic_cast<XMLNode *>(&raw);
    if (!result)
        throw std::logic_error("XML factory kind mismatch");
    if (!adaptor_)
        throw std::logic_error("XML clone adaptor is absent");
    auto cloned = adaptor_->clone(xml_);
    return *result->setXML(std::move(cloned));
}

void XMLNode::copyNativeStateTo(MmlNode &destination, MmlClone &) const {
    auto *xml = dynamic_cast<XMLNode *>(&destination);
    if (!xml)
        throw std::logic_error("XML exact-clone destination type mismatch");
    xml->xml_ = xml_;
    xml->adaptor_ = adaptor_;
}

std::string XMLNode::toString() const {
    return "XML data";
}
} // namespace folia::math::mml
