// MathJax 4.1.3 input/tex/NodeUtil.ts; typed MML property application.
export module folia.math.input.tex.node_util;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::tex {
struct NodeDefinition {
    mml::AttributeSet attributes;
    mml::NodeProperties properties;
    NodeDefinition(mml::AttributeSet value = {}) : attributes(std::move(value)) {}
};
class NodeUtil final {
public:
    static void setProperties(mml::MmlNode&, NodeDefinition);
};
}
