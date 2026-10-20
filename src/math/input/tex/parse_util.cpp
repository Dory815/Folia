// Native implementation of the three request-local ParseUtil operations.
// Source: MathJax 4.1.3 ts/input/tex/ParseUtil.ts (Apache-2.0).
module folia.math.input.tex.parse_util;
import std;
import folia.math.input.tex.tex_error;
import folia.math.util.unicode_normalization;

namespace folia::math::tex::parse_util {
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
} // namespace folia::math::tex::parse_util
