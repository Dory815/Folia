# Native fixture candidate (not enabled)

Keep this fixture outside `tests/` auto-discovery until the real peer graph
links. The fixture must receive a single initialized `ParserRuntime`, a real
`TeX`, and the `ParserConfiguration`/`ColumnParser`/`TexParser`/`ArrayItem`
instances produced by that runtime. It must not create substitute parser,
parse-options, MML-factory, node-factory, tags, or array-item implementations.

Candidate execution order:

1. Construct the native Context, callable and RegExp bindings, Unicode service,
   InputJax module, FindTeX module, and `ParserRuntime`; initialize the runtime
   once, then construct a real `TeX` with the base package and the actual MML
   factory.
2. Create an actual `Configuration` through `Configuration::local` and a
   second named configuration through `Configuration::create`. Assert local
   registration absence, named registration identity, and that the two actual
   handler objects/default arrays have distinct identities.
3. Construct `ParserConfiguration` with the actual package list and parser
   list, call `init()` and `config(realTeX)`, then use the actual `add()` path
   on an installed configuration. Capture callback order and receiver through
   retained native `FunctionRef`s; compare the observations with the JSON rows.
4. Obtain the actual `TagsFactory` products from that runtime, bind their
   configuration to the real `ParseOptions`, and run tag/finalize cases through
   the real MML factory. Preserve identity checks for the returned MML nodes.
5. Call `ColumnParser::process` through the real TeX/ParseOptions column parser
   on a real ArrayItem. Compare its resulting arraydef/frame and aliased
   `cstart`, `cend`, `cextra`, and `ralign` arrays, plus parser index and errors.

The JSON source observations are the expected side. The fixture should convert
the native values into the same tagged JSON shape and compare those generated
results. It should not duplicate expected behavior in C++ branches. If an
actual class is not yet constructible in the linked target, leave the case
unrun and identify that missing peer rather than accepting a stub or fake.
