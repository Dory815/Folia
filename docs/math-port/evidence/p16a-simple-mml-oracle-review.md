# P16a simple-MML oracle review

Read-only audit of the pinned five source files, `docs/math-port/simple-mml-contracts.md`, the generated native declarations, `tools/math-port/simple-mml-oracle.cjs`, and `tests/math/reference_simple_mml.cpp`. `node tools/math-port/simple-mml-oracle.cjs --check` succeeds and reproduces 7,627 assertions, 98 cases, 1,605 raw rows, and 281 value records. The source loader verifies each file's pinned scope hash and compares the installed CJS text to the clone before obtaining the five actual MML class constructors.

## Source and contract coverage

* `menclose.ts`: its concrete `setTeXclass` override and child-result/update sequence are exercised across ordinary returns, child result returning previous, rebinding, empty/null array/child, child throw, getter/rebind, update throw, plus both previous-argument states. Constructor/default/accessor, texclass field, inferred-row identity, and outer inheritance cases cover its generated members.
* `mpadded.ts`: raw `vbox` values span undefined/null/booleans/numbers/NaN/infinity/string/array/object truthiness, and its method cases record virtual getter, previous-class, class write and child call ordering, child rebind/throw/empty/null and both previous states. The valid inherited branch and truthy vbox branch both have observations. Defaults and class identity are checked separately.
* `maligngroup.ts`: tests include the concrete child-inheritance hook, live attributes access, virtual merge return/input alias, mutations and throws, child rebind/append/mutation/throw, null attributes/child/list, and raw inherited display/level/prime rows. The generic inherited propagation is exercised through actual children and outer trees.
* `malignmark.ts`: all concrete members are generated accessors/defaults/class identity, checked in the defaults/getters/fields case. Its outer tree behavior is also included for 0/1/2 children; no invented method implementation is expected.
* `mglyph.ts`: strict source/family/index raw-value combinations are generated from actual CJS, including empty/nonempty strings, missing-like, null, booleans, numbers, arrays and objects; the invalid error path, options alias, superclass validation path, error throw, null attributes, and real short/full error trees are included. Probe `attributes` logs its access, and `mError` logs the exact message/short flag and retains the options handle.

The explicit method ordering in the frozen contract agrees with the pinned sources. The TeXclass tests use source probe subclasses around the actual class methods, while the real outer-tree tests use actual classes. No incorrect default values or source-derived expectation mismatch was found in this audit. The oracle check validates generated-file reproducibility; no build was run as requested.

## Coverage gap: glyph verification without explicit `src`, `fontfamily`, and `index`

The 1,331-row glyph matrix (`11^3`) always calls `attributes.setList({src, fontfamily, index, ...})`, including when a matrix value is JavaScript `undefined`. Thus every one of the three keys exists explicitly on the attribute object. The separate glyph error/no-checks cases and real error-tree cases also set all three keys explicitly. None invokes `MmlMglyph.verifyAttributes` with one or more keys absent so that `Attributes.getList` resolves from the node's default/inherited layers.

That matters because upstream line 62 invokes the real `this.attributes.getList`, and defaults contribute `src:''` and `index:''`; the method's strict test at line 67 uses the resulting values. A faulty native implementation reading only explicit attributes could match every current glyph row and still differ for default or inherited values. Suggested actual-CJS witnesses:

1. Leave all attributes unset on a freshly constructed `mglyph` and invoke `verifyAttributes`. Assert the observed default values and whether the invalid-glyph diagnostic is issued.
2. Set only `src` to a nonempty string and leave `fontfamily`/`index` absent; observe how the default `index` participates in the branch.
3. Set `src:''`, leave `fontfamily` missing, and set `index` to a nonempty string; this distinguishes strict missing-value handling from coercion/defaulting of absent `fontfamily` to `''`.
4. Where the upstream Attributes API permits it, set an inherited `fontfamily` while leaving it non-explicit and verify `getList` sees that inherited value.

These would extend verification across the attribute lookup chain without changing the frozen expectations for existing rows. They are a coverage suggestion, not a source-port disposition.

## Boundary notes

The C++ types intentionally represent ordinary typed MML attributes and child vectors, not arbitrary JavaScript proxies or accessor descriptors. Accordingly, the current probe records semantic member calls and mutable attribute/tree state rather than attempting general JS reflection. The glyph matrix does distinguish strict values from coercion; however it does not distinguish absent properties from explicit `undefined`, which is the source-derived gap above. Geometry/resource behavior for `menclose`, `mpadded`, and `mglyph` remains outside these source files and is not implied by this oracle.
