# Actual native SVG execution, Gates70–72

Gate70 exits 1 with all 1,147 captured input hashes unchanged: only the
Foundation font-option test has a const-array compile error. The real document
probe and whole-layout driver freshly link. Gate71 fixes that error and builds
all 21 targets with the same frozen input count and no changed hashes.

## First actual completed Scene

| Gate | Probe SHA256 | PID | Actual exit |
| --- | --- | ---: | ---: |
| 70 | `23860381374f0ed190e40f782342f7c22cc2fb149d8625c8ff8e3e569876def6` | 66412 | 0 |
| 71 | `1be10f97cbcc92b17fd42363ff75d141227bfb6292c419df1a4c4ed50aaddb2a` | 27424 | 0 |

These probes call the actual NativeRenderSession → HTMLMathDocument → TeX →
Common wrappers → completed Scene → SVG DOM path for `x+y`. They verify one
completed Scene, retained public document handles, and eventual Context,
service-graph and host destruction while the Scene remains owned. No substitute
MathItem, MML or layout tree participates.

Gate70 runs 16:23:38.192–16:23:39.656 UTC; Gate71 runs
16:49:29.953–16:49:31.403 UTC on 2026-10-13. Gate71 uses the same displayOverflow
and inline-linebreak options as the locked Source plain case. The serialized SVG
is still unequal (2,422 native characters versus 2,502 Source characters), with
geometry, attributes and paint-call differences. Source itself has two line
boxes for this observation, so two lines alone are not evidence of a native bug.

## Whole observations and actual package selection

Gate70 driver SHA256 is
`dc3ec18327b6d3d531134550b1e211b9d7dab564df25332643fa11a632af230b`;
PID 46688 runs 16:23:54.925–16:24:40.591 UTC and exits 0, emitting all 75
records. There are 58 SVG outputs, 17 conversion errors and 0/75 exact whole
matches. Many outputs contain parser error MML because this run selects base
only. An emitted SVG is therefore not proof of the requested feature.

The next driver receives all 38 locked package names as input, passes them into
the actual TeX constructor, reads its live options, independently queries actual
ConfigurationHandler entries and captures real configuration warnings. The
runner no longer manufactures a base package header. The comparer requires all
38 registrations and zero configuration warnings in addition to every field of
every output record. A separate actual pinned CJS registry oracle confirms 38
registered handlers, 38 configurations and zero warnings. Its comparator
positive control matches 75/75; this is Source/comparer evidence only.

Gate71 driver SHA256 is
`7e0ce19dc3bc2512fb15f431af987a92879470c139cce5b7a6850ae72ad33927`;
PID 53016 runs 16:49:29.987–16:49:31.178 UTC and exits 1 before any case,
reporting `TypeError: Cannot read properties of null or undefined`. Inspection
finds the actual TeX native options cell was not bound onto its Context identity.
The live NativeDataField repair is enrolled in Gate72 but is not yet executed.

## Actual tests and next enrollment

Gate71 Foundation exits 0, passing 295 assertions in 49 tests. Executable
SHA256 is `f6b4d2b4f704fa510d1f67f9c7cbe1276d496f0490b487bbec5d74c512d7856b`;
PID 64912 runs 16:49:27.635–16:49:29.418 UTC.

Fresh full core exits 0 with 345,635 assertions, 5,322 tests and 171 suites.
The byte-identical copied executable SHA256 is
`39d8f2204df96d7faeabed132babf0c9ef7f72bc7d640d4d9574581635ee7732`;
PID 20764 runs 16:49:41.641–16:52:46.698 UTC. Host, Parser, Regexp and
LinkedList also exit 0. The actual NewCM rejected-provider fixture requires
first-call fulfillment after the failure placeholder, then failed-file rejection;
the synthetic lifecycle record with undefined setup is a different fixture.

Gate72 enrolls Action/BBox/Braket/Cancel/Color/Enclose, with Braket and Color
separated into their upstream file responsibilities, and five Scene script
partitions. It also introduces one shared transform type and value-owned HTML
foreignObject data for future direct consumers. The build exits 1 with all
1,525 captured hashes unchanged. Script/Scene/type/serializer compilation
succeeds; package module imports, invalid if initializers, Color methods and
candidate SymbolRef serialization prevent linking. None of this new package
or XML painting behavior is accepted yet. Complete FontData extension merges
also remain unequal; the next candidate fixes its class-receiver identity.

Folia still uses its existing math QuickJS path. Full 38-package and 75-record
acceptance remains required before integration. Mermaid remains paused.
