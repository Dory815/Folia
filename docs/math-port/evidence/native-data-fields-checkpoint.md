# Native ordinary data field checkpoint

`Context::bindData` binds an ordinary own data descriptor to operations on one
real C++ owner. The record stores descriptor flags and insertion order; it
does not keep a second copy of the field value. `bindNativeValue` binds fields
whose actual native type is `options::Value`.

The focused gate passed in `build/native-fields-host-1.log`: 271 assertions in
156 tests, including 32 assertions in seven new native field tests. These
exercise direct owner changes, Context writes, aliases, descriptor reflection,
metadata enumeration without value reads, read-only/non-configurable flags,
prototype shadowing, deletion/redefinition, invalid-context rejection, and
actual owner lifetime retention.

The required `cmd /c build_test.bat` gate also exited zero in
`build/native-fields-full-core-1.log`: 326766 assertions, 5233 tests, 161 suites.
This is a regression gate for the linked checkpoint; the separate real
frontend comparison still fails during BasePackage initialization because
the RegExp host has not yet implemented the required `i` flag.

Deleting a bound field removes its Source own descriptor and invokes the
owner's deletion operation. The physical native field binding remains so a
later ordinary data definition reaches the same C++ storage. Replacing a bound
field with a JS accessor is explicitly unsupported. Native arrays require
their own complete storage binding. Owners without a deletion operation reject
deletion before the descriptor is removed.

This checkpoint accepts the bounded host data-field API. It does not accept
arbitrary native record conversion, native array reflection, all parser class
fields, class inheritance metadata, TeX parsing, mathematical layout, SVG, or
Folia integration. Installing fields in their real Source initialization
phases and preserving real graph/factory lifetimes remain package integration
requirements. Existing host mutation evidence predates this host change;
its recorded source hashes are historical, not a current-source fault gate.
