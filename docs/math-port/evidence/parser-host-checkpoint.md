# Parser host checkpoint

This checkpoint covers ordinary Array methods and the emitted TypeScript
iterator protocol used by the pinned MathJax 4.1.3 TeX source. It does not
accept the TeX frontend or new base/textmacros package implementations.

- Array oracle: 69 original observations, 60 native matches, 9 explicit
  unsupported object-coercion/custom-species observations.
- Iterator oracle: 21 original observations, with native candidates checking
  values, identity, operation order, partial failures and IteratorClose.
- Production host gate: 239 assertions in 149 tests passed across the combined
  host, previously accepted Array map/iteration, and live Property suites.
- Fault gate: all 33 actual production-source faults compiled, failed their
  intended native assertion, and passed after exact source restoration.
  `parser-host-native-faults.json` records outcomes and baseline source hashes.
- Required core gate: `cmd /c build_test.bat` completed with exit 0, 326734
  assertions in 5226 tests across 160 reported suites. The ignored log is
  `build/parser-family-full-core-2.log`.

The host gate was run in the shared working tree with actual native
ParserRuntime/Configuration implementations. Its explicit target and parser
source integration remain part of the ongoing frontend checkpoint. Ordinary
host sources are committed separately so this change stays reviewable.

After this successful core run, the newly added base/textmacros source units
were included in a production archive compile. That first compile failed on
real interface/type errors; package owners are repairing them. Their presence
on disk, module metadata, original-side oracle success, and compilation alone
are not package behavior acceptance. Shared mathematical layout, SVG parity,
and Folia integration remain outstanding.
