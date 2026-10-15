'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../../..');
const bundle = path.join(root, '.reference/MathJax-src');
const src = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const sha = (file) => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const installed = JSON.parse(fs.readFileSync(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/package.json'), 'utf8'));
assert.equal(installed.version, '4.1.3', 'pinned MathJax CJS package version');
const cjsLocks = [
  ['core/MathItem.js', 'e6ed5008854e4d2ac6b34ab98c53bec6afb34e460895e8320c90a0d8d700d261'],
  ['input/tex.js', 'ef2852b197173f373e62326016e980aef01ffc838f5f766ab6076262f944eada'],
];
for (const [file, digest] of cjsLocks) assert.equal(sha(path.join(src, file)), digest, `CJS lock ${file}`);
const scope = JSON.parse(fs.readFileSync(path.join(root, 'docs/math-port/scope.json'), 'utf8'));
assert.equal(sha(path.join(root, scope.profile.file)), scope.profile.sha256);
for (const lock of scope.files) assert.equal(sha(path.join(bundle, lock.source)), lock.source_sha256, lock.source);
const load = (name) => require(path.join(src, name));
const { TeX } = load('input/tex.js');
const { MmlFactory } = load('core/MmlTree/MmlFactory.js');
const { AbstractMathItem } = load('core/MathItem.js');
load('input/tex/textmacros/TextMacrosConfiguration.js');
load('input/tex/textcomp/TextcompConfiguration.js');
const jax = new TeX({ packages: ['base', 'textmacros', 'textcomp'] });
const constructorStage = { adaptor: jax.adaptor, parseOptions: jax.parseOptions };
if (constructorStage.adaptor !== null) throw new Error('standalone Source TeX adaptor constructor value is not null');
jax.setMmlFactory(new MmlFactory());
const item = new AbstractMathItem('\\text{\\textcopyright\\textregistered\\texttrademark}', jax, true);
const expectedItemKeys = ['root', 'typesetRoot', 'metrics', 'inputData', 'outputData', '_state',
  'math', 'inputJax', 'display', 'start', 'end'];
assert.deepEqual(Reflect.ownKeys(item), expectedItemKeys, 'Source AbstractMathItem own-field order');
const prototypeSurface = Reflect.ownKeys(AbstractMathItem.prototype).map((key) => {
  const descriptor = Object.getOwnPropertyDescriptor(AbstractMathItem.prototype, key);
  if (descriptor.get) return { key, kind: 'get', name: descriptor.get.name, length: descriptor.get.length,
    enumerable: descriptor.enumerable, configurable: descriptor.configurable };
  return { key, kind: 'method', name: descriptor.value.name, length: descriptor.value.length,
    enumerable: descriptor.enumerable, configurable: descriptor.configurable };
});
const constructorPrototype = Object.getOwnPropertyDescriptor(AbstractMathItem, 'prototype');
assert.equal(AbstractMathItem.name, 'AbstractMathItem');
assert.equal(AbstractMathItem.length, 5);
assert.deepEqual({ writable: constructorPrototype.writable, enumerable: constructorPrototype.enumerable,
  configurable: constructorPrototype.configurable }, { writable: true, enumerable: false, configurable: false });
const result = jax.compile(item, {});
if (jax.adaptor !== null) throw new Error('standalone Source TeX adaptor after compile is not null');
if (jax.parseOptions.mathItem !== item) throw new Error('TeX compile did not retain its canonical AbstractMathItem');
assert.equal(item.root, null, 'direct jax.compile leaves AbstractMathItem.root unchanged');
assert.equal(item.typesetRoot, null, 'direct jax.compile leaves AbstractMathItem.typesetRoot unchanged');
process.stdout.write(JSON.stringify({
  sourceLocks: scope.files.length,
  cjsLocks: cjsLocks.map(([file, sha256]) => ({ file, sha256 })),
  mathJaxVersion: installed.version,
  profileSha256: scope.profile.sha256,
  constructor: { adaptor: constructorStage.adaptor, hasParseOptions: !!constructorStage.parseOptions },
  itemConstructor: { name: AbstractMathItem.name, length: AbstractMathItem.length,
    prototypeDescriptor: { writable: constructorPrototype.writable, enumerable: constructorPrototype.enumerable,
      configurable: constructorPrototype.configurable }, prototype: prototypeSurface },
  afterCompile: { adaptor: jax.adaptor, input: item.math, resultKind: result.kind,
    itemKeys: Reflect.ownKeys(item), itemState: item._state,
    itemRoot: item.root, itemTypesetRoot: item.typesetRoot,
    metrics: item.metrics, inputData: item.inputData, outputData: item.outputData },
}, null, 2) + '\n');
