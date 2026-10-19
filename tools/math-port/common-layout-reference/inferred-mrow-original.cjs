'use strict';
// Source witness for the real CommonInferredMrow mixin applied to the pinned
// SVG wrapper hierarchy. The MML nodes, factory, and wrappers are the actual
// CJS implementations; only the Jax service values are a bounded trace host.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../../..');
const bundle = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const lock = JSON.parse(fs.readFileSync(path.join(__dirname, 'profile-source-lock.json'), 'utf8'));
const sourceHashes = {};
for (const { file, sha256 } of lock.dependencies) {
  const source = file.startsWith('src/cjs/')
    ? path.join(bundle, 'cjs', file.slice('src/cjs/'.length))
    : path.join(packageRoot, file);
  const actual = crypto.createHash('sha256').update(fs.readFileSync(source)).digest('hex');
  if (actual !== sha256) throw new Error(`pinned CJS hash mismatch: ${file}`);
  sourceHashes[file] = actual;
}

const { SvgWrappers } = require(path.join(bundle, 'cjs/output/svg/Wrappers.js'));
const { SvgWrapperFactory } = require(path.join(bundle, 'cjs/output/svg/WrapperFactory.js'));
const { MmlFactory } = require(path.join(bundle, 'cjs/core/MmlTree/MmlFactory.js'));
const loadedFiles = [];
for (const filename of Object.keys(require.cache)) {
  const sourceRelative = path.relative(path.join(bundle, 'cjs'), filename);
  let sourceFile;
  if (!sourceRelative.startsWith('..') && !path.isAbsolute(sourceRelative)) {
    sourceFile = `src/cjs/${sourceRelative.split(path.sep).join('/')}`;
  } else {
    const packageRelative = path.relative(packageRoot, filename);
    if (packageRelative.startsWith('..') || path.isAbsolute(packageRelative)) continue;
    sourceFile = packageRelative.split(path.sep).join('/');
  }
  const expected = lock.dependencies.find(entry => entry.file === sourceFile);
  if (!expected) throw new Error(`loaded source is absent from lock: ${sourceFile}`);
  loadedFiles.push({ file: sourceFile, sha256: sourceHashes[sourceFile] });
}
const trace = [];
for (const [ctor, label] of [[SvgWrappers.mrow, 'mrow'], [SvgWrappers.inferredMrow, 'inferredMrow']]) {
  for (let proto = ctor.prototype; proto; proto = Object.getPrototypeOf(proto)) {
    const desc = Object.getOwnPropertyDescriptor(proto, 'getScale');
    if (!desc || typeof desc.value !== 'function') continue;
    const original = desc.value;
    Object.defineProperty(proto, 'getScale', { ...desc, value: function (...args) {
      trace.push(`${label}:${this.constructor.name}:getScale`);
      return Reflect.apply(original, this, args);
    }});
    break;
  }
}
const wrappers = new SvgWrapperFactory(SvgWrappers);
wrappers.jax = {
  font: { params: { rule_thickness: 0 }, getVariant: () => null },
  options: {}, math: { outputData: {} }, pxPerEm: 16, nodeMap: new Map()
};
const mml = new MmlFactory();
const parentNode = mml.create('math');
const parent = new SvgWrappers.math(wrappers, parentNode, null);
parent.bbox.scale = 2.5;
trace.length = 0;
const inferredNode = mml.create('inferredMrow');
const inferred = new SvgWrappers.inferredMrow(wrappers, inferredNode, parent);
const cases = [{
  name: 'inferred-mrow-derived-getScale-dispatch',
  actualConstructor: inferred.constructor.name,
  actualParentConstructor: parent.constructor.name,
  parentScale: parent.bbox.scale,
  ownBBox: { scale: inferred.bbox.scale, rscale: inferred.bbox.rscale },
  trace: [...trace],
  childSlotsLength: inferred.childNodes.length
}];
// Isolated direct-method error witnesses use the real Source prototype method;
// receiver records expose only the slots this method reads/writes.
for (const [name, receiver] of [
  ['null-parent', { parent: null, bbox: {} }],
  ['null-parent-bbox', { parent: { bbox: null }, bbox: {} }],
  ['null-own-bbox', { parent: { bbox: { scale: 3 } }, bbox: null }]
]) {
  try {
    Reflect.apply(SvgWrappers.inferredMrow.prototype.getScale, receiver, []);
    cases.push({ name, outcome: { returned: true } });
  } catch (error) {
    cases.push({ name, outcome: { name: error.name, message: error.message } });
  }
}
const result = {
  schema: 1,
  source: 'pinned MathJax CJS actual SvgWrappers + MmlFactory; bounded Jax service values',
  sourceHashes,
  loadedFiles: loadedFiles.sort((a, b) => a.file.localeCompare(b.file)),
  cases
};
const output = process.argv[2] || path.join(root, 'build/math-port/inferred-mrow-original.json');
fs.mkdirSync(path.dirname(output), { recursive: true });
fs.writeFileSync(output, JSON.stringify(result, null, 2) + '\n');
process.stdout.write(`1 actual inferred-Mrow Source case -> ${output}\n`);
