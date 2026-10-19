# Typed foundation graph transition

The root CMake graph now lists only reviewed typed mathematical foundation
units in the single `folia_math` library. The withdrawn object runtime and its
projection/adaptor graph are no longer build inputs. There is no selector for
the previous implementation and no compatibility library.

This is an incomplete reconstruction, not Renderer or SVG profile acceptance.
The TeX parser, package handlers, node-family algorithms, font repository,
shared CommonWrapper placement, Renderer and C ABI are not yet available in
this graph. Folia's current application worker has not switched backends.

`FoliaMathTests` is an explicit typed foundation suite. The historical
`tests/math` sources remain tracked but are excluded from the broad core-test
glob. Their previous pass counts cannot be reported for the rewrite. The
acceptance and inventory documents require mathematical assertions to be
migrated and rerun; only withdrawn JS object contracts may be retired. Old
math probe/bridge targets are removed from the current build until their typed
dependencies are implemented. Offline pinned Source oracles remain usable.

The core query tests still exercise the actual native regex service alongside
the editor's independent search policy. Console tests link the repository
runtime for stderr reporting. Builds use at most four compiler jobs and reuse
`build/core`; the two primary test executables avoid large incremental linker
outputs.

Old uncompiled source files, including earlier local lifecycle changes, have
not been silently deleted or accepted. Their disposition is recorded by the
inventory and they will be removed or rewritten in responsibility-sized
commits. The active import and source closure is the architectural check;
leaving these historical files present does not make them a runtime fallback.
