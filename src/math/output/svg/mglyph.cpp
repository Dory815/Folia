/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/mglyph.ts.
 *************************************************************/
module folia.math.output.svg.mglyph;
import folia.math.util.utf16;
import std;

namespace folia::math::output::svg {
namespace {
std::string attributeText(mml::AttributeSet const &attributes, std::u16string_view name) {
    const auto *value = attributes.lookup(name);
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr)
        return utf16::encode(*text);
    return {};
}
} // namespace

common::SceneNodes SvgMglyph::emitScene(scene::Builder &builder,
                                        std::span<scene::NodeId const> parents) {
    auto containers = standardSceneNodes(builder, parents);
    if (auto *character = charWrapper()) {
        (void)character->emitScene(builder, containers.content);
        return containers.nodes;
    }

    if (containers.content.empty())
        throw std::logic_error("SvgMglyph image branch requires a Source content root");

    const auto &attributes = node->attributes();
    const double sourceValign = std::isnan(valign) || valign == 0 ? 0 : valign;
    const double y = height + sourceValign;
    scene::Transform transform(std::vector<scene::TransformOperation>{
        scene::Translation{0, y, scene::TranslationSeparator::space},
        scene::Affine{1, 0, 0, -1, 0, 0},
    });
    builder.add_primitive(containers.content.front(),
                          scene::Image{{width, height},
                                       attributeText(attributes, u"src"),
                                       attributeText(attributes, u"alt"),
                                       std::move(transform)});
    return containers.nodes;
}

common::CommonWrapperRegistration mglyphRegistration() {
    using Args = common::WrapperBuildArguments;
    tree::FactoryClass<common::CommonWrapper, Args> creator;
    creator.construct = [](Args const &args) -> std::unique_ptr<common::CommonWrapper> {
        return std::make_unique<SvgMglyph>(args.factory, args.node, args.parent);
    };
    creator.isInstance = [](common::CommonWrapper const &product) {
        return dynamic_cast<SvgMglyph const *>(&product) != nullptr;
    };
    return {mml::NodeKind::mglyph, std::move(creator),
            common::CommonWrapperClassData::commonDefaults()};
}
} // namespace folia::math::output::svg
