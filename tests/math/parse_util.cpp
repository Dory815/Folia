#include "math/support/tex_fixture.hpp"
import folia.math.input.tex.parse_util;
import folia.math.input.tex.base_mappings;

using namespace boost::ut;
using namespace folia::math::tex;
using namespace folia::math::tex::parse_util;

namespace {
void helperSetup(Fixture &fixture) {
    BaseMappings::initialize(fixture.maps);
    fixture.effective.numberPattern =
        std::make_shared<folia::math::regexp_host::RegExp>(u"^[0-9]+");
    fixture.effective.identifierPattern =
        std::make_shared<folia::math::regexp_host::RegExp>(u"^[a-zA-Z]+");
    const std::array registrations{
        HandlerRegistration{HandlerKind::character, {u"command", u"special"}},
        HandlerRegistration{HandlerKind::character, {u"letter", u"digit"}, nullptr, 4},
        HandlerRegistration{HandlerKind::macro,
                            {u"delimiter", u"macros", u"lcGreek", u"ucGreek", u"mathchar0mi",
                             u"mathchar0mo", u"mathchar7"}},
        HandlerRegistration{HandlerKind::delimiter, {u"delimiter"}},
        HandlerRegistration{HandlerKind::environment, {u"environment"}}};
    fixture.options->handlers.add(registrations);
}
std::u16string shape(MmlNode const &node) {
    if (node.kind() == NodeKind::text)
        return u"text(" + std::u16string(node.text()) + u")";
    auto name = nodeKindName(node.kind());
    std::u16string result(name.begin(), name.end());
    result += u'(';
    for (std::size_t i = 0; i < node.children().size(); ++i) {
        if (i)
            result += u',';
        result += node.children()[i] ? shape(*node.children()[i]) : u"_";
    }
    return result + u')';
}
thread_local std::vector<std::string> *creationCalls{};
MmlNode *traceText(NodeFactory &factory, std::optional<std::u16string_view> text) {
    creationCalls->push_back("text");
    return NodeFactory::createText(factory, text);
}
MmlNode &traceNode(NodeFactory &factory, NodeKind kind, std::span<MmlNode *const> children,
                   NodeDefinition definition, MmlNode *text) {
    creationCalls->push_back("node");
    return NodeFactory::createNode(factory, kind, children, std::move(definition), text);
}
MmlNode &unexpectedToken(NodeFactory &, NodeKind, NodeDefinition, std::u16string_view) {
    throw std::logic_error("Internal text must use text and node creators");
}
struct CreationCallsScope {
    std::vector<std::string> *previous = creationCalls;
    explicit CreationCallsScope(std::vector<std::string> &calls) {
        creationCalls = &calls;
    }
    ~CreationCallsScope() {
        creationCalls = previous;
    }
};
template <class Operation>
void expectTexError(Operation &&operation, std::string_view expected_id,
                    std::u16string_view expected_message) {
    bool caught = false;
    try {
        operation();
    } catch (TexError const &error) {
        caught = true;
        expect(error.id() == expected_id);
        expect(error.message() == expected_message);
    } catch (...) {
        expect(false) << "wrong exception type";
    }
    expect(caught) << "expected a typed TexError";
}
} // namespace

suite parse_util_tests = [] {
    "ParseUtil internal text and math match original configured parser trees"_test = [] {
        struct Case {
            std::u16string text, expected;
        };
        const std::array cases{
            Case{u"", u""},
            Case{u"  a  b\n\n c  ", u"mtext(text(\u00a0a b  c\u00a0))"},
            Case{u"a $x$ b", u"mrow(mtext(text(a\u00a0)),TeXAtom(inferredMrow(mi(text(x)))),mtext("
                             u"text(\u00a0b)))"},
            Case{u"a \\(x\\) b", u"mrow(mtext(text(a\u00a0)),TeXAtom(inferredMrow(mi(text(x)))),"
                                 u"mtext(text(\u00a0b)))"},
            Case{u"\\$\\{x\\}\\\\", u"mtext(text(${x}\\))"},
            Case{u"\\U{1F600}", u"mtext(text(😀))"},
            Case{u"\\U4A", u"mtext(text(\u0004A))"}};
        Fixture fixture;
        helperSetup(fixture);
        TexParser parser(u"", ParserEnvironmentSeed{TexEnvironment{.font = u"bold"}},
                         *fixture.options);
        for (auto const &item : cases) {
            auto nodes = internalMath(parser, item.text);
            expect(nodes.size() == (item.expected.empty() ? 0u : 1u));
            if (!nodes.empty())
                expect(shape(*nodes[0]) == item.expected);
        }
        auto level = internalMath(parser, u"", ScriptLevel{0.0});
        expect(level.size() == 1_u);
        expect(level[0]->kind() == NodeKind::mstyle);
        expect(!std::get<bool>(*level[0]->attributes().own(u"displaystyle")));
        expect(std::get<double>(*level[0]->attributes().own(u"scriptlevel")) == 0.0);
        texError([&] { internalMath(parser, u"$x"); }, "MathNotTerminated");
        texError([&] { internalMath(parser, u"\\(x"); }, "MathNotTerminated");
        texError([&] { internalMath(parser, u"\\U{xyz}"); }, "BadRawUnicode");
        expect(throws<std::out_of_range>([&] { internalMath(parser, u"\\U{110000}"); }));
        std::vector<std::string> calls;
        CreationCallsScope scope(calls);
        fixture.options->nodeFactory.setCreators(
            NodeCreators{.node = traceNode, .token = unexpectedToken, .text = traceText});
        NodeDefinition definition;
        definition.attributes.set(u"mathvariant", u"-tex-calligraphic");
        auto &node = internalText(parser, u"  a\n\n b  ", std::move(definition));
        expect(std::ranges::equal(calls, std::array<std::string, 2>{"text", "node"}));
        expect(NodeUtil::getText(node) == u"\u00a0a  b\u00a0");
        expect(node.properties().tex.ignore_variant.value_or(false));
    };
    "ParseUtil copies Source nodes and registers copied sparse script lists"_test = [] {
        Fixture fixture;
        TexParser parser(u"", ParserEnvironmentSeed{}, *fixture.options);
        auto &base = fixture.token();
        auto &upper = fixture.token(NodeKind::mi, u"n");
        auto &script = fixture.node(NodeKind::msubsup);
        NodeUtil::setChild(script, 0, base);
        NodeUtil::setChild(script, 2, upper);
        script.attributes().set(u"id", u"drop");
        fixture.options->addNode(u"custom", script);
        auto &copy = copyNode(script, parser);
        expect(&copy != &script);
        expect(copy.children().size() == 3_u);
        expect(copy.children()[1] == nullptr);
        expect(copy.children()[0] != &base);
        expect(copy.children()[2]->parent() == &copy);
        expect(copy.attributes().own(u"id") == nullptr);
        expect(copy.properties().tex.in_lists->size() == 2_u);
        expect(fixture.options->nodeLists[u"custom"].back() == &copy);
        expect(script.properties().tex.in_lists->size() == 1_u);
    };
    "ParseUtil under over and finite NodeUtil definitions preserve Source properties"_test = [] {
        Fixture fixture;
        TexParser parser(u"", ParserEnvironmentSeed{}, *fixture.options);
        auto &base = fixture.token(NodeKind::mo, u"∑");
        auto &script = fixture.token(NodeKind::mi, u"a");
        auto &node = underOver(parser, base, script, u"over", true);
        expect(base.properties().tex.movable_limits == false);
        expect(!std::get<bool>(*base.attributes().own(u"movablelimits")));
        expect(node.kind() == NodeKind::tex_atom);
        expect(node.properties().tex.sub_sup_allowed.value_or(false));
        expect(node.properties().tex.move_sup_sub.value_or(false));
        auto inner = node.findNodes(NodeKind::munderover);
        expect(inner.size() == 1_u);
        expect(inner[0]->children()[1] == nullptr);
        expect(inner[0]->children()[2] == &script);
        AttributeSet values;
        values.set(u"variantForm", true);
        values.set(u"useHeight", false);
        values.set(u"texClass", 3.0);
        values.set(u"movablelimits", true);
        values.set(u"autoOP", false);
        values.set(u"open", u"(");
        values.set(u"inferred", true);
        auto &mapped = parser.createToken(NodeKind::mo, u"∑", std::move(values));
        expect(mapped.properties().tex.variant_form.value_or(false));
        expect(mapped.properties().table.use_height == false);
        expect(mapped.properties().tex.explicit_tex_class == TexClass::rel);
        expect(mapped.properties().tex.auto_operator == false);
        expect(mapped.properties().tex.open.value() == u"(");
        expect(mapped.attributes().own(u"variantForm") == nullptr);
        expect(mapped.attributes().own(u"inferred") == nullptr);
        expect(std::get<bool>(*NodeUtil::getMoAttribute(mapped, u"movablelimits")));
        mapped.attributes().set(u"movablelimits", false);
        expect(!std::get<bool>(*NodeUtil::getMoAttribute(mapped, u"movablelimits")));
    };
    "ParseUtil array equation and expansion helpers use actual request state"_test = [] {
        Fixture fixture;
        TexParser parser(u"", ParserEnvironmentSeed{}, *fixture.options);
        auto array = fixture.item(StackKind::array);
        auto &item = dynamic_cast<ArrayItem &>(*array);
        setArrayAlign(item, u" t ");
        expect(attr(item.arraydef, u"align") == u"baseline 1");
        parser.setString(u"oldtail");
        parser.setPosition(3);
        setArrayAlign(item, u" l ", &parser);
        expect(parser.string() == u"[ l ]tail");
        expect(parser.position() == 0_u);
        parser.stack.Pop();
        parser.stack.Push(fixture.item(StackKind::start, StartItemInit{parser.stack.global()}));
        checkEqnEnv(parser);
        parser.stack.Push(array);
        item.nestable = true;
        checkEqnEnv(parser);
        texError([&] { checkEqnEnv(parser, false); }, "ErroneousNestingEq");
        expect(addArgs(parser, u"\\alpha", u"beta") == u"\\alpha beta");
        const std::array<std::u16string, 1> args{u"x"};
        expect(substituteArgs(parser, args, u"#1#1") == u"xx");
        fixture.effective.maxMacros = 0;
        texError([&] { checkMaxMacros(parser, false); }, "MaxMacroSub2");
        expect(parser.macroCount == 1_u);
        expect(mmlFilterAttribute(parser, u"id", u" value ") == u" value ");
    };
    "add_args_control_word_boundaries"_test = [] {
        expect(addArgs(u"left", u"right", 20) == u"leftright");
        expect(addArgs(u"\\alpha", u"beta", 20) == u"\\alpha beta");
        expect(addArgs(u"prefix\\alpha", u"beta", 20) == u"prefix\\alpha beta");
        expect(addArgs(u"\\\\alpha", u"beta", 20) == u"\\\\alphabeta");
        expect(addArgs(u"\\\\\\alpha", u"beta", 20) == u"\\\\\\alpha beta");
        expect(addArgs(u"\\#", u"beta", 20) == u"\\#beta");
        expect(addArgs(u"", u"beta", 20) == u"beta");
    };

    "add_args_buffer_limit_includes_separator"_test = [] {
        expect(addArgs(u"\\alpha", u"b", 8) == u"\\alpha b");
        expectTexError([] { static_cast<void>(addArgs(u"\\alpha", u"b", 7)); }, "MaxBufferSize",
                       u"MathJax internal buffer size exceeded; is there a recursive macro call?");
        expect(addArgs(u"123", u"45", 5) == u"12345");
        expectTexError([] { static_cast<void>(addArgs(u"123", u"45", 4)); }, "MaxBufferSize",
                       u"MathJax internal buffer size exceeded; is there a recursive macro call?");
    };

    "substitute_args_preserves_escapes_and_boundaries"_test = [] {
        const std::array<std::u16string, 2> arguments{u"one", u"😀"};
        const std::array<std::u16string, 1> one_argument{u"beta"};
        const std::array<std::u16string, 2> extra_arguments{u"one", u"unused"};
        const std::array<std::u16string, 9> nine_arguments{u"1", u"2", u"3", u"4", u"5",
                                                           u"6", u"7", u"8", u"9"};
        expect(substituteArgs(arguments, u"#1/#2", 20) == u"one/😀");
        expect(substituteArgs(arguments, u"\\#1", 20) == u"\\#1");
        expect(substituteArgs(one_argument, u"\\{#1}", 20) == u"\\{beta}");
        expect(substituteArgs(one_argument, u"\\\\#1", 20) == u"\\\\beta");
        expect(substituteArgs(arguments, u"##1", 20) == u"#1");
        expect(substituteArgs(one_argument, u"\\alpha#1", 20) == u"\\alpha beta");
        expect(substituteArgs(extra_arguments, u"#1", 20) == u"one");
        expect(substituteArgs(nine_arguments, u"#9", 20) == u"9");
        expect(substituteArgs(std::span<std::u16string const>{}, u"tail\\", 20) == u"tail\\");
        expect(substituteArgs(arguments, u"plain text", 20) == u"plain text");
    };

    "substitute_args_parameter_errors_and_order"_test = [] {
        const std::array<std::u16string, 1> arguments{u"one"};
        for (const auto replacement : {u"#", u"#0", u"#x", u"#2", u"#9"}) {
            expectTexError(
                [replacement, &arguments] {
                    static_cast<void>(substituteArgs(arguments, replacement, 0));
                },
                "IllegalMacroParam", u"Illegal macro parameter reference");
        }
        expectTexError(
            [] {
                static_cast<void>(substituteArgs(std::span<std::u16string const>{}, u"#1", 100));
            },
            "IllegalMacroParam", u"Illegal macro parameter reference");
        expectTexError([&arguments] { static_cast<void>(substituteArgs(arguments, u"#1", 2)); },
                       "MaxBufferSize",
                       u"MathJax internal buffer size exceeded; is there a recursive macro call?");
    };

    "substitute_args_limit_counts_utf16_code_units"_test = [] {
        const std::array<std::u16string, 1> emoji{u"😀"};
        expect(addArgs(u"", u"😀", 2) == u"😀");
        expectTexError([] { static_cast<void>(addArgs(u"", u"😀", 1)); }, "MaxBufferSize",
                       u"MathJax internal buffer size exceeded; is there a recursive macro call?");
        expectTexError([&emoji] { static_cast<void>(substituteArgs(emoji, u"#1", 1)); },
                       "MaxBufferSize",
                       u"MathJax internal buffer size exceeded; is there a recursive macro call?");
    };

    "check_max_macros_increments_before_error"_test = [] {
        ExpansionBudget budget{2, 100, 0};
        checkMaxMacros(budget);
        checkMaxMacros(budget, false);
        expect(budget.macroCount == 2);
        expectTexError(
            [&budget] { checkMaxMacros(budget, true); }, "MaxMacroSub1",
            u"MathJax maximum macro substitution count exceeded; is here a recursive macro call?");
        expect(budget.macroCount == 3);

        ExpansionBudget environment{0, 100, 0};
        expectTexError([&environment] { checkMaxMacros(environment, false); }, "MaxMacroSub2",
                       u"MathJax maximum substitution count exceeded; is there a recursive latex "
                       u"environment?");
        expect(environment.macroCount == 1);
    };
};
