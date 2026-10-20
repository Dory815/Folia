// MathJax 4.1.3 input/tex/mathtools/MathtoolsMethods.ts; Apache-2.0.
module folia.math.input.tex.mathtools.methods;
import folia.math.input.tex.mathtools.items;
import folia.math.input.tex.mathtools.util;
import folia.math.input.tex.mathtools.tags;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.parse_util;
import folia.math.input.tex.unit_util;
import folia.math.util.lengths;
import folia.math.util.js_numeric_input;
import folia.math.util.js_case;
import folia.math.util.utf16;
import std;
namespace folia::math::tex {
namespace {
using mml::MmlNode;
using mml::NodeKind;
using Arguments = std::span<TexArgument const>;
Arguments arguments(MacroInvocation const &invocation) {
    return std::get<Arguments>(invocation.arguments);
}
bool truth(TexArgument const *value) {
    if (!value || std::holds_alternative<NullArgument>(*value))
        return false;
    if (auto b = std::get_if<bool>(value))
        return *b;
    if (auto n = std::get_if<double>(value))
        return *n != 0 && !std::isnan(*n);
    return !std::get<std::u16string>(*value).empty();
}
TexArgument const *argument(Arguments args, std::size_t i) {
    return i < args.size() ? &args[i] : nullptr;
}
bool flag(Arguments args, std::size_t i) {
    return truth(argument(args, i));
}
std::u16string text(Arguments args, std::size_t i, std::u16string_view fallback = {}) {
    auto value = argument(args, i);
    if (!value)
        return std::u16string(fallback);
    if (auto s = std::get_if<std::u16string>(value))
        return *s;
    throw std::invalid_argument("Mathtools argument requires a TeX string");
}
double count(TexArgument const &value) {
    if (std::holds_alternative<NullArgument>(value))
        return 0;
    if (auto n = std::get_if<double>(&value))
        return *n;
    if (auto b = std::get_if<bool>(&value))
        return *b ? 1 : 0;
    return js_input::number(std::get<std::u16string>(value));
}
NodeDefinition attrs(std::initializer_list<mml::AttributeSet::Entry> entries) {
    NodeDefinition result;
    for (auto const &[key, value] : entries)
        result.attributes.set(key, value);
    return result;
}
NodeDefinition texClass(mml::TexClass value) {
    NodeDefinition result;
    result.properties.tex.explicit_tex_class = value;
    return result;
}
MmlNode &node(TexParser &p, NodeKind kind, std::initializer_list<MmlNode *> children = {},
              NodeDefinition def = {}) {
    return p.create(kind, {children.begin(), children.size()}, std::move(def));
}
DispatchOutcome push(TexParser &p, MmlNode &value) {
    p.Push(&value);
    return NoParseResult{};
}
DispatchOutcome pushNull(TexParser &p) {
    p.Push(p.itemFactory().create(StackKind::null_item));
    return NoParseResult{};
}
std::u16string beginName(FrameHandle const &begin) {
    return std::u16string(begin->getName().value_or(u"undefined"));
}
std::u16string required(TexParser &p, std::u16string_view name) {
    return p.GetArgument(name).value();
}
std::u16string trim(std::u16string_view text) {
    while (!text.empty() && js_input::isWhitespace(text.front()))
        text.remove_prefix(1);
    while (!text.empty() && js_input::isWhitespace(text.back()))
        text.remove_suffix(1);
    return std::u16string(text);
}
MathtoolsOptions &options(TexParser &p) {
    if (!p.options().mathtools)
        throw std::logic_error("Mathtools is not configured");
    return *p.options().mathtools;
}
[[noreturn]] void error(std::string id, std::u16string message,
                        std::initializer_list<std::u16string> args = {}) {
    throw TexError(std::move(id), std::move(message), {args.begin(), args.size()});
}
MmlNode &parse(TexParser &p, std::u16string source) {
    auto *result =
        TexParser(std::move(source), {.environment = p.stack.env()}, p.configuration).mml();
    if (!result)
        throw std::logic_error("Mathtools argument parser returned no MML");
    return *result;
}
MacroHandlerId binding(TexParser &p, MacroHandler handler) {
    auto id = p.configuration.callbacks().macroBinding(handler);
    if (!id.value)
        throw std::logic_error("Mathtools macro callback is absent from the fixed profile");
    return id;
}
MathtoolsTags &tagForm(TexParser &p, std::u16string_view name) {
    auto *tags = dynamic_cast<MathtoolsTags *>(p.tags().tagForm());
    if (!tags)
        error("TagsNotMT", u"%1 can only be used with ams or mathtools tags",
              {std::u16string(name)});
    return *tags;
}
MacroInvocation named(std::u16string_view name) {
    return {name, name, Arguments{}};
}
TexArgument argCount(TexParser &p, std::u16string_view name) {
    auto n = NewcommandUtil::GetArgCount(p, name);
    // Source GetBrackets without a fallback returns undefined, so the called
    // PairedDelimiters parameter default applies. Explicit [] stays a string.
    return n ? TexArgument{std::move(*n)} : TexArgument{1.0};
}
std::u16string em(double value) {
    auto s = lengths::em(value);
    return {s.begin(), s.end()};
}
} // namespace
std::array<HandlerRegistration, 1> const MathtoolsMethods::LEGACYCONFIG{
    {{HandlerKind::macro, {u"mathtools-legacycolonsymbols"}, nullptr, LEGACYPRIORITY}}};
StackEntry MathtoolsMethods::MtMatrix(TexParser &p, FrameHandle const &begin, Arguments args) {
    auto align = p.GetBrackets(u"\\begin{" + beginName(begin) + u"}", u"c").value();
    std::vector<TexArgument> forwarded(args.begin(), args.end());
    forwarded.resize(3, NullArgument{});
    forwarded[2] = std::move(align);
    return Array(p, begin, forwarded);
}
StackEntry MathtoolsMethods::MtSmallMatrix(TexParser &p, FrameHandle const &begin, Arguments args) {
    auto align = flag(args, 2) ? text(args, 2)
                               : p.GetBrackets(u"\\begin{" + beginName(begin) + u"}",
                                               options(p).smallmatrixAlign)
                                     .value();
    std::vector<TexArgument> forwarded(args.begin(), args.end());
    forwarded.resize(7, NullArgument{});
    forwarded[2] = std::move(align);
    auto spacing = UnitUtil::instance().em(1.0 / 3);
    forwarded[3] = std::u16string(spacing.begin(), spacing.end());
    forwarded[4] = std::u16string(u".2em");
    forwarded[5] = std::u16string(u"S");
    forwarded[6] = 1.0;
    return Array(p, begin, forwarded);
}
StackEntry MathtoolsMethods::MtMultlined(TexParser &p, FrameHandle const &begin, Arguments) {
    auto name = u"\\begin{" + beginName(begin) + u"}";
    auto &opts = options(p);
    auto pos = opts.multlinedPos.value_or(u"");
    if (pos.empty())
        pos = u"c";
    auto width = opts.multlinedWidth.value_or(u"");
    if (!p.nextIsSpace()) {
        auto arg = p.GetBrackets(name, pos).value();
        if (regexp_host::RegExp(u"^[ctb]$").exec(arg)) {
            pos = std::move(arg);
            width = p.nextIsSpace() ? u"" : p.GetBrackets(name, u"").value();
        } else
            width = std::move(arg);
        if (!width.empty() && !UnitUtil::instance().matchDimen(width))
            error("BadWidth", u"Width for %1 must be a dimension", {name});
    }
    p.Push(begin);
    auto item = p.itemFactory().create(StackKind::multlined, MultlinedItemInit{});
    auto &array = dynamic_cast<ArrayItem &>(*item);
    array.arraydef = attrs({{u"displaystyle", true},
                            {u"rowspacing", std::u16string(u".5em")},
                            {u"width", width.empty() ? u"auto" : width},
                            {u"columnwidth", std::u16string(u"100%")}})
                         .attributes;
    parse_util::setArrayAlign(array, pos);
    return item;
}
DispatchOutcome MathtoolsMethods::HandleShove(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto *top = p.stack.Top();
    if (!top || (top->kind() != StackKind::multline && top->kind() != StackKind::multlined))
        error("CommandInMultlined",
              u"%1 can only appear within the multline or multlined environments",
              {std::u16string(name)});
    if (top->Size())
        error("CommandAtTheBeginingOfLine", u"%1 must come at the beginning of the line",
              {std::u16string(name)});
    auto shove = text(arguments(invocation), 0);
    dynamic_cast<ArrayItem &>(*top).shove = shove;
    auto shift = p.GetBrackets(name);
    auto *result = p.ParseArg(name);
    if (shift && !shift->empty()) {
        auto &space = node(p, NodeKind::mspace, {}, attrs({{u"width", *shift}}));
        result = shove == u"left" ? &node(p, NodeKind::mrow, {&space, result})
                                  : &node(p, NodeKind::mrow, {result, &space});
    }
    return push(p, *result);
}
StackEntry MathtoolsMethods::SpreadLines(TexParser &p, FrameHandle const &begin, Arguments) {
    auto name = beginName(begin);
    if (p.stack.env().closing_environment == name) {
        p.stack.env().closing_environment.reset();
        auto top = p.stack.Pop();
        auto &result = top->toMml();
        auto marker = dynamic_cast<BeginItem *>(top.get());
        auto apply = [&](MmlNode &node) {
            // A mismatched end can pop another frame. Source reads an absent
            // spread then ignores it for non-tables, before reporting the open
            // environment at the normal stack boundary.
            if (node.isKind(NodeKind::mtable) && (!marker || !marker->spread))
                throw std::invalid_argument("Table spread dimension is absent");
            MathtoolsUtil::spreadLines(node, marker ? marker->spread.value_or(u"") : u"");
        };
        if (result.isInferred()) {
            auto children = result.children();
            for (auto *child : children)
                if (child)
                    apply(*child);
        } else
            apply(result);
        p.Push(&result);
    } else {
        auto spread = p.GetDimen(u"\\begin{" + name + u"}");
        auto &item = dynamic_cast<BeginItem &>(*begin);
        item.spread = std::move(spread);
        item.nestStart = true;
        parse_util::checkEqnEnv(p);
        p.Push(begin);
    }
    return std::monostate{};
}
StackEntry MathtoolsMethods::Cases(TexParser &p, FrameHandle const &begin, Arguments args) {
    auto frame = p.itemFactory().create(StackKind::array);
    auto &array = dynamic_cast<ArrayItem &>(*frame);
    array.casesEnv = beginName(begin);
    array.arraydef = attrs({{u"rowspacing", std::u16string(u".2em")},
                            {u"columnspacing", std::u16string(u"1em")},
                            {u"columnalign", std::u16string(u"left")}})
                         .attributes;
    if (text(args, 2) == u"D")
        array.arraydef.set(u"displaystyle", true);
    if (auto value = argument(args, 0); value && !std::holds_alternative<NullArgument>(*value))
        array.open = text(args, 0);
    if (auto value = argument(args, 1); value && !std::holds_alternative<NullArgument>(*value))
        array.close = text(args, 1);
    p.Push(begin);
    return frame;
}
DispatchOutcome MathtoolsMethods::MathLap(TexParser &p, MacroInvocation const &invocation) {
    auto style = trim(p.GetBrackets(invocation.dispatch_token, u"").value());
    auto args = arguments(invocation);
    auto pos = text(args, 0);
    auto definition = attrs({{u"width", 0.0}});
    if (pos != u"r")
        definition.attributes.set(u"lspace", pos == u"l" ? u"-1width" : u"-.5width");
    auto &padded =
        node(p, NodeKind::mpadded, {p.ParseArg(invocation.dispatch_token)}, std::move(definition));
    auto &styled = node(p, NodeKind::mstyle, {&padded}, attrs({{u"data-cramped", flag(args, 1)}}));
    MathtoolsUtil::setDisplayLevel(styled, style);
    return push(p, node(p, NodeKind::tex_atom, {&styled}));
}
DispatchOutcome MathtoolsMethods::Cramped(TexParser &p, MacroInvocation const &invocation) {
    auto style = trim(p.GetBrackets(invocation.dispatch_token, u"").value());
    auto &result = node(p, NodeKind::mstyle, {p.ParseArg(invocation.dispatch_token)},
                        attrs({{u"data-cramped", true}}));
    MathtoolsUtil::setDisplayLevel(result, style);
    return push(p, result);
}
DispatchOutcome MathtoolsMethods::MtLap(TexParser &p, MacroInvocation const &invocation) {
    auto content =
        parse_util::internalMath(p, required(p, invocation.dispatch_token), ScriptLevel{0.0});
    auto &result = p.create(NodeKind::mpadded, content, attrs({{u"width", 0.0}}));
    auto pos = text(arguments(invocation), 0);
    if (pos != u"r")
        result.attributes().set(u"lspace", pos == u"l" ? u"-1width" : u"-.5width");
    return push(p, result);
}
DispatchOutcome MathtoolsMethods::MathMakeBox(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto width = p.GetBrackets(name);
    auto pos = p.GetBrackets(name, u"c").value();
    auto &result = node(p, NodeKind::mpadded, {p.ParseArg(name)});
    if (width && !width->empty())
        result.attributes().set(u"width", *width);
    auto lower = js_case::lowerCase(pos);
    if (lower == u"c" || lower == u"r")
        result.attributes().set(u"data-align", lower == u"c" ? u"center" : u"right");
    if (lower != pos)
        result.attributes().set(u"data-overflow", u"linebreak");
    return push(p, result);
}
DispatchOutcome MathtoolsMethods::MathMBox(TexParser &p, MacroInvocation const &invocation) {
    return push(p, node(p, NodeKind::mrow, {p.ParseArg(invocation.dispatch_token)}));
}
DispatchOutcome MathtoolsMethods::UnderOverBracket(TexParser &p,
                                                   MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto thickness = p.GetBrackets(name, u".1em").value();
    auto height = p.GetBrackets(name, u".2em").value();
    auto arg = required(p, name);
    bool over = name.size() > 1 && name[1] == u'o';
    auto t = em(lengths::length2em(utf16::encode(thickness), {.size_em = .1}));
    auto &base = parse(p, arg);
    auto &copy = parse(p, arg);
    auto &phantom = node(p, NodeKind::mphantom, {&copy});
    auto style = u"border: " + t + u" solid; border-" + (over ? u"bottom" : u"top") + u": none";
    auto &script = node(p, NodeKind::mpadded, {&phantom},
                        attrs({{u"style", style}, {u"height", height}, {u"depth", 0.0}}));
    auto &result = parse_util::underOver(p, base, script, over ? u"over" : u"under", true);
    result.children()[0]->children()[0]->attributes().set(over ? u"accent" : u"accentunder", true);
    return push(p, result);
}
DispatchOutcome MathtoolsMethods::Aboxed(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto &top = MathtoolsUtil::checkAlignment(p, name);
    if (top.row.size() % 2)
        top.row.push_back(&node(p, NodeKind::mtd));
    auto arg = required(p, name);
    std::u16string rest(p.string().substr(p.position()));
    p.setString(arg + u"&&\\endAboxed");
    p.setPosition(0);
    auto left = p.GetUpTo(name, u"&");
    auto right = p.GetUpTo(name, u"&");
    p.GetUpTo(name, u"\\endAboxed");
    auto args = arguments(invocation);
    bool math = argument(args, 1) ? flag(args, 1) : true;
    auto box = text(args, 0, u"boxed");
    auto templ = u"\\rlap{\\" + box + u"{" + (math ? u"" : u"$\\displaystyle{") + u"#1{}#2" +
                 (math ? u"" : u"}$") + u"}}\\kern.267em\\phantom{#1}&\\phantom{{}#2}\\kern.267em";
    std::array<std::u16string, 2> parts{left, right};
    p.setString(parse_util::substituteArgs(p, parts, templ) + rest);
    p.setPosition(0);
    return NoParseResult{};
}
DispatchOutcome MathtoolsMethods::MakeAboxedCommand(TexParser &p,
                                                    MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto star = p.GetStar();
    auto cs = NewcommandUtil::GetCSname(p, name);
    auto box = NewcommandUtil::GetCSname(p, std::u16string(name) + u"\\" + cs);
    if (!std::holds_alternative<std::monostate>(
            p.configuration.handlers.get(HandlerKind::macro)->lookup(cs)))
        error("AlreadyDefined", u"%1 is already defined", {u"\\" + cs});
    p.configuration.packageData.defineCommand(
        cs, CommandDefinition{cs, NativeMacroAlias{binding(p, &Aboxed), {box, star}}});
    return pushNull(p);
}
DispatchOutcome MathtoolsMethods::ArrowBetweenLines(TexParser &p,
                                                    MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto &top = MathtoolsUtil::checkAlignment(p, name);
    if (top.Size() || !top.row.empty())
        error("BetweenLines", u"%1 must be on a row by itself", {std::u16string(name)});
    auto star = p.GetStar();
    auto symbol = p.GetBrackets(name, u"\\Updownarrow").value();
    if (star) {
        top.EndEntry();
        top.EndEntry();
    }
    auto &result = parse(p, star ? u"\\quad" + symbol : symbol + u"\\quad");
    p.Push(&result);
    top.EndEntry();
    top.EndRow();
    return NoParseResult{};
}
DispatchOutcome MathtoolsMethods::VDotsWithin(TexParser &p, MacroInvocation const &invocation) {
    auto arg = u"\\mmlToken{mi}{}" + required(p, invocation.dispatch_token) + u"\\mmlToken{mi}{}";
    auto &base = parse(p, std::move(arg));
    auto &character = p.createText(u"\u22EE");
    auto &mo = node(p, NodeKind::mo, {&character});
    auto &dots = node(p, NodeKind::mpadded, {&mo},
                      attrs({{u"width", 0.0}, {u"lspace", std::u16string(u"-.5width")}}));
    auto &phantom = node(p, NodeKind::mphantom, {&base});
    return push(p, node(p, NodeKind::mpadded, {&dots, &phantom},
                        attrs({{u"lspace", std::u16string(u".5width")}})));
}
DispatchOutcome MathtoolsMethods::ShortVDotsWithin(TexParser &p, MacroInvocation const &) {
    auto *top = dynamic_cast<ArrayItem *>(p.stack.Top());
    auto star = p.GetStar();
    if (top) {
        FlushSpaceAbove(p, named(u"\\MTFlushSpaceAbove"));
        if (!star)
            top->EndEntry();
    }
    VDotsWithin(p, named(u"\\vdotswithin"));
    if (top) {
        if (star)
            top->EndEntry();
        FlushSpaceBelow(p, named(u"\\MTFlushSpaceBelow"));
    }
    return NoParseResult{};
}
DispatchOutcome MathtoolsMethods::FlushSpaceAbove(TexParser &p, MacroInvocation const &invocation) {
    auto &top = MathtoolsUtil::checkAlignment(p, invocation.dispatch_token);
    top.flushspaceabove = top.table.size();
    top.addRowSpacing(u"-" + options(p).shortvdotsAdjustAbove.value());
    return NoParseResult{};
}
DispatchOutcome MathtoolsMethods::FlushSpaceBelow(TexParser &p, MacroInvocation const &invocation) {
    auto &top = MathtoolsUtil::checkAlignment(p, invocation.dispatch_token);
    if (top.Size())
        top.EndEntry();
    top.EndRow();
    top.addRowSpacing(u"-" + options(p).shortvdotsAdjustBelow.value());
    return NoParseResult{};
}
DispatchOutcome MathtoolsMethods::PairedDelimiters(TexParser &p,
                                                   MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto args = arguments(invocation);
    auto star = p.GetStar();
    auto size = star ? std::u16string{} : p.GetBrackets(name).value_or(u"");
    auto open = text(args, 0), close = text(args, 1), body = text(args, 2, u"#1");
    TexArgument defaultCount{1.0};
    auto const *n = argument(args, 3);
    if (!n)
        n = &defaultCount;
    auto pre = text(args, 4), post = text(args, 5);
    std::u16string left, right, after, delim;
    if (star) {
        left = u"\\mathopen{\\left";
        right = u"\\right";
        after = u"}\\mathclose{}";
        delim = u"\\middle";
    } else if (!size.empty()) {
        left = size + u"l";
        right = size + u"r";
        delim = size;
    }
    if (truth(n)) {
        std::vector<std::u16string> substitutions;
        for (double i = 0; i < count(*n); ++i)
            substitutions.push_back(required(p, name));
        pre = parse_util::substituteArgs(p, substitutions, pre);
        body = parse_util::substituteArgs(p, substitutions, body);
        post = parse_util::substituteArgs(p, substitutions, post);
    }
    std::u16string replaced;
    std::size_t start = 0;
    for (auto pos = body.find(u"\\delimsize"); pos != std::u16string::npos;
         pos = body.find(u"\\delimsize", start)) {
        replaced.append(body, start, pos - start);
        replaced += delim;
        start = pos + 10;
    }
    replaced.append(body, start, std::u16string::npos);
    std::u16string expanded;
    for (auto const &part :
         std::array<std::u16string, 9>{pre, left, open, replaced, right, close, after, post,
                                       std::u16string(p.string().substr(p.position()))})
        expanded = parse_util::addArgs(p, expanded, part);
    p.setString(std::move(expanded));
    p.setPosition(0);
    parse_util::checkMaxMacros(p);
    return NoParseResult{};
}
DispatchOutcome MathtoolsMethods::DeclarePairedDelimiter(TexParser &p,
                                                         MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto cs = NewcommandUtil::GetCsNameArgument(p, name);
    auto open = required(p, name), close = required(p, name);
    MathtoolsUtil::addPairedDelims(p, std::move(cs), {open, close});
    return pushNull(p);
}
DispatchOutcome MathtoolsMethods::DeclarePairedDelimiterX(TexParser &p,
                                                          MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto cs = NewcommandUtil::GetCsNameArgument(p, name);
    auto n = argCount(p, name);
    auto open = required(p, name), close = required(p, name), body = required(p, name);
    MathtoolsUtil::addPairedDelims(p, std::move(cs), {open, close, body, std::move(n)});
    return pushNull(p);
}
DispatchOutcome MathtoolsMethods::DeclarePairedDelimiterXPP(TexParser &p,
                                                            MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto cs = NewcommandUtil::GetCsNameArgument(p, name);
    auto n = argCount(p, name);
    auto pre = required(p, name), open = required(p, name), close = required(p, name),
         post = required(p, name), body = required(p, name);
    MathtoolsUtil::addPairedDelims(p, std::move(cs), {open, close, body, std::move(n), pre, post});
    return pushNull(p);
}
DispatchOutcome MathtoolsMethods::CenterColon(TexParser &p, MacroInvocation const &invocation) {
    auto args = arguments(invocation);
    auto &opts = options(p);
    auto &colon = p.createToken(NodeKind::mo, u":");
    if (flag(args, 0) && (opts.centercolon.value_or(false) || flag(args, 1))) {
        auto dy = opts.centercolonOffset.value();
        auto def = attrs({{u"voffset", dy},
                          {u"height", std::u16string(u"+") + dy},
                          {u"depth", std::u16string(u"-") + dy}});
        if (flag(args, 2)) {
            def.attributes.set(u"width", opts.thincolonDw.value());
            def.attributes.set(u"lspace", opts.thincolonDx.value());
        }
        return push(p, node(p, NodeKind::mpadded, {&colon}, std::move(def)));
    }
    return push(p, colon);
}
DispatchOutcome MathtoolsMethods::Relation(TexParser &p, MacroInvocation const &invocation) {
    auto args = arguments(invocation);
    auto tex = text(args, 0), unicode = text(args, 1);
    if (options(p).useUnicode.value_or(false) && !unicode.empty())
        return push(p, p.createToken(NodeKind::mo, unicode, texClass(mml::TexClass::rel)));
    std::u16string replaced = u"\\mathrel{";
    for (auto c : tex) {
        if (c == u':')
            replaced += u"\\MTThinColon";
        else if (c == u'-')
            replaced += u"\\mathrel{-}";
        else
            replaced += c;
    }
    replaced += u"}";
    p.setString(parse_util::addArgs(p, replaced, p.string().substr(p.position())));
    p.setPosition(0);
    return NoParseResult{};
}
DispatchOutcome MathtoolsMethods::NArrow(TexParser &p, MacroInvocation const &invocation) {
    auto args = arguments(invocation);
    auto c = text(args, 0), dy = text(args, 1);
    auto &base = p.createToken(NodeKind::mtext, c);
    auto &space = node(p, NodeKind::mspace, {},
                       attrs({{u"height", std::u16string(u".2em")},
                              {u"depth", 0.0},
                              {u"width", std::u16string(u".4em")}}));
    auto &strike = node(p, NodeKind::menclose, {&space},
                        attrs({{u"notation", std::u16string(u"updiagonalstrike")},
                               {u"data-thickness", std::u16string(u".05em")},
                               {u"data-padding", 0.0}}));
    auto &raised =
        node(p, NodeKind::mpadded, {&strike},
             attrs({{u"width", 0.0}, {u"lspace", std::u16string(u"-.5width")}, {u"voffset", dy}}));
    auto &copy = p.createToken(NodeKind::mtext, c);
    auto &phantom = node(p, NodeKind::mphantom, {&copy});
    auto &overlay = node(p, NodeKind::mpadded, {&raised, &phantom},
                         attrs({{u"width", 0.0}, {u"lspace", std::u16string(u"-.5width")}}));
    return push(p, node(p, NodeKind::tex_atom, {&base, &overlay}, texClass(mml::TexClass::rel)));
}
DispatchOutcome MathtoolsMethods::SplitFrac(TexParser &p, MacroInvocation const &invocation) {
    auto *num = p.ParseArg(invocation.dispatch_token);
    auto *den = p.ParseArg(invocation.dispatch_token);
    auto &blank1 = p.createToken(NodeKind::mi, std::nullopt);
    auto &space1 =
        p.createToken(NodeKind::mspace, std::nullopt, attrs({{u"width", std::u16string(u"1em")}}));
    auto &numerator =
        node(p, NodeKind::mstyle, {num, &blank1, &space1}, attrs({{u"scriptlevel", 0.0}}));
    auto &space2 =
        p.createToken(NodeKind::mspace, std::nullopt, attrs({{u"width", std::u16string(u"1em")}}));
    auto &blank2 = p.createToken(NodeKind::mi, std::nullopt);
    auto &denominator =
        node(p, NodeKind::mstyle, {&space2, &blank2, den}, attrs({{u"scriptlevel", 0.0}}));
    auto &fraction = node(p, NodeKind::mfrac, {&numerator, &denominator},
                          attrs({{u"linethickness", 0.0},
                                 {u"numalign", std::u16string(u"left")},
                                 {u"denomalign", std::u16string(u"right")}}));
    return push(
        p, node(p, NodeKind::mstyle, {&fraction},
                attrs({{u"displaystyle", flag(arguments(invocation), 0)}, {u"scriptlevel", 0.0}})));
}
DispatchOutcome MathtoolsMethods::XMathStrut(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto dd = p.GetBrackets(name);
    auto dh = MathtoolsUtil::plusOrMinus(name, required(p, name));
    auto depth = MathtoolsUtil::plusOrMinus(name, dd && !dd->empty() ? *dd : dh);
    auto &paren = p.createToken(NodeKind::mo, u"(", attrs({{u"stretchy", false}}));
    auto &phantom = node(p, NodeKind::mphantom, {&paren});
    auto &padded =
        node(p, NodeKind::mpadded, {&phantom},
             attrs({{u"width", 0.0}, {u"height", dh + u"height"}, {u"depth", depth + u"depth"}}));
    return push(p, node(p, NodeKind::tex_atom, {&padded}, texClass(mml::TexClass::ord)));
}
DispatchOutcome MathtoolsMethods::Prescript(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto &sup = MathtoolsUtil::getScript(p, name, u"sup");
    auto &sub = MathtoolsUtil::getScript(p, name, u"sub");
    auto &base = MathtoolsUtil::getScript(p, name, u"arg");
    if (sup.isKind(NodeKind::none) && sub.isKind(NodeKind::none))
        return push(p, base);
    auto &result = node(p, NodeKind::mmultiscripts, {&base});
    NodeUtil::setChild(result, 1, nullptr);
    NodeUtil::setChild(result, 2, nullptr);
    auto &marker = node(p, NodeKind::mprescripts);
    result.document().append(result, marker);
    result.document().append(result, sub);
    result.document().append(result, sup);
    result.properties().tex.fix_prescript = true;
    return push(p, result);
}
DispatchOutcome MathtoolsMethods::NewTagForm(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto &tags = tagForm(p, name);
    auto id = trim(required(p, name));
    if (id.empty())
        error("InvalidTagFormID", u"Tag form name can't be empty");
    auto format = p.GetBrackets(name, u"").value();
    auto left = required(p, name), right = required(p, name);
    if (!flag(arguments(invocation), 0) && tags.mtFormats.contains(id))
        error("DuplicateTagForm", u"Duplicate tag form: %1", {id});
    tags.mtFormats.insert_or_assign(std::move(id), MathtoolsTags::Format{left, right, format});
    return pushNull(p);
}
DispatchOutcome MathtoolsMethods::UseTagForm(TexParser &p, MacroInvocation const &invocation) {
    auto name = invocation.dispatch_token;
    auto &tags = tagForm(p, name);
    auto id = trim(required(p, name));
    if (id.empty())
        tags.mtCurrent.reset();
    else {
        auto found = tags.mtFormats.find(id);
        if (found == tags.mtFormats.end())
            error("UndefinedTagForm", u"Undefined tag form: %1", {id});
        tags.mtCurrent = found->second;
    }
    return pushNull(p);
}
DispatchOutcome MathtoolsMethods::SetOptions(TexParser &p, MacroInvocation const &invocation) {
    auto &settings = options(p);
    const auto name = invocation.dispatch_token;
    if (!settings.allowMathtoolsset.value_or(false))
        error("ForbiddenMathtoolsSet", u"%1 is disabled", {std::u16string(name)});
    using StringMember = std::optional<std::u16string> MathtoolsOptions::*;
    static constexpr std::array<std::pair<std::u16string_view, StringMember>, 14> strings{
        {{u"multlined-gap", &MathtoolsOptions::multlinedGap},
         {u"multlined-pos", &MathtoolsOptions::multlinedPos},
         {u"multlined-width", &MathtoolsOptions::multlinedWidth},
         {u"firstline-afterskip", &MathtoolsOptions::firstlineAfterskip},
         {u"lastline-preskip", &MathtoolsOptions::lastlinePreskip},
         {u"smallmatrix-align", &MathtoolsOptions::smallmatrixAlign},
         {u"shortvdotsadjustabove", &MathtoolsOptions::shortvdotsAdjustAbove},
         {u"shortvdotsadjustbelow", &MathtoolsOptions::shortvdotsAdjustBelow},
         {u"centercolon-offset", &MathtoolsOptions::centercolonOffset},
         {u"thincolon-dx", &MathtoolsOptions::thincolonDx},
         {u"thincolon-dw", &MathtoolsOptions::thincolonDw},
         {u"prescript-sub-format", &MathtoolsOptions::prescriptSubFormat},
         {u"prescript-sup-format", &MathtoolsOptions::prescriptSupFormat},
         {u"prescript-arg-format", &MathtoolsOptions::prescriptArgFormat}}};
    std::vector<KeyValueOption> allowed;
    for (auto const &[key, member] : strings)
        if ((settings.*member).has_value())
            allowed.push_back({key, std::nullopt});
    if (settings.centercolon)
        allowed.push_back({u"centercolon", std::nullopt});
    if (settings.useUnicode)
        allowed.push_back({u"use-unicode", std::nullopt});
    if (settings.legacycolonsymbols)
        allowed.push_back({u"legacycolonsymbols", std::nullopt});
    // Source misspells its excluded name as pariedDelimiters. The real key is
    // recognized here, but scalar replacement of its typed map is not a native API.
    if (settings.pairedDelimiters)
        allowed.push_back({u"pairedDelimiters", std::nullopt});
    auto changes = parse_util::keyvalOptions(required(p, name),
                                             std::span<KeyValueOption const>(allowed), true);
    auto invalid = [&](std::u16string_view key) {
        error("InvalidValue", u"Invalid value for option %1", {std::u16string(key)});
    };
    auto nextFlag = [&](mml::AttributeValue const &value) {
        if (auto boolean = std::get_if<bool>(&value))
            return *boolean;
        if (auto text = std::get_if<std::u16string>(&value))
            return !text->empty();
        throw std::logic_error("Unexpected Mathtools key-value parser result");
    };
    for (auto const &[key, value] : changes.ownEntries()) {
        if (key == u"legacycolonsymbols") {
            std::variant<bool, std::u16string> next;
            if (auto boolean = std::get_if<bool>(&value))
                next = *boolean;
            else if (auto text = std::get_if<std::u16string>(&value))
                next = *text;
            else
                throw std::logic_error("Unexpected Mathtools key-value parser result");
            if (settings.legacycolonsymbols.value() != next) {
                const bool active = std::visit(
                    [](auto const &previous) {
                        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(previous)>, bool>)
                            return previous;
                        else
                            return !previous.empty();
                    },
                    settings.legacycolonsymbols.value());
                if (active)
                    p.configuration.handlers.remove(LEGACYCONFIG);
                else
                    p.configuration.handlers.add(LEGACYCONFIG);
            }
            settings.legacycolonsymbols = std::move(next);
        } else if (key == u"centercolon")
            settings.centercolon = nextFlag(value);
        else if (key == u"use-unicode")
            settings.useUnicode = nextFlag(value);
        else if (key == u"pairedDelimiters")
            invalid(key);
        else {
            auto field = std::ranges::find(strings, key, &decltype(strings)::value_type::first);
            if (field == strings.end())
                throw std::logic_error("Unrecognized Mathtools field");
            auto text = std::get_if<std::u16string>(&value);
            if (!text)
                invalid(key);
            settings.*(field->second) = *text;
        }
    }
    return pushNull(p);
}
} // namespace folia::math::tex
