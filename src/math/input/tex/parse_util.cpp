// Native implementation of the three request-local ParseUtil operations.
// Source: MathJax 4.1.3 ts/input/tex/ParseUtil.ts (Apache-2.0).
module folia.math.input.tex.parse_util;
import std;
import folia.math.input.tex.tex_error;
import folia.math.util.unicode_normalization;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.input.tex.unit_util;

namespace folia::math::tex::parse_util {
namespace {
std::u16string compressSpaces(std::u16string_view text) {
    std::u16string result;
    bool previousSpace{};
    for (auto c : text) {
        if (c != u' ' || !previousSpace)
            result += c;
        previousSpace = c == u' ';
    }
    return result;
}
bool booleanAttribute(mml::AttributeValue const &value) {
    auto boolean = std::get_if<bool>(&value);
    return boolean && *boolean;
}
std::u16string codePoint(std::u16string_view digits) {
    char32_t value{};
    for (auto c : digits) {
        value = value * 16 + (c <= u'9' ? c - u'0' : c - u'A' + 10);
        if (value > 0x10ffff)
            throw std::out_of_range("Invalid Unicode code point");
    }
    if (value <= 0xffff)
        return std::u16string(1, static_cast<char16_t>(value));
    value -= 0x10000;
    return {static_cast<char16_t>(0xd800 + (value >> 10)),
            static_cast<char16_t>(0xdc00 + (value & 0x3ff))};
}
} // namespace
std::vector<mml::MmlNode *> internalMath(TexParser &parser, std::u16string_view source,
                                         std::optional<ScriptLevel> level,
                                         std::optional<std::u16string_view> font) {
    auto text = compressSpaces(source);
    if (auto callback = parser.options().internalMath; callback && *callback)
        return (*callback)(parser, text, level, font);
    NodeDefinition definition;
    auto selectedFont =
        font && !font->empty() ? std::u16string(*font) : parser.stack.env().font.value_or(u"");
    if (!selectedFont.empty())
        definition.attributes.set(u"mathvariant", selectedFont);
    std::vector<mml::MmlNode *> nodes;
    std::size_t i{}, k{}, braces{};
    char16_t match{};
    auto textSegment = [&](std::size_t end) {
        if (k < end)
            nodes.push_back(
                &internalText(parser, std::u16string_view(text).substr(k, end - k), definition));
    };
    auto mathSegment = [&](std::size_t end, NodeDefinition attributes = {}) {
        auto *parsed =
            TexParser(text.substr(k, end - k), ParserEnvironmentSeed{}, parser.configuration).mml();
        if (!parsed)
            throw std::logic_error("Internal math parser returned no MML");
        std::array children{parsed};
        nodes.push_back(&parser.create(mml::NodeKind::tex_atom, children, std::move(attributes)));
    };
    if (text.find_first_of(u"${}\\") != std::u16string::npos) {
        regexp_host::RegExp reference(u"^(eq)?ref\\s*\\{");
        regexp_host::RegExp unicode(u"^\\s*(?:([0-9A-F])|\\{\\s*([0-9A-F]+)\\s*\\})");
        while (i < text.size()) {
            auto c = text[i++];
            if (c == u'$') {
                if (match == u'$' && braces == 0) {
                    mathSegment(i - 1);
                    match = 0;
                    k = i;
                } else if (!match) {
                    textSegment(i - 1);
                    match = u'$';
                    k = i;
                }
            } else if (c == u'{' && match)
                ++braces;
            else if (c == u'}') {
                if (match == u'}' && braces == 0) {
                    mathSegment(i, definition);
                    match = 0;
                    k = i;
                } else if (match && braces)
                    --braces;
            } else if (c == u'\\') {
                auto ref = !match ? reference.exec(text.substr(i)) : std::nullopt;
                if (ref) {
                    textSegment(i - 1);
                    match = u'}';
                    k = i - 1;
                    i += ref->at(0)->size();
                } else {
                    c = i < text.size() ? text[i] : 0;
                    ++i;
                    if (c == u'(' && !match) {
                        textSegment(i - 2);
                        match = u')';
                        k = i;
                    } else if (c == u')' && match == u')' && !braces) {
                        mathSegment(i - 2);
                        match = 0;
                        k = i;
                    } else if (!match && (c == u'$' || c == u'{' || c == u'}' || c == u'\\')) {
                        --i;
                        text.erase(i - 1, 1);
                    } else if (c == u'U') {
                        auto arg = unicode.exec(text.substr(i));
                        if (!arg) {
                            const std::array<std::u16string, 1> name{u"\\U"};
                            throw TexError(
                                "BadRawUnicode",
                                u"Argument to %1 must a hexadecimal number with 1 to 6 digits",
                                name);
                        }
                        auto encoded = codePoint(arg->at(1).value_or(arg->at(2).value_or(u"")));
                        text.replace(i - 2, 2 + arg->at(0)->size(), encoded);
                        i = i - 2 + encoded.size();
                    }
                }
            }
        }
        if (match)
            throw TexError("MathNotTerminated", u"Math mode is not properly terminated");
    }
    textSegment(text.size());
    if (level) {
        NodeDefinition style;
        style.attributes.set(u"displaystyle", false);
        std::visit([&](auto const &value) { style.attributes.set(u"scriptlevel", value); }, *level);
        nodes = {&parser.create(mml::NodeKind::mstyle, nodes, std::move(style))};
    } else if (nodes.size() > 1)
        nodes = {&parser.create(mml::NodeKind::mrow, nodes)};
    return nodes;
}
mml::MmlNode &internalText(TexParser &parser, std::u16string_view source,
                           NodeDefinition definition) {
    std::u16string text;
    bool newline{};
    for (auto c : source) {
        if (c != u'\n' || !newline)
            text += c == u'\n' ? u' ' : c;
        newline = c == u'\n';
    }
    auto start = text.find_first_not_of(u' ');
    if (!text.empty() && start != 0)
        text.replace(0, start == std::u16string::npos ? text.size() : start, 1, u'\u00a0');
    if (!text.empty() && text.back() == u' ') {
        auto end = text.find_last_not_of(u' ');
        text.replace(end + 1, text.size() - end - 1, 1, u'\u00a0');
    }
    auto &content = parser.createText(text);
    return parser.create(mml::NodeKind::mtext, {}, std::move(definition), &content);
}
void checkMovableLimits(mml::MmlNode &base) {
    auto symbol = base.kind() == mml::NodeKind::mo ? NodeUtil::getForm(base) : nullptr;
    bool movable = base.properties().tex.movable_limits.value_or(false);
    if (symbol)
        for (auto const &[key, value] : symbol->properties)
            if (key == u"movablelimits")
                movable = movable || booleanAttribute(value);
    if (movable) {
        NodeDefinition definition;
        definition.properties.tex.movable_limits = false;
        NodeUtil::setProperties(base, std::move(definition));
    }
}
mml::MmlNode &underOver(TexParser &parser, mml::MmlNode &original, mml::MmlNode &script,
                        std::u16string_view position, bool stack) {
    checkMovableLimits(original);
    auto base = &original;
    if (base->isKind(mml::NodeKind::munderover) && base->isEmbellished()) {
        auto core = base->coreMO();
        if (!core)
            throw std::logic_error("Embellished operator has no core");
        core->attributes().set(u"lspace", 0.0);
        core->attributes().set(u"rspace", 0.0);
        NodeDefinition spacing;
        spacing.attributes.set(u"rspace", 0.0);
        auto blank = &parser.create(mml::NodeKind::mo, {}, std::move(spacing));
        std::array children{blank, base};
        base = &parser.create(mml::NodeKind::mrow, children);
    }
    std::array children{base};
    auto &result =
        dynamic_cast<mml::MmlMunderover &>(parser.create(mml::NodeKind::munderover, children));
    NodeUtil::setChild(result, position == u"over" ? result.over() : result.under(), script);
    mml::MmlNode *node = &result;
    if (stack) {
        NodeDefinition style;
        style.attributes.set(u"displaystyle", true);
        style.attributes.set(u"scriptlevel", 0.0);
        std::array content{node};
        auto &mstyle = parser.create(mml::NodeKind::mstyle, content, std::move(style));
        std::array styled{&mstyle};
        NodeDefinition definition;
        definition.properties.tex.explicit_tex_class = mml::TexClass::op;
        definition.properties.tex.move_sup_sub = true;
        node = &parser.create(mml::NodeKind::tex_atom, styled, std::move(definition));
    }
    node->properties().tex.sub_sup_allowed = true;
    return *node;
}
ArrayItem &setArrayAlign(ArrayItem &array, std::optional<std::u16string_view> alignment,
                         TexParser *parser) {
    auto align = parser ? std::u16string(alignment.value_or(u""))
                        : UnitUtil::instance().trimSpaces(alignment.value_or(u""));
    if (align == u"t")
        array.arraydef.set(u"align", u"baseline 1");
    else if (align == u"b")
        array.arraydef.set(u"align", u"baseline -1");
    else if (align == u"c")
        array.arraydef.set(u"align", u"axis");
    else if (!align.empty()) {
        if (parser) {
            parser->setString(u"[" + align + u"]" +
                              std::u16string(parser->string().substr(parser->position())));
            parser->setPosition(0);
        } else
            array.arraydef.set(u"align", std::move(align));
    }
    return array;
}
void checkEqnEnv(TexParser &parser, bool nestable) {
    auto top = parser.stack.Top();
    if (!top)
        throw std::logic_error("Equation check requires an active stack");
    auto first = top->First();
    auto array = dynamic_cast<ArrayItem *>(top);
    auto begin = dynamic_cast<BeginItem *>(top);
    if ((array && array->nestable && nestable && !first) || (begin && begin->nestStart))
        return;
    if (!top->isKind(StackKind::start) || first)
        throw TexError("ErroneousNestingEq", u"Erroneous nesting of equation structures");
}
mml::MmlNode &copyNode(mml::MmlNode const &source, TexParser &parser) {
    auto &tree = source.copy();
    tree.walkTree([&](mml::MmlNode &node) {
        parser.configuration.addNode(node.kind(), node);
        auto lists = node.properties().tex.in_lists.value_or(std::vector<std::u16string>{});
        for (auto const &list : lists)
            if (!list.empty())
                parser.configuration.addNode(list, node);
    });
    return tree;
}
std::u16string mmlFilterAttribute(TexParser &, std::u16string_view, std::u16string_view value) {
    return std::u16string(value);
}
NodeDefinition getFontDef(TexParser &parser) {
    NodeDefinition result;
    auto const &font = parser.stack.env().font;
    if (font && !font->empty())
        result.attributes.set(u"mathvariant", *font);
    return result;
}
bool isLatinOrGreekChar(std::u16string_view text) {
    Unicode16Normalization unicode;
    return std::ranges::any_of(unicode.nfd(text), [](char16_t c) {
        return (c >= u'a' && c <= u'z') || (c >= u'A' && c <= u'Z') ||
               (c >= u'\u0370' && c <= u'\u03F0');
    });
}
void fixInitialMO(ParseOptions &options, std::vector<mml::MmlNode *> &nodes) {
    for (auto child : nodes)
        if (child && child->kind() != mml::NodeKind::mspace &&
            (child->kind() != mml::NodeKind::tex_atom ||
             (!child->children().empty() && !child->children().front()->children().empty()))) {
            if (child->isEmbellished() || (child->kind() == mml::NodeKind::tex_atom &&
                                           child->texClass() == mml::TexClass::rel)) {
                auto node = &options.nodeFactory.makeNode(mml::NodeKind::mi);
                nodes.insert(nodes.begin(), node);
            }
            break;
        }
}
mml::MmlNode &fenced(ParseOptions &configuration, std::u16string_view open, mml::MmlNode &contents,
                     std::u16string_view close, std::u16string_view big,
                     std::u16string_view color) {
    auto &factory = configuration.nodeFactory;
    NodeDefinition definition;
    definition.properties.tex.open = std::u16string(open);
    definition.properties.tex.close = std::u16string(close);
    definition.properties.tex.explicit_tex_class = mml::TexClass::inner;
    auto &row = factory.makeNode(mml::NodeKind::mrow, {}, std::move(definition));
    auto delimiter = [&](std::u16string_view text, bool left) -> mml::MmlNode & {
        if (!big.empty()) {
            auto active = configuration.parser();
            if (!active)
                throw std::logic_error("Sized fence requires its active parser");
            auto node =
                TexParser(u"\\" + std::u16string(big) + (left ? u"l" : u"r") + std::u16string(text),
                          ParserEnvironmentSeed{active->stack.env()}, configuration)
                    .mml();
            if (!node)
                throw std::logic_error("Fence parser returned no MML");
            return *node;
        }
        NodeDefinition attributes;
        attributes.attributes.set(u"fence", true);
        attributes.attributes.set(u"stretchy", true);
        attributes.attributes.set(u"symmetric", true);
        attributes.properties.tex.explicit_tex_class =
            left ? mml::TexClass::open : mml::TexClass::close;
        auto token = factory.makeText(text);
        return factory.makeNode(mml::NodeKind::mo, {}, std::move(attributes), token);
    };
    row.appendChild(delimiter(open, true));
    row.appendChild(contents);
    auto &right = delimiter(close, false);
    if (!color.empty())
        right.attributes().set(u"mathcolor", std::u16string(color));
    row.appendChild(right);
    return row;
}
mml::MmlNode &mathPalette(ParseOptions &configuration, std::u16string_view fence,
                          std::u16string_view side) {
    auto delimiter = std::u16string(fence);
    if (fence == u"{" || fence == u"}")
        delimiter = u"\\" + delimiter;
    auto display = u"{\\bigg" + std::u16string(side) + u" " + delimiter + u"}";
    auto text = u"{\\big" + std::u16string(side) + u" " + delimiter + u"}";
    auto node = TexParser(u"\\mathchoice" + display + text + text + text, ParserEnvironmentSeed{},
                          configuration)
                    .mml();
    if (!node)
        throw std::logic_error("Fence palette parser returned no MML");
    return *node;
}
mml::MmlNode &fixedFence(ParseOptions &configuration, std::u16string_view open,
                         mml::MmlNode &contents, std::u16string_view close) {
    NodeDefinition definition;
    definition.properties.tex.open = std::u16string(open);
    definition.properties.tex.close = std::u16string(close);
    definition.properties.tex.explicit_tex_class = mml::TexClass::ord;
    auto &row = configuration.nodeFactory.makeNode(mml::NodeKind::mrow, {}, std::move(definition));
    if (!open.empty())
        row.appendChild(mathPalette(configuration, open, u"l"));
    if (contents.kind() == mml::NodeKind::mrow) {
        auto children =
            std::vector<mml::MmlNode *>(contents.children().begin(), contents.children().end());
        for (auto child : children)
            row.appendChild(*child);
    } else
        row.appendChild(contents);
    if (!close.empty())
        row.appendChild(mathPalette(configuration, close, u"r"));
    return row;
}
namespace {
bool startsWithAsciiLetter(std::u16string_view text) {
    if (text.empty())
        return false;
    const auto first = text.front();
    return (first >= u'a' && first <= u'z') || (first >= u'A' && first <= u'Z');
}

bool endsWithControlWord(std::u16string_view text) {
    auto word_begin = text.size();
    while (word_begin > 0) {
        const auto character = text[word_begin - 1];
        if (!((character >= u'a' && character <= u'z') ||
              (character >= u'A' && character <= u'Z'))) {
            break;
        }
        --word_begin;
    }
    if (word_begin == text.size() || word_begin == 0 || text[word_begin - 1] != u'\\')
        return false;

    auto slash_begin = word_begin - 1;
    while (slash_begin > 0 && text[slash_begin - 1] == u'\\')
        --slash_begin;
    const auto slash_count = word_begin - slash_begin;
    return (slash_count % 2) == 1 && (slash_begin == 0 || text[slash_begin - 1] != u'\\');
}

[[noreturn]] void throwTexError(std::string id, std::u16string message) {
    throw TexError(std::move(id), std::move(message));
}

[[noreturn]] void throwIllegalMacroParameter() {
    throwTexError("IllegalMacroParam", u"Illegal macro parameter reference");
}

[[noreturn]] void throwMaxBuffer() {
    throwTexError("MaxBufferSize",
                  u"MathJax internal buffer size exceeded; is there a recursive macro call?");
}
} // namespace

std::u16string addArgs(std::u16string_view left, std::u16string_view right, std::size_t maxBuffer) {
    const bool insert_space = startsWithAsciiLetter(right) && endsWithControlWord(left);

    if (left.size() > maxBuffer)
        throwMaxBuffer();
    auto combined_size = left.size();
    if (insert_space) {
        if (combined_size == std::numeric_limits<std::size_t>::max())
            throwMaxBuffer();
        ++combined_size;
    }
    if (combined_size > maxBuffer || right.size() > maxBuffer - combined_size)
        throwMaxBuffer();

    std::u16string result;
    result.reserve(combined_size + right.size());
    result.append(left);
    if (insert_space)
        result.push_back(u' ');
    result.append(right);
    return result;
}

std::u16string substituteArgs(std::span<std::u16string const> arguments,
                              std::u16string_view replacement, std::size_t maxBuffer) {
    std::u16string pending;
    std::u16string expanded;
    std::size_t i = 0;
    while (i < replacement.size()) {
        const auto character = replacement[i++];
        if (character == u'\\') {
            pending.push_back(character);
            if (i < replacement.size())
                pending.push_back(replacement[i++]);
            continue;
        }
        if (character != u'#') {
            pending.push_back(character);
            continue;
        }

        if (i < replacement.size() && replacement[i] == u'#') {
            pending.push_back(replacement[i++]);
            continue;
        }
        if (i == replacement.size() || replacement[i] < u'1' || replacement[i] > u'9')
            throwIllegalMacroParameter();

        const auto argument_index = static_cast<std::size_t>(replacement[i++] - u'1');
        if (argument_index >= arguments.size())
            throwIllegalMacroParameter();

        auto prefix = addArgs(expanded, pending, maxBuffer);
        expanded = addArgs(prefix, arguments[argument_index], maxBuffer);
        pending.clear();
    }
    return addArgs(expanded, pending, maxBuffer);
}

void checkMaxMacros(ExpansionBudget &budget, bool isMacro) {
    const bool counter_saturated = budget.macroCount == std::numeric_limits<std::size_t>::max();
    if (!counter_saturated)
        ++budget.macroCount;
    if (!counter_saturated && budget.macroCount <= budget.maxMacros)
        return;

    if (isMacro) {
        throwTexError(
            "MaxMacroSub1",
            u"MathJax maximum macro substitution count exceeded; is here a recursive macro call?");
    }
    throwTexError(
        "MaxMacroSub2",
        u"MathJax maximum substitution count exceeded; is there a recursive latex environment?");
}
std::u16string addArgs(TexParser &parser, std::u16string_view left, std::u16string_view right) {
    return addArgs(left, right, parser.options().maxBuffer.value());
}
std::u16string substituteArgs(TexParser &parser, std::span<std::u16string const> arguments,
                              std::u16string_view replacement) {
    return substituteArgs(arguments, replacement, parser.options().maxBuffer.value());
}
void checkMaxMacros(TexParser &parser, bool isMacro) {
    ExpansionBudget budget{parser.options().maxMacros.value(), parser.options().maxBuffer.value(),
                           parser.macroCount};
    try {
        checkMaxMacros(budget, isMacro);
    } catch (...) {
        parser.macroCount = budget.macroCount;
        throw;
    }
    parser.macroCount = budget.macroCount;
}
} // namespace folia::math::tex::parse_util
