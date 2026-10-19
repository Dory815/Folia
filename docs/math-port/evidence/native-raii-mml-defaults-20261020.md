# Native MML default attributes

The reviewed native implementation installs the pinned Source tables for all
43 MML kinds: 318 node-default entries and 28 global defaults. AttributeSet owns
four mathematical data layers (explicit, inherited, node default, global), with
the Source INHERIT lookup rule. These are closed attribute values, without a
generic object or property runtime. Factory-created inferred and explicit rows
receive the same defaults as directly created nodes.

The Source snapshot and extraction witness were checked in the ignored candidate
packet. The native tests compare the complete ordered tables, precedence,
null/absent values, inherited sentinels and recursively constructed rows.

The final build and complete FoliaMathTests run returned 0: **71 tests / 4,646
assertions**. Compilation used `cmake --build build/core --parallel 4 --target
FoliaMathTests`. The first test compilation required correcting a comparison
helper overload and avoiding ambiguous Boost.UT string-view comparisons; no
production change was needed for those diagnostics. Logs are ignored under
`build/math-port/native-raii-defaults-*`.

This accepts the default-attribute component only. Typed node properties,
family semantics, complete TeX parsing/packages, shared layout and full SVG
rendering remain incomplete; the application renderer has not been switched.
