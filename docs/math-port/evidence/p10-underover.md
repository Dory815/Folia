# P10 `munderover` port evidence

Pinned MathJax 4.1.3 source: `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/munderover.ts`, SHA-256 `b9bf04f77922c50044c4d99b3093882cab9b35ec36203f20e1e999997f9adf82`.
Resolved CJS source: `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNodes/munderover.js`, SHA-256 `104a6ed2f1a8967726c4a0da6ffd0ffb917c23fe0b3847138161c975fa0785b9`.
The native definitions are in `src/math/core/mml_tree/mml_nodes/munderover.cpp`; declarations and constructor static bindings are root-owned in `munderover.ixx` and `munderover_bindings.cpp`.

## Member mapping

| Upstream member | Source | Native implementation / binding |
| --- | --- | --- |
| `MmlMunderover.defaults` | TS 38-45 | Root-owned `MmlMunderover::defaults()` binding in `munderover_bindings.cpp` to `munderoverDefaults()`; no duplicate record. |
| `MmlMunderover.ACCENTS` | TS 49 | Root-owned static binding through `constructorAccents()`; local `shared_ptr<vector<Property>>` retains the captured array identity while later static replacement does not affect this call. |
| `kind`, `arity`, `base`, `under`, `over`, `linebreakContainer` | TS 51-92 | `MmlMunderover` methods in `munderover.cpp`; child index getters remain virtual. |
| `setChildInheritedAttributes` | TS 102-137 | `MmlMunderover::setChildInheritedAttributes` in `munderover.cpp`. Captures `childNodes` once, stages source-observable getter/argument operations, calls base before force, captures constructor ACCENTS after force, and checks the third slot after earlier writes. |
| `getScriptlevel` | TS 144-161 | `MmlMunderover::getScriptlevel`. Uses numeric `level + 1` when force/accent falsiness requires increment; otherwise returns the original `Property`, preserving raw identity/type. |
| `setInheritedAccent` | TS 167-190 | `MmlMunderover::setInheritedAccent`. Reads current child array, tests explicit presence before embellishment, gets the core operator accent, writes inherited state, compares strictly with current default, and if needed re-inherits using a fresh empty attribute record and original display/prime. |
| `MmlMunder.defaults`, `kind`, `arity` | TS 196-220 | Root-owned defaults binding and native overrides in `munderover.cpp`; no own ACCENTS binding, so constructor capture resolves the inherited parent binding. |
| `MmlMover.defaults`, `ACCENTS`, `kind`, `arity`, `over`, `under` | TS 226-273 | Root-owned defaults/reversed ACCENTS bindings and native overrides in `munderover.cpp`. |

## Ordering and host adaptations

`setChildInheritedAttributes` uses `tree::TreeCallGuard<MmlNode>` around recursive calls. It checks/captures the base receiver before evaluating `prime || !!nodes[this.over]`; the virtual `over()` getter is called only on the falsey-prime branch. The base receives the original raw truthy `prime` or the booleanized current over-slot presence. After that virtual call, force is read only for falsey display, with the current base, core operator and `movablelimits` attribute accessed in source order. Only then is the constructor's ACCENTS array captured.

Under and over receiver checks precede their arguments. Accent slot lookup, script-level lookup, and prime computation are staged in source order. The prime expression preserves a truthy raw property and skips the virtual `under()` getter; on falsey input it evaluates that getter and passes its comparison result. Each child setter receives a false display value. The captured child-array handle preserves live slot changes; a missing third slot returns after base and under effects. Missing/null required receivers become host `TypeError`s rather than C++ undefined behavior.

The TS parameters are annotated as booleans/numbers but the protected hooks can receive raw runtime values. The port therefore uses `Property` for display, level, and prime. `host::number(level) + 1` models the source's `level++`; `getScriptlevel` otherwise returns the original level. Sparse or out-of-range ACCENTS slots become `Missing`, then `host::string`/`host::debugText` models JavaScript property-key conversion to `"undefined"`. Property strict equality preserves NaN inequality and shared-object identity.

`setInheritedAccent` rereads the node's current child array rather than using the earlier captured array. It reads current attributes and checks explicit presence before testing the child; if the child is absent, the typed host reports the source's dereference failure. It obtains embellishment/core operator/accent, writes the inherited value to the current attribute record, then rereads attributes for the default comparison. A non-default value triggers a fresh `AttributeList` and a new script-level calculation before forwarding the original display and prime values. These steps keep intervening virtual hooks and partial mutations observable.

The interfaces provide typed native nodes, attributes, and property values rather than arbitrary JavaScript proxies/getters. Where the production behavior depends on native virtual node/attribute hooks, the implementation calls them in source order. Static ACCENTS identity/mutation/replacement behavior is supplied by the root-owned constructor binding and shared vector handles; no local fallback array is synthesized.

No build or fixture execution was performed in this packet, per assignment. Root owns independent source review and validation.
