// Typed values and same-owner declarations for the MathJax TeX class graph.
export module folia.math.input.tex.parser:types;
export import folia.math.core.mml_tree.mml_document;
export import folia.math.core.math_item;
export import folia.math.core.input_jax;
export import folia.math.input.tex.environment;
export import folia.math.input.tex.node_factory;
export import folia.math.input.tex.session;
export import folia.math.input.tex.tex_error;
export import folia.math.input.tex.tex_scanner;
export import folia.math.input.tex.find_tex;
export import folia.math.core.tree.factory;
import std;

export namespace folia::math::tex {
class BaseItem;
class Stack;
class StackItemFactory;
class ParseOptions;
class TexParser;
class TeX;
class Configuration;
class ConfigurationHandler;
class ParserConfiguration;
class Tags;
class ColumnParser;
using TexFilterData = core::InputJaxFilterData<ParseOptions>;
class CharacterMap;
class TokenMap;
class MapHandler;
class MacroCallbacks;
class SubHandlers;
struct TexOptionValues;
struct Macro;

enum class StackKind {
    base, start, stop, open, close, null_item, prime, subsup, over, left, middle,
    right, break_item, begin, end, style, position, cell, mml, fn, not_item,
    nonscript, dots, array, eqnarray, equation, mstyle, cases_begin, multline,
    flalign, multlined, proof_tree, braket, begin_env, auto_open, dummy
};

using FrameHandle = std::shared_ptr<BaseItem>;

struct StackGlobal {
    bool is_inner{true};
    std::optional<bool> is_sandbox;
    std::optional<std::u16string> empheq;
    std::optional<std::size_t> begin_env_depth;
    std::optional<std::u16string> left_root, up_root;
    std::optional<std::u16string> indent_align;
};

using EnvironmentScope = std::shared_ptr<TexEnvironment>;
using SourceSnapshot = std::shared_ptr<std::u16string const>;
using StackEntry = std::variant<std::monostate, mml::MmlNode*, FrameHandle>;
// A handler can receive a different parser from the one doing bookkeeping.
// The small token value survives replacement of either scanner's source.
struct ParseInput { TexParser& parser; std::u16string token; };

// The selected TeX map callbacks consume mathematical definition data, not
// erased language arguments. Token, TexArgument and CommandDefinition are
// imported native values from the independent token/session modules above.
struct TokenInput { std::variant<std::u16string_view, Token const*> value; };
struct ContinueDispatch {};
struct Handled {};
struct ReturnedFrame { FrameHandle frame; };
struct InvokeFallback {};
struct NoParseResult {};
using DispatchOutcome = std::variant<ContinueDispatch, Handled, ReturnedFrame,
                                     InvokeFallback, NoParseResult>;
using MacroArguments = std::variant<std::span<TexArgument const>,
                                    std::reference_wrapper<CommandDefinition const>>;
struct MacroInvocation {
    std::u16string_view input_token;
    std::u16string_view dispatch_token;
    MacroArguments arguments;
};
using MacroHandler = DispatchOutcome (*)(TexParser&, MacroInvocation const&);
using EnvironmentBodyHandler = StackEntry (*)(TexParser&, FrameHandle const&,
                                               std::span<TexArgument const>);
using MacroMethod = std::variant<MacroHandler, EnvironmentBodyHandler>;
using EnvironmentHandler = DispatchOutcome (*)(TexParser&, std::u16string_view,
    EnvironmentBodyHandler, std::span<TexArgument const>);
using TokenInputHandler = DispatchOutcome (*)(TexParser&, TokenInput const&);
using TokenMapParser = std::variant<TokenInputHandler, MacroHandler,
                                    EnvironmentBodyHandler, EnvironmentHandler>;
enum class HandlerKind { delimiter, macro, character, environment };
using FallbackHandler = void (*)(TexParser&, std::u16string_view);

struct ReuseTop {};
struct ReuseIncoming {};
using ReplayEntry = std::variant<FrameHandle, mml::MmlNode*, ReuseTop, ReuseIncoming>;
enum class CheckAction { accept, drop, replay };

// Define special members out of line after the complete BaseItem declaration.
struct CheckResult {
    CheckAction action{CheckAction::accept};
    std::vector<ReplayEntry> replacements;
    CheckResult(CheckAction);
    CheckResult(std::vector<ReplayEntry>);
    ~CheckResult();
    CheckResult(CheckResult&&) noexcept;
    CheckResult& operator=(CheckResult&&) noexcept;
    CheckResult(CheckResult const&) = delete;
    CheckResult& operator=(CheckResult const&) = delete;
};

// Actual Source constructor shapes. Post-create fields stay on their classes.
struct NoItemInit {};
struct StartItemInit { StackGlobal& global; };
struct MmlItemInit { mml::MmlNode& node; };
struct FnItemInit { mml::MmlNode& function; };
struct SubsupItemInit { mml::MmlNode& base; };
struct PrimeItemInit { mml::MmlNode& base; mml::MmlNode& prime; };
struct LeftItemInit { std::u16string delimiter; };
struct MiddleRightItemInit { std::u16string delimiter; std::optional<std::u16string> color; };
struct BreakItemInit { std::u16string linebreak; bool insert; };
struct EquationItemInit { bool numbered; };
struct EqnArrayItemInit { std::u16string name; bool numbered; bool taggable; };
struct MultlineItemInit { bool numbered; };
struct FlalignItemInit { std::u16string name; bool numbered; bool padded; bool center; };
struct MstyleItemInit { mml::AttributeSet attributes; std::u16string name; };
// The configured MtMultlined call passes a parser object as defaultTags.
// Actual Source getTag consumes only its truth value: this marker means true,
// without retaining the parser/begin arguments that its constructor ignores.
struct MultlinedItemInit {};
using StackItemInit = std::variant<NoItemInit, StartItemInit, MmlItemInit,
    FnItemInit, SubsupItemInit, PrimeItemInit, LeftItemInit, MiddleRightItemInit,
    BreakItemInit, EquationItemInit, EqnArrayItemInit, MultlineItemInit,
    FlalignItemInit, MstyleItemInit, MultlinedItemInit>;
struct StackBuildArguments {
    StackItemFactory& factory;
    StackItemInit const& init;
};
}
