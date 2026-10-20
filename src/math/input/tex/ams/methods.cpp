// MathJax 4.1.3 input/tex/ams/AmsMethods.ts; Apache-2.0.
module folia.math.input.tex.ams.methods;
import folia.math.input.tex.ams.items;
import folia.math.input.tex.parse_methods;
import folia.math.input.tex.parse_util;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.unit_util;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import folia.math.core.mml_tree.mml_nodes.munderover;
import std;

namespace folia::math::tex {
namespace {
using mml::MmlNode;
using mml::NodeKind;
using Arguments = std::span<TexArgument const>;
TexArgument const *argument(Arguments args, std::size_t index) {
    return index < args.size() ? &args[index] : nullptr;
}
bool present(Arguments args, std::size_t index) {
    auto value = argument(args, index);
    return value && !std::holds_alternative<NullArgument>(*value);
}
std::u16string text(Arguments args, std::size_t index) {
    auto value = argument(args, index);
    if (!value || std::holds_alternative<NullArgument>(*value))
        return {};
    if (auto string = std::get_if<std::u16string>(value))
        return *string;
    if (auto number = std::get_if<double>(value)) {
        auto ascii = js_number::toString(*number);
        return {ascii.begin(), ascii.end()};
    }
    return std::get<bool>(*value) ? u"true" : u"false";
}
bool flag(Arguments args, std::size_t index) {
    auto value = argument(args, index);
    if (!value || std::holds_alternative<NullArgument>(*value))
        return false;
    if (auto boolean = std::get_if<bool>(value))
        return *boolean;
    if (auto number = std::get_if<double>(value))
        return *number != 0 && !std::isnan(*number);
    return !std::get<std::u16string>(*value).empty();
}
double number(Arguments args, std::size_t index) {
    auto value = argument(args, index);
    if (!value)
        return std::numeric_limits<double>::quiet_NaN();
    if (std::holds_alternative<NullArgument>(*value))
        return 0;
    if (auto numeric = std::get_if<double>(value))
        return *numeric;
    if (auto boolean = std::get_if<bool>(value))
        return *boolean ? 1 : 0;
    return js_input::number(std::get<std::u16string>(*value));
}
Arguments arguments(MacroInvocation const &invocation) {
    return std::get<Arguments>(invocation.arguments);
}
std::u16string required(TexParser &parser, std::u16string_view name) {
    return parser.GetArgument(name).value();
}
std::u16string beginName(FrameHandle const &begin) {
    return std::u16string(begin->getName().value_or(u"undefined"));
}
std::optional<std::u16string_view> view(std::optional<std::u16string> const &value) {
    return value ? std::optional<std::u16string_view>(*value) : std::nullopt;
}
[[noreturn]] void error(std::string id, std::u16string message,
                        std::initializer_list<std::u16string> args) {
    throw TexError(std::move(id), std::move(message), {args.begin(), args.size()});
}
MmlNode &node(TexParser &parser, NodeKind kind, std::initializer_list<MmlNode *> children = {},
              NodeDefinition definition = {}) {
    return parser.create(kind, {children.begin(), children.size()}, std::move(definition));
}
NodeDefinition attributes(std::initializer_list<mml::AttributeSet::Entry> entries) {
    NodeDefinition definition;
    for (auto const &[name, value] : entries)
        definition.attributes.set(name, value);
    return definition;
}
NodeDefinition texClass(mml::TexClass value) {
    NodeDefinition definition;
    definition.properties.tex.explicit_tex_class = value;
    return definition;
}
std::u16string em(double value) {
    auto ascii = UnitUtil::instance().em(value);
    return {ascii.begin(), ascii.end()};
}
MmlNode *child(MmlNode *parent, std::size_t index) {
    return parent && index < parent->children().size() ? parent->children()[index] : nullptr;
}
bool checkSideSetBase(MmlNode &mml) {
    auto base = child(&mml, 0);
    return base && base->kind() == NodeKind::mi && NodeUtil::getText(*base).empty();
}
std::pair<MmlNode *, MmlNode *> splitSideSet(MmlNode *mml) {
    if (!mml || (mml->isInferred() && mml->children().empty()))
        return {};
    if (mml->kind() == NodeKind::msubsup && checkSideSetBase(*mml))
        return {mml, nullptr};
    auto first = child(mml, 0);
    if (!(mml->isInferred() && first && checkSideSetBase(*first)))
        return {nullptr, mml};
    mml->removeChild(*first);
    return {first, mml};
}
std::u16string fromCodePoint(double value) {
    if (!std::isfinite(value) || std::trunc(value) != value || value < 0 || value > 0x10FFFF)
        throw std::out_of_range("Invalid arrow code point");
    auto code = static_cast<std::uint32_t>(value);
    if (code <= 0xFFFF)
        return std::u16string(1, static_cast<char16_t>(code));
    code -= 0x10000;
    return {static_cast<char16_t>(0xD800 + (code >> 10)),
            static_cast<char16_t>(0xDC00 + (code & 0x3FF))};
}
} // namespace

StackEntry AmsMethods::AmsEqnArray(TexParser &parser, FrameHandle const &begin, Arguments args) {
    auto align = parser.GetBrackets(u"\\begin{" + beginName(begin) + u"}");
    auto result = BaseMethods::EqnArray(parser, begin, args);
    auto &array = dynamic_cast<ArrayItem &>(*std::get<FrameHandle>(result));
    parse_util::setArrayAlign(array, view(align), &parser);
    return result;
}
StackEntry AmsMethods::AlignAt(TexParser &parser, FrameHandle const &begin, Arguments args) {
    const auto command = u"\\begin{" + beginName(begin) + u"}";
    std::optional<std::u16string> valign;
    if (!flag(args, 1))
        valign = parser.GetBrackets(command);
    auto count = required(parser, command);
    if (count.find_first_not_of(u"0123456789") != count.npos)
        error("PositiveIntegerArg", u"Argument to %1 must be a positive integer", {command});
    std::u16string align, balign, spacing;
    for (double n = js_input::parseInt(count, js_input::IntegerRadix::Decimal); n > 0; --n) {
        align += u"rl";
        balign += u"bt";
        if (!spacing.empty())
            spacing += u' ';
        spacing += u"0em 0em";
    }
    const std::array<TexArgument, 5> definitions{flag(args, 0), flag(args, 1), align, balign,
                                                 spacing};
    auto result = BaseMethods::EqnArray(parser, begin, definitions);
    if (!flag(args, 1)) {
        auto &array = dynamic_cast<ArrayItem &>(*std::get<FrameHandle>(result));
        parse_util::setArrayAlign(array, view(valign), &parser);
    }
    return result;
}
StackEntry AmsMethods::Multline(TexParser &parser, FrameHandle const &begin, Arguments args) {
    parse_util::checkEqnEnv(parser);
    parser.Push(begin);
    auto const &padding = parser.options().ams->multlineIndent.value();
    auto item = parser.itemFactory().create(StackKind::multline, MultlineItemInit{flag(args, 0)});
    auto &array = dynamic_cast<ArrayItem &>(*item);
    array.arraydef.set(u"displaystyle", true);
    array.arraydef.set(u"rowspacing", u".5em");
    array.arraydef.set(u"columnspacing", u"100%");
    array.arraydef.set(u"width", parser.options().ams->multlineWidth.value());
    array.arraydef.set(u"side", parser.options().tagSide.value());
    array.arraydef.set(u"minlabelspacing", parser.options().tagIndent.value());
    array.arraydef.set(u"data-array-padding", padding + u" " + padding);
    array.arraydef.set(u"data-width-includes-label", true);
    return item;
}
StackEntry AmsMethods::XalignAt(TexParser &parser, FrameHandle const &begin, Arguments args) {
    const auto command = u"\\begin{" + beginName(begin) + u"}";
    const auto count = required(parser, command);
    if (count.find_first_not_of(u"0123456789") != count.npos)
        error("PositiveIntegerArg", u"Argument to %1 must be a positive integer", {command});
    const bool padded = flag(args, 1);
    const std::array<TexArgument, 7> definitions{
        flag(args, 0),
        padded,
        false,
        std::u16string(padded ? u"crl" : u"rlc"),
        std::u16string(padded ? u"mbt" : u"btm"),
        std::u16string(padded ? u"fit auto auto" : u"auto auto fit"),
        true};
    auto result = FlalignArray(parser, begin, definitions);
    dynamic_cast<FlalignItem &>(*std::get<FrameHandle>(result)).xalignat =
        2 * js_input::parseInt(count);
    return result;
}
StackEntry AmsMethods::FlalignArray(TexParser &parser, FrameHandle const &begin, Arguments args) {
    parse_util::checkEqnEnv(parser);
    parser.Push(begin);
    std::u16string align;
    for (auto c : text(args, 3)) {
        if (!align.empty())
            align += u' ';
        if (c == u'r')
            align += u"right";
        else if (c == u'l')
            align += u"left";
        else if (c == u'c')
            align += u"center";
        else
            align += c;
    }
    auto balign = splitAlignArray(text(args, 4));
    auto item = parser.itemFactory().create(
        StackKind::flalign,
        FlalignItemInit{beginName(begin), flag(args, 0), flag(args, 1), flag(args, 2)});
    auto &array = dynamic_cast<FlalignItem &>(*item);
    array.arraydef.set(u"width", u"100%");
    array.arraydef.set(u"displaystyle", true);
    array.arraydef.set(u"columnalign", align);
    array.arraydef.set(u"columnspacing", u"0em");
    array.arraydef.set(u"columnwidth", text(args, 5));
    array.arraydef.set(u"rowspacing", u"3pt");
    array.arraydef.set(u"data-break-align", balign);
    array.arraydef.set(u"side", parser.options().tagSide.value());
    array.arraydef.set(u"minlabelspacing",
                       flag(args, 6) ? u"0" : parser.options().tagIndent.value());
    array.arraydef.set(u"data-width-includes-label", true);
    array.zeroWidthLabel = flag(args, 6);
    return item;
}
DispatchOutcome AmsMethods::HandleDeclareOp(TexParser &parser, MacroInvocation const &invocation) {
    auto star = parser.GetStar() ? u"*" : u"";
    auto cs = NewcommandUtil::GetCsNameArgument(parser, invocation.dispatch_token);
    auto op = required(parser, invocation.dispatch_token);
    MacroDefinition definition;
    definition.replacement_source = u"\\operatorname" + std::u16string(star) + u"{" + op + u"}";
    NewcommandUtil::addMacro(parser, cs, CommandDefinition{cs, std::move(definition)});
    parser.Push(parser.itemFactory().create(StackKind::null_item));
    return NoParseResult{};
}
DispatchOutcome AmsMethods::HandleOperatorName(TexParser &parser,
                                               MacroInvocation const &invocation) {
    const bool star = parser.GetStar();
    auto op = UnitUtil::instance().trimSpaces(required(parser, invocation.dispatch_token));
    auto env = parser.stack.env();
    env.font = u"normal";
    env.multi_letter_identifiers = parser.options().ams->operatornamePattern;
    env.operator_letters = true;
    env.no_auto_op = true;
    auto result = TexParser(op, ParserEnvironmentSeed{env}, parser.configuration).mml();
    if (result->kind() == NodeKind::mi)
        result->properties().tex.auto_operator.reset();
    else
        result = &node(parser, NodeKind::tex_atom, {result});
    auto definition = texClass(mml::TexClass::op);
    definition.properties.tex.move_sup_sub = star;
    definition.properties.tex.movable_limits = true;
    NodeUtil::setProperties(*result, std::move(definition));
    if (!star) {
        auto next = parser.GetNext();
        auto position = parser.position();
        if (next == u"\\") {
            parser.setPosition(position + 1);
            if (parser.GetCS() != u"limits")
                parser.setPosition(position);
        }
    }
    parser.Push(parser.itemFactory().create(StackKind::fn, FnItemInit{*result}));
    return NoParseResult{};
}
DispatchOutcome AmsMethods::SideSet(TexParser &parser, MacroInvocation const &invocation) {
    auto [preScripts, preRest] = splitSideSet(parser.ParseArg(invocation.dispatch_token));
    auto [postScripts, postRest] = splitSideSet(parser.ParseArg(invocation.dispatch_token));
    auto base = parser.ParseArg(invocation.dispatch_token);
    auto result = base;
    if (preScripts) {
        if (preRest) {
            auto &copy = parse_util::copyNode(*base, parser);
            auto &padded = node(parser, NodeKind::mpadded, {&copy}, attributes({{u"width", 0.0}}));
            auto &phantom = node(parser, NodeKind::mphantom, {&padded});
            preScripts->replaceChild(phantom, *child(preScripts, 0));
        } else {
            result = &node(parser, NodeKind::mmultiscripts, {base});
            if (postScripts) {
                auto sub = child(postScripts, 1), sup = child(postScripts, 2);
                result->appendChild(sub ? *sub : node(parser, NodeKind::none));
                result->appendChild(sup ? *sup : node(parser, NodeKind::none));
            }
            result->properties().tex.script_alignment = mml::HorizontalAlignment::left;
            result->appendChild(node(parser, NodeKind::mprescripts));
            auto sub = child(preScripts, 1), sup = child(preScripts, 2);
            result->appendChild(sub ? *sub : node(parser, NodeKind::none));
            result->appendChild(sup ? *sup : node(parser, NodeKind::none));
        }
    }
    if (postScripts && result == base) {
        postScripts->replaceChild(*base, *child(postScripts, 0));
        result = postScripts;
    }
    auto definition = texClass(mml::TexClass::op);
    definition.properties.tex.move_sup_sub = true;
    definition.properties.tex.movable_limits = true;
    auto &row = node(parser, NodeKind::tex_atom, {}, std::move(definition));
    if (preRest) {
        if (preScripts)
            row.appendChild(*preScripts);
        row.appendChild(*preRest);
    }
    row.appendChild(*result);
    if (postRest)
        row.appendChild(*postRest);
    parser.Push(&row);
    return NoParseResult{};
}
DispatchOutcome AmsMethods::operatorLetter(TexParser &parser, TokenInput const &input) {
    return parser.stack.env().operator_letters.value_or(false)
               ? ParseMethods::variable(parser, input)
               : DispatchOutcome{ContinueDispatch{}};
}
DispatchOutcome AmsMethods::MultiIntegral(TexParser &parser, MacroInvocation const &invocation) {
    auto integral = text(arguments(invocation), 0);
    auto next = parser.GetNext();
    if (next == u"\\") {
        auto position = parser.position();
        next = required(parser, invocation.dispatch_token);
        parser.setPosition(position);
        if (next == u"\\limits")
            integral = u"\\!\\!\\mathop{\\,\\," + integral + u"}";
    }
    parser.setString(integral + u" " + std::u16string(parser.string().substr(parser.position())));
    parser.setPosition(0);
    return NoParseResult{};
}
DispatchOutcome AmsMethods::xArrow(TexParser &parser, MacroInvocation const &invocation) {
    auto args = arguments(invocation);
    auto left = number(args, 1), right = number(args, 2);
    auto definition =
        attributes({{u"width", u"+" + em((left + right) / 18)}, {u"lspace", em(left / 18)}});
    auto bottom = parser.GetBrackets(invocation.dispatch_token);
    auto first = parser.ParseArg(invocation.dispatch_token);
    auto &strut =
        node(parser, NodeKind::mspace, {}, attributes({{u"depth", std::u16string(u".2em")}}));
    auto arrowDefinition = texClass(mml::TexClass::ord);
    arrowDefinition.attributes.set(u"stretchy", true);
    auto &arrow = parser.createToken(NodeKind::mo, fromCodePoint(number(args, 0)),
                                     std::move(arrowDefinition));
    if (flag(args, 3))
        arrow.attributes().set(u"minsize", em(number(args, 3)));
    auto &style = node(parser, NodeKind::mstyle, {&arrow}, attributes({{u"scriptlevel", 0.0}}));
    auto &scripts =
        dynamic_cast<mml::MmlMunderover &>(node(parser, NodeKind::munderover, {&style}));
    auto &above = node(parser, NodeKind::mpadded, {first, &strut}, definition);
    above.attributes().set(u"voffset", u"-.2em");
    above.attributes().set(u"height", u"-.2em");
    NodeUtil::setChild(scripts, scripts.over(), above);
    if (bottom && !bottom->empty()) {
        auto below =
            TexParser(*bottom, ParserEnvironmentSeed{parser.stack.env()}, parser.configuration)
                .mml();
        auto &strut =
            node(parser, NodeKind::mspace, {}, attributes({{u"height", std::u16string(u".75em")}}));
        auto &padded = node(parser, NodeKind::mpadded, {below, &strut}, definition);
        padded.attributes().set(u"voffset", u".15em");
        padded.attributes().set(u"depth", u"-.15em");
        NodeUtil::setChild(scripts, scripts.under(), padded);
    }
    scripts.properties().tex.sub_sup_allowed = true;
    auto &empty = node(parser, NodeKind::tex_atom, {}, texClass(mml::TexClass::none));
    auto &outer =
        node(parser, NodeKind::tex_atom, {&empty, &scripts}, texClass(mml::TexClass::rel));
    parser.Push(&outer);
    return NoParseResult{};
}
DispatchOutcome AmsMethods::HandleShove(TexParser &parser, MacroInvocation const &invocation) {
    auto top = parser.stack.Top();
    if (top->kind() != StackKind::multline)
        error("CommandOnlyAllowedInEnv", u"%1 only allowed in %2 environment",
              {parser.currentCS(), u"multline"});
    if (top->Size())
        error("CommandAtTheBeginingOfLine", u"%1 must come at the beginning of the line",
              {parser.currentCS()});
    dynamic_cast<ArrayItem &>(*top).shove = text(arguments(invocation), 0);
    return NoParseResult{};
}
DispatchOutcome AmsMethods::CFrac(TexParser &parser, MacroInvocation const &invocation) {
    auto align =
        UnitUtil::instance().trimSpaces(parser.GetBrackets(invocation.dispatch_token, u"").value());
    auto numerator = required(parser, invocation.dispatch_token);
    auto denominator = required(parser, invocation.dispatch_token);
    auto num = TexParser(u"\\strut\\textstyle{" + numerator + u"}",
                         ParserEnvironmentSeed{parser.stack.env()}, parser.configuration)
                   .mml();
    auto den = TexParser(u"\\strut\\textstyle{" + denominator + u"}",
                         ParserEnvironmentSeed{parser.stack.env()}, parser.configuration)
                   .mml();
    auto &fraction = node(parser, NodeKind::mfrac, {num, den});
    if (!align.empty() && align != u"l" && align != u"r")
        error("IllegalAlign", u"Illegal alignment specified in %1", {parser.currentCS()});
    if (!align.empty()) {
        auto side = align == u"l" ? u"left" : u"right";
        fraction.attributes().set(u"numalign", side);
        fraction.attributes().set(u"denomalign", side);
    }
    parser.Push(&fraction);
    return NoParseResult{};
}
DispatchOutcome AmsMethods::Genfrac(TexParser &parser, MacroInvocation const &invocation) {
    auto args = arguments(invocation);
    auto left = present(args, 0) ? text(args, 0)
                                 : parser.GetDelimiterArg(invocation.dispatch_token).value_or(u"");
    auto right = present(args, 1) ? text(args, 1)
                                  : parser.GetDelimiterArg(invocation.dispatch_token).value_or(u"");
    auto thickness = present(args, 2) ? text(args, 2) : required(parser, invocation.dispatch_token);
    auto style = present(args, 3)
                     ? text(args, 3)
                     : UnitUtil::instance().trimSpaces(required(parser, invocation.dispatch_token));
    auto numerator = parser.ParseArg(invocation.dispatch_token);
    auto denominator = parser.ParseArg(invocation.dispatch_token);
    auto fraction = &node(parser, NodeKind::mfrac, {numerator, denominator});
    if (!thickness.empty())
        fraction->attributes().set(u"linethickness", thickness);
    if (!left.empty() || !right.empty()) {
        fraction->properties().tex.with_delimiters = true;
        fraction = &parse_util::fixedFence(parser.configuration, left, *fraction, right);
    }
    if (!style.empty()) {
        auto digit = js_input::parseInt(style, js_input::IntegerRadix::Decimal);
        if (!std::isfinite(digit) || digit < 0 || digit > 3)
            error("BadMathStyleFor", u"Bad math style for %1", {parser.currentCS()});
        fraction = &node(parser, NodeKind::mstyle, {fraction});
        fraction->attributes().set(u"displaystyle", digit == 0);
        fraction->attributes().set(u"scriptlevel", digit == 0 ? 0.0 : digit - 1);
    }
    parser.Push(fraction);
    return NoParseResult{};
}
DispatchOutcome AmsMethods::HandleTag(TexParser &parser, MacroInvocation const &invocation) {
    auto const &tag = parser.configuration.equation.current_tag;
    auto env = parser.tags().env();
    if (!tag.taggable && !env.empty())
        error("CommandNotAllowedInEnv", u"%1 not allowed in %2 environment",
              {parser.currentCS(), std::u16string(env.begin(), env.end())});
    if (tag.tag && !tag.tag->empty())
        error("MultipleCommand", u"Multiple %1", {parser.currentCS()});
    auto star = parser.GetStar();
    auto id = UnitUtil::instance().trimSpaces(required(parser, invocation.dispatch_token));
    parser.tags().tag(id, star);
    parser.Push(parser.itemFactory().create(StackKind::null_item));
    return NoParseResult{};
}
} // namespace folia::math::tex
