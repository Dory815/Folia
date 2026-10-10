# P12c root review — complete MmlMo packet

Status: locally accepted by root after source review, core regression and all
33 compiling/assertion-failing mutants. This is local semantic-node acceptance,
not a renderer or production gate.

## Root authority and frozen architecture

Root committed the declarations, constructor-static services, generated fields,
initial actual-CJS oracle, file ownership and contract in 400b5c3 before dispatch.
The three agents could modify only one assigned CPP and one mapping report each.
Root owns all interfaces, data, observations, mutations, builds, scope and
acceptance. The agent reports establish correspondence, not acceptance.

MmlMo remains an actual token in the established MathML hierarchy. Its three
CPP files partition the 17 assigned source methods: lookup/forms, core/class
adjustment, and inherited/script/accent checks. Root owns the remaining three
executable accessors and exact static/instance fields. Generic class metadata
transports captured family services and does not contain operator algorithms.
There is no custom math tree, parser switch, renderer geometry or application
registration in this packet.

The source is the previously locked MathJax 4.1.3 mo.ts and resolved CJS; every
oracle imports mo-pattern-reference.cjs, which validates the package/clone,
Node version and the entire pinned compiled package tree. The AST ledger has
32 members, including 20 executable members. Every member has its individual
root-reviewed mapping in p12-mo-members.json.

## Independent review corrections

Root found and corrected these source-order errors in the delivered code:

- Reading Parent after an already-defined mathaccent early return.
- Reading under/over children after base callbacks instead of before them.
- Coercing each prime remap before all mapping reads had completed.
- Reading definition properties before the virtual attributes receiver.
- Writing prev.texClass before this.prevClass in a right-associative assignment.
- Dereferencing an unchecked virtual coreParent result.

The source's under.coreMO() expression for isOver and explicit-parent-accent
false result are retained. They were not replaced with more intuitive behavior.
Native field initialization preserves Missing during superclass hooks and then
null/5/18 after that phase. Lazy texClass reads retain strict null and no cache.

The initial coreText failure was a test-class-profile gap: native isKind needs
the TeXAtom constructor binding just as source instanceof does. The main
fixture supplies an uninstantiable negative type predicate for the nodes it
contains. The order fixture uses a real test-only Atom subclass of the accepted
base hierarchy to exercise the true unwrapping branch. Neither registers a
partial production TeXAtom or weakens source behavior.

Root also found a required host-boundary gap for operator form names that hit
native functions on Object.prototype. Function.prototype identity, constructor,
call/apply/bind/toString, name/length/prototype and restricted caller/arguments
access are now separately bound. Number.prototype string conversion is zero.
The intrinsic metadata oracle observes 26 source identities directly. This
binding exposes metadata and typed failures, not ECMAScript execution.

## Verification evidence

The final actual-CJS fixtures contain:

| Fixture | Assertions | Cases | Independent observations |
| --- | ---: | ---: | --- |
| reference_mo.cpp | 23,577 | 14 | All 1,252 dictionary entries; raw class/level matrix; form/spacing; primes; aliases and constructor capture |
| reference_mo_order.cpp | 538 | 50 | Virtual/static read traces; fields during construction; snapshot mutation; null/error states; nested traversal; live replacement; intrinsic identities |
| reference_mo_values.cpp | 74 | 14 | Primitive string/number/boolean/array definition properties; exact own-key sets; malformed tables and partial state |
| reference_mo_patterns.cpp | 1,136 | 16 | Existing P12b complete UTF-16-domain pattern and metadata evidence |

Generated fields have 22 source default overrides, ten prime remaps and five
default pattern handles. Data and all fixtures reproduce under --check. Expected
values come exclusively from actual pinned CJS or source intrinsic observations.
Compaction of dictionary expectations shares equal expected vectors; it does
not reduce observed fields or dictionary coverage.

The complete `cmd /c build_test.bat 2>&1` passed in build/p12c-mo-core.log,
including 23,577 and 538 Mo assertions, all prior differential suites and the
existing editor/core regression. Subsequent unchanged-source value witnesses
passed the focused build; the final 74 assertions, including the exact own-key
extension, pass in build/p12c-mo-values-final.log. The isolated final baselines
also compile and pass all four unchanged fixtures together.

The first 31 isolated mutants all compiled and failed unchanged assertions,
with working source/fixture bytes unchanged. They cover null/Missing, caching,
spacing, source misses, form priority/filtering, identity, constructors, setter
and receiver order, ancestor/BIN rules, snapshots, source accent quirks, script
order, optional text, prime mapping and intrinsic metadata. The initial
isKind mutation was equivalent because native isKind itself uses instanceof;
root replaced it with an actually wrong string-kind comparison. It is not
counted as a detected error. The final two mutants target omitted string keys
and UTF-8-byte key enumeration instead of source UTF-16 keys. Both compile and
fail the strengthened unchanged assertions, retaining byte-equal working
sources and fixtures. There are 33 distinct detections across these runs.

Full-run results: build/math-port-mo-mutations/results.json. Selected added
results: results-string-property-keys-omitted.json and
results-string-property-keys-count-utf8-bytes.json in the same ignored directory.
Compilation failures, crashes and surviving/equivalent mutants never count as
successful detection.

## Explicit adaptation and remaining obligations

Property values retain null/Missing, scalar values, object/array identities,
ordinary/prototype lookup, ordered own keys and array/string iterator behavior.
Native constructor services retain live replacement and captured handle lifetime.
They bind source semantics required by the raw-TeX profile and native extension
classes; they do not expose arbitrary evaluated JS descriptors, symbols,
iterator overrides or regex compilation. Invalid native service bindings fail
explicitly. Node callbacks and malformed source-reachable string names have
actual-CJS order and failure witnesses.

This packet cannot establish G0–G7, SVG parity or Folia replacement. The complete
MML registry and remaining nodes, TeX packages, fonts, common wrappers, extracted
scene placement, SVG emission, Direct2D and application integration remain.
The sole production math backend is unchanged until the full release gates pass.
Local source acceptance is now 26/246; the remaining 220 source entries still
block full G0.
