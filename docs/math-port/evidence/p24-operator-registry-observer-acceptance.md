# Root acceptance: operator/registry native observer bindings

Root reviewed each emitter and generated fixture diff. Only the native
children helper, registry child projection, one local Mo factory list and one
known-node parent observation change. Original expected values, calls,
dictionary/BMP observations and source loaders remain unchanged. Registry
captures the child array and initial length and uses Has/Get mapping to retain
holes; this packet does not certify hypothetical null-child error text outside
the frozen constructor stimuli.

All three frozen profiles and original `--check` oracles reproduce:
mo-values 74 assertions / 14 cases, registry 175 cases with 43 entries and
172 constructions, and Mo 23,577 assertions / 14 cases with 1,252 dictionary
entries. The scope-selected registry integrity check remains dependent on the
separately frozen scope manifest, as documented in the contract.

`node tools/math-port/operator-registry-observer-compile.cjs` compiles all
three concrete fixtures against the full 90-module implementation graph.
Compile-result SHA-256:
`08509d260c885cf213293a75ae1748e2ddd2b7c8fda1da6dd81f122fdf702d45`.
Build-log SHA-256:
`bb044a0dc70dec5dc0546a99e24f8de8093e4d0bddc890d339dc52cba040c235`.
This is a compile-only observer repair: zero semantic tests were executed by
this runner, and no missing production algorithm was accepted.

The normal repository gate still fails. A successful keep-going compiler
inventory invocation reaches nine stale fixtures (mo_order, table,
atom_semantics, simple_mml, mfenced, multiscript, arithmetic, maction,
arithmetic_order), down from the previous twelve. Inventory log SHA-256:
`439043c619705141a6310a1acbda543dae63a169637b8c90f92d266d3aabc95a`.
An initial inventory invocation used a slash-containing cmd path and failed
before CMake; that invocation is excluded. The corrected literal Windows
path reached CMake and all remaining compiler failures. No core tests ran.
