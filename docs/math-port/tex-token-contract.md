# Root P17a — native TeX token and callable type foundation

Authority: complete pinned Types.ts, Token.ts, HandlerTypes.ts and TexConstants.ts.
Root read these files and TokenMap/MapHandler callers before freezing this packet.
This starts the real configurable TeX input dependency graph; it is not a parser,
map implementation, package registry or permission to switch Folia.

## Module ownership and cycle seam

Token.ts imports Types.ts, whose ParseMethod references Token and TexParser.
Direct C++ named-module imports would cycle. Split the two independent source
classes into token.ixx/token.cpp (Token), and macro.ixx/macro.cpp (Macro).
types.ixx imports the already-complete Token and owns only native callable/
parser/stack identity interfaces. macro.ixx imports types.ixx. Each of the eight
source members remains individually mapped; this is dependency splitting,
not a second token model or a generic command-name router.

ParserContext is a stateless native interface for currentCS get/set and a
virtual destructor. Future actual TexParser owns its source fields and derives
this interface. Typed handler adapters can checked-cast the real parser for
their complete methods; the interface does not manufacture a parser or return
guessed parse results. StackItemContext similarly carries native object identity;
the later real StackItem supplies all source behavior.

ParseResult is variant<Property,StackItemRef,ParseSymbolRef>. Property preserves
Missing/null/false/raw scalar results; stack and symbol objects retain shared
identity. ParseAtom is variant<Property,TokenRef,StackItemRef>. Rest arguments
are native std::any slots, retaining typed callbacks, argument arrays and MML
objects without a JS interpreter. ParseMethod is variant<Property,ParseCallableRef>,
so missing/null/non-callable values remain distinct from actual function identity.
The callback signature is ParseResult(ParserContext&,const ParseAtom&,
span<const any>). This is the injected C++ callable seam, not a second parser.

## Exact declarations and owned files

Root owns all .ixx declarations, types/interfaces, generated constant modules,
module/build lists, TypeScript member ledger, actual-CJS oracle/expectations,
tests/mutations, builds, review and acceptance.

tex_parser Luna may write only src/math/input/tex/token.cpp,
src/math/input/tex/macro.cpp and docs/math-port/evidence/p17a-token-methods.md.
No other edits, builds, tests, commits, delegation or additional APIs.

Implement all eight Token.ts members. Token stores original Property values for
token/character/attributes, with constructor parameter properties assigned in
source order; accessors return those values, retaining record/array identity.
Root maps source char getter to character() because char is a C++ keyword.
Macro stores token and ParseMethod unchanged. Its args constructor parameter
uses raw Missing default; only Missing becomes a fresh PropertyArray. Explicit
null/other raw values remain unchanged. args() returns the shared original graph.
Do not clone attributes/args, stringify Unicode, coerce null, replace handlers
with names, create global parser state, or implement parsing in these classes.

Token/Macro properties use the accepted lossless native Property graph/string
representation; it is the same carrier used by semantic attributes. Normal TeX
strings remain UTF-16 in the future parser, with the existing explicit boundary
conversion. Editor positions are unaffected. Property permits broader raw values
for source-observation fidelity; this does not add an arbitrary JS execution API.

Root generates all 12+4 HandlerTypes string entries and the complete TexConstant
object from pinned executable/AST source. No manually invented enums, corrected
source spellings, added values or reduced categories. Types.ts aliases are
documented as the native host adaptation above, not executable source methods.

## Root acceptance

Execute actual pinned Token/Macro CJS constructors/getters, compare every member,
Unicode/lone-surrogate values, missing/null/scalars, attribute and argument
identity/mutations, fresh default argument arrays, and callable identity/raw
non-callable distinctions. Compare every enum/constant name, order and value.
Compile protocol witnesses for parser/stack/symbol/callback identity, heterogeneous
rest arguments and raw undefined/false forwarding. Deliberate compiling omissions
must fail unchanged assertions. Run focused tests and the core build before
local acceptance. No G0–G7 parser/render gate is passed by this packet.
