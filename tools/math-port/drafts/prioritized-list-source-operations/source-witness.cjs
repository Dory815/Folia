'use strict';
// Run from the repository root with:
// node tools/math-port/drafts/prioritized-list-source-operations/source-witness.cjs
// Oracle: MathJax 4.1.3 at .reference/MathJax-src commit
// fb987178c2d279a99b0db5ea02e751691703955e.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const { execFileSync } = require('node:child_process');

const root = path.resolve(__dirname, '../../../..');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const sourcePath = path.join(sourceRoot, 'ts/util/PrioritizedList.ts');
const cjsPath = path.join(packageRoot, 'cjs/util/PrioritizedList.js');
const pinnedCommit = 'fb987178c2d279a99b0db5ea02e751691703955e';
const pins = new Map([
  [sourcePath, 'a700459f984ba1a5f18bf0e7dc183c242961b94c33587a9aa81bbaacf97180c8'],
  [cjsPath, '787bd21f3634312af666917d8f140145864f98d09ede1bdde3c957d28ec2bc25'],
]);
assert.equal(execFileSync('git', ['-C', sourceRoot, 'rev-parse', 'HEAD'], { encoding: 'utf8' }).trim(), pinnedCommit);
assert.equal(require(path.join(packageRoot, 'package.json')).version, '4.1.3');
for (const [file, expected] of pins) {
  assert.equal(crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex'), expected, `${path.basename(file)} hash`);
}

const { PrioritizedList } = require(cjsPath);
const add = PrioritizedList.prototype.add;
const remove = PrioritizedList.prototype.remove;

// Each Source `this.items` access gets a fresh array, and splice is fetched
// before the record argument is constructed and called with that exact array.
const addTrace = [];
const firstRecord = { get priority() { addTrace.push('priority-20'); return 20; } };
const secondRecord = { get priority() { addTrace.push('priority-5'); return 5; } };
const firstLoop = new Array(2), secondLoop = new Array(1), spliceTarget = [];
Object.defineProperty(firstLoop, '1', { get() { addTrace.push('index-1'); return firstRecord; } });
Object.defineProperty(secondLoop, '0', { get() { addTrace.push('index-0'); return secondRecord; } });
Object.defineProperty(spliceTarget, 'splice', { get() {
  addTrace.push('splice-get');
  return function (...args) {
    assert.equal(this, spliceTarget);
    addTrace.push('splice-call');
    return Array.prototype.splice.apply(this, args);
  };
} });
const addList = new PrioritizedList();
let addItemsReads = 0;
const addItems = [new Array(2), firstLoop, secondLoop, spliceTarget];
Object.defineProperty(addList, 'items', { get() { addTrace.push('items'); return addItems[addItemsReads++]; } });
assert.equal(add.call(addList, 'inserted', 10), 'inserted');
assert.equal(addItemsReads, 4);
assert.deepEqual(addTrace, [
  'items', 'items', 'index-1', 'priority-20', 'items', 'index-0', 'priority-5',
  'items', 'splice-get', 'splice-call',
]);
assert.equal(spliceTarget[0].item, 'inserted');
assert.equal(spliceTarget[0].priority, 10);

// remove has the same repeated-owner reads and reads splice only after finding
// the target; its splice receiver is again the exact fetched array.
const removeTrace = [];
const removeItems = [{ item: 'a' }, { item: 'target' }, { item: 'b' }];
for (const [index, value] of removeItems.entries()) {
  Object.defineProperty(removeItems, String(index), { configurable: true, get() {
    removeTrace.push(`index-${index}`);
    return { get item() { removeTrace.push(`item-${value.item}`); return value.item; } };
  } });
}
Object.defineProperty(removeItems, 'splice', { get() {
  removeTrace.push('splice-get');
  return function (...args) {
    assert.equal(this, removeItems);
    removeTrace.push('splice-call');
    assert.deepEqual(args, [1, 1]);
    return [];
  };
} });
const removeList = new PrioritizedList();
let removeItemsReads = 0;
Object.defineProperty(removeList, 'items', { get() { removeTrace.push('items'); removeItemsReads++; return removeItems; } });
assert.equal(remove.call(removeList, 'target'), removeList);
assert.equal(removeItemsReads, 4);
assert.deepEqual(removeTrace, [
  'items', 'items', 'index-2', 'item-b', 'items', 'index-1', 'item-target',
  'items', 'splice-get', 'splice-call',
]);

// Raw numeric loop behavior: NaN skips indexed reads but reaches add's splice
// with NaN; fractional length preserves the 1.5 read and remove start.
let nanStart;
const nanItems = { length: NaN, splice(start, count, record) {
  assert.equal(this, nanItems);
  nanStart = start;
  assert(Number.isNaN(start));
  assert.equal(count, 0);
  assert.equal(record.item, 'nan');
} };
const nanList = new PrioritizedList();
nanList.items = nanItems;
assert.equal(add.call(nanList, 'nan', 1), 'nan');
assert(Number.isNaN(nanStart));

let fractionalStart;
const fractionalItems = {
  length: 2.5,
  '1.5': { item: 'fractional' },
  splice(start, count) {
    assert.equal(this, fractionalItems);
    fractionalStart = start;
    assert.equal(count, 1);
  },
};
const fractionalList = new PrioritizedList();
fractionalList.items = fractionalItems;
assert.equal(remove.call(fractionalList, 'fractional'), fractionalList);
assert.equal(fractionalStart, 1.5);

process.stdout.write(JSON.stringify({
  version: '4.1.3', sourceCommit: pinnedCommit,
  sourceSha256: pins.get(sourcePath), cjsSha256: pins.get(cjsPath),
  addTrace, addItemsReads, removeTrace, removeItemsReads,
  nanAddStart: 'NaN', fractionalRemoveStart: fractionalStart,
}) + '\n');
