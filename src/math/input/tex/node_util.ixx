// Selected NodeUtil.ts child-array services; one existing MML node graph.
export module folia.math.input.tex.node_util;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::tex {
class NodeUtil {
public:
    virtual ~NodeUtil()=default;
    virtual mml::NodeValue getChildAt(mml::NodeValue node,std::size_t position) const;
    virtual void setChild(mml::NodeValue node,std::size_t position,mml::NodeValue child);
    virtual void copyChildren(mml::NodeValue oldNode,mml::NodeValue newNode);
};
}
