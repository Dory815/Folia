# Native HTML MathDocument provider

The Source-matched owners are separated before CMake enrollment:

- `src/math/core/math_document.*` owns generic `core/MathDocument.ts` option, action, and convert flow; `math_render_actions.*` owns RenderList behavior.
- `src/math/handlers/html/html_math_document.*` owns the concrete HTML document provider and actual HTMLMathItem construction.
- `src/math/handlers/html/html_math_item.*` owns `handlers/html/HTMLMathItem.ts` DOM replacement and restoration.
- `src/math/output/svg/native_document.*` only assembles the actual session providers and exposes `svg::createNativeSvgDocument(session, options)`.

None of these candidate pairs is compiler-validated yet. The factory is intended to use the session's actual ParserRuntime and TeX Jax, SVG Jax, Lite adaptor, MML factory, RuntimeModules, and a real adaptor Document. It projects MML roots through `ParserRuntime::mmlNodeIdentities`; the returned public document lease retains the Context and service graph, while the document stored in Context retains only the graph that borrows Context.

The candidate currently covers a narrow synchronous path: Context-backed document/item identities, a canonical `LinkedListModule` and live `AbstractMathList`, the real `RenderActions` service, live `document.inputJax` and `options.MathItem` reads, InputJax selection by current `name`, three-argument MathItem construction, live start-location mutation, raw metric values, output family propagation, and HTML DOM replacement/removal through the same adaptor. MathItem's Context fields use native data cells over the typed owner; the root getter/setter use the actual MML identity projection.

Unaccepted Source surface remains substantial: full `MathDocument` class/prototype identity and `OPTIONS`, Context identity for the MathList object, custom `options.MathItem` constructors, complete document/item fields and accessors, updateStyleSheet and promise handling, non-string MathItem lifecycle paths, multi-Jax source-class dispatch, all actual handlers, and the 75-case TeX-to-SVG profile. The native session currently initializes only BasePackage; it is not a full package-profile fixture. Do not treat the source-shaped method draft or a successful first conversion as renderer acceptance.
