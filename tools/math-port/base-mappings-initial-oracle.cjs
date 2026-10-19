'use strict';
// Extract the pristine base TeX mapping tables from pinned MathJax 4.1.3 CJS.
// This process intentionally performs no parser corpus before reading MapHandler.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const reference = path.join(root, '.reference/MathJax-src');
const bundle = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const source = path.join(bundle, 'src');
const locks = [
  ['ts/input/tex/base/BaseConfiguration.ts', '578d2c6b03f2f3e902476a0b73a5323b12262215fcbdc831a1135c1d9c1737ad'],
  ['ts/input/tex/base/BaseMethods.ts', '7336240c8f9c5037f9fdcd5a84ea079312854f1ef3f406aeb5a8b6a8ff765cda'],
  ['ts/input/tex/base/BaseMappings.ts', '08dada0ed7fab0134da2c3a341e30637959ebb730512835b9482d7c78d72adae'],
];
const cjsLocks = new Map([
  ['input/tex/base/BaseConfiguration.js', '0feedc60ac600b647cf2a1437c7244196a5aec04715f4a301fe6e9ec09722c57'],
  ['input/tex/base/BaseMethods.js', 'b8d5c74e01f85ae391ec8d16ffcd216a4ca4267bd8ffa091c5bfe1462ad7b491'],
  ['input/tex/base/BaseMappings.js', '3522fc3fbb50af2f65442be3b2fbb2623824620899ebe775a87b885928c4acce'],
]);
const sha = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const sourceLocks = [];
for (const [relative, expected] of locks) {
  const tsPath = path.join(reference, relative);
  assert.equal(sha(tsPath), expected, `${relative} pinned TS hash`);
  const cjs = relative.slice(3).replace(/\.ts$/, '.js');
  const cjsPath = path.join(source, 'cjs', cjs);
  assert.equal(sha(cjsPath), cjsLocks.get(cjs), `${cjs} pinned CJS hash`);
  assert.equal(fs.readFileSync(path.join(source, relative), 'utf8').replaceAll('\r\n', '\n'),
    fs.readFileSync(tsPath, 'utf8').replaceAll('\r\n', '\n'), `${relative} installed TS differs`);
  sourceLocks.push({ ts: relative, tsSha256: expected, cjs, cjsSha256: cjsLocks.get(cjs) });
}
const load = name => require(path.join(source, 'cjs', name));
const { BaseConfiguration } = load('input/tex/base/BaseConfiguration.js');
const { MapHandler } = load('input/tex/MapHandler.js');
assert.equal(BaseConfiguration.name, 'base');
function plain(value, seen = new WeakSet()) {
  if (value === undefined) return { $: 'undefined' };
  if (value === null || typeof value === 'number' || typeof value === 'boolean' || typeof value === 'string') return value;
  if (typeof value === 'function') return { $: 'function', name: value.name || '', length: value.length };
  if (seen.has(value)) return { $: 'cycle' };
  seen.add(value);
  if (Array.isArray(value)) return { $: 'array', items: Array.from({ length: value.length }, (_, i) =>
    Object.hasOwn(value, i) ? plain(value[i], seen) : { $: 'hole' }) };
  const out = {};
  for (const key of Reflect.ownKeys(value))
    out[typeof key === 'symbol' ? `symbol:${key.description || ''}` : key] = plain(value[key], seen);
  return out;
}
function methodMeta(value) {
  return typeof value === 'function' ? { name: value.name || '', length: value.length } : plain(value);
}
function tokenMap(name) {
  const map = MapHandler.getMap(name);
  if (!map) return null;
  const entries = map.map instanceof Map
    ? [...map.map.entries()].map(([key, value]) => [key, plain(value)]) : null;
  const regex = map._regExp ? { source: map._regExp.source, flags: map._regExp.flags } : null;
  return { class: map.constructor.name, name: map.name, parser: methodMeta(map.parser), regex, entries };
}
const names = ['letter', 'digit', 'command', 'special', 'lcGreek', 'ucGreek', 'mathchar0mi', 'mathchar0mo',
  'mathchar7', 'delimiter', 'macros', 'environment', 'remap', 'not_remap'];
const mappings = names.map(tokenMap);
for (let i = 0; i < names.length; i++) assert.ok(mappings[i], `Missing initialized base map ${names[i]}`);
const snapshot = { schema: 1, version: '4.1.3', sourceLocks, mappings };
const output = process.argv[2] || path.join(root, 'build/math-port/base-mappings-initial.json');
fs.mkdirSync(path.dirname(output), { recursive: true });
fs.writeFileSync(output, JSON.stringify(snapshot, null, 2) + '\n');
console.log(JSON.stringify({ maps: mappings.length, initial: true, output: path.relative(root, output),
  sha256: sha(output), sourceLocks }, null, 2));
