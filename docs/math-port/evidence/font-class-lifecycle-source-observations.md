# FontData class and owner source observations

This is a supplemental source-only observation, not native acceptance. The
runner is `tools/math-port/font-class-lifecycle-oracle.cjs`; it loads the locked
MathJax 4.1.3 CJS package and writes its observations under ignored `build/`.

The runner pins these TypeScript files:

| Source | SHA-256 |
| --- | --- |
| `.reference/MathJax-src/ts/output/common/FontData.ts` | `7d8eeca536fa1b7c14517590b9c5a372ad10b34667d196d09b030f88cda780b2` |
| `.reference/MathJax-src/ts/output/svg/FontData.ts` | `403f59e6cb71fa8baf291402e085daaa25d1d11e8fc48640e370498da6a83da9` |

It also records the actual installed CJS hashes and checks the package version.
The probes cover `FontData`'s constructor and `CLASS` getter (lines 747–748,
919–938), its static `charOptions`/`addExtension` (747–777, 870–917), the SVG
overrides (101–119), and the instance `addExtension`/dynamic loading and access
methods (955 onward, 1280 onward).

The observations establish several binding-sensitive facts:

- The constructor stores own fields in this order: `variant`, `delimiters`,
  `cssFontMap`, `cssFontPrefix`, `remapChars`, `skewIcFactor`, `options`,
  `params`, `sizeVariants`, `stretchVariants`, `cssFamilyPrefix`. They are
  ordinary writable, enumerable, configurable data properties.
- The constructor captures `const CLASS = this.CLASS` once before making
  virtual calls. Its call order is `defineCssFonts`, `createVariants`,
  `defineDelimiters`, each `defineChars` in source key order, the three
  `defineRemap` calls, and `defineDynamicCharacters`. If an early override
  replaces the receiver's `constructor`, the remaining constructor phases
  still use the captured class value; a later `CLASS` getter read observes the
  replaced own `constructor`.
- `SvgFontData` has its own `OPTIONS`, `JAX`, `charOptions`, and
  `addExtension`, inherits the actual `FontData` constructor/prototype, and
  its `OPTIONS` retains the base options before overriding `dynamicPrefix`.
  `MathJaxNewcmFont` is a further derived constructor with its own `NAME`,
  `OPTIONS`, font tables, and `variantCacheIds`; its class identity is not
  inferred from `JAX` or a name string.
- Inherited static calls retain the derived constructor as `this`: base
  `addExtension` reads that receiver's `JAX`, invokes its
  `defineDynamicFiles`/`adjustDelimiters`, and mutates its own extension
  registry. The SVG override first runs the base static algorithm and then
  merges `cacheIds` on the same derived constructor.
- `charOptions` adds a new fourth slot only when tuple length is exactly three;
  when a fourth slot already exists it returns that exact options object.
  Non-array glyph data throws an ordinary `Error` with the Source message.
- `getVariant` returns the same live variant record, and `getChar` returns the
  same tuple object stored in the character map. The instance `styles`
  accessor is on the prototype; it is not an own `styles` data field.

The emitted CJS method inventory and descriptors are included in the ignored
JSON result. This evidence can guide the existing `FontDataContextBindings`
owner without introducing another constructor registry or mirrored font
state. Native Context constructor identity, dynamic method invocation,
descriptor reflection, and owner retention still require their own native
tests and acceptance.

The runner additionally serializes every own static constructor descriptor and every own prototype descriptor for FontData, SvgFontData, and NewCM (including all method arities and accessor flags), rather than only the selected dispatch probes. These are recorded in uild/math-port/font-class-lifecycle-original.json under each class's staticDescriptors and prototypeDescriptors.
