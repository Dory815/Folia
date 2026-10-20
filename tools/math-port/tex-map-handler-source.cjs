'use strict';
// Real pinned TokenMap/MapHandler classes; lookup/composition only, no parser stand-in.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const base = 'tools/mathjax-bundle/node_modules/@mathjax/src';
const pairs = [
  ['input/tex/TokenMap', '0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e', '9e3f086080a6ab052b68b22a5d9ee3dbc5e17d7b0c03a31e789664b01df36cf7'],
  ['input/tex/MapHandler', '7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2', 'd82e3034776d2392689aad86500b5e60dbe0aa5540853b48f308841879f3cf67'],
  ['input/tex/Token', '6b4c0e42f277daebe8e676afc3b4d3b6083ebc87939ef8203672a831be1595bf', 'd317b026fc1ae0ff93d6939a15041d225385a826f8c33115284464bc2f527e35'],
  ['util/PrioritizedList', 'a700459f984ba1a5f18bf0e7dc183c242961b94c33587a9aa81bbaacf97180c8', '787bd21f3634312af666917d8f140145864f98d09ede1bdde3c957d28ec2bc25'],
  ['util/FunctionList', 'd4106c42268e0d7cb3549f8ffdbeb5bab87eae6daffae7314d5e724045a6918f', '571f57801b863b2d9f01a66dc72f2836b7fd06a1a48a3fe0057e4ec3d51a7278']
];
assert.equal(JSON.parse(fs.readFileSync(path.join(root, base, 'package.json'))).version, '4.1.3');
for (const [file, ts, cjs] of pairs) {
  for (const [relative, expected] of [[`.reference/MathJax-src/ts/${file}.ts`, ts], [`${base}/cjs/${file}.js`, cjs]])
    assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root, relative))).digest('hex'), expected, relative);
}
const {CharacterMap, RegExpMap, MacroMap, parseResult} = require(path.join(root, base, 'cjs/input/tex/TokenMap.js'));
const {Token, Macro} = require(path.join(root, base, 'cjs/input/tex/Token.js'));
const {MapHandler, SubHandler, SubHandlers} = require(path.join(root, base, 'cjs/input/tex/MapHandler.js'));

const chars = new CharacterMap('characters', null, {x: 'a'});
chars.add('nil', new Token('nil', null, null));
chars.add('alias', new Token('original', 'b', null));
assert.equal(chars.contains('alias'), true);
assert.equal(chars.contains('original'), false);
assert.equal(chars.lookup('alias').token, 'original');
assert.equal(chars.lookup('alias').char, 'b');
chars.add('alias', new Token('changed', 'c', null));
assert.equal(chars.lookup('alias').token, 'changed');
chars.remove('alias');
assert.equal(chars.lookup('alias'), undefined);
assert.equal(chars.contains('nil'), true);
assert.equal(chars.lookup('nil').char, null);

const regex = /^a$/g;
const regexp = new RegExpMap('letters', null, regex);
assert.equal(regexp.contains('a'), true);
assert.equal(regex.lastIndex, 1);
assert.equal(regexp.lookup('a'), null);
assert.equal(regex.lastIndex, 0);
assert.equal(regexp.lookup('a'), 'a');
assert.equal(regex.lastIndex, 1);
const method = () => {};
const macros = new MacroMap('macros', {source: [method, false, 2, 'x', null]});
macros.add('alias', new Macro('source', method, ['bound']));
macros.add('missing', new Macro('missing', null));
assert.equal(macros.lookup('alias').token, 'source');
assert.deepEqual(macros.lookup('source').args, [false, 2, 'x', null]);
assert.equal(macros.parserFor('source'), method);
assert.equal(macros.contains('missing'), true);
assert.equal(macros.parserFor('missing'), null);
assert.equal(parseResult(undefined), true);
assert.equal(parseResult(false), false);
assert.equal(parseResult(SubHandler.FALLBACK), SubHandler.FALLBACK);

for (const name of ['a', 'b', 'c', 'early']) new CharacterMap(name, null, {x: name});
const handler = new SubHandler();
handler.add(['a', 'b', 'c'], null);
assert.equal(handler.toString(), 'c, b, a');
assert.equal(handler.lookup('x').char, 'c');
handler.add(['early'], null, 1);
assert.equal(handler.toString(), 'early, c, b, a');
handler.add(['a'], null);
handler.remove(['a']);
assert.equal(handler.toString(), 'early, c, b, a');
new CharacterMap('first', null, {x: 'f'});
new CharacterMap('last', null, {x: 'l'});
const partial = new SubHandler();
const warnings = [];
const log = console.log;
try {
  console.log = message => warnings.push(message);
  partial.add(['first', 'missing', 'last'], null);
} finally { console.log = log; }
assert.equal(partial.toString(), 'last');
assert.deepEqual(warnings, ["TexParser Warning: Configuration 'missing' not found! Omitted."]);
new CharacterMap('visible', null, {x: 'v'});
const suppressed = new CharacterMap('suppressed', null, {});
suppressed.add('x', new Token('x', null, null));
const hidden = new SubHandler();
hidden.add(['visible', 'suppressed'], null);
assert.equal(hidden.contains('x'), false);
assert.equal(hidden.lookup('x').char, null);

const old = new CharacterMap('same', null, {x: 'old'});
const selected = new SubHandler();
selected.add(['same'], null);
const replacement = new CharacterMap('same', null, {x: 'new'});
assert.equal(MapHandler.getMap('same'), replacement);
assert.equal(selected.retrieve('same'), old);
assert.equal(selected.lookup('x').char, 'old');
selected.add(['same'], null);
selected.remove(['same']);
assert.equal(selected.lookup('x').char, 'new');

const groups = new SubHandlers();
new CharacterMap('grouped', null, {x: 'old'});
groups.add({macro: ['grouped']}, {});
new CharacterMap('grouped', null, {x: 'new'});
groups.add({character: ['grouped']}, {});
assert.deepEqual(Array.from(groups.keys()), ['macro', 'character']);
assert.equal(groups.retrieve('grouped').lookup('x').char, 'old');
groups.set('macro', new SubHandler());
assert.deepEqual(Array.from(groups.keys()), ['macro', 'character']);
assert.equal(groups.retrieve('grouped').lookup('x').char, 'new');
groups.remove({character: ['grouped']}, {});
assert.equal(groups.retrieve('grouped'), null);
console.log(JSON.stringify({status: 'passed', sourceFiles: pairs.length * 2,
  limit: 'Actual map lookup, priority composition and replacement semantics; no parser/CommandMap execution or full TeX pipeline acceptance.'}));
