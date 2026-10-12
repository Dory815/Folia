# P46 authoritative parser options graph

Root's binding decision before actual parser-family declaration/body delegation.
The Source partition plan remains p45-parser-module-ownership.md. No parser or
option implementation is accepted by this document.

## Owner and lifetime

Use the already accepted options::Context/Value graph and util/Options algorithms
for Configuration.options, ParserConfiguration.options, TeX.options and
ParseOptions.options. These fields are distinct Source destinations when Source
creates distinct objects; they are handles into one graph, never synchronized
PropertyList copies. All defaultOptions/userOptions/insert/copy calls use that
same Context and the actual Source destination handles.

One native MathJax runtime owns that Context for the lifetime of its loaded
package descriptors and jax/session instances. TeX and ParseOptions borrow it;
neither creates a Context per formula or per nested parser. Multiple jax objects
in the same runtime share package option/callback/regexp objects when Source does.
Native constructors receive the owning Context explicitly, or borrow it from the
actual owning Configuration/ParserConfiguration. They do not obtain it from a
hidden current-parser variable. Destroy sessions/jax/descriptors before the graph.

This is an engine-local native host owner, corresponding to one loaded MathJax
module environment. It is not another parser, option dictionary, evaluator or
state service. The existing Context contract explicitly allows engine-local
lifetime. Loading package descriptors happens once per runtime, not once per
formula. Stable package functions/regex instances are retained across renders.
For the initial serialized runtime, the Context lives for the loaded native
module/registry lifetime; it cannot be destroyed while static registrations still
retain handles. A future explicit unload must clear registry references first,
then sessions/descriptors, and only then release the Context. A per-request
Context with module-static registrations is forbidden.

Existing Source-global MapHandler/STATE and future ConfigurationHandler/TagsFactory
must retain their original module-state roles. This decision does not quietly
turn their state into per-expression copies or promise isolated concurrent engines.
The initial Folia route has serialized operations in one native math runtime,
matching the existing worker scheduling. Independent engines/concurrent mutations
would require a separate explicit registry-lifetime audit before being exposed.

## Concrete declaration requirements

- Configuration/ParserConfiguration carry a borrowed options::Context edge and
  options::Value option fields; the native constructor host parameter is not a
  second Source option list. Source names/defaults/callback ordering stay intact.
- ParseOptions borrows its ParserConfiguration's Context, initializes its own
  fresh option object at the Source field-initialization phase, and invokes the
  accepted option functions on it. Factory, handlers, tags, packageData, parsers,
  MathItem, root and nodeLists remain their actual typed owners.
- TexParser's configuration is a reference/pointer to the actual ParseOptions,
  not a by-value copy. A complete ParseOptions declaration is needed when a body
  accesses fields, but not simply to declare this reference. ArrayItem likewise
  holds the actual TexParser; ColumnParser has its actual complete declaration
  before ParseOptions's by-value ColumnParser member.
- TeX.options and ParseOptions.options remain the distinct destinations produced
  by the Source constructor. No requirement here aliases them merely because
  they share a Context. Source merges determine actual values and object aliases.
- Source option functions and regexp instances are real function/instance nodes
  in Context. Native FunctionRef or regexp-service identity lives in their native
  payload/binding. Calls preserve the original receiver and arguments. Repeated
  retrieval must not allocate a different function/regexp wrapper. Declare typed
  native resolvers for retained FunctionRef/regexp payloads and explicit callback
  receiver/argument bindings before delegation; Context::payload alone supplies
  storage and does not already solve the existing signature mismatch. Source Options
  copy/insert decides whether ordinary objects/arrays are copied; class instances
  and functions preserve identity exactly where its algorithms do.

## MML and existing callable boundary

The existing Property graph remains authoritative for MML scalar attributes and
properties, environments and existing handler/factory dictionaries. It is not an
option projection. Primitive values are converted only at a real Source boundary
such as setting an MML attribute; string conversion preserves UTF-16/WTF-8 values,
Missing/Null, numbers and booleans. Source construction/enumeration of an attribute
object is an operation to port at that caller, not permission to copy the entire
option graph preemptively.

An option object/function/array that itself crosses a boundary must retain a
shared identity binding or the receiving typed API must consume its Context
handle directly. A recursive conversion into an independent PropertyList is not
allowed. Callback bindings may expose the same existing FunctionRef; they must
not maintain a second callable body or duplicate captured state. No broad generic
parser-method dispatch is introduced to perform these conversions.

Examples verified from pinned Source:

- BaseConfiguration.ts:160-175 and 222-228: numberPattern/digits alias and native
  regex values, initialDigit/initialLetter maps; these need actual regexp behavior.
- ParseUtil.ts:434-435 and textmacros/TextMacrosConfiguration.ts:144-153: live
  configurable internalMath callback; its replacement must remain callable.
- tagformat/TagFormatConfiguration.ts:58-117: dynamically selected Tags subtype
  calls current nested number/tag/ref/id/url option functions and updates tags.
- setoptions/SetOptionsConfiguration.ts:151-155: mutates the actual nested options.
- color/ColorMethods.ts:130,154: padding(option scalar) constructs properties and
  NodeUtil.setProperties writes those values. Source passes the result of padding,
  not the whole nested color options object. This does not justify a shadow graph.

## Root gates before Luna implementation

1. Freeze Source field/default/nullable types and all actual parser partitions;
   compile their declaration graph together in folia_math. Include all 26 base
   item declarations and real Tags/configuration types, not test probe reducers.
2. Bind option callbacks/regex/native references with original alias/receiver
   observations. Existing controlled RegExpContext tests accept map dispatch,
   not a complete native regexp engine; real required patterns remain a gate.
3. Freeze original configuration/ParseOptions lifecycle observations, including
   nested options/callback identity across two jax instances, clear persistence,
   dynamic package addition and live option mutation. Do not substitute fake
   TeX/ParseOptions algorithms in an observer.
4. Delegate exclusive Source-aligned body files only after contracts and gold are
   reviewable. Root owns original comparisons, build and acceptance.

Unicode NFD for the French math style also requires a proper portable native host.
The existing Windows encoding-test ICU target is not a math dependency and does
not already establish such a binding. No ASCII approximation is accepted.
Complete TeX/packages, common layout/fonts, SVG parity and Folia remain the goal;
Mermaid remains paused.

## Original observations already frozen

tools/math-port/parser-options-oracle.cjs observes actual locked CJS classes:
Configuration, ParserConfiguration, ParseOptions, TeX, TexParser, TagsFactory,
MmlFactory and enabled textmacros. It verifies every loaded module's TS twin
against the cloned upstream bytes and records TS/CJS hashes for 85 files. Its
52 observations reproduce with --check in p46-parser-options-source.json.

The records cover retained descriptors, nullish/default priorities and actual
fallback dispatch, package order, registration/target warnings, constructor
back-edges and absent tags, default merge precedence, ordinary object/array copies
versus function/regexp aliases, all mathStyle choices, actual two-jax option
identities, clear persistence, node-list identity/filter/removal and actual dynamic
package additions. Real execution witnesses also include:

- FunctionList init/config callbacks bind their actual priority record; dynamic
  package add directly binds Configuration as its config callback receiver.
- digits aliases numberPattern; one real parser consumes 1234 as two two-digit
  numbers, then a live replacement regexp yields one four-digit number.
- Actual TeX.compile invokes the current formatError option with its options
  object receiver, same jax argument and actual TexError; it produces an merror.
- ParseUtil invokes a live replaced internalMath function with the current options
  receiver and actual parser, and retains each returned node-array identity.
- Actual textmacros shares its child ParseOptions.options and packageData with
  the TeX session, installs its actual internalMath callback and parses text with
  a textbf child using real TextParser. Both parser stacks are empty afterward.

Root corrects the read-only reviewer's claim that FunctionList's receiver is
undefined: Source item.item(...data) binds the priority record, consistent with
the existing accepted FunctionList tests. The new original traces lock this fact.

These are Source-only observations, not native callback/regexp/parser acceptance.
They guide real declarations and later native fixtures; they are not complete
coverage for every parser-family method. No alternate Source parser/configuration
algorithm, fake TeX jax or test probe reducer is used to obtain the records.
