#!/usr/bin/env node
'use strict';

// Bounded replay of the real CasesMethods.environment -> NumCases ->
// BaseMethods.EqnArray -> Stack.Push chain. The expected record and loaded
// Source hashes are fixed inputs; this script never rewrites them.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

function findRoot(start) {
  for (let dir = start; ; dir = path.dirname(dir)) {
    if (fs.existsSync(path.join(dir, '.reference', 'MathJax-src', 'ts', 'input', 'tex', 'cases', 'CasesConfiguration.ts'))) return dir;
    const parent = path.dirname(dir);
    if (parent === dir) throw new Error('repository root not found from ' + start);
  }
}
const root = findRoot(__dirname);
const fixtures = path.join(root, 'tests/math/fixtures/tex-source');
const cjs = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const tsFiles = [
  '.reference/MathJax-src/ts/input/tex/cases/CasesConfiguration.ts',
  '.reference/MathJax-src/ts/input/tex/base/BaseMethods.ts',
  '.reference/MathJax-src/ts/input/tex/base/BaseItems.ts',
  '.reference/MathJax-src/ts/input/tex/Stack.ts',
];
const cjsFiles = [
  'input/tex/cases/CasesConfiguration.js',
  'input/tex/base/BaseConfiguration.js',
  'input/tex/base/BaseMethods.js',
  'input/tex/base/BaseItems.js',
  'input/tex/Stack.js',
  'input/tex/StackItemFactory.js',
  'input/tex/StackItem.js',
  'input/tex/ParseUtil.js',
  'core/Tree/Factory.js',
];
const sha256 = (file) => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const hashes = Object.fromEntries([
  ...tsFiles.map((rel) => [rel, sha256(path.join(root, rel))]),
  ...cjsFiles.map((rel) => ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/' + rel, sha256(path.join(cjs, rel))]),
]);
assert.deepEqual(hashes, JSON.parse(fs.readFileSync(path.join(fixtures, 'frame-activation-source-lock.json'), 'utf8')));

const base = require(path.join(cjs, 'input/tex/base/BaseConfiguration.js')).BaseConfiguration;
const cases = require(path.join(cjs, 'input/tex/cases/CasesConfiguration.js'));
const Stack = require(path.join(cjs, 'input/tex/Stack.js')).default;
const StackItemFactory = require(path.join(cjs, 'input/tex/StackItemFactory.js')).default;

// Real item classes, factory, Stack.Push, CasesMethods, and BaseMethods are
// used. Only tag callbacks and argument scanning are bounded stubs; execution
// stops after the environment's opening array is pushed, before row/end,
// closure, tagging completion, MML finalization, or getBBox.
const itemFactory = new StackItemFactory({ ...base.items, ...cases.CasesConfiguration.items });
itemFactory.configuration = {
  tags: { start() {}, getTag() { return null; } },
  options: { tagSide: 'right', tagIndent: '0.8em' },
};
const seedEnv = { marker: 'seed-env', display: true };
const stack = new Stack(itemFactory, seedEnv, false);
let casesBegin;
const realCreate = itemFactory.create.bind(itemFactory);
const parserItemFactory = {
  create(kind, ...args) {
    const item = realCreate(kind, ...args);
    if (kind === 'cases-begin') casesBegin = item;
    return item;
  },
};
const ids = new WeakMap();
let nextId = 1;
const id = (item) => {
  if (!ids.has(item)) ids.set(item, nextId++);
  return ids.get(item);
};
const pushes = [];
const parser = {
  stack,
  itemFactory: parserItemFactory,
  configuration: { mathItem: { display: false }, options: { tagSide: 'right', tagIndent: '0.8em' } },
  options: { tagSide: 'right', tagIndent: '0.8em' },
  GetArgument() { return '{x}'; },
  Push(item) {
    stack.Push(item);
    pushes.push({
      pushedId: id(item),
      stack: stack.stack.map((frame) => ({ id: id(frame), kind: frame.kind })),
    });
  },
};

cases.CasesMethods.environment(parser, 'numcases', cases.CasesMethods.NumCases, ['cases']);
assert.ok(casesBegin, 'actual CasesMethods.environment must create CasesBeginItem');
const indices = stack.stack.map((frame, i) => frame === casesBegin ? i : -1).filter((i) => i >= 0);
const observed = {
  environment: 'numcases',
  bodyPath: 'CasesMethods.environment -> CasesMethods.NumCases -> BaseMethods.EqnArray -> actual Stack.Push',
  pushes,
  finalStack: stack.stack.map((frame, i) => ({
    index: i,
    id: id(frame),
    kind: frame.kind,
    name: frame.getName?.() ?? null,
    properties: frame._properties,
    env: frame.env,
    global: frame.global,
    sameBeginFrame: frame === casesBegin,
  })),
  repeatedBeginIndices: indices,
  beginIdentityCount: indices.length,
  beginEnvironment: casesBegin.env,
  beginGlobal: casesBegin.global,
  sameBeginEnvironmentAlias: stack.stack.filter((frame) => frame === casesBegin)
    .every((frame) => frame.env === casesBegin.env),
  differentEnvironmentOnEqnArray: stack.stack.at(-1).env !== casesBegin.env,
  sharedGlobalAcrossStack: stack.stack.every((frame) => frame.global === stack.global),
};
assert.equal(observed.beginIdentityCount, 2);
assert.equal(observed.sameBeginEnvironmentAlias, true);
assert.equal(observed.differentEnvironmentOnEqnArray, true);
assert.equal(observed.sharedGlobalAcrossStack, true);
assert.deepEqual(observed, JSON.parse(fs.readFileSync(path.join(fixtures, 'frame-activation-source-expected.json'), 'utf8')));
process.stdout.write(JSON.stringify({ ok: true, observed, sourceHashes: hashes }, null, 2) + '\n');
