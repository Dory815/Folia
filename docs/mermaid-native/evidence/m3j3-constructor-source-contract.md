# M3j3 Root constructor observation contract

This packet freezes original constructor observations before Native constructor
implementation. It is not blitter, renderer, or Folia acceptance.

## File responsibilities and ownership

Root owns the architecture, Source pins, observation schema, acceptance decision
and the later Native types/stubs/red gate. Sol may create only
`tools/mermaid-native/blitter-constructor-observer.cjs`,
`blitter-constructor-oracle.cjs`, and `blitter-constructor-reference/` (manifest,
lock and Rust transport main). Sol may build an append-only copy of tiny-skia
under ignored `build/mermaid-blitter-constructor-*`. No Native production, tests,
existing expected records, or Folia changes are permitted in this assignment.

Original methods run unchanged. Every original Rust file in the copy must equal
its pinned original bytes plus its exact declared observer suffix, checked before
and after execution. Do not edit a method, expose a synthetic constructor result,
reproduce blend formulas, or call a fourth shader producer. Reuse actual full/tail
function-pointer and 15-group context readers from `shader-run-observer.cjs`.

## Observation schema

Each JSON record has `input`, `factory` (Some/None/panic), `actual_variant`,
`constructor` (Some/None/panic/not-called), ordered `warnings`, and nullable
`snapshot`. All floating values use exact f32 bits. On Some, snapshot contains
`is_mask`, optional `logical_mask` logical dimensions/real width/offset/retained slice
length, target logical dimensions/real width/offset/retained slice length,
source dimensions/retained bytes/Pattern incoming-buffer identity, optional
premultiplied RGBA memset bytes, and separately named `anti_h`, `rect`, `mask`
compiled snapshots. Each compiled snapshot reads its own precision, lane width,
full and tail function IDs (including Return), and complete context. Retain
before/after full target backing bytes. No execution or partial pipeline state is
invented for None or panic. Dummy source is observed as 1x1 transparent data,
without exposing process-specific addresses. `new_mask` has no factory and uses
the same snapshot shape with its original A8 destination.

Inputs use actual public Paint/Shader constructors and accepted original surface
constructors. Preserve distinct requested and actual shader variants. Source
pixel bytes and target/mask buffers must outlive construction and snapshot.
Warnings are captured by an actual log subscriber, with order and multiplicity;
panic hooks do not change constructor outcome. Transport serializes observations,
never supplies expected state from Native output.

## Required independent cases

The baseline is all 29 raster blend modes x 4 colour spaces x 5 valid public
Shader families (580 records). Add explicit Source witnesses for AA/HQ and mask
quadrants, opaque/translucent and SourceOver strength reduction, DestinationIn
solid versus non-solid, logical mask mismatch before Destination early return,
Source solid premultiplied byte boundaries, Clear cached transparency, Pattern
all three filters and inversion warning, factory None/panic where reachable,
nonzero clipped subviews with equal logical sizes and unequal parent strides,
extra retained Pattern bytes, and new_mask. Include constructor capacity panic
only through reachable public inputs; report it unobserved if it cannot be reached.

## Root acceptance sequence

Sol delivers candidate Source records only under ignored build output and a
small outcome summary. Root reviews suffixes, transport and case coverage,
reruns original guard/build/observations, then freezes immutable Source records.
Only afterward will Root declare Native file responsibilities and exact lifetime
types, install honest constructor stubs and matching Native observers, compile
and run a real red gate, and delegate implementation. Passing constructor records
will still require independent compiling fault injection and callback/executor
observations before complete blitter acceptance. Full shared scene/SVG/Direct2D
acceptance remains the prerequisite for Folia integration.
