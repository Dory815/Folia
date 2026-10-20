#include "support/folia_test.hpp"
import folia.math.input.tex.parser;

using namespace boost::ut;
using namespace folia::math::tex;
namespace {
DispatchOutcome macroMethod(TexParser&, MacroInvocation const&) { return Handled{}; }
DispatchOutcome expansion(TexParser&, MacroInvocation const&) { return NoParseResult{}; }
DispatchOutcome tokenMethod(TexParser&, TokenInput const&) { return Handled{}; }
}

suite token_map = [] {
    "token map lookup keeps the lookup key separate from the captured token"_test = [] {
        CharacterMap map(CharacterMapInit{{u"characters", std::nullopt},
            {Token(u"x", std::u16string(u"a")), Token(u"nil", std::nullopt)}});
        map.add(u"alias", Token(u"original", std::u16string(u"b")));
        expect(map.contains(u"alias"));
        expect(!map.contains(u"original"));
        auto value = std::get<Token const*>(map.lookup(u"alias"));
        expect(value->token == u"original");
        expect(value->character.value() == u"b");
        map.add(u"alias", Token(u"changed", std::u16string(u"c")));
        expect(std::get<Token const*>(map.lookup(u"alias"))->token == u"changed");
        map.remove(u"alias");
        expect(std::holds_alternative<std::monostate>(map.lookup(u"alias")));
        expect(map.contains(u"nil"));
        expect(!std::get<Token const*>(map.lookup(u"nil"))->character.has_value());
        map.setParser(TokenMapParser{tokenMethod});
        expect(std::get<TokenInputHandler>(map.parserFor(u"x").value()) == tokenMethod);
        map.setParser(std::nullopt);
        expect(!map.parserFor(u"x").has_value());
    };

    "token map regexp lookup preserves the original stateful test operations"_test = [] {
        auto expression = std::make_shared<folia::math::regexp_host::RegExp>(u"^a$", u"g");
        RegExpMap map(RegExpMapInit{{u"letters", TokenMapParser{tokenMethod}}, expression});
        expect(map.contains(u"a"));
        expect(expression->last_index == 1_u);
        expect(std::holds_alternative<std::monostate>(map.lookup(u"a")));
        expect(expression->last_index == 0_u);
        expect(std::get<std::u16string_view>(map.lookup(u"a")) == u"a");
        expect(expression->last_index == 1_u);
    };

    "token map macro values preserve all argument types and absent callbacks"_test = [] {
        MacroMap map(MacroMapInit{{u"macros", std::nullopt}, {
            Macro(u"source", MacroMethod{macroMethod}, {false, 2.0, std::u16string(u"x"), NullArgument{}}),
            Macro(u"missing", MacroMethod{static_cast<MacroHandler>(nullptr)})}});
        map.add(u"alias", Macro(u"source", MacroMethod{macroMethod}, {std::u16string(u"bound")}));
        expect(std::get<Macro const*>(map.lookup(u"alias"))->token == u"source");
        auto value = std::get<Macro const*>(map.lookup(u"source"));
        expect(value->arguments.size() == 4_u);
        expect(!std::get<bool>(value->arguments[0]));
        expect(std::get<double>(value->arguments[1]) == 2.0);
        expect(std::get<std::u16string>(value->arguments[2]) == u"x");
        expect(std::holds_alternative<NullArgument>(value->arguments[3]));
        expect(std::get<MacroHandler>(map.parserFor(u"source").value()) == macroMethod);
        expect(map.contains(u"missing"));
        expect(!map.parserFor(u"missing").has_value());
    };

    "token map session lookup selects current definitions and typed alias bindings"_test = [] {
        std::array<MacroHandler, 1> macros{macroMethod};
        std::array<TokenInputHandler, 1> tokens{tokenMethod};
        MacroCallbacks callbacks(macros, tokens);
        TexSessionState session;
        session.defineMacro(u"x", MacroDefinition{.replacement_source = u"outer"});
        MacroMap map(SessionMacroMapInit{{u"session", std::nullopt}, session, callbacks, expansion});
        expect(std::get<MacroHandler>(map.parserFor(u"x").value()) == expansion);
        session.definitions().groups.push_back(DefinitionLayer{-101});
        session.defineCommand(u"x", CommandDefinition{u"original", NativeMacroAlias{MacroHandlerId{1}, {true}}});
        expect(std::get<MacroHandler>(map.parserFor(u"x").value()) == macroMethod);
        auto selected = std::get<CommandDefinition const*>(map.lookup(u"x"));
        expect(selected == session.findCommand(u"x"));
        expect(selected->source_token == u"original");
        session.defineCommand(u"x", CommandDefinition{u"captured",
            CharacterMacroAlias{TokenParserId{1}, Token(u"captured", std::u16string(u"c"))}});
        expect(std::get<TokenInputHandler>(map.parserFor(u"x").value()) == tokenMethod);
        session.defineCommand(u"x", CommandDefinition{u"x", MacroSuppression{}});
        expect(map.contains(u"x"));
        expect(!map.parserFor(u"x").has_value());
        expect(!session.findMacro(u"x"));
        session.definitions().groups.pop_back();
        expect(std::get<MacroHandler>(map.parserFor(u"x").value()) == expansion);
    };

    "token map parseResult normalizes only the undefined result category"_test = [] {
        expect(std::holds_alternative<Handled>(parseResult(NoParseResult{})));
        expect(std::holds_alternative<Handled>(parseResult(Handled{})));
        expect(std::holds_alternative<ContinueDispatch>(parseResult(ContinueDispatch{})));
        expect(std::holds_alternative<InvokeFallback>(parseResult(InvokeFallback{})));
    };
};
