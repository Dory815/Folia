# Public math interface and Lite dispatch: isolated Gate101 diagnostics

The dependency library and existing module interfaces are the completed
Gate101 artifacts. New public-interface sources and narrowly selected output
objects are compiled into separate ignored directories. The shared build
directory and its completed checkpoint are not rebuilt or updated. These
observations do not accept the ongoing parser, Physics or Maction packets.

## Public interface

`src/math/math.{ixx,cpp}` compiles and links with a candidate that imports only
`std` and `folia.math`. The actual native document pipeline produces the SVG
and immutable Scene. No input-side or layout-side implementation is copied
into this facade.

The first isolated executable produces 8/9 exact Source records. The only
difference is the known table transform separator: `translate(0,1400)` versus
the original `translate(0 1400)`. Compiling the current mtable placement
correction and linking that object into the isolated executable produces
9/9 complete exact SVG/error records. No comparator rule or Source record is
changed. In both runs, all nine Scenes remain traversable after Renderer
destruction; 213 immutable nodes are visited.

The Source observation SHA-256 is
`4fe64e87ccaf386e19f67defd1d940d5332b247151d4d0f41b90274a500f443c`.
Its generator verifies the pinned profile and dependency hashes.

| Observation | Copied executable SHA-256 | PID | UTC interval | Exit |
| --- | --- | ---: | --- | ---: |
| Initial 8/9 | `f8ecdaf7566bae8d25a0880c00fb4fd89c12793e2d2e2e96b66c99d24c8eab11` | 38612 | 2026-10-14 16:47:40.031–16:47:44.876 | 0 |
| Table correction, 9/9 | `b263ede5ffdacb4f85c5c6bcfc3f0bf1cd61e00f2dbaacae8fb8ce034339b063` | 68096 | 2026-10-14 16:50:40.544–16:50:45.577 | 0 |

Outputs, comparison summaries, dependency hashes and process provenance are
under `build/math-port/api-isolated-g101`. The dependency library still has
23/38 registered packages and emits its 15 real configuration warnings.
These nine cases therefore prove bounded facade behavior, not full profile
acceptance. Folia still uses its existing math worker.

## Generic Lite dispatch

The Context implementation of `node` now follows actual Source
`create -> setAttributes -> append` calls. `setAttributes` uses the current
`setStyle` and `setAttribute` methods, preserving Source reads and order.
The returned nodes retain the existing canonical Lite owner and field storage.

The new regression checks a real node with style and arbitrary properties,
ordered children, replacement of append during the first child, parent
identity, and propagation of a later append exception. Its expected call
sequence is checked against the actual pinned original implementation.

The corrected Lite implementation and updated test object are linked with
the other Gate101 Foundation objects in an isolated executable. All 390
assertions in 59 tests pass. The fresh copied executable SHA-256 is
`0d3aab2e3bfccdb5ca5de1c6512648caa2d2d5ab50928fb3dbb6defece8b271c`;
PID 15180 runs from 2026-10-14T16:55:14.186Z to 16:55:16.731Z, exit 0.
Its build and process records are under `build/math-port/lite-isolated-g101`.

The complete current shared build, the 75-record renderer comparison and the
separate 30-case Menclose method comparison still require fresh validation.
