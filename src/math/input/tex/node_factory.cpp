// MathJax 4.1.3 NodeFactory.ts semantic methods; Apache-2.0.
module folia.math.input.tex.node_factory;
import folia.math.input.tex.node_factory_host;
import folia.math.input.tex.node_util;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
struct CreatorSlot {
    std::string name;
    options::Key key;
};

std::vector<CreatorSlot> enumerateCreatorSlots(options::Context& context, options::Value source) {
    std::vector<CreatorSlot> result;
    if (std::holds_alternative<options::Null>(source.data) ||
        std::holds_alternative<options::Undefined>(source.data)) return result;

    std::unordered_set<std::u16string> seen;
    options::Value current = source;
    while (true) {
        for (const auto& key : context.ownKeys(current)) {
            const auto* name = std::get_if<std::u16string>(&key.data);
            if (!name || !seen.insert(*name).second) continue;
            const auto property = context.descriptor(current, key);
            if (property && property->enumerable) {
                result.push_back({property_host::debugText(*name), key});
            }
        }

        std::optional<options::ObjectRef> prototype;
        if (const auto* object = std::get_if<options::ObjectRef>(&current.data)) {
            prototype = context.prototype(*object);
        } else if (std::holds_alternative<std::u16string>(current.data)) {
            prototype = context.intrinsics().stringPrototype;
        } else if (std::holds_alternative<double>(current.data)) {
            prototype = context.intrinsics().numberPrototype;
        } else if (std::holds_alternative<bool>(current.data)) {
            prototype = context.intrinsics().booleanPrototype;
        } else if (std::holds_alternative<options::SymbolRef>(current.data)) {
            prototype = context.intrinsics().symbolPrototype;
        }
        if (!prototype) break;
        current = options::Value(*prototype);
    }
    return result;
}
}

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
void NodeFactory::installCreators(
    std::span<const std::string> names,
    const std::function<std::optional<Property>(std::string_view)>& readCreator) {
    for (const std::string& kind : names) {
        if (const auto creator = readCreator(kind)) this->set(kind, *creator);
    }
}
void NodeFactory::setCreators(Property maps) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    const std::vector<std::string> names = node_factory_host::creatorKeys(maps);
    installCreators(names, [&maps](std::string_view kind) -> std::optional<Property> {
        return mml::host::get(maps, kind);
    });
}
void NodeFactory::setCreators(options::Context& context,OptionFunctionBindings& bindings,options::Value maps) {
    tree::TreeCallGuard<mml::MmlNode> guard;
    if (&context != &bindings.context()) {
        throw options::Exception("TypeError",u"NodeFactory creator record and callable bindings belong to different Contexts");
    }
    const std::vector<CreatorSlot> slots = enumerateCreatorSlots(context, maps);
    std::vector<std::string> names;
    names.reserve(slots.size());
    for (const auto& slot : slots) names.push_back(slot.name);
    installCreators(names, [&context,&bindings,maps,&slots](std::string_view name) -> std::optional<Property> {
        const auto slot = std::find_if(slots.begin(), slots.end(), [name](const CreatorSlot& candidate) {
            return candidate.name == name;
        });
        if (slot == slots.end() || !context.has(maps, slot->key)) return std::nullopt;
        const auto value = context.get(maps, slot->key);
        return functions::toProperty(bindings.functionValue(value));
    });
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
