'use strict';
// Development witness for original AMS configuration, array items and real TeX.
const fs = require('node:fs'), path = require('node:path');
const assert = require('node:assert/strict'), crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
for (const [file, hash] of [
  ['.reference/MathJax-src/ts/input/tex/ams/AmsItems.ts', '13757ee9f3f4555ef6f86bcec0161ddcb47494a8b08459f04124e7f77fde4f94'],
  ['.reference/MathJax-src/ts/input/tex/ams/AmsConfiguration.ts', '77bfb06042c09e5732fab46c67e8c2f7b4fa7d0404e160c318d349cd7ae93e66'],
  ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/ams/AmsItems.js', '24f1b6b0dabb84eddc52150bf5f629fb8203a3e9eeaecb5f425c7df8fe4656d5'],
  ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/ams/AmsConfiguration.js', 'ff8e1aa5936fb48e68da3cc5ff8745f1365d5dbd2416c1a908c6a06842989f79']
]) assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root, file))).digest('hex'), hash);
const reference = require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256, 'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load = file => require(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs', file));
const {TeX} = load('input/tex.js'), {MmlFactory} = load('core/MmlTree/MmlFactory.js');
const {AbstractMathItem} = load('core/MathItem.js');
const {AmsConfiguration, AmsTags} = load('input/tex/ams/AmsConfiguration.js');
const {MultlineItem, FlalignItem} = load('input/tex/ams/AmsItems.js');
const {NewcommandConfig} = load('input/tex/newcommand/NewcommandConfiguration.js');
assert.equal(AmsConfiguration.config, NewcommandConfig);
assert.equal(AmsConfiguration.items.multline, MultlineItem);
assert.equal(AmsConfiguration.items.flalign, FlalignItem);
assert.equal(AmsConfiguration.tags.ams, AmsTags);
assert.equal(AmsConfiguration.options.multlineWidth, '');
assert.equal(AmsConfiguration.options.ams.operatornamePattern.source, '^[-*a-zA-Z0-9]+');
assert.equal(AmsConfiguration.options.ams.multlineWidth, '100%');
assert.equal(AmsConfiguration.options.ams.multlineIndent, '1em');
let cases = 0;
function compile(source, options = {}) {
  let error = '';
  const jax = new TeX({packages: ['base', 'ams'], tags: 'ams', ...options,
    formatError(jax, e) { error = e.id; return jax.formatError(e); }});
  jax.setMmlFactory(new MmlFactory()); ++cases;
  const tree = jax.compile(new AbstractMathItem(source, jax, true), reference.document);
  const tables = []; tree.walkTree(node => { if (node.kind === 'mtable') tables.push(node); });
  return {jax, tree, tables, error};
}
const fixtures = [
  ['multline-right', '\\begin{multline}a\\\\b\\\\c\\end{multline}', {}],
  ['multline-left', '\\begin{multline}a\\\\b\\\\c\\end{multline}', {tagSide: 'left'}],
  ['multline-shove', '\\begin{multline*}\\shoveright a\\\\b\\\\\\shoveleft c\\end{multline*}', {}],
  ['flalign-center', '\\begin{flalign}a&=b\\end{flalign}', {}],
  ['flalign-wide', '\\begin{flalign}a&=b&c&=d\\end{flalign}', {}],
  ['xalignat-pad', '\\begin{xalignat}{2}a&=b\\end{xalignat}', {}],
  ['xalignat-left', '\\begin{xalignat}{2}a&=b\\end{xalignat}', {tagSide: 'left'}],
  ['xxalignat-pad', '\\begin{xxalignat}{2}a&=b\\end{xxalignat}', {}],
  ['xalignat-empty', '\\begin{xalignat}{}a&=b\\end{xalignat}', {}]
];
const observations = fixtures.map(([name, source, options]) => {
  const result = compile(source, options); assert.equal(result.error, '', name);
  assert.equal(result.tables.length, 1, name);
  const table = result.tables[0];
  const zero = [];
  for (const row of table.childNodes) if (row.kind === 'mlabeledtr')
    row.childNodes[0].walkTree(node => { if (node.kind === 'mpadded')
      zero.push([node.attributes.getExplicit('width'), node.attributes.getExplicit('lspace') ?? '']); });
  return {name, scopes: result.jax.parseOptions.tags.stack.length, width: table.attributes.getExplicit('width') ?? '',
    align: table.attributes.getExplicit('columnalign') ?? '',
    padding: table.attributes.getExplicit('data-array-padding') ?? '',
    rows: table.childNodes.map(row => ({kind: row.kind, cells: row.childNodes.length,
      align: row.childNodes.map(cell => cell.attributes.getExplicit('columnalign') ?? '')})),
    zero};
});
const row = (kind, align) => ({kind, cells: align.length, align});
const expected = [
  ['multline-right', 0, '100%', '', '1em 1em', [row('mtr', ['left']), row('mtr', ['']), row('mlabeledtr', ['', 'right'])], []],
  ['multline-left', 0, '100%', '', '1em 1em', [row('mlabeledtr', ['', 'left']), row('mtr', ['']), row('mtr', ['right'])], []],
  ['multline-shove', 0, '100%', '', '1em 1em', [row('mtr', ['right']), row('mtr', ['']), row('mtr', ['left'])], []],
  ['flalign-center', 1, '', 'right left', '', [row('mlabeledtr', ['', '', ''])], []],
  ['flalign-wide', 1, '100%', 'right left center right left', '', [row('mlabeledtr', Array(6).fill(''))], []],
  ['xalignat-pad', 1, '100%', 'center right left center right left center', '', [row('mlabeledtr', Array(8).fill(''))], [[0, '-1width']]],
  ['xalignat-left', 1, '100%', 'center right left center right left center', '', [row('mlabeledtr', Array(8).fill(''))], [[0, '']]],
  ['xxalignat-pad', 1, '100%', 'right left center right left', '', [row('mtr', Array(5).fill(''))], []],
  ['xalignat-empty', 1, '100%', 'center right left center', '', [row('mlabeledtr', Array(5).fill(''))], [[0, '-1width']]]
].map(([name, scopes, width, align, padding, rows, zero]) => ({name, scopes, width, align, padding, rows, zero}));
assert.deepEqual(observations, expected);
for (const [source, error] of [
  ['\\begin{multline}a&b\\end{multline}', 'MultlineRowsOneCol'],
  ['\\begin{xalignat}{1}a&b&c\\end{xalignat}', 'XalignOverflow'],
  ['\\begin{xxalignat}{1}a&b&c\\end{xxalignat}', 'XalignOverflow']
]) assert.equal(compile(source).error, error, source);
console.log(JSON.stringify({configurations: 1, itemKinds: 2, cases, compiled_source_sha256: reference.provenance.compiled_source_sha256}));
