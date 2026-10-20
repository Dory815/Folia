'use strict';
// Development reference: pinned original package, real TeX and MML classes.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
const files = {
  '.reference/MathJax-src/ts/input/tex/noundefined/NoUndefinedConfiguration.ts': 'c1e86010e9fe9f7a8c193e2b788e26f0f600b56ece6c0f9c329dfd93dcf48e9f',
  'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/noundefined/NoUndefinedConfiguration.js': '572efa297c0f8718bb8c367ae78fcbc58f35904e2483ed8e3a2c275b8407cf4b'
};
for (const [file, hash] of Object.entries(files))
  assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root, file))).digest('hex'), hash);
const reference = require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256, 'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load = name => require(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs', name));
const {TeX} = load('input/tex.js');
const {MmlFactory} = load('core/MmlTree/MmlFactory.js');
const {AbstractMathItem} = load('core/MathItem.js');
const {NodeFactory} = load('input/tex/NodeFactory.js');
const {NoUndefinedConfiguration: packageConfig} = load('input/tex/noundefined/NoUndefinedConfiguration.js');
assert.equal(packageConfig.priority, 3);
assert.equal(typeof packageConfig.fallback.macro, 'function');
assert.deepEqual(packageConfig.options.noundefined, {color: 'red', background: '', size: ''});
let cases = 0;
function fixture(options = {}, packages = ['base', 'noundefined']) {
  let error = '';
  const jax = new TeX({packages, ...options, formatError(jax, e) { error = e.id; return jax.formatError(e); }});
  jax.setMmlFactory(new MmlFactory());
  return {jax, compile(source) {
    ++cases; error = '';
    const tree = jax.compile(new AbstractMathItem(source, jax, false), reference.document);
    const tokens = []; tree.walkTree(node => { if (node.isToken) tokens.push(node); });
    return {tree, tokens, error};
  }};
}
{
  const f = fixture();
  const result = f.compile('\\missing+x+\\?+\\未知');
  assert.equal(result.error, '');
  assert.deepEqual(result.tokens.filter(node => node.kind === 'mtext').map(node => node.getText()), ['\\missing', '\\?', '\\未']);
  for (const node of result.tokens.filter(node => node.kind === 'mtext')) {
    assert.equal(node.attributes.getExplicit('mathcolor'), 'red');
    assert.equal(node.attributes.getExplicit('mathbackground'), undefined);
    assert.equal(node.attributes.getExplicit('mathsize'), undefined);
  }
  assert.equal(f.jax.parseOptions.error, false);
  f.jax.reset(12);
  assert.equal(f.compile('\\again').tokens[0].getText(), '\\again');
}
for (const options of [
  {color: '', background: '', size: ''},
  {color: 'blue', background: 'yellow', size: '125%'},
  {background: 'black', size: '2em'}
]) {
  const result = fixture({noundefined: options}).compile('\\missing');
  assert.equal(result.error, '');
  for (const key of ['color', 'background', 'size']) {
    const expected = key in options ? options[key] : key === 'color' ? 'red' : '';
    assert.equal(result.tokens[0].attributes.getExplicit('math' + key), expected || undefined);
    assert.equal(result.tokens[0].attributes.get('math' + key), expected || {color: '', background: 'transparent', size: 'normal'}[key]);
  }
}
{
  const f = fixture();
  assert.equal(f.compile('\\frac{\\missing}{x}').error, '');
  assert.equal(f.compile('\\begin{unknown}x\\end{unknown}').error, 'UnknownEnv');
  assert.equal(f.compile('{\\missing').error, 'ExtraOpenMissingClose');
  assert.equal(f.compile('\\after').error, '');
  assert.equal(fixture({}, ['base']).compile('\\missing').error, 'UndefinedControlSequence');
}
{
  const f = fixture();
  const events = [];
  f.jax.parseOptions.nodeFactory.setCreators({
    text(factory, value) { if (value === '\\missing') events.push('text'); return NodeFactory.createText(factory, value); },
    token(factory, kind, attrs, text) { if (kind === 'mtext') events.push('token'); return NodeFactory.createToken(factory, kind, attrs, text); },
    node(factory, kind, children, attrs, text) { if (kind === 'mtext') { events.push('node'); assert.equal(text.getText(), '\\missing'); assert.equal(attrs.mathcolor, 'red'); } return NodeFactory.createNode(factory, kind, children, attrs, text); }
  });
  assert.equal(f.compile('\\missing').error, '');
  assert.deepEqual(events, ['text', 'node']);
}
console.log(JSON.stringify({package: 'noundefined', cases, compiled_source_sha256: reference.provenance.compiled_source_sha256}));
