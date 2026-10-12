// MathJax 4.1.3 NodeFactory.ts semantic methods; Apache-2.0.
module folia.math.input.tex.node_factory;
import folia.math.input.tex.node_factory_host;
import folia.math.input.tex.node_util;
import std;
namespace folia::math::tex {
mml::NodeValue NodeFactory::createNode(NodeFactory& factory,Property kind,
                                       mml::Children children,Property definition,
                                       mml::NodeValue text) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    if (definition.missing()) definition = PropertyList{};
    mml::MmlNode* const rawNode = factory.requireMmlFactory().create(node_factory_host::kind(kind), {});
    const mml::NodeValue node(rawNode);
    node.requireNode("setChildren")->setChildren(std::move(children));
    if (text.isNode()) node.requireNode("appendChild")->appendChild(text);
    NodeUtil::instance().setProperties(node, std::move(definition));
    return node;
}
mml::NodeValue NodeFactory::createToken(NodeFactory& factory,Property kind,
                                       Property definition,Property text) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    if (definition.missing()) definition = PropertyList{};
    if (text.missing()) text = std::string{};

    const std::array<std::any, 1> textArguments{std::any(text)};
    const mml::NodeValue textNode = factory.create("text", textArguments);
    const mml::Children children = std::make_shared<tree::ChildArray<mml::MmlNode>>();
    const std::array<std::any, 4> nodeArguments{
        std::any(kind), std::any(children), std::any(definition), std::any(textNode)};
    return factory.create("node", nodeArguments);
}
mml::NodeValue NodeFactory::createText(NodeFactory& factory,Property text) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    if (mml::host::nullish(text)) return PropertyNull{};
    mml::MmlNode* const rawNode = factory.requireMmlFactory().create("text", {});
    return node_factory_host::setText(mml::NodeValue(rawNode), node_factory_host::text(text));
}
mml::NodeValue NodeFactory::createError(NodeFactory& factory,Property message) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    const std::array<std::any, 1> textArguments{std::any(message)};
    const mml::NodeValue textNode = factory.create("text", textArguments);

    const mml::Children noChildren = std::make_shared<tree::ChildArray<mml::MmlNode>>();
    const Property emptyDefinition = PropertyList{};
    const std::array<std::any, 4> mtextArguments{
        std::any(Property("mtext")), std::any(noChildren),
        std::any(emptyDefinition), std::any(textNode)};
    const mml::NodeValue mtext = factory.create("node", mtextArguments);

    const mml::Children errorChildren = std::make_shared<tree::ChildArray<mml::MmlNode>>();
    errorChildren->push_back(mtext);
    const Property errorDefinition = PropertyList{{"data-mjx-error", message}};
    const std::array<std::any, 3> errorArguments{
        std::any(Property("merror")), std::any(errorChildren), std::any(errorDefinition)};
    return factory.create("node", errorArguments);
}
void NodeFactory::setMmlFactory(MmlFactoryReference value) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    mmlFactory = std::move(value);
}
void NodeFactory::set(std::string_view kind,Property creator) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    factory_.set(kind, std::move(creator));
}
void NodeFactory::setCreators(Property maps) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    for (const std::string& kind : node_factory_host::creatorKeys(maps)) {
        const Property creator = mml::host::get(maps, kind);
        this->set(kind, creator);
    }
}
mml::NodeValue NodeFactory::create(std::string_view kind,FunctionArgs rest) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    Property creator = factory_.get(kind);
    if (!mml::host::truthy(creator)) creator = factory_.get("node");
    const mml::NodeValue node = node_factory_host::invoke(creator, identity(), rest);
    if (kind == "node") {
        const Property requestedKind = node_factory_host::scalar(rest, 0);
        requireConfiguration().addNode(requestedKind, node);
    }
    return node;
}
Property NodeFactory::get(std::string_view kind) const {
    tree::TreeCallGuard<mml::MmlNode> guard;
    return factory_.get(kind);
}
}
