# P56 ordinary native Array.map acceptance

The existing Context now supplies the ordinary-array map host required by
actual Configuration._create's preprocessor/postprocessor maps. It retains the
real prototype callable and descriptor, captures length, validates callback,
Gets the current constructor and creates a real result array before live
Has/Get/callback iteration. Returned values retain identity; holes stay holes;
own result data properties bypass inherited setters.

Root froze declarations, registration, original observations and a failing stub
before implementation (ee4a52d). Luna implemented the complete arrayMap body,
while another Luna independently reviewed the original/native observers and
the stable body. Review strengthened exact object thisArg and own/inherited
accessor receiver observations, and guarded native callback arguments, optional
metadata and looked-up method shapes against malformed implementations. No
blocking review finding remains. The existing forEach body is unchanged.

The pinned Source caller hash is Configuration.ts. The oracle executes original
Node 24.13.0 ordinary Array.map and produces 41 exact observations; two separate
native assertions check explicit unsupported custom constructors and rejected
generic receivers. The complete original corpus and scope assertions pass:
43 assertions in 42 tests, exit 0, in build/p56-array-map-baseline.log.
The oracle reproduces with --check.

All eighteen compiling faults in p56-option-array-map-mutations.json produce
assertion failures, exit 1, then the restored correct implementation passes.
They exercise length/holes/prototypes, output length, constructor Get order,
callback argument shape/index/this/array identity, own result data creation,
callback return, indexed placement, completion, invalid callbacks and writable
descriptors. The custom-constructor defaulting fault is detected by the explicit
native-scope assertion; that assertion is not claimed as original species parity.
Gold is byte-identical during injection and the forEach suffix is preserved.

The complete normal core build and test suite passes, exit 0, recorded in
build/p56-full-core.log. All validation reuses build/core.

This host supports ordinary Context arrays with default intrinsic Array or
Undefined constructors, native callbacks and accessor effects. Primitive invalid
constructors produce the observed TypeError. Nondefault object constructors/
species fail explicitly as Unimplemented; generic non-array receivers and JS
evaluation remain outside this host profile. Empty-result identity and signed
zero index observations are minor coverage limits noted by independent review;
the body allocates a new result and generates nonnegative integer indices.

Configuration, runtime initialization, Tags/ColumnParser and their real parser
peers remain separate incomplete Source responsibilities. The sustained batch
in parser-family-work-plan.md assigns those complete units and independent
original verification. No functioning native TeX, SVG parity or Folia integration
is accepted by this host gate.
