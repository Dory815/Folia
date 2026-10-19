# TeX input-jax implementation status

Pinned Source is MathJax 4.1.3 `ts/input/tex.ts`; CJS is locked by the existing
MathJax scope and independently rechecked by
`tools/math-port/tex-jax-original-oracle.cjs`. The script generated eleven cases
from the actual installed/pinned CJS class, including constructor and method
metadata, constructor state, static configure/tags, reset arguments, FindTeX
delegation, MML error creation, MmlFactory assignment, successful real parser
compilation, and a real TeX error callback. Its generated observations are in
`tools/math-port/tex-jax-original.json`.

`TeX::initializeModule` registers the Source class and prototype shape;
`TeX::initializeTeX` performs option splitting, the AbstractInputJax phase,
actual FindTeX/ParserConfiguration/ParseOptions setup, configuration hooks,
tag setup, and seven Source-ordered postfilters. Constructor defaults now pass
the actual `[this.options, TagsFactory.OPTIONS]` records to ParseOptions before
applying the rest options. `reset` and `compile` re-read the virtual
`parseOptions()` accessor at the corresponding Source accesses. `formatError`,
`compile`, `findMath`, `configure`, and `tags` have direct typed paths.

The Source-owned field sequence is recorded for the shared live-field adapter:
AbstractInputJax creates `adaptor`, `mmlFactory`, `options`, `preFilters`, and
`postFilters`; TeX then creates `findTeX`, `configuration`, and `_parseOptions`.
`latex` and `mathNode` are first assigned during `compile`. `parseOptions` is a
nonenumerable prototype getter with no setter. `reset` reads that getter twice
(clear, then tags reset), and `compile` repeatedly re-reads it; the oracle logs
the actual getter/method trace. These fields must be connected to the same
Context object rather than synchronized copies.

The C++ source has not been compiled or compared against native observations in
this packet. Context invocation of `compile` remains blocked because the core
Context layer has no canonical MathItem/MathDocument service adapter, and
Context invocation of `formatError` cannot return the resulting MML node until
`mml::NodeValue` is mapped to the same Context object. No mock parser or second
MML graph was introduced. The candidate native comparisons and these missing
identity edges are recorded in
`tools/math-port/parser-family-reference/tex-jax-native-candidate.md`.

The TeX compile filters are implemented locally in `tex_jax.cpp` to preserve
their ordering and actual node/ParseOptions mutations. They still require
native comparisons through the actual parser graph before acceptance.
