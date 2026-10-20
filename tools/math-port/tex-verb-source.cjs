'use strict';
// Development reference: original VerbConfiguration with the actual TeX parser.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
for (const [file, hash] of [
  ['.reference/MathJax-src/ts/input/tex/verb/VerbConfiguration.ts', '8cd31d544fd5b61ad725e9623e6d2d5e1906299d8e309d7c22cd97f42c25bfab'],
  ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/verb/VerbConfiguration.js', 'a1bc5b9827b6dfd1606424cffb71cff73ad11a1667b0a5f80b7a1dd26b6163e9']
]) assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root, file))).digest('hex'), hash);
const reference = require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256, 'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load = file => require(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs', file));
const {TeX} = load('input/tex.js');
const {MmlFactory} = load('core/MmlTree/MmlFactory.js');
const {AbstractMathItem} = load('core/MathItem.js');
const {NodeFactory} = load('input/tex/NodeFactory.js');
const {VerbConfiguration} = load('input/tex/verb/VerbConfiguration.js');
const {MapHandler} = load('input/tex/MapHandler.js');
assert.deepEqual(VerbConfiguration.handler.macro, ['verb']);
assert.equal(MapHandler.getMap('verb').lookup('verb').func.name, 'Verb');
let cases = 0;
function fixture(packages = ['base', 'verb']) {
  let error = '', message = '';
  const jax = new TeX({packages, formatError(jax, e) { error = e.id; message = e.message; return jax.formatError(e); }});
  jax.setMmlFactory(new MmlFactory());
  return {jax, compile(source) {
    ++cases; error = ''; message = '';
    const root = jax.compile(new AbstractMathItem(source, jax, false), reference.document);
    const tokens = []; root.walkTree(node => { if (node.isToken) tokens.push(node); });
    return {root, tokens, error, message};
  }};
}
for (const [source, expected] of [
  ['\\verb|a b  c|', 'a\u00a0b\u00a0\u00a0c'],
  ['\\verb||', ''],
  ['\\verb+a{b}\\x%$#+', 'a{b}\\x%$#'],
  ['\\verb*abc*', 'abc'],
  ['\\verb \t\n|a b|', 'a\u00a0b'],
  ['\\verb|a\tb\nc\u00a0d|', 'a\tb\nc\u00a0d'],
  ['\\verb|A😀B|', 'A😀B'],
  ['\\verb§a b§', 'a\u00a0b'],
  ['\\verb aXYa', 'XY']
]) {
  const result = fixture().compile(source);
  assert.equal(result.error, '', source);
  assert.equal(result.tokens.length, 1, source);
  assert.equal(result.tokens[0].kind, 'mtext');
  assert.equal(result.tokens[0].getText(), expected, source);
  assert.equal(result.tokens[0].attributes.get('mathvariant'), 'monospace');
  assert.equal(result.tokens[0].childNodes.length, 1);
}
{
  const f = fixture(['base', 'verb', 'boldsymbol']);
  const result = f.compile('x+\\mathbf{\\verb|a b|}+\\boldsymbol{\\verb|c d|}+y');
  assert.equal(result.error, '');
  const text = result.tokens.filter(node => node.kind === 'mtext');
  assert.deepEqual(text.map(node => node.getText()), ['a\u00a0b', 'c\u00a0d']);
  assert.deepEqual(text.map(node => node.attributes.get('mathvariant')), ['monospace', 'monospace']);
}
{
  const f = fixture();
  for (const [source, error, message] of [
    ['\\verb', 'MissingArgFor', 'Missing argument for \\verb'],
    ['\\verb \t\n', 'MissingArgFor', 'Missing argument for \\verb'],
    ['\\verb|abc', 'NoClosingDelim', "Can't find closing delimiter for \\verb"],
    ['\\verb😀abc😀', 'NoClosingDelim', "Can't find closing delimiter for \\verb"]
  ]) {
    const result = f.compile(source);
    assert.equal(result.error, error, source); assert.equal(result.message, message, source);
  }
  assert.equal(f.compile('\\verb|ok|').error, '');
}
{
  const f = fixture(['base', 'verb', 'newcommand']);
  assert.equal(f.compile('\\let\\v\\verb\\v|a b|').tokens[0].getText(), 'a\u00a0b');
  let result = f.compile('\\v'); assert.equal(result.error, 'MissingArgFor'); assert.equal(result.message, 'Missing argument for \\verb');
  result = f.compile('\\v|x'); assert.equal(result.error, 'NoClosingDelim'); assert.equal(result.message, "Can't find closing delimiter for \\v");
  f.jax.reset(7); assert.equal(f.compile('\\v|ok|').tokens[0].getText(), 'ok');
}
{
  const f = fixture(); let calls = 0;
  f.jax.parseOptions.nodeFactory.set('token', (factory, kind, attrs, text) => {
    ++calls; assert.equal(kind, 'mtext'); assert.equal(text, 'a\u00a0b'); assert.equal(attrs.mathvariant, 'monospace');
    return NodeFactory.createToken(factory, kind, attrs, text);
  });
  assert.equal(f.compile('\\verb|a b|').error, ''); assert.equal(calls, 1);
}
console.log(JSON.stringify({package: 'verb', cases, compiled_source_sha256: reference.provenance.compiled_source_sha256}));
