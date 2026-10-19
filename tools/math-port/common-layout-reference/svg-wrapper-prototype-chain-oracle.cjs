'use strict';
// Additive read-only probe of actual SVG wrapper constructor/prototype
// identities. The locked SvgWrappers oracle intentionally reports own graph
// values but does not traverse [[Prototype]] links; this probe closes that
// observation gap without changing its expected rows.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../../..');
const bundle = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const locked = JSON.parse(fs.readFileSync(path.join(__dirname, 'profile-source-lock.json'), 'utf8'));
const source = locked.dependencies.find(x => x.file === 'src/cjs/output/svg/Wrappers.js');
assert.ok(source, 'pinned SVG wrapper source is present');
const digest = crypto.createHash('sha256')
  .update(fs.readFileSync(path.join(bundle, 'cjs/output/svg/Wrappers.js'))).digest('hex');
assert.equal(digest, source.sha256, 'pinned SVG wrapper source hash');
const { SvgWrappers } = require(path.join(bundle, 'cjs/output/svg/Wrappers.js'));

const ids = new WeakMap();
const graph = [];
function id(value) {
  if ((typeof value !== 'object' || value === null) && typeof value !== 'function') return null;
  if (!ids.has(value)) { ids.set(value, graph.length); graph.push(null); }
  return ids.get(value);
}
function describeFunction(fn) {
  const ref = id(fn);
  if (graph[ref] !== null) return { $ref: ref };
  const prototype = Object.getOwnPropertyDescriptor(fn, 'prototype')?.value;
  graph[ref] = {
    id: ref, type: 'function', name: fn.name, length: fn.length,
    ownKeys: Reflect.ownKeys(fn).map(key => typeof key === 'symbol'
      ? { symbol: key.description ?? null } : key),
    prototype: prototype ? { $ref: id(prototype) } : null,
    constructorParent: typeof Object.getPrototypeOf(fn) === 'function'
      ? { $ref: id(Object.getPrototypeOf(fn)) } : null
  };
  return { $ref: ref };
}
function describeObject(value) {
  const ref = id(value);
  if (graph[ref] !== null) return { $ref: ref };
  const props = [];
  graph[ref] = { id: ref, type: 'object', properties: props, prototype: null };
  for (const key of Reflect.ownKeys(value)) {
    const d = Object.getOwnPropertyDescriptor(value, key);
    props.push({
      key: typeof key === 'symbol' ? { symbol: key.description ?? null } : key,
      enumerable: d.enumerable, configurable: d.configurable,
      writable: Object.hasOwn(d, 'writable') ? d.writable : null,
      value: Object.hasOwn(d, 'value') ? summarize(d.value) : undefined,
      getter: d.get ? describeFunction(d.get) : null,
      setter: d.set ? describeFunction(d.set) : null
    });
  }
  const parent = Object.getPrototypeOf(value);
  graph[ref].prototype = parent && parent !== Object.prototype
    ? { $ref: id(parent) } : parent === Object.prototype ? { intrinsic: 'Object.prototype' } : null;
  return { $ref: ref };
}
function summarize(value) {
  if (typeof value === 'function') return describeFunction(value);
  if (value && typeof value === 'object') return describeObject(value);
  if (typeof value === 'symbol') return { symbol: value.description ?? null };
  if (typeof value === 'number' && Number.isNaN(value)) return { number: 'NaN' };
  if (Object.is(value, -0)) return { number: '-0' };
  return value;
}
const rows = Object.keys(SvgWrappers).map((kind, order) => {
  const ctor = SvgWrappers[kind];
  const constructorChain = [];
  for (let current = ctor; typeof current === 'function'; current = Object.getPrototypeOf(current)) {
    constructorChain.push({ name: current.name, ref: describeFunction(current) });
    if (current === Function.prototype) break;
  }
  const prototypeChain = [];
  for (let current = ctor.prototype; current && current !== Object.prototype;
       current = Object.getPrototypeOf(current)) {
    prototypeChain.push({
      constructorName: current.constructor?.name ?? null,
      ref: describeObject(current)
    });
  }
  return { order, kind, constructorName: ctor.name, constructor: describeFunction(ctor),
    constructorChain, prototypeChain };
});
const output = {
  schema: 1,
  source: 'MathJax 4.1.3 actual SVG wrapper [[Prototype]] chain',
  sourceSha256: source.sha256,
  kinds: Object.keys(SvgWrappers), rows, graph
};
const outputPath = process.argv[2] || path.join(root, 'build/math-port/svg-wrapper-prototype-chain-original.json');
fs.mkdirSync(path.dirname(outputPath), { recursive: true });
fs.writeFileSync(outputPath, JSON.stringify(output, null, 2) + '\n');
process.stdout.write(`${rows.length} actual constructor chains observed -> ${outputPath}\n`);
