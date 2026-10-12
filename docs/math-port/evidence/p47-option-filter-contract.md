# P47 live option filter arrays before core InputJax

Root freezes the missing binding from real Context option arrays to the existing
FunctionList. The scope is ordinary array iteration with Source's typed optional
numeric tuple priorities and canonical bound native functions. It does not accept
custom Symbol.iterator, unbound arbitrary option objects/functions/Symbols or a
general JavaScript callable evaluator. Those produce explicit unimplemented host
failures, not compatible-looking data or a copied option dictionary.

## Sole responsibilities

- function_list.ixx: Root's borrowed FunctionDefinitionsSource declaration and
  typed addList entry; one private appendDefinitions algorithm owner. Virtual add
  takes an optional priority so omittedness reaches the actual virtual callee;
  that callee resolves its own default. A C++ double default argument binds too
  early and cannot represent Source subclass defaults.
- function_list.cpp: Root keeps existing constructor, execute/async algorithms
  and native-vector adapter unchanged. Luna implements ONLY appendDefinitions:
  current length/read each iteration, branch direct/tuple, capture tuple item then
  priority before virtual add, resolve omitted priority at add time. The prior
  native-vector entry and option-array entry both use that one algorithm.
- option_functions.ixx: Root's option Context/canonical FunctionRef identity
  registry and captured array view. It borrows one Context, never mirrors options
  arrays or clones FunctionBody. The same retained function binds to one Context
  function object, with explicit Source-signature receiver/argument/result
  converters. The host always calls its retained FunctionRef; registration cannot
  supply an unrelated second callback body. Context
  and FunctionList calls must ultimately use that same body. Repeated bind retains
  the first registration; callers supply the same signature binding for a function.
- OptionFunctionDefinitions captures one ObjectRef; current length and indexed Get
  remain live, including holes/prototype getters. A tuple handle is captured before
  reading slot 0 and slot 1. The temporary FunctionTuple is only the two captured
  call arguments; it is not a synchronized second tuple/option array.
- Root owns original oracle/fixture, red baseline, review, builds and acceptance.
  No agent edits declarations, host, tests, Source gold or unrelated files.

Pinned util/FunctionList.ts hash:
d4106c42268e0d7cb3549f8ffdbeb5bab87eae6daffae7314d5e724045a6918f.
Source observations run its actual locked CJS and actual Options.defaultOptions.
The 19 records exercise direct/tuple ordering, holes/inherited elements, growth,
shrink, array rebinding, tuple reads before add, accessor order/current priority,
indexed Get after add, getter exception prefixes, option-copy function aliases,
binding identity, Context invocation of the same body, a virtual callee's own
default for direct and undefined tuple entries, and falsy constructor input.

This is the missing collection/callable bridge required by InputJax's constructor,
not core InputJax or TeX/SVG acceptance. The full core InputJax Source fields,
class metadata, option merge and filter-record lifecycle still follow next.
Mermaid stays paused.
