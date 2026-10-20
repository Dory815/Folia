'use strict';
// Real Configuration/ParserConfiguration and MapHandler classes; no input-jax stand-in.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const base = 'tools/mathjax-bundle/node_modules/@mathjax/src';
const pairs = [
  ['input/tex/Configuration', '8184716579abf3ffa14c64413d276dab4fc721e3e3f43afb0cdccbe2ab782719', '86f62a6f6e44fec20156546b7e8d424cac493c7465c2c8ec621c604332a276f3'],
  ['input/tex/MapHandler', '7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2', 'd82e3034776d2392689aad86500b5e60dbe0aa5540853b48f308841879f3cf67'],
  ['util/Options', 'b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3', '76f3df8a3e899fba1d0944d5b79818b586c1d9f95cc7021c395fbb10295db660'],
  ['util/PrioritizedList', 'a700459f984ba1a5f18bf0e7dc183c242961b94c33587a9aa81bbaacf97180c8', '787bd21f3634312af666917d8f140145864f98d09ede1bdde3c957d28ec2bc25'],
  ['util/FunctionList', 'd4106c42268e0d7cb3549f8ffdbeb5bab87eae6daffae7314d5e724045a6918f', '571f57801b863b2d9f01a66dc72f2836b7fd06a1a48a3fe0057e4ec3d51a7278']
];
assert.equal(JSON.parse(fs.readFileSync(path.join(root, base, 'package.json'))).version, '4.1.3');
for (const [file, ts, cjs] of pairs)
  for (const [relative, expected] of [[`.reference/MathJax-src/ts/${file}.ts`, ts], [`${base}/cjs/${file}.js`, cjs]])
    assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root, relative))).digest('hex'), expected, relative);
const {Configuration, ConfigurationHandler, ParserConfiguration} = require(path.join(root, base, 'cjs/input/tex/Configuration.js'));
const {CharacterMap} = require(path.join(root, base, 'cjs/input/tex/TokenMap.js'));
const {NodeFactory} = require(path.join(root, base, 'cjs/input/tex/NodeFactory.js'));
const initA = c => { c.options.maxMacros = (c.options.maxMacros || 0) * 10 + 1; };
const initB = c => { c.options.maxMacros = (c.options.maxMacros || 0) * 10 + 2; };
const configMethod = () => {};
const filter = () => {};
const keys = Array.from(ConfigurationHandler.keys());
const local = Configuration.local({priority: 7, parser: '', init: initA, config: [configMethod, 3],
  preprocessors: [filter, [filter, 0]]});
assert.equal(local.name, '');
assert.equal(local.priority, 7);
assert.equal(local.parser, 'tex');
assert.deepEqual(Object.keys(local.handler), ['character', 'delimiter', 'macro', 'environment']);
assert.deepEqual(local.initMethod, [initA, 7]);
assert.deepEqual(local.configMethod, [configMethod, 3]);
assert.deepEqual(local.preprocessors, [[filter, 7], [filter, 0]]);
assert.deepEqual(Array.from(ConfigurationHandler.keys()), keys);
Configuration.create('registry-a'); Configuration.create('registry-b');
const replacement = Configuration.create('registry-a');
assert.equal(ConfigurationHandler.get('registry-a'), replacement);
assert.deepEqual(Array.from(ConfigurationHandler.keys()), keys.concat(['registry-a', 'registry-b']));

const selected = c => c.handlers.get('macro').lookup('x').char;
function make(name, map, buffer, init, priority = 5, token = NodeFactory.createToken) {
  new CharacterMap(map, null, {x: name});
  return Configuration.create(name, {handler: {macro: [map]}, options: {maxBuffer: buffer}, init, priority, nodes: {token}});
}
make('a', 'a-map', 10, initA);
make('b', 'b-map', 20, initB, 5, function alternate() {});
const tied = new ParserConfiguration(['a', 'b']);
assert.equal(tied.options.maxBuffer, 10);
assert.equal(selected(tied), 'b');
assert.equal(tied.nodes.token, NodeFactory.createToken);
tied.init();
assert.equal(tied.options.maxMacros, 21);
make('zero-a', 'zero-a-map', 10, initA, 10);
make('zero-b', 'zero-b-map', 20, initB, 7);
const zero = new ParserConfiguration([['zero-a', 0], ['zero-b', 5]]);
assert.equal(zero.options.maxBuffer, 20);
assert.equal(selected(zero), 'zero-b');
zero.init();
assert.equal(zero.options.maxMacros, 21);

Configuration.create('append', {init: c => {
  if (c.options.maxMacros) return;
  c.options.maxMacros = 1;
  c.append(Configuration.local({init: initB}));
}});
const growing = new ParserConfiguration(['append']);
growing.init(); assert.equal(growing.options.maxMacros, 12);
growing.init(); assert.equal(growing.options.maxMacros, 122);
Configuration.create('install', {init: c => {
  new CharacterMap('installed', null, {x: 'installed'});
  c.append(Configuration.local({handler: {macro: ['installed']}}), 1);
}});
const installed = new ParserConfiguration(['install']);
installed.init(); assert.equal(selected(installed), 'installed');

const warnings = [];
const warn = console.warn;
let missing;
try { console.warn = text => warnings.push(text); missing = new ParserConfiguration(['absent-package']); }
finally { console.warn = warn; }
assert.deepEqual(warnings, ["MathJax Warning: Package 'absent-package' not found.  Omitted."]);
assert.deepEqual(Array.from(missing.handlers.keys()), []);
Configuration.create('text-only', {parser: 'text'});
assert.throws(() => new ParserConfiguration(['text-only']), /Package 'text-only' doesn't target the proper parser/);
assert.deepEqual(Array.from(new ParserConfiguration(['text-only'], ['text']).handlers.keys()),
  ['character', 'delimiter', 'macro', 'environment']);
const old = Configuration.create('same', {options: {maxBuffer: 10}});
const holding = new ParserConfiguration(['same']);
Configuration.create('same', {options: {maxBuffer: 20}});
assert.equal(Array.from(holding.configurations)[0].item, old);
assert.equal(holding.options.maxBuffer, 10);
console.log(JSON.stringify({status: 'passed', sourceFiles: pairs.length * 2,
  limit: 'Configuration records, registry, package selection, collation and init only; no native input-jax config/add, Tags construction or complete TeX pipeline acceptance.'}));
