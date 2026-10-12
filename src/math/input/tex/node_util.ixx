// MathJax 4.1.3 NodeUtil.ts: one utility service over the existing MML graph.
export module folia.math.input.tex.node_util;
export import folia.math.core.mml_tree.mml_node;
import std;
export namespace folia::math::tex {
class NodeUtil {
public:
    virtual ~NodeUtil()=default;
    static std::unordered_set<std::string>& attrs();
    static NodeUtil& instance();
    virtual std::u16string createEntity(std::u16string_view code) const;
    virtual mml::Children getChildren(mml::NodeValue node) const;
    virtual std::u16string getText(mml::NodeValue node) const;
    virtual void appendChildren(mml::NodeValue node,mml::Children children);
    virtual void setAttribute(mml::NodeValue node,std::string_view attribute,Property value);
    virtual void setProperty(mml::NodeValue node,std::string property,Property value);
    virtual void setProperties(mml::NodeValue node,Property properties);
    virtual Property getProperty(mml::NodeValue node,std::string_view property) const;
    virtual Property getAttribute(mml::NodeValue node,std::string_view attribute) const;
    virtual void removeAttribute(mml::NodeValue node,std::string_view attribute);
    virtual void removeProperties(mml::NodeValue node,std::span<const std::string> properties);
    // Enabled packages use only integral indices; preserve the accepted seam.
    virtual mml::NodeValue getChildAt(mml::NodeValue node,std::size_t position) const;
    virtual void setChild(mml::NodeValue node,std::size_t position,mml::NodeValue child);
    virtual void copyChildren(mml::NodeValue oldNode,mml::NodeValue newNode);
    virtual void copyAttributes(mml::NodeValue oldNode,mml::NodeValue newNode);
    virtual bool isType(mml::NodeValue node,std::string_view kind) const;
    virtual bool isEmbellished(mml::NodeValue node) const;
    virtual Property getTexClass(mml::NodeValue node) const;
    virtual mml::NodeValue getCoreMO(mml::NodeValue node) const;
    // Canonical heterogeneous parser arguments carry NodeValue or MmlNode*.
    virtual bool isNode(const std::any& value) const;
    virtual bool isInferred(mml::NodeValue node) const;
    virtual Property getForm(mml::NodeValue node) const;
    virtual Property getOp(mml::NodeValue node,Property form=Missing{}) const;
    virtual Property getMoAttribute(mml::NodeValue node,std::string_view attribute) const;
};
}
