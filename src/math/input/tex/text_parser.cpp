// MathJax 4.1.3 input/tex/textmacros/TextParser.ts; Apache-2.0.
module folia.math.input.tex.parser;
import :text_parser;
import folia.math.input.tex.parse_util;
import std;

namespace folia::math::tex {
namespace {
bool textSizeTruthy(std::variant<double, std::u16string> const &size) {
    return std::visit(
        [](auto const &value) {
            if constexpr (std::same_as<std::decay_t<decltype(value)>, double>)
                return value != 0 && !std::isnan(value);
            else
                return !value.empty();
        },
        size);
}
void setTextSize(mml::MmlNode &node, std::variant<double, std::u16string> const &size) {
    std::visit([&](auto const &value) { node.attributes().set(u"mathsize", value); }, size);
}
} // namespace
TextParser::TextParser(std::u16string source, ParserEnvironmentSeed environment,
                       ParseOptions &options, std::optional<ScriptLevel> level)
    : TexParser(std::move(source), std::move(environment), options, DeferParsing{}) {
    initializeTexParser();
    level_ = std::move(level);
}
TexParser &TextParser::texParser() const {
    if (!configuration.textmacros || !configuration.textmacros->texParser)
        throw std::logic_error("TextParser requires an active TeX parser");
    return *configuration.textmacros->texParser;
}
Tags &TextParser::tags() const {
    return texParser().tags();
}
mml::MmlNode *TextParser::mml() {
    copyLists();
    finishParserScope();
    if (level_) {
        NodeDefinition definition;
        definition.attributes.set(u"displaystyle", false);
        std::visit([&](auto const &value) { definition.attributes.set(u"scriptlevel", value); },
                   *level_);
        return &create(mml::NodeKind::mstyle, nodes_, std::move(definition));
    }
    return nodes_.size() == 1 ? nodes_[0] : &create(mml::NodeKind::mrow, nodes_);
}
void TextParser::copyLists() {
    auto &target = texParser().configuration;
    for (auto const &[name, list] : configuration.nodeLists)
        for (auto *node : list)
            target.addNode(name, *node);
    configuration.nodeLists.clear();
}
void TextParser::Parse() {
    text_.clear();
    nodes_.clear();
    envStack.clear();
    TexParser::Parse();
}
void TextParser::saveText() {
    if (text_.empty())
        return;
    NodeDefinition definition;
    auto const &variant = stack.env().math_variant;
    if (variant && !variant->empty())
        definition.attributes.set(u"mathvariant", *variant);
    auto &node = parse_util::internalText(*this, text_, std::move(definition));
    text_.clear();
    Push(&node);
}
void TextParser::Push(StackEntry entry) {
    if (!text_.empty())
        saveText();
    if (auto frame = std::get_if<FrameHandle>(&entry); frame && *frame) {
        if (dynamic_cast<StopItem *>((*frame).get())) {
            TexParser::Push(std::move(entry));
        } else if (dynamic_cast<StyleItem *>((*frame).get())) {
            stack.env().math_color = stack.env().color;
        }
        return;
    }
    if (auto node = std::get_if<mml::MmlNode *>(&entry);
        node && *node && dynamic_cast<mml::AbstractMmlNode *>(*node)) {
        addAttributes(**node);
        nodes_.push_back(*node);
    }
}
void TextParser::PushMath(mml::MmlNode &math) {
    auto *node = &math;
    auto const &environment = stack.env();
    auto wrap = [&] {
        if (!node->isToken() && node->kind() != mml::NodeKind::mstyle) {
            const std::array children{node};
            node = &create(mml::NodeKind::mstyle, children);
        }
    };
    if (environment.math_size && textSizeTruthy(*environment.math_size) &&
        !node->attributes().own(u"mathsize")) {
        wrap();
        setTextSize(*node, *environment.math_size);
    }
    if (environment.math_color && !environment.math_color->empty() &&
        !node->attributes().own(u"mathcolor")) {
        wrap();
        node->attributes().set(u"mathcolor", *environment.math_color);
    }
    if (node->isInferred()) {
        const std::vector<mml::MmlNode *> children(node->children().begin(),
                                                   node->children().end());
        node = &create(mml::NodeKind::mrow, children);
    }
    if (node->kind() != mml::NodeKind::tex_atom) {
        const std::array children{node};
        node = &create(mml::NodeKind::tex_atom, children);
    }
    nodes_.push_back(node);
}
void TextParser::addAttributes(mml::MmlNode &node) {
    if (!node.isToken())
        return;
    auto const &environment = stack.env();
    if (environment.math_size && textSizeTruthy(*environment.math_size) &&
        !node.attributes().own(u"mathsize"))
        setTextSize(node, *environment.math_size);
    if (environment.math_color && !environment.math_color->empty() &&
        !node.attributes().own(u"mathcolor"))
        node.attributes().set(u"mathcolor", *environment.math_color);
    if (environment.math_variant && !environment.math_variant->empty() &&
        !node.attributes().own(u"mathvariant"))
        node.attributes().set(u"mathvariant", *environment.math_variant);
}
mml::MmlNode *TextParser::ParseTextArg(std::u16string_view name, TexEnvironment environment) {
    auto source = GetArgument(name).value();
    auto merged = stack.env();
    overlayEnvironment(merged, environment);
    return TextParser(std::move(source), ParserEnvironmentSeed{std::move(merged)}, configuration)
        .mml();
}
mml::MmlNode *TextParser::ParseArg(std::u16string_view name) {
    return TextParser(GetArgument(name).value(), ParserEnvironmentSeed{stack.env()}, configuration)
        .mml();
}
[[noreturn]] void TextParser::Error(std::string id, std::u16string message,
                                    std::span<std::u16string const> arguments) {
    throw TexError(std::move(id), std::move(message), arguments);
}
} // namespace folia::math::tex
