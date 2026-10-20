/*************************************************************
 * MathJax 4.1.3 input/tex/ColumnParser.ts; Apache-2.0.
 * Array column-template algorithms using native per-array state.
 *************************************************************/
module folia.math.input.tex.parser;
import :column_parser;
import :parse_options;
import :tex_parser;
import folia.math.input.tex.parse_util;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.unit_util;
import folia.math.util.js_number;
import folia.math.util.js_numeric_input;
import folia.math.util.utf16;
import std;

namespace folia::math::tex {
namespace {
using mml::AttributeValue;

template <class T> void put(std::vector<T> &values, std::size_t index, T value) {
    if (values.size() <= index)
        values.resize(index + 1);
    values[index] = std::move(value);
}

template <class T> T const *get(std::vector<T> const &values, std::size_t index) noexcept {
    return index < values.size() ? &values[index] : nullptr;
}

bool truthy(std::u16string const *value) noexcept {
    return value && !value->empty();
}

std::u16string value_or(std::vector<std::u16string> const &values, std::size_t index,
                        std::u16string_view fallback) {
    const auto *value = get(values, index);
    return truthy(value) ? *value : std::u16string(fallback);
}

std::u16string join(std::span<std::u16string const> values, std::u16string_view separator) {
    std::size_t length = values.empty() ? 0 : (values.size() - 1) * separator.size();
    for (auto const &value : values)
        length += value.size();
    std::u16string result;
    result.reserve(length);
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i)
            result.append(separator);
        result.append(values[i]);
    }
    return result;
}

std::u16string fromCodePoint(char32_t point) {
    if (point > 0x10FFFF)
        throw std::range_error("invalid Unicode code point");
    if (point <= 0xFFFF)
        return std::u16string(1, static_cast<char16_t>(point));
    point -= 0x10000;
    return {static_cast<char16_t>(0xD800 + (point >> 10)),
            static_cast<char16_t>(0xDC00 + (point & 0x3FF))};
}

std::u16string ascii16(std::string_view value) {
    std::u16string result;
    result.reserve(value.size());
    for (const auto c : value)
        result.push_back(static_cast<char16_t>(static_cast<unsigned char>(c)));
    return result;
}

[[noreturn]] void error(std::string id, std::u16string message) {
    throw TexError(std::move(id), std::move(message));
}

[[noreturn]] void errorWith(std::string id, std::u16string message, std::u16string argument) {
    const std::array<std::u16string, 1> arguments{std::move(argument)};
    throw TexError(std::move(id), std::move(message), arguments);
}

std::size_t maxBuffer(TexParser &parser) {
    return parser.options().maxBuffer.value_or(std::numeric_limits<std::size_t>::max());
}

std::u16string lowerAscii(std::u16string value) {
    for (auto &c : value) {
        if (c >= u'A' && c <= u'Z')
            c = static_cast<char16_t>(c + (u'a' - u'A'));
    }
    return value;
}

} // namespace

ColumnState::ColumnState(TexParser &sourceParser, std::u16string sourceTemplate, ArrayItem &array)
    : parser(sourceParser), template_text(std::move(sourceTemplate)), cstart(array.cstart),
      cend(array.cend), cextra(array.cextra), ralign(array.ralign) {}

ColumnParser::ColumnParser(ColumnMacroDefinitions &definitions) : macro_definitions_(definitions) {}

std::span<ColumnParser::BuiltinEntry const> ColumnParser::builtinHandlers() noexcept {
    static const std::array<BuiltinEntry, 21> handlers{{
        {u'l', [](ColumnParser &self,
                  ColumnState &state) { put(state.calign, state.j++, std::u16string(u"left")); }},
        {u'c', [](ColumnParser &self,
                  ColumnState &state) { put(state.calign, state.j++, std::u16string(u"center")); }},
        {u'r', [](ColumnParser &self,
                  ColumnState &state) { put(state.calign, state.j++, std::u16string(u"right")); }},
        {u'p', [](ColumnParser &self, ColumnState &state) { self.getColumn(state, u"top"); }},
        {u'm', [](ColumnParser &self, ColumnState &state) { self.getColumn(state, u"middle"); }},
        {u'b', [](ColumnParser &self, ColumnState &state) { self.getColumn(state, u"bottom"); }},
        {u'w', [](ColumnParser &self, ColumnState &state) { self.getColumn(state, u"top", u""); }},
        {u'W', [](ColumnParser &self, ColumnState &state) { self.getColumn(state, u"top", u""); }},
        {u'|', [](ColumnParser &self, ColumnState &state) { self.addRule(state, u"solid"); }},
        {u':', [](ColumnParser &self, ColumnState &state) { self.addRule(state, u"dashed"); }},
        {u'>',
         [](ColumnParser &self, ColumnState &state) {
             auto prefix = self.getBraces(state);
             const auto *old = get(state.cstart, state.j);
             prefix += old ? *old : u"";
             put(state.cstart, state.j, std::move(prefix));
         }},
        {u'<',
         [](ColumnParser &self, ColumnState &state) {
             const auto prefix = self.getBraces(state);
             if (state.j != 0) {
                 const auto index = state.j - 1;
                 const auto *old = get(state.cend, index);
                 put(state.cend, index, prefix + (old ? *old : u""));
             }
         }},
        {u'@',
         [](ColumnParser &self, ColumnState &state) { self.addAt(state, self.getBraces(state)); }},
        {u'!', [](ColumnParser &self,
                  ColumnState &state) { self.addBang(state, self.getBraces(state)); }},
        {u'*', [](ColumnParser &self, ColumnState &state) { self.repeat(state); }},
        {u'{', [](ColumnParser &self, ColumnState &state) { self.brace(state); }},
        {u'P', [](ColumnParser &self,
                  ColumnState &state) { self.macroColumn(state, u">{$}p{#1}<{$}", 1); }},
        {u'M', [](ColumnParser &self,
                  ColumnState &state) { self.macroColumn(state, u">{$}m{#1}<{$}", 1); }},
        {u'B', [](ColumnParser &self,
                  ColumnState &state) { self.macroColumn(state, u">{$}b{#1}<{$}", 1); }},
        {u' ', [](ColumnParser &, ColumnState &) {}},
        {u'\n', [](ColumnParser &, ColumnState &) {}},
    }};
    return handlers;
}

void ColumnParser::process(TexParser &parser, std::u16string templateText, ArrayItem &array) {
    ColumnState state(parser, std::move(templateText), array);
    if (!state.template_text.empty() && state.template_text.front() == u'{' &&
        state.template_text.back() == u'}') {
        const auto braced = getBraces(state);
        if (braced.size() == state.template_text.size() - 2)
            state.template_text = braced;
        state.i = 0;
    }

    double n = 0;
    while (state.i < state.template_text.size()) {
        if (n++ > MAXCOLUMNS)
            error("MaxColumns",
                  u"Too many column specifiers (perhaps looping column definitions?)");
        const auto codePoint = utf16::codePointAt(state.template_text, state.i);
        if (!codePoint)
            throw std::range_error("ColumnParser cursor is outside template");
        state.c = fromCodePoint(*codePoint);
        state.i += state.c.size();
        processColumn(state, state.c);
    }

    setColumnAlign(state, array);
    setColumnWidths(state, array);
    setColumnSpacing(state, array);
    setColumnLines(state, array);
    setPadding(state, array);
}

void ColumnParser::processColumn(ColumnState &state, std::u16string c) {
    if (c.size() == 1) {
        if (const auto *definition = macro_definitions_.find(c.front())) {
            macroColumn(state, definition->replacement, definition->argument_count);
            return;
        }
        for (auto const &[specifier, handler] : builtinHandlers()) {
            if (specifier == c.front()) {
                handler(*this, state);
                return;
            }
        }
    }
    errorWith("BadPreamToken", u"Illegal pream-token (%1)", std::move(c));
}

void ColumnParser::setColumnAlign(ColumnState &state, ArrayItem &array) {
    array.arraydef.set(u"columnalign", join(state.calign, u" "));
}

void ColumnParser::setColumnWidths(ColumnState &state, ArrayItem &array) {
    if (state.cwidth.empty())
        return;
    auto widths = state.cwidth;
    if (widths.size() < state.calign.size())
        widths.emplace_back(u"auto");
    for (auto &width : widths)
        if (width.empty())
            width = u"auto";
    array.arraydef.set(u"columnwidth", join(widths, u" "));
}

void ColumnParser::setColumnSpacing(ColumnState &state, ArrayItem &array) {
    if (state.cspace.empty())
        return;
    auto spacing = state.cspace;
    if (spacing.size() < state.calign.size())
        spacing.emplace_back(u"1em");
    std::vector<std::u16string> columns;
    if (spacing.size() > 1) {
        columns.reserve(spacing.size() - 1);
        for (std::size_t i = 1; i < spacing.size(); ++i)
            columns.push_back(spacing[i].empty() ? u"1em" : spacing[i]);
    }
    array.arraydef.set(u"columnspacing", join(columns, u" "));
}

void ColumnParser::setColumnLines(ColumnState &state, ArrayItem &array) {
    if (state.clines.empty())
        return;
    auto lines = state.clines;
    if (!lines[0].empty())
        array.frame.push_back({ArrayFrameRule::Edge::left, lines[0]});
    if (lines.size() > state.calign.size()) {
        array.frame.push_back({ArrayFrameRule::Edge::right, lines.back()});
        lines.pop_back();
    } else if (lines.size() < state.calign.size()) {
        lines.emplace_back(u"none");
    }
    if (lines.size() > 1) {
        std::vector<std::u16string> columns;
        columns.reserve(lines.size() - 1);
        for (std::size_t i = 1; i < lines.size(); ++i)
            columns.push_back(lines[i].empty() ? u"none" : lines[i]);
        array.arraydef.set(u"columnlines", join(columns, u" "));
    }
}

void ColumnParser::setPadding(ColumnState &state, ArrayItem &array) {
    const bool first = !state.cextra.empty() && state.cextra[0];
    const bool last = !state.calign.empty() && state.calign.size() <= state.cextra.size() &&
                      state.cextra[state.calign.size() - 1];
    if (!first && !last)
        return;
    const auto i = state.calign.empty() ? 0 : state.calign.size() - 1;
    const bool extra = !state.calign.empty() && i < state.cextra.size() && state.cextra[i];
    const auto *space = extra ? get(state.cspace, i) : nullptr;
    const auto left = value_or(state.cspace, 0, u".5em");
    const auto right = truthy(space) ? *space : std::u16string(u".5em");
    array.arraydef.set(u"data-array-padding", left + u" " + right);
}

void ColumnParser::getColumn(ColumnState &state, std::u16string ralign, std::u16string calign) {
    const auto align = calign.empty() ? getAlign(state) : std::move(calign);
    put(state.calign, state.j, align);
    put(state.cwidth, state.j, getDimen(state));
    if (state.ralign.size() <= state.j)
        state.ralign.resize(state.j + 1);
    state.ralign[state.j] =
        ArrayColumnAlignment{std::move(ralign), state.cwidth[state.j], state.calign[state.j]};
    ++state.j;
}

std::u16string ColumnParser::getDimen(ColumnState &state) {
    auto const dim = getBraces(state);
    if (!UnitUtil::instance().matchDimen(dim))
        errorWith("MissingColumnDimOrUnits",
                  u"Missing dimension or its units for %1 column declaration", state.c);
    return dim;
}

std::u16string ColumnParser::getAlign(ColumnState &state) {
    const auto align = lowerAscii(getBraces(state));
    if (align == u"l")
        return u"left";
    if (align == u"c")
        return u"center";
    if (align == u"r")
        return u"right";
    return {};
}

std::u16string ColumnParser::getBraces(ColumnState &state) {
    while (state.i < state.template_text.size() && state.template_text[state.i] == u' ')
        ++state.i;
    if (state.i >= state.template_text.size())
        errorWith("MissingArgForColumn", u"Missing argument for %1 column declaration", state.c);
    if (state.template_text[state.i] != u'{')
        return std::u16string(1, state.template_text[state.i++]);

    const auto start = ++state.i;
    std::size_t braces = 1;
    while (state.i < state.template_text.size()) {
        switch (state.template_text[state.i++]) {
        case u'\\':
            ++state.i;
            break;
        case u'{':
            ++braces;
            break;
        case u'}':
            if (--braces == 0)
                return state.template_text.substr(start, state.i - start - 1);
            break;
        }
    }
    error("MissingCloseBrace", u"Missing close brace");
}

void ColumnParser::macroColumn(ColumnState &state, std::u16string macro, double n) {
    std::vector<std::u16string> arguments;
    while (n > 0 && n--)
        arguments.push_back(getBraces(state));
    const auto substituted = parse_util::substituteArgs(arguments, macro, maxBuffer(state.parser));
    const auto suffix = state.template_text.substr(std::min(state.i, state.template_text.size()));
    state.template_text = substituted + suffix;
    state.i = 0;
}

void ColumnParser::addRule(ColumnState &state, std::u16string rule) {
    if (truthy(get(state.clines, state.j)))
        addAt(state, u"\\,");
    put(state.clines, state.j, std::move(rule));
    if (const auto *spacing = get(state.cspace, state.j); spacing && *spacing == u"0")
        put(state.cstart, state.j, std::u16string(u"\\hspace{.5em}"));
}

void ColumnParser::addAt(ColumnState &state, std::u16string macro) {
    const auto j = state.j;
    put(state.cextra, j, true);
    put(state.calign, j, std::u16string(u"center"));
    if (truthy(get(state.clines, j))) {
        if (const auto *spacing = get(state.cspace, j); spacing && *spacing == u".5em") {
            if (j > 0)
                put(state.cstart, j - 1, value_or(state.cstart, j - 1, u"") + u"\\hspace{.25em}");
        } else if (!truthy(get(state.cspace, j))) {
            if (j > 0)
                put(state.cend, j - 1, value_or(state.cend, j - 1, u"") + u"\\hspace{.5em}");
        }
    }
    put(state.cstart, j, std::move(macro));
    put(state.cspace, j, std::u16string(u"0"));
    ++state.j;
    put(state.cspace, state.j, std::u16string(u"0"));
}

void ColumnParser::addBang(ColumnState &state, std::u16string macro) {
    const auto j = state.j;
    put(state.cextra, j, true);
    put(state.calign, j, std::u16string(u"center"));
    const auto *spacing = get(state.cspace, j);
    const bool before = spacing && *spacing == u"0" && truthy(get(state.clines, j));
    put(state.cstart, j, (before ? std::u16string(u"\\hspace{.25em}") : u"") + macro);
    if (!truthy(get(state.cspace, j)))
        put(state.cspace, j, std::u16string(u".5em"));
    ++state.j;
    put(state.cspace, state.j, std::u16string(u".5em"));
}

void ColumnParser::repeat(ColumnState &state) {
    const auto number = getBraces(state);
    const auto columns = getBraces(state);
    const double count = js_input::parseInt(number);
    if (ascii16(js_number::toString(count)) != number)
        errorWith("ColArgNotNum", u"First argument to %1 column specifier must be a number", u"*");
    if (!std::isfinite(count) || count < 0 || count > 4294967295.0 || std::trunc(count) != count)
        throw std::length_error("Invalid array length");
    const auto repeats = static_cast<std::size_t>(count);
    if (!columns.empty() && repeats > std::u16string{}.max_size() / columns.size())
        throw std::length_error("Invalid array length");
    std::u16string repeated;
    repeated.reserve(columns.size() * repeats);
    for (std::size_t i = 0; i < repeats; ++i)
        repeated.append(columns);
    const auto suffix = state.template_text.substr(std::min(state.i, state.template_text.size()));
    state.template_text = repeated + suffix;
    state.i = 0;
}

void ColumnParser::brace(ColumnState &state) {
    if (state.i != 0)
        --state.i;
    processColumn(state, getBraces(state));
}

} // namespace folia::math::tex
