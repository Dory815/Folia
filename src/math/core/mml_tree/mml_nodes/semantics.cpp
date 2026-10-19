module folia.math.core.mml_tree.mml_nodes.semantics;
import folia.math.core.mml_tree.node_defaults;
import std;

namespace folia::math::mml {
MmlSemantics::MmlSemantics(MmlDocument &document)
    : AbstractMmlBaseNode(document, NodeKind::semantics) {}

AttributeSet MmlSemantics::defaults() {
    return nodeDefaultAttributes(NodeKind::semantics);
}

NodeKind MmlSemantics::kind() const noexcept {
    return NodeKind::semantics;
}

double MmlSemantics::arity() const {
    return 1.0;
}

bool MmlSemantics::notParent() const {
    return true;
}

MmlAnnotationXML::MmlAnnotationXML(MmlDocument &document)
    : MmlAnnotationXML(document, NodeKind::annotation_xml) {}

MmlAnnotationXML::MmlAnnotationXML(MmlDocument &document, NodeKind kind)
    : AbstractMmlNode(document, kind) {}

AttributeSet MmlAnnotationXML::defaults() {
    return nodeDefaultAttributes(NodeKind::annotation_xml);
}

NodeKind MmlAnnotationXML::kind() const noexcept {
    return NodeKind::annotation_xml;
}

void MmlAnnotationXML::setChildInheritedAttributes(AttributeList const &, bool, double, bool) {
    // MathJax annotation-xml is an opaque payload boundary.
}

MmlAnnotation::MmlAnnotation(MmlDocument &document)
    : MmlAnnotationXML(document, NodeKind::annotation) {}

AttributeSet MmlAnnotation::defaults() {
    return nodeDefaultAttributes(NodeKind::annotation);
}

NodeKind MmlAnnotation::kind() const noexcept {
    return NodeKind::annotation;
}
} // namespace folia::math::mml
