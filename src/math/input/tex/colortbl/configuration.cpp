// MathJax 4.1.3 input/tex/colortbl/ColortblConfiguration.ts; Apache-2.0.
module folia.math.input.tex.colortbl.configuration;
import std;

namespace folia::math::tex {
void ColorArrayItem::EndEntry() {
    ArrayItem::EndEntry();
    const auto index = row.size() - 1;
    const auto column = index < color.col.size() && color.col[index]
                            ? std::u16string_view(*color.col[index])
                            : std::u16string_view{};
    const auto value = !color.cell.empty()  ? std::u16string_view(color.cell)
                       : !color.row.empty() ? std::u16string_view(color.row)
                                            : column;
    if (!value.empty()) {
        row.back()->attributes().set(u"mathbackground", std::u16string(value));
        color.cell.clear();
        hasColor = true;
    }
}
void ColorArrayItem::EndRow() {
    ArrayItem::EndRow();
    color.row.clear();
}
mml::MmlNode &ColorArrayItem::createMml() {
    auto &result = ArrayItem::createMml();
    auto child = [](mml::MmlNode &node, std::size_t index) -> mml::MmlNode & {
        if (index >= node.children().size() || !node.children()[index])
            throw std::out_of_range("Colortbl array wrapper has no table child");
        return *node.children()[index];
    };
    auto table = result.isKind(mml::NodeKind::mrow) ? &child(result, 1) : &result;
    if (table->isKind(mml::NodeKind::mstyle))
        table = &child(child(*table, 0), 0);
    if (table->isKind(mml::NodeKind::menclose))
        table = &child(child(*table, 0), 0);
    if (hasColor) {
        auto &attributes = table->attributes();
        auto frame = attributes.lookup(u"frame");
        if (frame && std::get_if<std::u16string>(frame) &&
            std::get<std::u16string>(*frame) == u"none" && !attributes.lookup(u"data-frame-styles"))
            attributes.set(u"data-frame-styles", u"");
    }
    return result;
}
DispatchOutcome TableColor(TexParser &parser, MacroInvocation const &invocation) {
    auto lookup = parser.configuration.packageData.colorModel();
    if (!lookup)
        throw std::logic_error("Colortbl requires the configured color model");
    auto const &name = invocation.dispatch_token;
    auto model = parser.GetBrackets(name, u"").value();
    auto color = lookup->getColor(model, parser.GetArgument(name).value());
    auto top = dynamic_cast<ColorArrayItem *>(parser.stack.Top());
    if (!top)
        throw TexError("UnsupportedTableColor", u"Unsupported use of %1",
                       std::array{parser.currentCS()});
    const auto args = std::get<std::span<TexArgument const>>(invocation.arguments);
    auto const &type = std::get<std::u16string>(args[0]);
    if (type == u"col") {
        const auto index = top->row.size();
        if (!top->table.empty() && (index >= top->color.col.size() || !top->color.col[index] ||
                                    *top->color.col[index] != color))
            throw TexError("ColumnColorNotTop", u"%1 must be in the top row or preamble",
                           std::array{std::u16string(name)});
        if (index >= top->color.col.size())
            top->color.col.resize(index + 1);
        top->color.col[index] = color;
        if (!parser.GetBrackets(name, u"")->empty())
            parser.GetBrackets(name, u"");
    } else {
        if (type == u"cell")
            top->color.cell = color;
        else if (type == u"row")
            top->color.row = color;
        else
            throw std::invalid_argument("Unknown Colortbl color target");
        if (type == u"row" && (top->Size() || !top->row.empty()))
            throw TexError("RowColorNotFirst", u"%1 must be at the beginning of a row",
                           std::array{std::u16string(name)});
    }
    return NoParseResult{};
}
void colortblConfig(ParserConfiguration &config, TeX &jax) {
    if (jax.session().colorModel())
        return;
    auto color = config.registry().get(u"color");
    if (!color || !color->config())
        throw std::logic_error("Colortbl requires a registered color configuration");
    color->config()(config, jax);
}
ConfigurationRef ColortblConfiguration::initializeModule(ConfigurationHandler &registry,
                                                         MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{
        MacroMapInit{{u"colortbl", {}},
                     {Macro(u"cellcolor", &TableColor, {std::u16string(u"cell")}),
                      Macro(u"rowcolor", &TableColor, {std::u16string(u"row")}),
                      Macro(u"columncolor", &TableColor, {std::u16string(u"col")})}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"colortbl"}}};
    definition.items = {
        {StackKind::array,
         StackItemClass{[](StackBuildArguments const &args) -> std::unique_ptr<BaseItem> {
                            return std::make_unique<ColorArrayItem>(args.factory);
                        },
                        [](BaseItem const &item) {
                            return dynamic_cast<ColorArrayItem const *>(&item) != nullptr;
                        }}}};
    definition.priority = 10;
    definition.config = ProcessorSpec<ConfigMethod>{&colortblConfig, 10};
    return Configuration::create(registry, u"colortbl", std::move(definition));
}
} // namespace folia::math::tex
