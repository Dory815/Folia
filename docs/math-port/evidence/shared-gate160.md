# Gate160: Scene owner runtime regression

Gate159 integrated shared Source list add/remove operations and live Scene
children/attributes owners, but did not compile. Gate160 applies the reviewed
three-file type correction: Context Value calls use callMember, property/string
conversions use their actual boundaries, imported SVG children resolve actual
DOM identities, and the Scene tests use the correct module types.

All 55 targets compile. The 2,050 captured inputs remain unchanged through
evaluation. The build and evaluation reuse build/core and do not copy binaries.

Runtime results:

- Host, Parser, Regexp and LinkedList complete with exit 0.
- Core, public API and NativeDocument terminate with 3221225477.
- Full public SVG native capture terminates with 3221225477; its Node runner
  returns 1. These are separate process exit codes, both retained in evidence.
- Common layout stops at the first plain conversion with the same native code.
- Foundation exits 1, including the new SVG-helper identity test and the
  existing live Scene-handle test.
- All 25 package captures exit 0. The twenty comparisons are not run while
  the public renderer is unavailable.

Authoritative ignored artifacts are build/math-font-package-gate-160.log,
build/math-port/gate-160-inputs-before.json,
build/math-port/gate-160-post-evaluation-changes.json,
build/math-port/failed-renderer-evaluation-g160.json, and the binary-specific
provenance files named by that evaluation summary. All executable hashes are
checked before and after their runs. Gate159's original diagnostics remain in
build/math-font-package-gate-159.log.

The regression is unresolved at this checkpoint. Gate158's 150/150 public SVG
comparison applies to its earlier code only. No renderer acceptance, Folia
integration or Mermaid progress is claimed here.
