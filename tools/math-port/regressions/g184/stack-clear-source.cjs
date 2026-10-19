'use strict';
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const { root } = require('../../simple-mml-reference.cjs');

const stackSource = 'ts/input/tex/StackItem.ts';
const stackPath = path.join(root, '.reference/MathJax-src', stackSource);
const stackHash = 'ac138d321e1efbc54afbd961a4b6bfa24f7d0923eddba1c8d891dee9424602b5';
const stackBytes = fs.readFileSync(stackPath);
assert.equal(crypto.createHash('sha256').update(stackBytes).digest('hex'), stackHash);
const bundleRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
assert.equal(stackBytes.toString().replaceAll('\r\n', '\n'), fs.readFileSync(path.join(bundleRoot, stackSource), 'utf8').replaceAll('\r\n', '\n'));

const stackCjsPath = path.join(bundleRoot, 'cjs/input/tex/StackItem.js');
const nodeFactoryCjsPath = path.join(bundleRoot, 'cjs/input/tex/NodeFactory.js');
const stackCjsHash = crypto.createHash('sha256').update(fs.readFileSync(stackCjsPath)).digest('hex');
const nodeFactoryCjsHash = crypto.createHash('sha256').update(fs.readFileSync(nodeFactoryCjsPath)).digest('hex');
assert.equal(stackCjsHash, '53baa91d7c27475a91a9b6993235071ab32b04a29736c5920bdda2836ab5a91b');
assert.equal(nodeFactoryCjsHash, '426e3ca8822d209edc8a22b5e9f11c5631e52213b232220a2a55e81828374db5');
const { MmlStack } = require(stackCjsPath);
const { NodeFactory } = require(nodeFactoryCjsPath);

const oldNodes = [{ kind: 'mi' }];
const stack = new MmlStack(oldNodes);
const factory = new NodeFactory();
let received;
factory.factory.childProbe = function (factoryReceiver, properties, children) {
  received = { receiver: this, factoryReceiver, properties, children };
  return { kind: 'probe', childNodes: children };
};
stack.factory = { configuration: { nodeFactory: factory } };
stack.Clear();
const clearedNodes = stack.nodes;
assert.notEqual(clearedNodes, oldNodes);
assert.ok(Array.isArray(clearedNodes));
assert.equal(clearedNodes.length, 0);
stack.create('childProbe', { marker: 1 }, clearedNodes);
assert.equal(received.receiver, undefined);
assert.equal(received.children, clearedNodes);
assert.equal(received.factoryReceiver, factory);
assert.equal(received.properties.marker, 1);
console.log(JSON.stringify({
  stackSource,
  stackSourceSha256: stackHash,
  stackCjs: 'cjs/input/tex/StackItem.js',
  stackCjsSha256: stackCjsHash,
  nodeFactoryCjs: 'cjs/input/tex/NodeFactory.js',
  nodeFactoryCjsSha256: nodeFactoryCjsHash,
  sourceBehavior: 'Clear replaces _nodes with a fresh ordinary Array; StackItem.create forwards that exact array as NodeFactory children; NodeFactory selects this.factory[kind], then invokes it with the NodeFactory instance followed by the original properties and children arrays.',
  oldArrayRetained: oldNodes.length === 1,
  clearedArrayIsFresh: clearedNodes !== oldNodes,
  creatorReceivedSameArray: received.children === clearedNodes
}));




