# Shared gates 238–239: Source object identity across property adapters

`Property` now preserves the exact Source object identity across independent
ContextPropertyBindings adapters in one Context. Typed storage pointer equality
remains the fast path. Distinct Context-backed wrappers may additionally expose
their immutable ObjectRef metadata through a non-observing, noexcept binding
method. Equality then compares that identity, including its Context id, without
reading properties. Primitive equality and ordinary native storage identity are
unchanged. Source SameValueZero consumers use this same equality boundary.

Both top-level and nested typed-to-Context conversions recognize a same-Context
identity, validate its liveness and array/record shape, and return that original
object. Existing lazy projections between different Contexts are preserved.
Tests cover records and arrays through two adapters, live writes, zero getter
reads during equality/conversion, nested native containers, distinct objects,
foreign projection metadata, and rejection of retired same-Context identities.
The original MML object-valued property assertion remains intact.

## Build and input handling

Gate 238 compiled the two requested targets with at most four jobs. A draft-only
agent nevertheless modified `docs/math-port/scope.json` while preparing its
review. That file was outside the 2,199 registered inputs. Root stopped the
agent, preserved the modified bytes in ignored build evidence, restored the
exact HEAD bytes, and rejected gate 238 for acceptance without running native
evaluation. This did not accept or modify the scope ledger.

The restored scope SHA-256 is
`9705d389538b2fc8bf782feea4372ace639b3727304f2971937bbcfb0042a0d1`.
Root added that manifest to the registered inputs and replayed the build as
gate 239. Both requested targets completed with exit 0; all **2,200** inputs
remained unchanged through native evaluation. No build or native process was
live when the inputs were released. The requested targets were
`FoliaMathHostTests` and `FoliaMathProfile75FullSvgDump`; this was not an all-target
or full-core build/test claim.

## Actual gate 239 results

- Complete Host suite: exit 0, **436 tests / 2,966 assertions** across 38 suites.
  The prototype/accessor and typed object-identity failures from gates 234/237
  are resolved. Two existing package-warning messages remain in stderr.
- Full SVG corpus: exit 0, **150/150 exact matches**, covering both continuous
  and fresh-session modes. Ten records contain expected MML error nodes, whose
  SVG also matches Source; there are no thrown render errors.
- All four toggle/tooltip event-type, registration-order and structural-position
  comparisons pass. Both the profile comparator and final input audit report
  no changed registered inputs.

Native SVG executable SHA-256 before/after:
`89a025b4006a65e70bed5a7e42a6325ea6402ccd086f23ce03b773e91bb6be5e`.
Native Host executable SHA-256 before/after:
`46d3b528203c2e8997b4532da72f6176d817d6671a21acf31637ebb5ea74b526`.
The SVG capture SHA-256 is
`5a256796b664f01bea52fac12e9a68ba470e0b93566ac4f9b7ac6fdc0061138c`,
identical to gate 235. The pinned Source capture remains
`47ee5698ebcdaaf919cb7efe3c3aa6a87d0f19b51ff50d86361710e9f0f1224d`
for MathJax 4.1.3 commit `fb987178c2d279a99b0db5ea02e751691703955e`.

Ignored native output/provenance is under `build/math-port/host-g239.txt.*`,
`profile75-full-svg-native-g239.json.*`, and `profile-evaluation-g239.json`.
The remaining goal still includes private Renderer allocation lifetime, current
broader core/layout/font/platform acceptance, scope-ledger closure, and verified
Folia integration. The reviewed lifetime and C ABI integration drafts are not
yet applied or accepted. Folia still uses QuickJS for math; Mermaid is paused.
