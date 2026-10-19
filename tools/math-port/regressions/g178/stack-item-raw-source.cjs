'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(process.cwd(), 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const pkg = require(path.join(root, '../package.json'));
assert.equal(pkg.version, '4.1.3');
for (const [file, expected] of Object.entries({
  'input/tex/StackItem.js': '53baa91d7c27475a91a9b6993235071ab32b04a29736c5920bdda2836ab5a91b',
  'input/tex/base/BaseItems.js': '5310d0429b7b62d405ae9b93c8d21ad17b787e5eb3fd1f01439c62f2094f7d1a',
  'input/tex/ams/AmsItems.js': '24f1b6b0dabb84eddc52150bf5f629fb8203a3e9eeaecb5f425c7df8fe4656d5',
  'input/tex/base/BaseMethods.js': 'b8d5c74e01f85ae391ec8d16ffcd216a4ca4267bd8ffa091c5bfe1462ad7b491',
  'input/tex/ParseUtil.js': 'adafa18cefad39e8408bcc0e4eb7103dceb6d16c9e7ea2eb013f23602c3f8941',
  'input/tex/NodeUtil.js': '4a55841fd22167d25f505e8f41adea072715ef024e00ea5ac6df3f199f2cb809',
  'input/tex/bussproofs/BussproofsMethods.js': 'c671b2b31b229e73454c70c794a82923f8d059f43b5f1af134588b530055c054'
})) {
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(root, file))).digest('hex');
  assert.equal(actual, expected, `${file} source lock changed`);
}
const { StartItem, PrimeItem, StyleItem, ArrayItem, OverItem } = require(path.join(root, 'input/tex/base/BaseItems.js'));
const { MultlineItem } = require(path.join(root, 'input/tex/ams/AmsItems.js'));
const BaseMethods = require(path.join(root, 'input/tex/base/BaseMethods.js')).default;
const { ParseUtil } = require(path.join(root, 'input/tex/ParseUtil.js'));
const BussproofsMethods = require(path.join(root, 'input/tex/bussproofs/BussproofsMethods.js')).default;
const item = new StartItem({}, {});
assert.deepEqual(Object.keys(item), ['_nodes', 'startStr', 'startI', 'stopI', 'factory', 'global', '_properties', '_env']);
for (const key of ['_nodes', '_properties']) {
  assert.deepEqual(Object.getOwnPropertyDescriptor(item, key), {
    value: item[key], writable: true, enumerable: true, configurable: true
  });
}

const previousNodes = item._nodes;
const foreignNodes = ['first', 'second'];
item._nodes = foreignNodes;
assert.strictEqual(item.nodes, foreignNodes);
assert.equal(item.Size(), 2);
assert.equal(item.First, 'second');
assert.deepEqual(item.Peek(1), ['second']);
assert.equal(item.Pop(), 'second');
assert.equal(foreignNodes.length, 1);
item.Push('third');
assert.deepEqual(foreignNodes, ['first', 'third']);
item.Clear();
assert.notStrictEqual(item._nodes, foreignNodes);
assert.deepEqual(item._nodes, []);
assert.deepEqual(foreignNodes, ['first', 'third']);
assert.equal(previousNodes.length, 0);

const sparseNodes = new Array(3);
sparseNodes[1] = undefined;
let sliceReceiver = false;
let popReceiver = false;
sparseNodes.slice = function (start) {
  sliceReceiver = this === sparseNodes;
  return Array.prototype.slice.call(this, start);
};
sparseNodes.pop = function () {
  popReceiver = this === sparseNodes;
  return Array.prototype.pop.call(this);
};
item._nodes = sparseNodes;
const sparseCopy = item.Peek(3);
assert.equal(sparseCopy.length, 3);
assert.equal(0 in sparseCopy, false);
assert.equal(1 in sparseCopy, true);
assert.equal(sparseCopy[1], undefined);
assert.equal(2 in sparseCopy, false);
assert.equal(sliceReceiver, true);
assert.equal(item.Pop(), undefined); // a hole produces the Source undefined result
assert.equal(popReceiver, true);
assert.equal(sparseNodes.length, 2);
assert.equal(1 in sparseNodes, true);
assert.equal(item.Pop(), undefined); // own undefined is also returned raw
assert.equal(sparseNodes.length, 1);
assert.equal(0 in sparseNodes, false);

const replacementProperties = { probe: 'before' };
item._properties = replacementProperties;
assert.strictEqual(item._properties, replacementProperties);
assert.equal(item.getProperty('probe'), 'before');
item.setProperty('probe', 'after');
assert.equal(replacementProperties.probe, 'after');
const assigned = { copied: 1 };
item.setProperties(assigned);
assert.equal(replacementProperties.copied, 1);

const accessorNodes = ['accessor'];
let nodeGets = 0;
Object.defineProperty(item, '_nodes', {
  enumerable: true, configurable: true,
  get() { nodeGets++; return accessorNodes; }
});
assert.equal(item.Size(), 1);
assert.equal(item.Pop(), 'accessor');
assert.equal(nodeGets, 2);
assert.deepEqual(Object.keys(item).slice(0, 1), ['_nodes']);

const peekOrder = [];
const peekNodes = [];
Object.defineProperty(item, '_nodes', { configurable: true, get() { peekOrder.push('nodes-get'); return peekNodes; } });
Object.defineProperty(peekNodes, 'slice', { configurable: true, get() {
  peekOrder.push('slice-get');
  return function (start) { peekOrder.push('slice-call'); assert.equal(this, peekNodes); assert.equal(start, 2); return []; };
} });
Object.defineProperty(item, 'Size', { configurable: true, get() { peekOrder.push('size-get'); return function () { peekOrder.push('size-call'); return 3; }; } });
item.Peek(undefined);
assert.deepEqual(peekOrder, ['nodes-get', 'slice-get', 'size-get', 'size-call', 'slice-call']);
peekOrder.length = 0;
item.Peek(null);
assert.deepEqual(peekOrder, ['nodes-get', 'slice-get', 'size-get', 'size-call', 'slice-call']);

const concatTrace = [];
const kindObject = { [Symbol.toPrimitive](hint) { concatTrace.push(`kind-${hint}`); return 'custom-kind'; } };
const joinObject = { [Symbol.toPrimitive](hint) { concatTrace.push(`join-${hint}`); return false; } };
const concatNodes = [];
Object.defineProperty(concatNodes, 'join', { configurable: true, get() {
  concatTrace.push('join-get');
  return function (separator) { concatTrace.push('join-call'); assert.equal(this, concatNodes); assert.equal(separator, '; '); return joinObject; };
} });
Object.defineProperty(item, 'kind', { configurable: true, value: kindObject });
Object.defineProperty(item, '_nodes', { configurable: true, get() { concatTrace.push('nodes-get'); return concatNodes; } });
assert.equal(item.toString(), 'custom-kind[false]');
assert.deepEqual(concatTrace, ['kind-default', 'nodes-get', 'join-get', 'join-call', 'join-default']);
const over = new OverItem({});
over.setProperty('num', { [Symbol.toPrimitive](hint) { assert.equal(hint, 'default'); return 5; } });
over._nodes = { join() { return null; } };
assert.equal(over.toString(), 'over[5 / null]');

const accessorProperties = { live: 'yes' };
let propertyGets = 0;
Object.defineProperty(item, '_properties', {
  enumerable: true, configurable: true,
  get() { propertyGets++; return accessorProperties; }
});
assert.equal(item.getProperty('live'), 'yes');
item.setProperty('live', 'updated');
assert.equal(accessorProperties.live, 'updated');
assert.equal(propertyGets, 2);

const strictItem = new StartItem({}, {});
const createCalls = [];
strictItem.create = (...args) => {
  createCalls.push(args);
  return args;
};
strictItem._nodes = { 0: 'only', length: 1 };
assert.equal(strictItem.toMml(), 'only');
assert.equal(createCalls.length, 0);
const stringLengthNodes = { 0: 'only' };
Object.defineProperty(stringLengthNodes, 'length', { get() { return '1'; } });
strictItem._nodes = stringLengthNodes;
const created = strictItem.toMml();
assert.equal(createCalls.length, 1);
assert.equal(created[0], 'node');
assert.equal(created[1], 'inferredMrow');
assert.strictEqual(created[2], stringLengthNodes);

const propertyItem = new StartItem({}, {});
propertyItem._properties = { retained: true };
assert.doesNotThrow(() => propertyItem.setProperties(null));
assert.deepEqual(propertyItem._properties, { retained: true });
propertyItem._properties = null;
assert.throws(() => propertyItem.setProperties({}), TypeError);
assert.throws(() => propertyItem.setProperties(null), TypeError);
const stringItem = new StartItem({}, {});
const stringNodes = ['raw-a', 'raw-b'];
let nodesGetterCalls = 0;
Object.defineProperty(stringItem, 'nodes', {
  configurable: true, get() { nodesGetterCalls++; return stringNodes; }
});
let joinReceiver = false;
stringNodes.join = function (separator) {
  joinReceiver = this === stringNodes && separator === '; ';
  return 'joined raw values';
};
stringItem._nodes = stringNodes;
assert.equal(stringItem.toString(), 'start[joined raw values]');
assert.equal(joinReceiver, true);
assert.equal(nodesGetterCalls, 1);
const nodeFactory = { create(...args) { return { factoryArgs: args }; } };
const configuration = { nodeFactory };
const firstSpace = { isKind(kind) { return kind === 'mspace'; }, isEmbellished: false };
const trace = [];
const changingNodes = new Proxy([firstSpace, null], {
  get(target, key, receiver) {
    if (key === 'length') trace.push('length');
    if (key === '0') { trace.push('0'); target.length = 0; }
    if (key === '1') trace.push('1');
    return Reflect.get(target, key, receiver);
  }
});
ParseUtil.fixInitialMO(configuration, changingNodes);
assert.deepEqual(trace, ['length', '0', '1']);
const prime = new PrimeItem({}, {});
const iteratorTrace = [];
const top0 = { isKind() { return false; }, getProperty() { return false; } };
const top1 = {};
let iteratorIndex = 0;
prime.Peek = function (count) {
  assert.equal(count, 2);
  return {
    [Symbol.iterator]() {
      iteratorTrace.push('iterator');
      return {
        next() {
          iteratorTrace.push(`next${iteratorIndex}`);
          return iteratorIndex++ < 2
            ? { done: false, value: iteratorIndex === 1 ? top0 : top1 }
            : { done: true };
        },
        return() { iteratorTrace.push('return'); return { done: true }; }
      };
    }
  };
};
prime.create = (...args) => args;
prime.checkItem({ marker: true });
assert.deepEqual(iteratorTrace, ['iterator', 'next0', 'next1', 'return']);
for (const methodName of ['Superscript', 'Subscript']) {
  const trace = [];
  let index = 0;
  const base = { isKind() { return false; }, getProperty() { return false; }, sup: 1, over: 1 };
  const top = {
    isKind(kind) { return kind === 'prime'; }, Peek(count) {
      assert.equal(count, 2);
      return { [Symbol.iterator]() { trace.push('iterator'); return {
        next() { trace.push(`next${index}`); return index++ < 2 ? { done: false, value: index === 1 ? base : {} } : { done: true }; },
        return() { trace.push('return'); return { done: true }; }
      }; } };
    }
  };
  const parser = {
    i: 0, GetNext() { return ''; }, stack: { Top() { return top; }, Pop() {} }, Push() {},
    create() { return { isKind() { return false; }, getProperty() { return false; }, sup: 1, over: 1 }; },
    itemFactory: { create() { return { setProperties() { return this; } }; } }
  };
  BaseMethods[methodName](parser, '');
  assert.deepEqual(trace, ['iterator', 'next0', 'next1', 'return']);
}
let bussproofPeekCalls = 0;
let bussproofCreates = 0;
const proofTop = {
  kind: 'proofTree', Size() { return 2; }, getProperty() { return false; },
  Peek() { bussproofPeekCalls++; throw new Error('unexpected Peek'); }, Pop() { return {}; }
};
const sentinel = new Error('stop after short-circuit');
const proofParser = {
  stack: { Top() { return proofTop; } },
  create() { bussproofCreates++; throw sentinel; }
};
assert.throws(() => BussproofsMethods.Inference(proofParser, 'name', 2), error => error === sentinel);
assert.equal(bussproofPeekCalls, 0);
assert.equal(bussproofCreates, 1);
proofTop.Size = () => 1;
proofTop.Peek = () => [];
assert.throws(() => BussproofsMethods.Inference(proofParser, 'name', 1), TypeError);

// Native C++ callers mirror these exact Source expressions: each reads the
// configurable `nodes` property at its point in evaluation, not `_nodes`.
{
  const trace = [];
  const style = Object.create(StyleItem.prototype);
  const nodes = ['style-raw'];
  Object.defineProperty(style, 'create', { configurable: true, get() {
    trace.push('create-get');
    return (...args) => { trace.push('create-call'); assert.deepEqual(args, ['node', 'mstyle', nodes, 'styles']); return 'mml'; };
  }});
  Object.defineProperty(style, 'nodes', { configurable: true, get() { trace.push('nodes-get'); return nodes; }});
  style.getProperty = name => { trace.push(`get-${name}`); return 'styles'; };
  style.factory = { create(...args) { trace.push('factory-create'); assert.deepEqual(args, ['mml', 'mml']); return 'wrapper'; } };
  const close = { isClose: true };
  assert.deepEqual(StyleItem.prototype.checkItem.call(style, close), [[ 'wrapper', close ], true]);
  assert.deepEqual(trace, ['create-get', 'nodes-get', 'get-styles', 'create-call', 'factory-create']);
}
{
  const trace = [];
  const array = Object.create(ArrayItem.prototype);
  const nodes = ['array-raw'];
  Object.defineProperty(array, 'create', { configurable: true, get() {
    trace.push('create-get');
    return (...args) => { trace.push('create-call'); assert.deepEqual(args, ['node', 'mtd', nodes]); return { childNodes: [] }; };
  }});
  Object.defineProperty(array, 'nodes', { configurable: true, get() { trace.push('nodes-get'); return nodes; }});
  array.hfill = []; array.ralign = []; array.row = []; array.breakAlign = { cell: '' };
  array.Size = () => 0; array.Clear = () => trace.push('clear');
  array.EndEntry();
  assert.deepEqual(trace, ['create-get', 'nodes-get', 'create-call', 'clear']);
  assert.equal(array.row.length, 1);
}
{
  const trace = [];
  const multline = Object.create(MultlineItem.prototype);
  const nodes = ['multline-raw'];
  Object.defineProperty(multline, 'create', { configurable: true, get() {
    trace.push('create-get');
    return (...args) => { trace.push('create-call'); assert.deepEqual(args, ['node', 'mtd', nodes, {}]); return 'cell'; };
  }});
  Object.defineProperty(multline, 'nodes', { configurable: true, get() { trace.push('nodes-get'); return nodes; }});
  multline.table = []; multline.row = [];
  multline.getProperty = name => { trace.push(`get-${name}`); return null; };
  multline.setProperty = (name, value) => trace.push(`set-${name}-${value}`);
  multline.Clear = () => trace.push('clear');
  multline.EndEntry();
  assert.deepEqual(trace, ['get-shove', 'create-get', 'nodes-get', 'create-call', 'set-shove-null', 'clear']);
  assert.deepEqual(multline.row, ['cell']);
}
console.log(JSON.stringify({ version: pkg.version, ownKeys: Object.keys(item), nodeGets, propertyGets }, null, 2));

