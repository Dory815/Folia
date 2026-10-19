'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const repo = path.resolve(__dirname, '../../../..');
const cjs = path.join(repo, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
assert.equal(require(path.join(cjs, '../package.json')).version, '4.1.3');
for (const [file, sha] of Object.entries({
  'input/tex/Stack.js': '78743f9dfcf1ebf9beb890629d908111be5c1e47f49c16d5ffa0877c5935c385',
  'input/tex/TexParser.js': '106d7ff9298dce899d046339eab1996358f3efca3a7ce66f5ca915747dc9c6a8',
  'input/tex/StackItem.js': '53baa91d7c27475a91a9b6993235071ab32b04a29736c5920bdda2836ab5a91b',
  'input/tex/base/BaseItems.js': '5310d0429b7b62d405ae9b93c8d21ad17b787e5eb3fd1f01439c62f2094f7d1a'
})) {
  const bytes = fs.readFileSync(path.join(cjs, file));
  assert.equal(crypto.createHash('sha256').update(bytes).digest('hex'), sha, `${file} hash`);
}
const stackExports = require(path.join(cjs, 'input/tex/StackItem.js'));
const baseExports = require(path.join(cjs, 'input/tex/base/BaseItems.js'));
const Stack = require(path.join(cjs, 'input/tex/Stack.js')).default;
assert.equal(Object.hasOwn(stackExports, 'PrimeItem'), false);
assert.equal(typeof baseExports.PrimeItem, 'function');
// TexParser's constructor ends by pushing stop. This is the Stack portion of
// that exact Source sequence, with an inner stack so StartItem skips tag hooks.
const factory = {
  configuration: {nodeFactory: {create() { return {}; }}},
  create(kind, ...args) {
    if (kind === 'start') return new baseExports.StartItem(this, args[0]);
    if (kind === 'stop') return new baseExports.StopItem(this);
    if (kind === 'mml') return new baseExports.MmlItem(this);
    throw new Error(`unexpected Source stack factory kind ${kind}`);
  }
};
const parserStack = new Stack(factory, {}, true);
parserStack.Push(factory.create('stop'));
assert.equal(parserStack.stack[0].kind, 'mml');
const parserPostStopMmlOwnKeys = Object.keys(parserStack.stack[0]);
const parserPostStopEnvironmentIdentity = parserStack.stack[0]._env===parserStack.env;
assert.deepEqual(parserPostStopMmlOwnKeys,
  ['_nodes', 'startStr', 'startI', 'stopI', 'factory', 'global', '_properties', '_env']);
assert.equal(parserPostStopEnvironmentIdentity, true);
const liveMml = parserStack.stack[0];
delete liveMml._env;
assert.equal(liveMml.env, undefined);
const reassignedEnvironment = {};
liveMml.env = reassignedEnvironment;
assert.equal(liveMml._env, reassignedEnvironment);
let getterEnvironment = {};
Object.defineProperty(liveMml, '_env', {configurable:true, get(){return getterEnvironment;}});
assert.equal(liveMml.env, getterEnvironment);
const dataEnvironment = {};
Object.defineProperty(liveMml, '_env', {configurable:true, writable:true, enumerable:true, value:dataEnvironment});
assert.equal(liveMml.env, dataEnvironment);
delete liveMml._env;
let inheritedEnvironment;
const inherited = Object.create(Object.getPrototypeOf(liveMml));
Object.defineProperty(inherited, '_env', {configurable:true, get(){return inheritedEnvironment;}, set(value){inheritedEnvironment=value;}});
Object.setPrototypeOf(liveMml, inherited);
const inheritedAssigned = {};
liveMml.env = inheritedAssigned;
assert.equal(inheritedEnvironment, inheritedAssigned);
assert.equal(Object.hasOwn(liveMml, '_env'), false);
assert.equal(liveMml.env, inheritedAssigned);
const start = new baseExports.StartItem({});
start._nodes = ['first', 'second'];
start._nodes.length = 3;
start._nodes[2] = 'third';
const firstAtLength3 = start.First;
const peekAtLength3 = start.Peek(1);
assert.equal(firstAtLength3, 'third');
assert.deepEqual(peekAtLength3, ['third']);
assert.equal(start.Pop(), 'third');
start.Push('fourth');
assert.deepEqual(start._nodes, ['first', 'second', 'fourth']);
const sparse = new Array(3);
sparse[1] = undefined;
const copied = sparse.slice(0);
assert.deepEqual(Object.keys(copied), ['1']);
const firstPop = sparse.pop();
assert.equal(firstPop, undefined);
const afterFirstPopOwnKeys = Object.keys(sparse);
assert.deepEqual(afterFirstPopOwnKeys, ['1']);
assert.equal(Object.hasOwn(sparse, '0'), false);
assert.equal(Object.hasOwn(sparse, '1'), true);
assert.equal(sparse.pop(), undefined);
assert.equal(Object.hasOwn(sparse, '0'), false);
const rawNodes = [];
const alternateNodes = {length: '1'};
const mmlProbe = new baseExports.StartItem({});
mmlProbe._nodes = rawNodes;
Object.defineProperty(mmlProbe, 'nodes', {configurable:true, get(){return alternateNodes;}});
let createArgs;
mmlProbe.create = (...args) => { createArgs = args; return 'created'; };
assert.equal(mmlProbe.toMml(), 'created');
assert.equal(createArgs[0], 'node');
assert.equal(createArgs[1], 'inferredMrow');
assert.equal(createArgs[2], rawNodes);
const customNodes = {join(separator){assert.equal(this, customNodes); assert.equal(separator, '; '); return 'joined raw values';}};
let nodesGets = 0;
Object.defineProperty(mmlProbe, 'nodes', {configurable:true, get(){++nodesGets; return customNodes;}});
assert.equal(mmlProbe.toString(), 'start[joined raw values]');
assert.equal(nodesGets, 1);
console.log(JSON.stringify({
  pinned:'MathJax 4.1.3',
  StackItemExportsPrimeItem:Object.hasOwn(stackExports, 'PrimeItem'),
  BaseItemsExportsPrimeItem:typeof baseExports.PrimeItem,
  ParserPostStopStackFirstKind:parserStack.stack[0].kind,
  ParserPostStopMmlOwnKeys:parserPostStopMmlOwnKeys,
  ParserPostStopEnvironmentIdentity:parserPostStopEnvironmentIdentity,
  liveEnvironmentGetterReplacement:liveMml.env===inheritedEnvironment,
  inheritedEnvironmentDoesNotCreateOwnField:!Object.hasOwn(liveMml, '_env'),
  FirstAtLength3:firstAtLength3,
  Peek1AtLength3:peekAtLength3,
  sparseAfterFirstPop:afterFirstPopOwnKeys,
  toMmlUsesRawNodes:createArgs[2]===rawNodes,
  toStringUsesNodesGetter:nodesGets
}, null, 2));

