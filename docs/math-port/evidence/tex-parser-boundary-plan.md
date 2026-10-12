# TeX parser dependency boundary plan

Root design, not an implementation acceptance. The original files remain the
responsibility map; this plan does not license a custom simplified parser.
Reference: MathJax-src 4.1.3, fb987178c2d279a99b0db5ea02e751691703955e.

`TexParser` owns source scanning and parser driving; `Stack` owns reduction and
environment transitions; `StackItem` owns node accumulation and item transitions.
`StackItemFactory` registers item constructors. `NodeFactory` owns configurable
MML creator callbacks, including node/token/text/error creators. `ParseOptions`
owns parser state, node lists, factories, handlers, tags and options. Configuration
owns package composition. None of these moves into an umbrella switch parser.

Source has cycles: TexParser stores ParseOptions while ParseOptions stores parser
instances; StackItemFactory stores ParseOptions while ParseOptions owns factories;
NodeFactory references ParseOptions and ParseOptions owns NodeFactory. C++ named
modules cannot import these cycles. Break only reference edges using narrow abstract
services declared in a dependency leaf, then have the real owning classes implement
them. Concrete implementation units can import both modules without moving behavior
out of its original class. No dummy ParseOptions object or singleton implements a
partial real configuration.

Before freezing declarations, Root must inspect all enabled package uses of these
services, including Tags and nested parsers. The current ParserContext (currentCS
only) and empty StackItemContext are insufficient. Existing callable dispatch,
parse-symbol identity and raw Property state must remain the shared type boundary.
Retain UTF-16 string indices and Source argument/default distinctions.

Next implementation order after semantic-tree completion:

1. Complete NodeUtil and TexError with original-source tests; determine complete
   NodeFactory creator argument and node-list callback contract.
2. Freeze service declarations and port NodeFactory, MmlStack/BaseItem and
   StackItemFactory, including callback overrides and environment/property identity.
3. Port Stack transitions; its checkItem reductions and recursive replacement Push
   must be tested against the original constructor/Push/Pop/Top/Prev sequence.
4. Port configuration and ParseOptions lifecycle, Tags, then TexParser and base
   methods/items/mappings together through the registered handlers.
5. Complete enabled packages and filters, then common wrappers/font services and
   scene placement before SVG emission and Folia integration.

No source file is accepted merely because declarations compile. Parser milestones
must include real reductions producing the same MML as the locked JavaScript
configuration. The release milestone remains identical TeX-to-layout/SVG behavior,
not an isolated utility count.

## Current prerequisite audit after P39

All currently declared MML consumers execute; the complete core runner passes.
NodeUtil currently declares only getChildAt/setChild/copyChildren. Complete its
Source singleton responsibilities: entity/text/children operations, property versus
attribute routing through the eleven-name attrs Set, attribute/property copying,
type/core/class queries, and ordered operator definition/attribute lookup. Keep
the one MML graph and raw Property definitions; do not make a second parser AST.
Source setter callbacks, attributes identity, live child arrays and captured
Object.entries values must survive the typed adaptation. Audit enabled-package
argument domains before freezing new declarations.

TexError is a plain Source class with id and processed message, not an Error
subclass. Its complete private regexp/split/placeholder/plural behavior is required
by the parser's error protocol; a generic C++ formatter is not equivalent.

NodeFactory creators can be temporarily overridden and restored by physics;
get/set must retain the canonical FunctionRef identity. Reuse the existing
FunctionBody/FunctionArgs (std::any) primitive protocol, not a second function
object model. The parser-specific ParseCallable signature alone cannot express
NodeFactory's heterogeneous rest arguments and nullable MML result.

Required reference-edge services include NodeFactory -> configuration.addNode,
StackItem -> factory.configuration.nodeFactory, factories -> real ParseOptions,
and ParseOptions -> active TexParser/Tags/ColumnParser. The real owning classes
implement narrow services; a dummy configuration must not satisfy these edges.
ParseOptions preserves shared packageData, parser stack, nodeLists and live raw
parent filtering. StackItemContext cannot remain an empty final protocol.

The enabled profile includes nested textmacros, physics, ams/amscd, action and
other configured packages in entry.cjs. Do not reduce it to base TeX to accelerate
acceptance. Root freezes the full module declarations and per-file responsibilities
before releasing the next Luna implementations. These observations are a dependency
audit, not new accepted parser implementations.
