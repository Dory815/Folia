// MathJax 4.1.3 input/tex/base/BaseItems.ts; Apache-2.0.
module folia.math.input.tex.parser;
import :base_items;
import :parse_options;
import :tex_parser;
import :tags;
import :map_handler;
import folia.math.input.tex.node_util;
import folia.math.input.tex.parse_util;
import folia.math.input.tex.unit_util;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.util.js_number;
import folia.math.util.regexp_host;
import folia.math.util.utf16;
import std;

namespace folia::math::tex {
namespace {
using mml::AttributeSet;
using mml::AttributeValue;
using mml::MmlNode;
using mml::NodeKind;
using mml::TexClass;

AttributeSet attributes(std::initializer_list<AttributeSet::Entry> values) {
    AttributeSet result;
    for (auto const &[name, value] : values)
        result.set(name, value);
    return result;
}
MmlNode &node(BaseItem &item, NodeKind kind, std::initializer_list<MmlNode *> children,
              AttributeSet attrs = {}) {
    return item.create(kind, {children.begin(), children.size()}, std::move(attrs));
}
CheckResult replay(std::initializer_list<ReplayEntry> entries) {
    return CheckResult(std::vector<ReplayEntry>(entries));
}
[[noreturn]] void error(std::string id, std::u16string message,
                        std::initializer_list<std::u16string> arguments = {}) {
    throw TexError(std::move(id), std::move(message), {arguments.begin(), arguments.size()});
}
std::u16string nameOf(BaseItem const &item) {
    const auto name = item.getName();
    return name ? std::u16string(*name) : std::u16string(u"undefined");
}
bool truthy(AttributeValue const *value) {
    if (!value)
        return false;
    return std::visit(
        [](auto const &v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::same_as<T, mml::AttributeNull>)
                return false;
            else if constexpr (std::same_as<T, std::u16string>)
                return !v.empty();
            else if constexpr (std::same_as<T, double>)
                return v != 0 && !std::isnan(v);
            else
                return v;
        },
        *value);
}
std::u16string text(AttributeValue const *value) {
    if (!value)
        return u"undefined";
    return std::visit(
        [](auto const &v) -> std::u16string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::same_as<T, mml::AttributeNull>)
                return u"null";
            else if constexpr (std::same_as<T, std::u16string>)
                return v;
            else if constexpr (std::same_as<T, bool>)
                return v ? u"true" : u"false";
            else {
                auto s = js_number::toString(v);
                return {s.begin(), s.end()};
            }
        },
        *value);
}
std::u16string numeric(double value) {
    auto s = js_number::toString(value);
    return {s.begin(), s.end()};
}
std::u16string em(double value) {
    auto s = UnitUtil::instance().em(value);
    return {s.begin(), s.end()};
}
std::vector<std::u16string> split(std::u16string_view value) {
    std::vector<std::u16string> result;
    std::size_t start = 0;
    for (std::size_t i = 0; i <= value.size(); ++i) {
        if (i == value.size() || value[i] == u' ') {
            result.emplace_back(value.substr(start, i - start));
            start = i + 1;
        }
    }
    return result;
}
std::u16string join(std::span<std::u16string const> values) {
    std::u16string result;
    for (auto const &value : values) {
        if (&value != values.data())
            result += u' ';
        result += value;
    }
    return result;
}
MmlNode *child(MmlNode &parent, std::size_t index) {
    auto children = parent.children();
    return index < children.size() ? children[index] : nullptr;
}
double maxBuffer(TexParser &parser) {
    return parser.options().maxBuffer.value_or(std::numeric_limits<double>::infinity());
}
std::u16string addArgs(TexParser &parser, std::u16string_view left, std::u16string_view right) {
    return parse_util::addArgs(left, right, maxBuffer(parser));
}
std::u16string at(std::vector<std::u16string> const &values, std::size_t i) {
    return i < values.size() ? values[i] : std::u16string{};
}
bool at(std::vector<bool> const &values, std::size_t i) {
    return i < values.size() && values[i];
}
bool aligned(std::vector<ArrayColumnAlignment> const &values, std::size_t i) {
    return i < values.size() && !values[i].vertical_align.empty();
}
std::u16string_view sideName(ArrayFrameRule::Edge side) {
    switch (side) {
    case ArrayFrameRule::Edge::top:
        return u"top";
    case ArrayFrameRule::Edge::bottom:
        return u"bottom";
    case ArrayFrameRule::Edge::left:
        return u"left";
    case ArrayFrameRule::Edge::right:
        return u"right";
    }
    std::unreachable();
}
} // namespace

StartItem::StartItem(StackItemFactory &f, StackGlobal &g) : BaseItem(f, true) {
    setGlobal(g);
}
StopItem::StopItem(StackItemFactory &f) : BaseItem(f, false) {}
OpenItem::OpenItem(StackItemFactory &f) : BaseItem(f, true) {}
CloseItem::CloseItem(StackItemFactory &f) : BaseItem(f, false) {}
NullItem::NullItem(StackItemFactory &f) : BaseItem(f, false) {}
PrimeItem::PrimeItem(StackItemFactory &f, MmlNode &base, MmlNode &prime)
    : BaseItem(f, false, {&base, &prime}) {}
SubsupItem::SubsupItem(StackItemFactory &f, MmlNode &base) : BaseItem(f, false, {&base}) {}
OverItem::OverItem(StackItemFactory &f) : BaseItem(f, false) {}
LeftItem::LeftItem(StackItemFactory &f, std::u16string delim)
    : BaseItem(f, true), delimiter(std::move(delim)) {}
Middle::Middle(StackItemFactory &f, std::u16string delim, std::optional<std::u16string> c)
    : BaseItem(f, false), delimiter(std::move(delim)), color(std::move(c)) {}
RightItem::RightItem(StackItemFactory &f, std::u16string delim, std::optional<std::u16string> c)
    : BaseItem(f, false), delimiter(std::move(delim)), color(std::move(c)) {}
BreakItem::BreakItem(StackItemFactory &f, std::u16string lb, bool ins)
    : BaseItem(f, false), linebreak(std::move(lb)), insert(ins) {}
BeginItem::BeginItem(StackItemFactory &f) : BaseItem(f, true) {}
EndItem::EndItem(StackItemFactory &f) : BaseItem(f, false) {}
StyleItem::StyleItem(StackItemFactory &f) : BaseItem(f, false) {}
PositionItem::PositionItem(StackItemFactory &f) : BaseItem(f, false) {}
CellItem::CellItem(StackItemFactory &f) : BaseItem(f, false) {}
MmlItem::MmlItem(StackItemFactory &f, MmlNode &n) : BaseItem(f, false, {&n}) {}
FnItem::FnItem(StackItemFactory &f, MmlNode &n) : BaseItem(f, false, {&n}) {}
NotItem::NotItem(StackItemFactory &f)
    : BaseItem(f, false), remap(f.configuration().maps().getMap(u"not_remap")) {}
NonscriptItem::NonscriptItem(StackItemFactory &f) : BaseItem(f, false) {}
DotsItem::DotsItem(StackItemFactory &f) : BaseItem(f, false) {}
ArrayItem::ArrayItem(StackItemFactory &f) : BaseItem(f, true) {}
EqnArrayItem::EqnArrayItem(StackItemFactory &f, std::u16string n, bool num, bool tag)
    : ArrayItem(f), name(std::move(n)), numbered(num), taggable(tag) {
    const auto environment = utf16::encode(name);
    f.configuration().tags->start(environment, taggable, numbered);
}
MstyleItem::MstyleItem(StackItemFactory &f, AttributeSet attrs, std::u16string n)
    : BeginItem(f), attributes(std::move(attrs)) {
    name = std::move(n);
}
EquationItem::EquationItem(StackItemFactory &f, bool num) : BaseItem(f, true), numbered(num) {
    f.configuration().tags->start("equation", true, numbered);
}

#define KIND(Type, Kind)                                                                           \
    StackKind Type::kind() const {                                                                 \
        return StackKind::Kind;                                                                    \
    }
KIND(StartItem, start)
KIND(StopItem, stop) KIND(OpenItem, open) KIND(CloseItem, close) KIND(NullItem, null_item)
    KIND(PrimeItem, prime) KIND(SubsupItem, subsup) KIND(OverItem, over) KIND(LeftItem, left)
        KIND(Middle, middle) KIND(RightItem, right) KIND(BreakItem, break_item)
            KIND(BeginItem, begin) KIND(EndItem, end) KIND(StyleItem, style)
                KIND(PositionItem, position) KIND(CellItem, cell) KIND(MmlItem, mml)
                    KIND(FnItem, fn) KIND(NotItem, not_item) KIND(NonscriptItem, nonscript)
                        KIND(DotsItem, dots) KIND(ArrayItem, array) KIND(EqnArrayItem, eqnarray)
                            KIND(MstyleItem, mstyle) KIND(EquationItem, equation)
#undef KIND
                                bool StartItem::isOpen() const noexcept {
    return true;
}
bool OpenItem::isOpen() const noexcept {
    return true;
}
bool LeftItem::isOpen() const noexcept {
    return true;
}
bool BeginItem::isOpen() const noexcept {
    return true;
}
bool ArrayItem::isOpen() const noexcept {
    return true;
}
bool EquationItem::isOpen() const noexcept {
    return true;
}
std::optional<std::u16string_view> EquationItem::getName() const noexcept {
    return name ? std::optional<std::u16string_view>(*name) : std::nullopt;
}
bool StopItem::isClose() const noexcept {
    return true;
}
bool CloseItem::isClose() const noexcept {
    return true;
}
bool OverItem::isClose() const noexcept {
    return true;
}
bool Middle::isClose() const noexcept {
    return true;
}
bool RightItem::isClose() const noexcept {
    return true;
}
bool EndItem::isClose() const noexcept {
    return true;
}
bool CellItem::isClose() const noexcept {
    return true;
}
bool MmlItem::isFinal() const noexcept {
    return true;
}
bool ArrayItem::copyEnv() const noexcept {
    return false;
}
std::optional<std::u16string_view> OverItem::getName() const noexcept {
    return name;
}
std::optional<std::u16string_view> BeginItem::getName() const noexcept {
    return name ? std::optional<std::u16string_view>(*name) : std::nullopt;
}
std::optional<std::u16string_view> EndItem::getName() const noexcept {
    return name ? std::optional<std::u16string_view>(*name) : std::nullopt;
}
std::optional<std::u16string_view> PositionItem::getName() const noexcept {
    return name ? std::optional<std::u16string_view>(*name) : std::nullopt;
}
std::optional<std::u16string_view> CellItem::getName() const noexcept {
    return name ? std::optional<std::u16string_view>(*name) : std::nullopt;
}
std::optional<StackErrorDefinition> OpenItem::getErrors(StackErrorKey key) const {
    if (key == StackErrorKey::stop)
        return StackErrorDefinition{"ExtraOpenMissingClose",
                                    u"Extra open brace or missing close brace"};
    return BaseItem::getErrors(key);
}
std::optional<StackErrorDefinition> LeftItem::getErrors(StackErrorKey key) const {
    if (key == StackErrorKey::stop)
        return StackErrorDefinition{"ExtraLeftMissingRight", u"Extra \\left or missing \\right"};
    return BaseItem::getErrors(key);
}
std::optional<StackErrorDefinition> SubsupItem::getErrors(StackErrorKey key) const {
    if (key == StackErrorKey::stop)
        return StackErrorDefinition{"MissingScript", u"Missing superscript or subscript argument"};
    if (key == StackErrorKey::sup)
        return StackErrorDefinition{"MissingOpenForSup", u"Missing open brace for superscript"};
    if (key == StackErrorKey::sub)
        return StackErrorDefinition{"MissingOpenForSub", u"Missing open brace for subscript"};
    return BaseItem::getErrors(key);
}
CheckResult StartItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::stop)) {
        auto *result = &toMml();
        if (!global().is_inner)
            result = &factory().configuration().tags->finalize(*result, env());
        return replay({factory().create(StackKind::mml, MmlItemInit{*result})});
    }
    return BaseItem::checkItem(incoming);
}
CheckResult OpenItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::close)) {
        auto &result = node(*this, NodeKind::tex_atom, {&toMml()});
        incoming->addLatexItem(result);
        return replay({factory().create(StackKind::mml, MmlItemInit{result})});
    }
    return BaseItem::checkItem(incoming);
}
CheckResult PrimeItem::checkItem(FrameHandle const &incoming) {
    const auto tops = Peek(2);
    auto &base = *tops.at(0);
    auto &prime = *tops.at(1);
    const bool supKind = base.isKind(NodeKind::msubsup) || base.isKind(NodeKind::msup);
    const bool overKind = base.isKind(NodeKind::munderover) || base.isKind(NodeKind::mover);
    const auto sup = supKind ? dynamic_cast<mml::MmlMsubsup &>(base).sup() : 2;
    const auto over = overKind ? dynamic_cast<mml::MmlMunderover &>(base).over() : 2;
    const bool isSup = supKind && !child(base, sup);
    const bool isOver =
        overKind && !child(base, over) && !base.properties().tex.sub_sup_allowed.value_or(false);
    if (!isSup && !isOver) {
        auto &result = node(*this,
                            base.properties().tex.move_sup_sub.value_or(false) ? NodeKind::mover
                                                                               : NodeKind::msup,
                            {&base, &prime});
        return replay({&result, ReuseIncoming{}});
    }
    NodeUtil::setChild(base, isSup ? sup : over, prime);
    return replay({&base, ReuseIncoming{}});
}
CheckResult SubsupItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::open) || incoming->isKind(StackKind::left))
        return CheckResult(CheckAction::accept);
    auto &top = *First();
    const auto pos = static_cast<std::size_t>(position.value());
    if (incoming->isKind(StackKind::mml)) {
        if (primes) {
            if (pos != 2)
                NodeUtil::setChild(top, 2, *primes);
            else {
                primes->properties().tex.variant_form = true;
                incoming->setFirst(node(*this, NodeKind::mrow, {primes, incoming->First()}));
            }
        }
        NodeUtil::setChild(top, pos, *incoming->First());
        if (movesupsub)
            top.properties().tex.move_sup_sub = *movesupsub;
        return replay({factory().create(StackKind::mml, MmlItemInit{top})});
    }
    BaseItem::checkItem(incoming);
    auto definition = getErrors(pos == 1 ? StackErrorKey::sub : StackErrorKey::sup).value();
    error(std::move(definition.id), std::move(definition.message));
}
CheckResult OverItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::over))
        error("AmbiguousUseOf", u"Ambiguous use of %1", {nameOf(*incoming)});
    if (!incoming->isClose())
        return BaseItem::checkItem(incoming);
    auto *result = &node(*this, NodeKind::mfrac, {numerator, &toMml(false)});
    if (thickness)
        std::visit([&](auto const &v) { result->attributes().set(u"linethickness", v); },
                   *thickness);
    if ((leftDelimiter && !leftDelimiter->empty()) ||
        (rightDelimiter && !rightDelimiter->empty())) {
        result->properties().tex.with_delimiters = true;
        result = &parse_util::fixedFence(factory().configuration(), leftDelimiter.value_or(u""),
                                         *result, rightDelimiter.value_or(u""));
    }
    result->attributes().set(u"data-latex-item", name);
    return replay({factory().create(StackKind::mml, MmlItemInit{*result}), ReuseIncoming{}});
}
std::string OverItem::toString() const {
    std::string result = "over[" + (numerator ? numerator->toString() : "undefined") + " / ";
    for (auto *value : nodes()) {
        if (value != nodes().front())
            result += "; ";
        result += value->toString();
    }
    return result + ']';
}
CheckResult LeftItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::right)) {
        auto &right = dynamic_cast<RightItem &>(*incoming);
        auto &fenced = parse_util::fenced(factory().configuration(), delimiter, toMml(),
                                          right.delimiter, u"", right.color.value_or(u""));
        addLatexItem(*fenced.children().front(), u"\\left");
        incoming->addLatexItem(*fenced.children().back(), u"\\right");
        const auto source = incoming->sourceAtCreation();
        const auto begin = std::min(startI, source.size());
        const auto end = std::clamp(incoming->stopI, begin, source.size());
        fenced.attributes().set(u"data-latex-item",
                                u"\\left" + std::u16string(source.substr(begin, end - begin)));
        return replay({factory().create(StackKind::mml, MmlItemInit{fenced})});
    }
    if (incoming->isKind(StackKind::middle)) {
        auto &mid = dynamic_cast<Middle &>(*incoming);
        auto attrs = attributes({{u"stretchy", true}, {u"symmetric", true}});
        if (mid.color && !mid.color->empty())
            attrs.set(u"mathcolor", *mid.color);
        auto &middle = createToken(NodeKind::mo, mid.delimiter, std::move(attrs));
        incoming->addLatexItem(middle, u"\\middle");
        NodeDefinition closeDef, openDef;
        closeDef.properties.tex.explicit_tex_class = TexClass::close;
        openDef.properties.tex.explicit_tex_class = TexClass::open;
        auto &close = factory().configuration().request().nodeFactory.makeNode(
            NodeKind::tex_atom, {}, std::move(closeDef));
        auto &open = factory().configuration().request().nodeFactory.makeNode(
            NodeKind::tex_atom, {}, std::move(openDef));
        const std::array<MmlNode *, 3> values{&close, &middle, &open};
        Push(values);
        setEnv(std::make_shared<TexEnvironment>());
        return replay({ReuseTop{}});
    }
    return BaseItem::checkItem(incoming);
}
CheckResult BreakItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::mml) && incoming->First()->isKind(NodeKind::mo)) {
        auto &mo = *incoming->First();
        std::u16string style;
        if (const auto definition = NodeUtil::getOp(mo)) {
            for (auto const &[key, value] : definition->properties)
                if (key == u"linebreakstyle" && truthy(&value))
                    style = text(&value);
        }
        if (style.empty())
            style = text(mo.attributes().lookup(u"linebreakstyle"));
        if (style != u"after") {
            mo.attributes().set(u"linebreak", linebreak);
            return replay({ReuseIncoming{}});
        }
        if (!insert)
            return replay({ReuseIncoming{}});
    }
    auto &space = createToken(NodeKind::mspace, u"", attributes({{u"linebreak", linebreak}}));
    return replay({factory().create(StackKind::mml, MmlItemInit{space}), ReuseIncoming{}});
}
CheckResult BeginItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::end)) {
        if (incoming->getName() != getName())
            error("EnvBadEnd", u"\\begin{%1} ended with \\end{%2}",
                  {nameOf(*this), nameOf(*incoming)});
        auto &result = toMml();
        incoming->addLatexItem(result);
        return replay({factory().create(StackKind::mml, MmlItemInit{result})});
    }
    if (incoming->isKind(StackKind::stop))
        error("EnvMissingEnd", u"Missing \\end{%1}", {nameOf(*this)});
    return BaseItem::checkItem(incoming);
}
CheckResult StyleItem::checkItem(FrameHandle const &incoming) {
    if (!incoming->isClose())
        return BaseItem::checkItem(incoming);
    auto &result = create(NodeKind::mstyle, nodes(), styles);
    return replay({factory().create(StackKind::mml, MmlItemInit{result}), ReuseIncoming{}});
}
CheckResult PositionItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isClose())
        error("MissingBoxFor", u"Missing box for %1", {nameOf(*this)});
    if (incoming->isFinal()) {
        auto &result = incoming->toMml();
        if (move == PositionMove::vertical) {
            auto attrs = AttributeSet{};
            if (dh) {
                attrs.set(u"height", *dh);
                attrs.set(u"voffset", *dh);
            }
            if (dd)
                attrs.set(u"depth", *dd);
            auto &padded = node(*this, NodeKind::mpadded, {&result}, std::move(attrs));
            return replay({factory().create(StackKind::mml, MmlItemInit{padded})});
        }
        if (move == PositionMove::horizontal)
            return replay({factory().create(StackKind::mml, MmlItemInit{*left}), ReuseIncoming{},
                           factory().create(StackKind::mml, MmlItemInit{*right})});
    }
    return BaseItem::checkItem(incoming);
}
CheckResult FnItem::checkItem(FrameHandle const &incoming) {
    auto *top = First();
    if (!top)
        return BaseItem::checkItem(incoming);
    if (incoming->isOpen())
        return CheckResult(CheckAction::accept);
    if (!incoming->isKind(StackKind::fn)) {
        auto *next = incoming->First();
        if (!incoming->isKind(StackKind::mml) || !next)
            return replay({top, ReuseIncoming{}});
        if ((next->isKind(NodeKind::mstyle) && child(*next, 0) && child(*child(*next, 0), 0) &&
             child(*child(*next, 0), 0)->isKind(NodeKind::mspace)) ||
            next->isKind(NodeKind::mspace))
            return replay({top, ReuseIncoming{}});
        if (next->isEmbellished())
            next = next->coreMO();
        const auto form = NodeUtil::getForm(*next);
        if (form && (form->texClass == TexClass::bin || form->texClass == TexClass::rel ||
                     form->texClass == TexClass::close || form->texClass == TexClass::punct))
            return replay({top, ReuseIncoming{}});
    }
    if (top->isKind(NodeKind::tex_atom) && top->isEmpty())
        return replay({top, ReuseIncoming{}});
    NodeDefinition def;
    def.properties.tex.explicit_tex_class = TexClass::none;
    auto &apply = factory().configuration().request().nodeFactory.makeToken(NodeKind::mo, u"\u2061",
                                                                            std::move(def));
    return replay({top, &apply, ReuseIncoming{}});
}
CheckResult NotItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::open) || incoming->isKind(StackKind::left))
        return CheckResult(CheckAction::accept);
    auto *first = incoming->First();
    if (incoming->isKind(StackKind::mml) && first &&
        (first->isKind(NodeKind::mo) || first->isKind(NodeKind::mi) ||
         first->isKind(NodeKind::mtext))) {
        auto chars = NodeUtil::getText(*first);
        if (chars.size() == 1 && !first->properties().tex.move_sup_sub.value_or(false) &&
            first->children().size() == 1) {
            auto &nf = factory().configuration().request().nodeFactory;
            if (remap && remap->contains(chars)) {
                const auto lookup = remap->lookup(chars);
                auto const *token = std::get<Token const *>(lookup);
                auto *replacement = nf.makeText(
                    token->character ? std::optional<std::u16string_view>(*token->character)
                                     : std::nullopt);
                if (replacement)
                    NodeUtil::setChild(*first, 0, *replacement);
            } else
                first->appendChild(*nf.makeText(u"\u0338"));
            return replay({ReuseIncoming{}});
        }
    }
    auto &nf = factory().configuration().request().nodeFactory;
    auto &slash = nf.makeNode(NodeKind::mtext, {}, {}, nf.makeText(u"\u29F8"));
    auto &padded = node(*this, NodeKind::mpadded, {&slash}, attributes({{u"width", 0.0}}));
    NodeDefinition def;
    def.properties.tex.explicit_tex_class = TexClass::rel;
    const std::array<MmlNode *, 1> contents{&padded};
    auto &result = nf.makeNode(NodeKind::tex_atom, contents, std::move(def));
    return replay({&result, ReuseIncoming{}});
}
CheckResult NonscriptItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::mml) && incoming->Size() == 1) {
        auto *value = incoming->First();
        if (value->isKind(NodeKind::mstyle) && value->notParent()) {
            value = child(*child(*value, 0), 0);
        }
        if (value->isKind(NodeKind::mspace)) {
            if (value != incoming->First()) {
                auto &row = node(*this, NodeKind::mrow, {incoming->Pop()});
                const std::array<MmlNode *, 1> list{&row};
                incoming->Push(list);
            }
            factory().configuration().addNode(u"nonscript", *incoming->First());
        }
    }
    return replay({ReuseIncoming{}});
}
CheckResult DotsItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::open) || incoming->isKind(StackKind::left))
        return CheckResult(CheckAction::accept);
    auto *dots = ldots;
    if (incoming->isKind(StackKind::mml) && incoming->First()->isEmbellished()) {
        auto cls = incoming->First()->coreMO()->texClass();
        if (cls == TexClass::bin || cls == TexClass::rel)
            dots = cdots;
    }
    return replay({dots, ReuseIncoming{}});
}
CheckResult ArrayItem::checkItem(FrameHandle const &incoming) {
    if (!incoming->isClose() || incoming->isKind(StackKind::over))
        return BaseItem::checkItem(incoming);
    auto *cell = dynamic_cast<CellItem *>(incoming.get());
    if (cell && cell->isEntry) {
        EndEntry();
        clearEnv();
        StartEntry();
        return CheckResult(CheckAction::drop);
    }
    if (cell && cell->isCR) {
        EndEntry();
        EndRow();
        clearEnv();
        StartEntry();
        return CheckResult(CheckAction::drop);
    }
    EndTable();
    clearEnv();
    auto result = factory().create(StackKind::mml, MmlItemInit{createMml()});
    if (requireClose) {
        if (incoming->isKind(StackKind::close))
            return replay({std::move(result)});
        error("MissingCloseBrace", u"Missing close brace");
    }
    return replay({std::move(result), ReuseIncoming{}});
}
MmlNode &ArrayItem::createMml() {
    std::optional<AttributeValue> level;
    if (auto *value = arraydef.own(u"scriptlevel"))
        level = *value;
    arraydef.erase(u"scriptlevel");
    auto *result = &create(NodeKind::mtable, table, arraydef);
    if (level && truthy(&*level))
        result->properties().tex.small_matrix = true;
    if (!breakAlign.table.empty())
        result->attributes().set(u"data-break-align", breakAlign.table);
    if (arrayPadding && !arrayPadding->empty()) {
        result->attributes().set(u"data-frame-styles", u"");
        result->attributes().set(u"framespacing", *arrayPadding);
    }
    result = &handleFrame(*result);
    if (level)
        result = &node(*this, NodeKind::mstyle, {result}, attributes({{u"scriptlevel", *level}}));
    if ((open && !open->empty()) || (close && !close->empty()))
        result = &parse_util::fenced(factory().configuration(), open.value_or(u""), *result,
                                     close.value_or(u""));
    return *result;
}
MmlNode &ArrayItem::handleFrame(MmlNode &tableNode) {
    if (frame.empty())
        return tableNode;
    std::vector<ArrayFrameRule> sides;
    for (auto const &rule : frame) {
        auto found = std::ranges::find(sides, rule.edge, &ArrayFrameRule::edge);
        if (found == sides.end())
            sides.push_back(rule);
        else
            found->style = rule.style;
    }
    auto style = frame.front().style.value_or(u"");
    for (auto const &rule : frame)
        if (rule.style.value_or(u"") != style)
            style.clear();
    if (!style.empty()) {
        if (frame.size() == 4) {
            tableNode.attributes().set(u"frame", style);
            tableNode.attributes().erase(u"data-frame-styles");
            return tableNode;
        }
        if (style == u"solid") {
            tableNode.attributes().set(u"data-frame-styles", u"");
            std::vector<std::u16string> notation;
            for (auto const &side : sides)
                notation.emplace_back(sideName(side.edge));
            return node(*this, NodeKind::menclose, {&tableNode},
                        attributes({{u"notation", join(notation)}, {u"data-padding", 0.0}}));
        }
    }
    std::vector<std::u16string> styles;
    for (auto edge : {ArrayFrameRule::Edge::top, ArrayFrameRule::Edge::right,
                      ArrayFrameRule::Edge::bottom, ArrayFrameRule::Edge::left}) {
        auto found = std::ranges::find(sides, edge, &ArrayFrameRule::edge);
        styles.push_back(found != sides.end() && found->style && !found->style->empty()
                             ? *found->style
                             : u"none");
    }
    tableNode.attributes().set(u"data-frame-styles", join(styles));
    return tableNode;
}
void ArrayItem::StartEntry() {
    const auto n = row.size();
    auto start = at(cstart, n), end = at(cend, n);
    const bool align = aligned(ralign, n);
    if (start.empty() && end.empty() && !align && !at(cextra, n) && !at(cextra, n + 1))
        return;
    auto [prefix, entry, term, found] = getEntry();
    if (at(cextra, n) && (!atEnd || at(cextra, n + 1)))
        start += u'&';
    if (term != u"&") {
        found = !UnitUtil::instance().trimSpaces(entry).empty() || n != 0 ||
                (!term.empty() && !term.starts_with(u"\\end"));
        if (at(cextra, n + 1) && !at(cextra, n)) {
            end += u'&';
            atEnd = true;
        }
    }
    if (!found && prefix.empty())
        return;
    if (found) {
        if (!start.empty())
            entry = addArgs(*parser, start, entry);
        if (!end.empty())
            entry = addArgs(*parser, entry, end);
        if (align)
            entry = u"\\text{" + UnitUtil::instance().trimSpaces(entry) + u"}";
        if (!start.empty() || !end.empty() || align) {
            if (++templateSubs > parser->options().maxTemplateSubtitutions.value_or(
                                     std::numeric_limits<double>::infinity()))
                error("MaxTemplateSubs", u"Maximum template substitutions exceeded; is there an "
                                         u"invalid use of \\\\ in the template?");
        }
    }
    if (!prefix.empty())
        entry = addArgs(*parser, prefix, entry);
    parser->setString(addArgs(*parser, entry, parser->string()));
    parser->setPosition(0);
}
ArrayEntryScan ArrayItem::getEntry() {
    regexp_host::RegExp pattern(uR"(^([^]*?)([&{}]|\\\\|\\(?:begin|end)\s*\{array\}|\\cr|\\))");
    regexp_host::RegExp prefixPattern(
        uR"(^(?:\s*\\(?:h(?:dash)?line|hfil{1,3}|rowcolor\s*\{.*?\}))+)");
    std::size_t braces = 0, envs = 0, i = parser->position();
    while (i <= parser->string().size()) {
        auto match = pattern.exec(std::u16string(parser->string().substr(i)));
        if (!match)
            break;
        const auto whole = match->at(0).value();
        const auto term = match->at(2).value();
        i += whole.size();
        if (term == u"\\") {
            ++i;
            continue;
        }
        if (term == u"{") {
            ++braces;
            continue;
        }
        if (term == u"}") {
            if (!braces)
                return {};
            --braces;
            continue;
        }
        if (term == u"\\begin{array}") {
            if (!braces)
                ++envs;
            continue;
        }
        if (term == u"\\end{array}" && !braces && envs) {
            --envs;
            continue;
        }
        if (braces || envs)
            continue;
        i -= term.size();
        auto entry = UnitUtil::instance().trimSpaces(
            parser->string().substr(parser->position(), i - parser->position()));
        std::u16string prefix;
        if (auto matched = prefixPattern.exec(entry)) {
            prefix = matched->at(0).value();
            entry.erase(0, prefix.size());
        }
        parser->setString(std::u16string(parser->string().substr(i)));
        parser->setPosition(0);
        return {std::move(prefix), std::move(entry), term, true};
    }
    return {};
}
void ArrayItem::EndEntry() {
    auto &cell = create(NodeKind::mtd, nodes());
    if (!hfill.empty()) {
        if (hfill.front() == 0)
            cell.attributes().set(u"columnalign", u"right");
        if (hfill.back() == Size())
            cell.attributes().set(u"columnalign", truthy(cell.attributes().lookup(u"columnalign"))
                                                      ? u"center"
                                                      : u"left");
    }
    if (aligned(ralign, row.size())) {
        const auto &align = ralign[row.size()];
        auto *inferred = child(cell, 0);
        auto contents =
            std::vector<MmlNode *>(inferred->children().begin(), inferred->children().end());
        inferred->setChildren({});
        auto &box = create(NodeKind::mpadded, contents,
                           attributes({{u"width", align.width},
                                       {u"data-overflow", std::u16string(u"auto")},
                                       {u"data-align", align.align},
                                       {u"data-vertical-align", align.vertical_align}}));
        box.properties().tex.vertical_box = align.vertical_align;
        cell.appendChild(box);
    } else if (!breakAlign.cell.empty())
        cell.attributes().set(u"data-vertical-align", breakAlign.cell);
    breakAlign.cell.clear();
    row.push_back(&cell);
    Clear();
    hfill.clear();
}
void ArrayItem::EndRow() {
    auto kind = NodeKind::mtr;
    if (isNumbered && row.size() == 3) {
        auto *tag = row.back();
        row.pop_back();
        row.insert(row.begin(), tag);
        kind = NodeKind::mlabeledtr;
    } else if (isLabeled) {
        kind = NodeKind::mlabeledtr;
        isLabeled = false;
    }
    auto &result = create(kind, row);
    if (!breakAlign.row.empty()) {
        result.attributes().set(u"data-break-align", breakAlign.row);
        breakAlign.row.clear();
    }
    addLatexItem(result);
    table.push_back(&result);
    row.clear();
    atEnd = false;
}
void ArrayItem::EndTable() {
    if (Size() || !row.empty()) {
        EndEntry();
        EndRow();
    }
    checkLines();
}
void ArrayItem::checkLines() {
    if (truthy(arraydef.own(u"rowlines"))) {
        auto lines = split(text(arraydef.own(u"rowlines")));
        if (lines.size() == table.size()) {
            frame.push_back({ArrayFrameRule::Edge::bottom, lines.back()});
            lines.pop_back();
            if (!lines.empty())
                arraydef.set(u"rowlines", join(lines));
            else
                arraydef.erase(u"rowlines");
        } else if (table.size() > 1 && lines.size() < table.size() - 1)
            arraydef.set(u"rowlines", text(arraydef.own(u"rowlines")) + u" none");
    }
    const bool hasSpacing = rowspacing && *rowspacing != 0 && !std::isnan(*rowspacing);
    if (hasSpacing) {
        auto rows = split(text(arraydef.own(u"rowspacing")));
        const auto spacing = numeric(*rowspacing);
        while (rows.size() < table.size())
            rows.push_back(spacing + u"em");
        arraydef.set(u"rowspacing", join(rows));
    }
}
void ArrayItem::addRowSpacing(std::u16string_view spacing) {
    if (!truthy(arraydef.own(u"rowspacing")))
        return;
    auto rows = split(text(arraydef.own(u"rowspacing")));
    bool hasSpacing = rowspacing && *rowspacing != 0 && !std::isnan(*rowspacing);
    if (!hasSpacing)
        rowspacing = UnitUtil::instance().dimen2em(rows.front());
    const double base = *rowspacing;
    while (rows.size() < table.size())
        rows.push_back(em(base));
    if (!table.empty()) {
        const auto sum = base + UnitUtil::instance().dimen2em(spacing);
        rows[table.size() - 1] = em(std::isnan(sum) ? sum : std::max(0.0, sum));
    }
    arraydef.set(u"rowspacing", join(rows));
}
void EqnArrayItem::EndEntry() {
    auto alignments = split(text(arraydef.own(u"columnalign")));
    auto align =
        !row.empty() && !alignments.empty() ? alignments[row.size() % alignments.size()] : u"right";
    if (align != u"right")
        parse_util::fixInitialMO(factory().configuration(), nodes());
    ArrayItem::EndEntry();
}
void EqnArrayItem::EndRow() {
    maxrow = std::max(maxrow, row.size());
    auto &tags = *factory().configuration().tags;
    if (auto *tag = tags.getTag()) {
        row.insert(row.begin(), tag);
        isLabeled = true;
    }
    tags.clearTag();
    ArrayItem::EndRow();
}
void EqnArrayItem::EndTable() {
    ArrayItem::EndTable();
    factory().configuration().tags->end();
    extendArray(u"columnalign", static_cast<std::ptrdiff_t>(maxrow));
    extendArray(u"columnwidth", static_cast<std::ptrdiff_t>(maxrow));
    extendArray(u"columnspacing", static_cast<std::ptrdiff_t>(maxrow) - 1);
    extendArray(u"data-break-align", static_cast<std::ptrdiff_t>(maxrow));
    addIndentshift();
}
void EqnArrayItem::extendArray(std::u16string_view name, std::ptrdiff_t max) {
    if (!truthy(arraydef.own(name)))
        return;
    const auto repeat = split(text(arraydef.own(name)));
    auto columns = repeat;
    if (columns.size() <= 1)
        return;
    while (max > 0 && columns.size() < static_cast<std::size_t>(max))
        columns.insert(columns.end(), repeat.begin(), repeat.end());
    auto count =
        max < 0 ? std::max<std::ptrdiff_t>(0, static_cast<std::ptrdiff_t>(columns.size()) + max)
                : std::min<std::ptrdiff_t>(max, static_cast<std::ptrdiff_t>(columns.size()));
    columns.resize(static_cast<std::size_t>(count));
    arraydef.set(std::u16string(name), join(columns));
}
void EqnArrayItem::addIndentshift() {
    const auto align = split(text(arraydef.own(u"columnalign")));
    std::u16string previous;
    for (std::size_t i = 0; i < align.size(); ++i) {
        if (align[i] == u"left" && i > 0) {
            const auto shift = previous == u"center" ? u".7em" : u"2em";
            for (auto *row : table) {
                auto *cell = child(*row, row->isKind(NodeKind::mlabeledtr) ? i + 1 : i);
                if (!cell)
                    continue;
                auto *inferred = child(*cell, 0);
                auto contents = std::vector<MmlNode *>(inferred->children().begin(),
                                                       inferred->children().end());
                inferred->setChildren({});
                auto &style = create(NodeKind::mstyle, contents,
                                     attributes({{u"indentshift", std::u16string(shift)}}));
                cell->appendChild(style);
            }
        }
        previous = align[i];
    }
}
CheckResult MstyleItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::end) && incoming->getName() == getName()) {
        return replay({&node(*this, NodeKind::mstyle, {&toMml()}, attributes)});
    }
    return BeginItem::checkItem(incoming);
}
CheckResult EquationItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::end)) {
        auto *result = &toMml();
        auto &tags = *factory().configuration().tags;
        auto *tag = tags.getTag();
        tags.end();
        if (tag)
            result = &tags.enTag(*result, *tag);
        return replay({result, ReuseIncoming{}});
    }
    if (incoming->isKind(StackKind::stop))
        error("EnvMissingEnd", u"Missing \\end{%1}", {nameOf(*this)});
    return BaseItem::checkItem(incoming);
}

namespace {
template <class Item> StackItemClass simpleClass() {
    return {[](StackBuildArguments const &args) -> std::unique_ptr<BaseItem> {
                (void)std::get<NoItemInit>(args.init);
                return std::make_unique<Item>(args.factory);
            },
            [](BaseItem const &value) { return dynamic_cast<Item const *>(&value) != nullptr; }};
}
template <class Item, class Init, class Constructor>
StackItemClass initializedClass(Constructor construct) {
    return {[construct](StackBuildArguments const &args) -> std::unique_ptr<BaseItem> {
                return construct(args.factory, std::get<Init>(args.init));
            },
            [](BaseItem const &value) { return dynamic_cast<Item const *>(&value) != nullptr; }};
}
} // namespace
StackItemCatalog baseItemClasses() {
    return {
        {StackKind::start, initializedClass<StartItem, StartItemInit>([](auto &f, auto const &a) {
             return std::make_unique<StartItem>(f, a.global);
         })},
        {StackKind::stop, simpleClass<StopItem>()},
        {StackKind::open, simpleClass<OpenItem>()},
        {StackKind::close, simpleClass<CloseItem>()},
        {StackKind::null_item, simpleClass<NullItem>()},
        {StackKind::prime, initializedClass<PrimeItem, PrimeItemInit>([](auto &f, auto const &a) {
             return std::make_unique<PrimeItem>(f, a.base, a.prime);
         })},
        {StackKind::subsup,
         initializedClass<SubsupItem, SubsupItemInit>(
             [](auto &f, auto const &a) { return std::make_unique<SubsupItem>(f, a.base); })},
        {StackKind::over, simpleClass<OverItem>()},
        {StackKind::left, initializedClass<LeftItem, LeftItemInit>([](auto &f, auto const &a) {
             return std::make_unique<LeftItem>(f, a.delimiter);
         })},
        {StackKind::middle,
         initializedClass<Middle, MiddleRightItemInit>([](auto &f, auto const &a) {
             return std::make_unique<Middle>(f, a.delimiter, a.color);
         })},
        {StackKind::right,
         initializedClass<RightItem, MiddleRightItemInit>([](auto &f, auto const &a) {
             return std::make_unique<RightItem>(f, a.delimiter, a.color);
         })},
        {StackKind::break_item,
         initializedClass<BreakItem, BreakItemInit>([](auto &f, auto const &a) {
             return std::make_unique<BreakItem>(f, a.linebreak, a.insert);
         })},
        {StackKind::begin, simpleClass<BeginItem>()},
        {StackKind::end, simpleClass<EndItem>()},
        {StackKind::style, simpleClass<StyleItem>()},
        {StackKind::position, simpleClass<PositionItem>()},
        {StackKind::cell, simpleClass<CellItem>()},
        {StackKind::mml, initializedClass<MmlItem, MmlItemInit>([](auto &f, auto const &a) {
             return std::make_unique<MmlItem>(f, a.node);
         })},
        {StackKind::fn, initializedClass<FnItem, FnItemInit>([](auto &f, auto const &a) {
             return std::make_unique<FnItem>(f, a.function);
         })},
        {StackKind::not_item, simpleClass<NotItem>()},
        {StackKind::nonscript, simpleClass<NonscriptItem>()},
        {StackKind::dots, simpleClass<DotsItem>()},
        {StackKind::array, simpleClass<ArrayItem>()},
        {StackKind::eqnarray,
         initializedClass<EqnArrayItem, EqnArrayItemInit>([](auto &f, auto const &a) {
             return std::make_unique<EqnArrayItem>(f, a.name, a.numbered, a.taggable);
         })},
        {StackKind::equation,
         initializedClass<EquationItem, EquationItemInit>(
             [](auto &f, auto const &a) { return std::make_unique<EquationItem>(f, a.numbered); })},
        {StackKind::mstyle,
         initializedClass<MstyleItem, MstyleItemInit>([](auto &f, auto const &a) {
             return std::make_unique<MstyleItem>(f, a.attributes, a.name);
         })}};
}
} // namespace folia::math::tex
