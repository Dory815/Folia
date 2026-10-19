# Shared FrozenScene producer candidate (G153)

This candidate is isolated under `.scene-only-producer-g153/candidate/`; it does not edit the formal source tree, CMake, or source expectations. It contains a unified diff at `tools/math-port/candidates/shared-scene-producer-g153.patch`.

## Boundary

`SvgOutputJax::produceScene(CommonWrapper&, DOMNodeRef)` now owns the single placement/build/freeze operation. It runs the existing CommonWrapper outer-BBox path, SVG wrapper placements, inline-break handling, local font-cache registration, and `Builder::finish()` once. `processMath()` sends that exact owned Scene to a `SceneOutputConsumer` only after production succeeds. The initialized default consumer keeps the current SVG behavior: `Serializer::materialize()` followed by adaptor append.

`Renderer::renderScene()` temporarily installs a capture consumer, runs the existing HTMLMathDocument conversion, and returns a value-owned `Scene`. It does not call `Serializer::materialize()` through the default consumer or call `serializeXML()`. RAII restores the previous consumer even if conversion throws. Ordinary `Renderer::render()` still uses the default SVG consumer and serializes its result as before.

The consumer interface is a backend handoff, not a layout mode flag: the producer and placement registrations are identical for both entry points. A future Direct2D consumer can receive the same FrozenScene and read its immutable nodes/resources without parsing SVG output. Per-kind placement algorithms remain in `src/math/output/scene` and are not duplicated.

## Dependencies and limits

The producer still depends on the existing SVG `WrapperFactory` registrations, `FontCache` glyph callback, `SvgMaction`/`SvgMenclose` source surfaces, CommonWrapper/BBox, font data, and the current SVG-related Source placement equations. `initializeSvg()`'s text-measurement callback also builds and materializes a temporary snapshot when the existing unknown-text measurement path is used. The focused test uses `x+1`, whose native glyph path does not request unknown-text measurement; this candidate therefore establishes zero scene serialization/materialization for the ordinary scene-only route, not a universal guarantee for every unknown-text fallback. That fallback should be separated from DOM materialization before claiming the same guarantee for all inputs.

`CommonOutputJax::toDOM()` and `HTMLMathDocument::convert()` still create and return their ordinary wrapper/container DOM. The new API avoids materializing the completed SVG Scene into that DOM and avoids XML serialization; it is not a DOM-free parser/document conversion.

The focused candidate test checks that the capture sink receives the same Scene owner exposed by the Jax, that no SVG child is appended to its parent, and that a public scene-only result remains readable after renderer destruction. It has not been built or run, by the G153 freeze instruction. No SVG150 expected data is changed.

## Expected integration files

- `src/math/output/svg/output_jax.ixx/.cpp`
- `src/math/math.ixx/.cpp`
- existing `tests/math/bussproofs_renderer_session.cpp`

No CMake registration is needed because the API resides in already-compiled modules and the Foundation test is in an existing recursively discovered source file; Root owns shared build integration and acceptance.
