# Root P19b prerequisite — one generic Property host below MML and TeX

Root reviewed all mml_node_host.ixx dependencies/bodies and the P19b raw-rest
audit. Its string/number/truthiness/get/prototype/WTF-8 operations use only the
already accepted Property, option-error, numeric and UTF16 primitives. None
requires a MML node. Only inheritanceDefaults belongs to the MML responsibility.
TeX dispatch must not import the mathematical node model for generic value work.

Move the generic operations unchanged into util/property_host.ixx and its
thread-local traversal state implementation unit util/property_host.cpp, named module
folia.math.util.property_host, namespace folia::math::property_host. Keep exactly
one implementation of each operation and the same dependencies. No parsing,
macro, regex, iterable, callback receiver, numeric or Unicode algorithm changes
are authorized by this refactor. No Source count increase: this is a native host
boundary, not another MathJax file port.

The existing mml_node_host named module becomes the MML-specific
inheritanceDefaults function and an exported import/name lookup for the shared
operations. MML keeps its domain namespace; its generic bodies are removed.
TeX imports property_host directly. This is one common implementation used by
both domains, with no old/new behavior, fallback, feature flag or duplicate path.
Root owns both declarations, CMake and verification; no agent edits are needed.
Update isolated mutation build manifests to include the shared interface/unit;
their source stimuli and expected assertions remain unchanged.

Verify generic function bodies are byte-identical after only module/namespace
header changes and removing inheritanceDefaults. Keep all existing source oracle
expectations unchanged. Run the complete core build/tests because the export
boundary is shared by the accepted MML modules. This freezes no TokenMap or
MapHandler implementation API and accepts no parser/layout/SVG/Folia gate.

Root verification: interface operation bodies and the TLS implementation body
are byte-identical after the named module/namespace moves and separating only
inheritanceDefaults. The first link exposed the omitted private TLS unit; it
was moved with the declaration and added to the explicit CMake manifest.
The repaired complete build and all core tests pass (ignored log
build/p19b-property-host-core.log). Seventeen isolated build manifests now copy
the shared interface/unit; generic mutation targets follow their moved bodies,
while MML-default mutation targets remain in the MML facade. Representative
TeX-token and raw-inheritance baselines compile/pass and their unchanged
token-constructor-attrs and spread-proto-is-data mutants are still detected.
No existing source expectations changed. Local MathJax acceptance remains 51/246.
