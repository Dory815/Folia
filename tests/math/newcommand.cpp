#include "math/support/tex_fixture.hpp"
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.begingroup.stack;
import folia.math.input.tex.parse_methods;

namespace {
constexpr std::array<MacroHandler, 7> macroCallbacks{&BaseMethods::Frac,
                                                     &BaseMethods::NamedFn,
                                                     &BaseMethods::Macro,
                                                     &BaseMethods::Superscript,
                                                     &NewcommandMethods::MacroDef,
                                                     &NewcommandMethods::Let,
                                                     &NewcommandMethods::NewCommand};
constexpr std::array<TokenInputHandler, 6> tokenCallbacks{
    &ParseMethods::lcGreek,     &ParseMethods::ucGreek,   &ParseMethods::mathchar0mi,
    &ParseMethods::mathchar0mo, &ParseMethods::mathchar7, &ParseMethods::delimiter};
thread_local std::string *errorTarget{};
MmlNode &recordError(TeX &jax, TexError const &error) {
    if (errorTarget)
        *errorTarget = error.id();
    return jax.formatError(error);
}
DispatchOutcome beginScope(TexParser &p, MacroInvocation const &) {
    BegingroupStack(p.configuration.packageData.definitions()).push();
    return NoParseResult{};
}
DispatchOutcome endScope(TexParser &p, MacroInvocation const &) {
    BegingroupStack(p.configuration.packageData.definitions()).pop();
    return NoParseResult{};
}
DispatchOutcome globalDefinition(TexParser &p, MacroInvocation const &) {
    p.stack.env().is_global = true;
    return NoParseResult{};
}
void removeScopes(TexFilterData &arg) {
    BegingroupStack(arg.data.packageData.definitions()).remove();
}
void finishScopes(TexFilterData &arg) {
    BegingroupStack(arg.data.packageData.definitions()).finish();
}
struct NewcommandFixture {
    ConfigurationHandler registry;
    MapHandler maps;
    MacroCallbacks callbacks{macroCallbacks, tokenCallbacks};
    Warnings warnings;
    MmlDocument document;
    std::unique_ptr<TeX> jax;
    std::string lastError;
    explicit NewcommandFixture(bool scopes = false, std::size_t maxMacros = 1000) {
        NewcommandConfiguration::initializeModule(registry, maps);
        TexOptionValues options;
        options.packages = std::vector<PackageSelection>{{u"base", {}}, {u"newcommand", {}}};
        options.maxMacros = maxMacros;
        options.formatError = &recordError;
        if (scopes) {
            maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{
                MacroMapInit{{u"newcommand-scope-tests", std::nullopt},
                             {Macro(u"begingroup", &beginScope), Macro(u"endgroup", &endScope),
                              Macro(u"global", &globalDefinition)}}}));
            ConfigurationDefinition definition;
            definition.handler = {{HandlerKind::macro, {u"newcommand-scope-tests"}}};
            definition.preprocessors.emplace_back(FilterList::Callback{&removeScopes});
            definition.postprocessors.emplace_back(FilterList::Callback{&finishScopes});
            Configuration::create(registry, u"newcommand-scope-tests", std::move(definition));
            options.packages->push_back({u"newcommand-scope-tests", {}});
        }
        jax = std::make_unique<TeX>(registry, maps, callbacks, warnings, document.factory(),
                                    std::move(options));
    }
    MmlNode &compile(std::u16string source) {
        lastError.clear();
        auto previous = errorTarget;
        errorTarget = &lastError;
        struct Restore {
            std::string *previous;
            ~Restore() {
                errorTarget = previous;
            }
        } restore{previous};
        folia::math::core::AbstractMathItem math(std::move(source), jax.get());
        math.setDisplay(false);
        return jax->compile(math, document);
    }
    std::u16string text(std::u16string source) {
        auto &root = compile(std::move(source));
        std::u16string result;
        root.walkTree([&](auto &node) {
            if (node.isToken())
                result += NodeUtil::getText(node);
        });
        return result;
    }
};
std::vector<MmlNode *> find(MmlNode &root, NodeKind kind) {
    std::vector<MmlNode *> result;
    root.walkTree([&](auto &node) {
        if (node.kind() == kind)
            result.push_back(&node);
    });
    return result;
}
} // namespace

suite newcommand = [] {
    "newcommand registers original six commands BeginEnv class defaults and session maps once"_test =
        [] {
            NewcommandFixture f;
            auto package = f.registry.get(u"newcommand");
            expect(package->items.size() == 1_u);
            expect(package->options.maxMacros.value() == 1000_u);
            expect(package->options.protectedMacros->front() == u"begingroupSandbox");
            auto commands = std::dynamic_pointer_cast<MacroMap>(f.maps.getMap(NEW_COMMAND));
            expect(commands->sessionOwner() == &f.jax->session());
            auto delimiter = f.maps.getMap(NEW_DELIMITER),
                 environment = f.maps.getMap(NEW_ENVIRONMENT);
            NewcommandConfig(f.jax->configuration(), *f.jax);
            expect(f.maps.getMap(NEW_COMMAND) == commands);
            expect(f.maps.getMap(NEW_DELIMITER) == delimiter);
            expect(f.maps.getMap(NEW_ENVIRONMENT) == environment);
            auto built = f.jax->parseOptions().itemFactory->create(StackKind::begin_env);
            expect(dynamic_cast<BeginEnvItem *>(built.get()) != nullptr);
            expect(built->isOpen());
            expect(NewcommandMethods::Macro == &BaseMethods::Macro);
            for (auto name : {u"newcommand", u"renewcommand", u"newenvironment",
                              u"renewenvironment", u"def", u"let"})
                expect(f.maps.getMap(u"Newcommand-macros")->contains(name));
        };
    "newcommand optional parameters renewals and more than nine arguments use actual parser"_test =
        [] {
            NewcommandFixture f;
            expect(f.text(u"\\newcommand{\\pair}[2][d]{#1+#2}\\pair{x}+\\pair[q]{r}") ==
                   u"d+x+q+r");
            expect(f.lastError.empty());
            expect(f.text(u"\\renewcommand\\pair[1]{#1#1}\\pair{x}") == u"xx");
            expect(f.text(u"\\newcommand{\\ten}[10]{#1+#9}\\ten abcdefghij+z") == u"a+i+z");
            auto definition = f.jax->session().findMacro(u"ten");
            expect(definition != nullptr);
            if (definition) {
                expect(definition->arity == 10.0);
                expect(definition->parameter_delimiters.empty());
            }
            expect(f.text(u"\\newcommand{\\zero}[0][d]{#1}\\zero[q]") == u"q");
            f.compile(u"\\newcommand{\\zero}[0]{#1}\\zero");
            expect(f.lastError == "IllegalMacroParam");
        };
    "newcommand definitions persist across formulas tag reset and isolated jaxes"_test = [] {
        NewcommandFixture f;
        expect(f.text(u"\\newcommand{\\persistent}{x}").empty());
        expect(f.text(u"\\persistent") == u"x");
        f.jax->reset(11);
        expect(f.text(u"\\persistent") == u"x");
        NewcommandFixture other;
        other.compile(u"\\persistent");
        expect(other.lastError == "UndefinedControlSequence");
        expect(f.text(u"\\persistent") == u"x");
    };
    "newcommand def scans prefixes delimited arguments braces and control sequence boundaries"_test =
        [] {
            NewcommandFixture f;
            expect(f.text(u"\\def\\pair(#1,#2){#2+#1}\\pair(a,b)") == u"b+a");
            expect(f.text(u"\\def\\take#1abc{#1}\\take{xy}abc") == u"xy");
            expect(f.text(u"\\def\\foo#1\\stop{#1}\\foo a\\stop") == u"a");
            expect(f.text(u"\\def\\plain#1#2 {#2#1}\\plain ab") == u"ba");
            auto definition = f.jax->session().findMacro(u"pair");
            expect(definition && definition->parameter_template);
            if (definition) {
                expect(definition->parameter_prefix == u"(");
                expect(definition->parameter_delimiters == std::vector<std::u16string>{u",", u")"});
            }
            f.compile(u"\\pair[a,b)");
            expect(f.lastError == "MismatchUseDef");
            f.compile(u"\\def\\foo#1\\stop{#1}\\foo a\\stopmore");
            expect(f.lastError == "RunawayArgument");
        };
    "newcommand let snapshots macros templates native callbacks and character definitions"_test =
        [] {
            NewcommandFixture f;
            expect(f.text(u"\\newcommand{\\a}{x}\\let\\b\\a\\renewcommand{\\a}{y}\\b\\a") == u"xy");
            expect(
                f.text(u"\\def\\pair(#1,#2){#2+#1}\\let\\saved\\pair\\def\\pair{x}\\saved(a,b)") ==
                u"b+a");
            auto &fraction = f.compile(u"\\let\\fraction\\frac\\fraction12");
            expect(find(fraction, NodeKind::mfrac).size() == 1_u);
            expect(f.lastError.empty());
            expect(f.text(u"\\let\\a\\alpha\\let\\b\\a\\let\\a\\beta\\b\\a") == u"αβ");
            auto alias = f.jax->session().findCommand(u"b");
            expect(alias && std::holds_alternative<CharacterMacroAlias>(alias->body));
            expect(f.text(u"\\let\\a\\a\\a") == u"β");
            expect(f.text(u"\\let\\define\\def\\define\\c{x}\\c") == u"x");
        };
    "newcommand let copies literal characters delimiters and suppresses later definitions"_test =
        [] {
            NewcommandFixture f;
            expect(f.text(u"\\let\\open(\\left\\open x\\right)") == u"(x)");
            expect(f.text(u"\\let\\angle\\langle\\angle") == u"⟨");
            expect(f.text(u"\\let\\bgroup={\\bgroup a}") == u"a");
            auto &script = f.compile(u"\\let\\car^ a\\car b");
            expect(find(script, NodeKind::msup).size() == 1_u);
            f.compile(u"\\let\\sin\\notDefined\\sin");
            expect(f.lastError == "UndefinedControlSequence");
            auto hidden = f.jax->session().findCommand(u"sin");
            expect(hidden && std::holds_alternative<MacroSuppression>(hidden->body));
            expect(f.text(u"\\let\\langle\\notDefined\\langle").empty());
            expect(f.lastError.empty());
            auto delimiter = f.jax->session().findDelimiter(u"\\langle");
            expect(delimiter && !delimiter->character);
            f.compile(u"\\let\\foo\\notDefined\\foo");
            expect(f.lastError == "UndefinedControlSequence");
        };
    "newcommand environments substitute begin definitions parse end definitions and nest"_test =
        [] {
            NewcommandFixture f;
            expect(f.text(u"\\newenvironment{e}[2][d]{#1+#2+}{+z}\\begin{e}{x}y\\end{e}") ==
                   u"d+x+y+z");
            expect(f.text(u"\\begin{e}[q]{r}s\\end{e}") == u"q+r+s+z");
            expect(f.text(u"\\newenvironment{n}{a}{b}\\begin{n}x\\begin{n}y\\end{n}z\\end{n}") ==
                   u"axaybzb");
            expect(f.text(u"\\renewenvironment{n}{c}{d}\\begin{n}x\\end{n}") == u"cxd");
            expect(f.text(u"\\newenvironment{z}[0][d]{#1}{x}\\begin{z}[q]\\end{z}") == u"qx");
            expect(f.text(u"\\newenvironment{m}{\\begin{array}{cc}}{\\end{array}}\\begin{m}a&"
                          u"b\\end{m}") == u"ab");
        };
    "newcommand reports scanning protection and template errors through real TeX formatter"_test =
        [] {
            struct Case {
                std::u16string source;
                std::string id;
            };
            const std::array cases{
                Case{u"\\def x{a}", "MissingCS"},
                Case{u"\\newcommand{a-b}{x}", "IllegalControlSequenceName"},
                Case{u"\\newcommand{\\a}[x]{x}", "IllegalParamNumber"},
                Case{u"\\def\\a#0{x}", "CantUseHash2"},
                Case{u"\\def\\a#2{x}", "SequentialParam"},
                Case{u"\\def\\a", "MissingReplacementString"},
                Case{u"\\newcommand{\\begingroupSandbox}{x}", "ProtectedMacro"},
                Case{u"\\newenvironment{e}{a}{b}\\begin{e}", "EnvMissingEnd"},
                Case{u"\\newenvironment{e}{a}{b}\\begin{e}\\end{equation}", "EnvBadEnd"},
                Case{u"\\newenvironment{e}[1]{#1}{#1}\\begin{e}{x}", "IllegalMacroParam"}};
            for (auto const &test : cases) {
                NewcommandFixture f;
                f.compile(test.source);
                expect(f.lastError == test.id) << test.id;
                expect(f.jax->parseOptions().parsers.empty());
            }
        };
    "newcommand global definitions use existing owned definition layers"_test = [] {
        NewcommandFixture f(true);
        expect(f.text(u"\\def\\a{x}\\begingroup\\def\\a{y}\\a\\endgroup\\a") == u"yx");
        expect(f.text(u"\\begingroup\\def\\a{y}\\global\\def\\a{z}\\a\\endgroup\\a") == u"zz");
        expect(f.jax->session().definitions().groups.empty());
        expect(f.jax->session().definitions().global.commands.at(u"a").source_token == u"a");
        expect(f.text(u"\\begingroup\\def\\a{q}").empty());
        expect(f.text(u"\\a") == u"q");
        expect(f.text(u"\\endgroup\\a") == u"z");
    };
    "newcommand expansion budgets reject recursion and recover on following formulas"_test = [] {
        NewcommandFixture f(false, 12);
        f.compile(u"\\def\\loop{\\loop}\\loop");
        expect(f.lastError == "MaxMacroSub1");
        expect(f.jax->parseOptions().parsers.empty());
        expect(f.text(u"\\def\\loop{x}\\loop") == u"x");
        NewcommandFixture environment(false, 2);
        environment.compile(u"\\newenvironment{e}{}{}\\begin{e}\\begin{e}\\end{e}\\end{e}");
        expect(environment.lastError == "MaxMacroSub2");
    };
};
