'use strict';
// Independent Source witness for the native FindTeX string-scanning contract.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');

const root = path.resolve(__dirname, '../..');
const fixturePath = path.join(root, 'tests/math/fixtures/find_tex.inc');
const pins = [
  ['.reference/MathJax-src/ts/input/tex/FindTeX.ts',
    '65355d94cac0c1e8a790313c52e231212733b5a98d6f6697c2c7864812a72944'],
  ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/FindTeX.js',
    '1361bd1171e5ef0c8210342d89c9cebd660dbb9d4810bc571c64172b3a5c3d4b'],
  ['.reference/MathJax-src/testsuite/tests/input/tex/Tex.test.ts',
    '480eb2ccc84e6c87576c31ca2c63d5e136e5638f98cc5ab45264e4a3cacc5f29']
];
const sha = file => crypto.createHash('sha256')
  .update(fs.readFileSync(path.join(root, file))).digest('hex');
for (const [file, expected] of pins)
  assert.equal(sha(file), expected, 'pinned input changed: ' + file);

const {FindTeX} = require(path.join(root,
  'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/FindTeX.js'));
const sourceTests = [
  'abc $$ x + 1 $$ def',
  'abc \\begin{equation} x=y \\end{equation} def',
  'abc $$a + {\\bf b} + c$$ def',
  'abc \\$ def',
  'abc \\ref{x} def',
  'abc $$ x + 1',
  'abc \\begin{align} x + 1'
];
const sourceOptions = c => ({
  inlineMath: c.inlineMath || [['\\(', '\\)']],
  displayMath: c.displayMath || [['$$', '$$'], ['\\[', '\\]']],
  processEscapes: c.processEscapes,
  processEnvironments: c.processEnvironments,
  processRefs: c.processRefs
});

const cases = [];
for (let bits = 0; bits < 8; ++bits) {
  cases.push({name: 'options-' + bits, strings: [
    'a \\$ b \\ref{x} c $$q$$ \\begin{equation}z\\end{equation}',
    'again \\$ $$r$$ \\eqref{y}'
  ], processEscapes: !!(bits & 1), processEnvironments: !!(bits & 2),
  processRefs: !!(bits & 4)});
}
for (let i = 0; i < sourceTests.length; ++i)
  cases.push({name: 'upstream-' + i, strings: [sourceTests[i]],
    processEscapes: true, processEnvironments: true, processRefs: true});
cases.push(
  {name: 'brace-controls', strings: ['L $$a {b $$ c} \\} d \\{ e$$ R \\text{\\$}'],
    processEscapes: true, processEnvironments: true, processRefs: true},
  {name: 'overlap-longest', strings: ['<a>one</a> <ab>two</ab> <a>three</a>'],
    inlineMath: [['<a>', '</a>'], ['<ab>', '</ab>']], displayMath: [],
    processEscapes: false, processEnvironments: false, processRefs: false},
  {name: 'duplicate-last-mode', strings: ['##d##'],
    inlineMath: [['##', '##']], displayMath: [['##', '##']],
    processEscapes: false, processEnvironments: false, processRefs: false},
  {name: 'environment-cases', strings: [
    'x \\begin {align*}a {b} \\end{align*} y \\begin{align}c\\end{align}',
    'bad \\begin{align}open'
  ], processEscapes: false, processEnvironments: true, processRefs: false},
  {name: 'ref-escapes', strings: ['\\ref {x} \\eqref{y} \\$ \\\\ \\% \\ref{unfinished'],
    processEscapes: true, processEnvironments: false, processRefs: true},
  {name: 'utf16-nul-surrogates',
    strings: ['😀 $$x$$ \u0000 \ud800 \udfff $$y$$', 'tail 😀 \\ref{z}'],
    processEscapes: false, processEnvironments: false, processRefs: true},
  {name: 'repeat-cursor', strings: ['$$first$$ then $$second$$ \\$'],
    processEscapes: true, processEnvironments: true, processRefs: true}
);

const cppString = value => {
  let out = 'std::u16string_view{u\"';
  for (let i = 0; i < value.length; ++i) {
    const code = value.charCodeAt(i), ch = value[i];
    if (ch === '\\') out += '\\\\';
    else if (ch === '\"') out += '\\\"';
    else if (code === 0 || (code >= 0xd800 && code <= 0xdfff))
      out += '\\x' + code.toString(16).padStart(4, '0') + '\" u\"';
    else if (code === 10) out += '\\n';
    else if (code === 13) out += '\\r';
    else if (code === 9) out += '\\t';
    else if (code < 32 || code > 126) out += '\\u' + code.toString(16).padStart(4, '0');
    else out += ch;
  }
  return out + '\", ' + value.length + '}';
};
function capture() {
  return cases.map(c => {
    const finder = new FindTeX(sourceOptions(c));
    const found = finder.findMath(c.strings);
    const strings = c.strings.map(cppString).join(', ');
    const inlines = (c.inlineMath || [['\\(', '\\)']])
      .map(pair => '{' + cppString(pair[0]) + ', ' + cppString(pair[1]) + '}').join(', ');
    const displays = (c.displayMath || [['$$', '$$'], ['\\[', '\\]']])
      .map(pair => '{' + cppString(pair[0]) + ', ' + cppString(pair[1]) + '}').join(', ');
    const rows = found.map(m => '    {' + cppString(m.math) + ', ' + cppString(m.open) + ', ' +
      cppString(m.close) + ', ' + m.n + ', ' + m.start.n + ', ' + m.end.n + ', ' +
      'MathDisplay::' + (m.display === null ? 'escaped' : m.display ? 'display_math' :
        'inline_math') + '}').join(',\n');
    return '    {' + cppString(c.name) + ', {' + strings + '}, {' + inlines + '}, {' + displays +
      '}, ' + c.processEscapes + ', ' + c.processEnvironments + ', ' + c.processRefs +
      ', {\n' + rows + '\n    }}';
  }).join(',\n');
}

const fixture = '// Generated from pinned MathJax FindTeX Source; UTF-16 lengths are explicit.\n' +
  'static const std::vector<FindTexCase> findTexCases{\n' + capture() + '\n};\n';
const existing = fs.existsSync(fixturePath) ? fs.readFileSync(fixturePath) : null;
if (process.argv.includes('--write-fixture')) {
  if (!existing || !existing.equals(Buffer.from(fixture))) fs.writeFileSync(fixturePath, fixture);
} else {
  assert.ok(existing, 'fixture missing; run with --write-fixture');
  assert.deepEqual(existing, Buffer.from(fixture),
    'Source witness differs from checked-in fixture');
}
const recordCount = cases.reduce((n, c) =>
  n + new FindTeX(sourceOptions(c)).findMath(c.strings).length, 0);
console.log('FindTeX Source witness: ' + cases.length + ' parser inputs; ' +
  recordCount + ' records; ' + sourceTests.length +
  ' upstream Tex.test.ts Finder inputs');
