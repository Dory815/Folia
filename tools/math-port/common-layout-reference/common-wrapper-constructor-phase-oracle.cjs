'use strict';
// Additive constructor-dispatch witness for actual pinned SVG wrapper classes.
// It instruments method entry on the original prototypes, then constructs the
// real Common mixins applied to SvgWrapper (never a test-only subclass).
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../../..');
const bundle = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const mathjaxModules = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const lockFile = path.join(root, 'tools/math-port/common-layout-reference/profile-source-lock.json');
const lock = JSON.parse(fs.readFileSync(lockFile, 'utf8'));
function verify(file) {
  const expected = lock.dependencies.find(x => x.file === file);
  if (!expected) throw new Error(`missing source lock: ${file}`);
  const source = file.startsWith('src/cjs/')
    ? path.join(bundle, 'cjs', file.slice('src/cjs/'.length))
    : path.join(mathjaxModules, file);
  const actual = crypto.createHash('sha256').update(fs.readFileSync(source)).digest('hex');
  if (actual !== expected.sha256) throw new Error(`pinned CJS hash mismatch: ${file}`);
  return expected.sha256;
}
// Verify the complete locked dependency profile before the first require(),
// then also validate the actual loaded CJS closure below.
const sourceHashes = Object.fromEntries(lock.dependencies.map(({ file }) => [file, verify(file)]));
const { SvgWrappers } = require(path.join(bundle, 'cjs/output/svg/Wrappers.js'));
const { SvgWrapperFactory } = require(path.join(bundle, 'cjs/output/svg/WrapperFactory.js'));
const { MmlFactory } = require(path.join(bundle, 'cjs/core/MmlTree/MmlFactory.js'));
const watched = new Set(['getStyles', 'getStyleData', 'getVariant', 'getScale', 'getSpace', 'getMathMLSpacing', 'getTeXSpacing']);
const saved = [];
const patched = new WeakMap();
let trace = null;
for (const [kind, ctor] of Object.entries(SvgWrappers)) {
  if (!['mrow', 'mtext', 'text'].includes(kind)) continue;
  for (let p = ctor.prototype; p && p !== Object.prototype; p = Object.getPrototypeOf(p)) {
    for (const name of watched) {
      const d = Object.getOwnPropertyDescriptor(p, name);
      if (!d || typeof d.value !== 'function') continue;
      let methods = patched.get(p);
      if (!methods) patched.set(p, methods = new Set());
      if (methods.has(name)) continue;
      methods.add(name);
      const original = d.value;
      const owner = p.constructor?.name ?? '<anonymous>';
      const wrapped = function (...args) {
        if (trace) trace.push(`${owner}.${name}`);
        return Reflect.apply(original, this, args);
      };
      Object.defineProperty(p, name, { ...d, value: wrapped });
      saved.push([p, name, d]);
    }
  }
}
const factory = new SvgWrapperFactory(SvgWrappers);
// The constructor accesses the jax's font/options data only; these values are
// supplied by this observation fixture. Wrapper classes, factory and nodes
// are actual pinned Source classes/products.
factory.jax = {
  font: { params: { rule_thickness: 0 }, getVariant: () => null },
  options: {},
  math: { outputData: {} },
  pxPerEm: 16
};
const mml = new MmlFactory();
const loadedFiles = [];
for (const filename of Object.keys(require.cache)) {
  let sourcePath;
  const sourceRelative = path.relative(path.join(bundle, 'cjs'), filename);
  if (!sourceRelative.startsWith('..') && !path.isAbsolute(sourceRelative)) {
    sourcePath = `src/cjs/${sourceRelative.split(path.sep).join('/')}`;
  } else {
    const packageRelative = path.relative(mathjaxModules, filename);
    if (packageRelative.startsWith('..') || path.isAbsolute(packageRelative)) continue;
    sourcePath = packageRelative.split(path.sep).join('/');
  }
  const expected = lock.dependencies.find(entry => entry.file === sourcePath);
  if (!expected) throw new Error(`loaded source is absent from lock: ${sourcePath}`);
  loadedFiles.push({ file: sourcePath, sha256: verify(sourcePath) });
}
const cases = [];
try {
  for (const kind of ['mrow', 'mtext', 'text']) {
    const node = mml.create(kind);
    trace = [];
    const wrapper = new SvgWrappers[kind](factory, node, null);
    cases.push({ kind, ctor: wrapper.constructor.name, trace: [...trace],
      ownState: {
        childCount: wrapper.childNodes?.length ?? 0,
        hasStyles: Object.hasOwn(wrapper, 'styles'),
        hasVariant: Object.hasOwn(wrapper, 'variant'),
        variant: wrapper.variant,
        scale: wrapper.scale,
        space: wrapper.space,
        bbox: wrapper.bbox && { w: wrapper.bbox.w, h: wrapper.bbox.h, d: wrapper.bbox.d }
      }
    });
  }
} finally {
  trace = null;
  for (const [p, name, d] of saved.reverse()) Object.defineProperty(p, name, d);
}
const result = { schema: 1, source: 'pinned MathJax CJS; actual SvgWrappers constructors', sourceHashes,
  loadedFiles: loadedFiles.sort((a,b)=>a.file.localeCompare(b.file)), cases };
const out = process.argv[2] || path.join(root, 'build/math-port/common-wrapper-constructor-phase-original.json');
fs.mkdirSync(path.dirname(out), { recursive: true });
fs.writeFileSync(out, JSON.stringify(result, null, 2) + '\n');
process.stdout.write(`${cases.length} actual constructor traces -> ${out}\n`);
