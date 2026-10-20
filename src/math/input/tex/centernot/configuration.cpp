// MathJax 4.1.3 input/tex/centernot/CenternotConfiguration.ts; Apache-2.0.
module folia.math.input.tex.centernot.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.node_util;
import std;

namespace folia::math::tex {
namespace {
mml::MmlNode &node(TexParser &parser, mml::NodeKind kind,
                   std::initializer_list<mml::MmlNode *> children, NodeDefinition definition = {}) {
    return parser.create(kind, {children.begin(), children.size()}, std::move(definition));
}
mml::MmlNode &firstChild(mml::MmlNode &node) {
    const auto children = node.children();
    if (children.empty() || !children.front())
        throw std::out_of_range("centerOver base has no class-bearing child");
    return *children.front();
}
} // namespace
DispatchOutcome CenterOver(TexParser &parser, MacroInvocation const &invocation) {
    const auto arg = u"{" + parser.GetArgument(invocation.dispatch_token).value() + u"}";
    auto over = parser.ParseArg(invocation.dispatch_token);
    auto base =
        TexParser(arg, ParserEnvironmentSeed{parser.stack.env()}, parser.configuration).mml();
    // Source parses the base again for the visible branch; the first copy
    // belongs to the phantom and to the class-copy postprocessor's node list.
    auto visible =
        TexParser(arg, ParserEnvironmentSeed{parser.stack.env()}, parser.configuration).mml();
    NodeDefinition centered;
    centered.attributes.set(u"width", 0.0);
    centered.attributes.set(u"lspace", u"-.5width");
    auto &paddedOver = node(parser, mml::NodeKind::mpadded, {over}, centered);
    auto &phantom = node(parser, mml::NodeKind::mphantom, {base});
    auto &overlay = node(parser, mml::NodeKind::mpadded, {&paddedOver, &phantom}, centered);
    auto &result = node(parser, mml::NodeKind::tex_atom, {visible, &overlay});
    parser.configuration.addNode(u"centerOver", *base);
    parser.Push(&result);
    return NoParseResult{};
}
void filterCenterOver(TexFilterData &data) {
    for (auto base : data.data.getList(u"centerOver")) {
        auto texClass = firstChild(firstChild(*base)).texClass();
        if (!texClass)
            continue;
        auto target = base;
        for (int level = 0; level < 6; ++level) {
            target = target->parent();
            if (!target)
                throw std::out_of_range("centerOver outer atom is absent");
        }
        NodeDefinition definition;
        definition.properties.tex.explicit_tex_class = *texClass;
        NodeUtil::setProperties(*target, std::move(definition));
    }
}
ConfigurationRef CenternotConfiguration::initializeModule(ConfigurationHandler &registry,
                                                          MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{
        MacroMapInit{{u"centernot", std::nullopt},
                     {Macro(u"centerOver", &CenterOver),
                      Macro(u"centernot", &BaseMethods::Macro,
                            {std::u16string(u"\\centerOver{#1}{{\u29F8}}"), 1.0})}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"centernot"}}};
    definition.postprocessors.emplace_back(FilterList::Callback{&filterCenterOver});
    return Configuration::create(registry, u"centernot", std::move(definition));
}
} // namespace folia::math::tex