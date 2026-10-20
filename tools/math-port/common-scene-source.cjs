'use strict';
// Independent pinned MathJax Source witness for Common -> immutable Scene SVG.
// This deliberately never loads, reads, or derives expectations from native output.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const {createReference} = require('./reference-profile.cjs');

const root = path.resolve(__dirname, '../..');
const out = path.join(root, 'tests/math/fixtures/common_scene.inc');
const digest = data => crypto.createHash('sha256').update(data).digest('hex');
const profileCases = JSON.parse(fs.readFileSync(path.join(__dirname,
  'common-layout-reference/profile-cases.json'), 'utf8'));
const sourceLock = JSON.parse(fs.readFileSync(path.join(__dirname,
  'common-layout-reference/profile-source-lock.json'), 'utf8'));
const scopePath = path.join(root, 'docs/math-port/scope.json');
assert.equal(digest(fs.readFileSync(scopePath)), sourceLock.scopeHash, 'profile scope lock changed');
assert.equal(digest(fs.readFileSync(path.join(__dirname,
  'common-layout-reference/profile-cases.json'))), sourceLock.corpusSha256, '75-case corpus changed');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
for (const dependency of sourceLock.dependencies)
  assert.equal(digest(fs.readFileSync(path.join(packageRoot, dependency.file))), dependency.sha256,
    `pinned profile/font dependency changed: ${dependency.file}`);
const cases = [
  ['plain', 'x+y', false, 1280], ['mathbb', '\\mathbb{R}^n', false, 1280],
  ['fraction-inline', '\\frac{a+b}{c}', false, 1280], ['fraction-display', '\\frac{a+b}{c}', true, 1280],
  ['atop', '{a+b\\atop c}', true, 1280], ['root', '\\sqrt{x^2+1}', true, 1280],
  ['indexed-root', '\\sqrt[3]{x}', false, 1280], ['scripts', 'x_i^{j^2}', false, 1280],
  ['sum-limits', '\\sum_{i=1}^{n} i', true, 1280],
  ['stretch-fences', '\\left(\\frac{1}{x}\\right)', true, 1280],
  ['accent', '\\widehat{abc}+\\overline{x}', false, 1280],
  ['matrix', '\\begin{pmatrix}a&b\\\\c&d\\end{pmatrix}', true, 1280],
  ['arrow', '\\xrightarrow[a]{b}', true, 1280], ['text', '\\text{hello }x', false, 1280],
  ['color', '\\color{red}{x+1}', false, 1280],
  ['bbox', '\\bbox[2px,border:1px solid blue]{x}', false, 1280],
  ['phantom', 'a\\phantom{x}+b', false, 1280],
  ['linebreak', 'a+b+c+d+e+f+g+h+i+j', true, 100]
].map(([name, tex, display, width]) => ({name, tex, display, width, features: ['base']}));
for (const item of profileCases.cases) cases.push({name: item.name, tex: item.tex,
  display: item.display ?? false, width: item.width ?? 1280, features: item.features});
for (const [name, tex, display, width] of [
  ['scene-identifiers-numbers', 'abc+123.45+\\mathbb{R}', false, 1280],
  ['scene-text-composition', '\\text{漢字😀 é}+x', false, 1280],
  ['scene-unknown-run', '\\text{☃☃☃}+\\text{漢字}', false, 1280],
  ['scene-fixed-operator', '\\sum_{i=1}^{n}+\\int_0^1 x\\,dx', false, 1280],
  ['scene-assembled-operator', '\\left\\langle\\frac{1}{x}\\right\\rangle', true, 1280],
  ['scene-css-id-href', '\\href{https://example.test/a?b=1&c=2}{\\cssId{probe-id}{x+1}}', false, 1280],
  ['scene-row-multiline', '\\begin{aligned}a&=b+c+d+e+f\\\\x&=y+z\\end{aligned}', true, 120],
]) cases.push({name, tex, display, width, features: ['base']});
// Script-family acceptance inputs are appended after the original 82 records.
// Keep each input focused on one mathematical family so an inline break in
// another family cannot obscure its Source geometry.
for (const [name, tex, display, width, features] of [
  ['scene-script-sub', 'x_i', false, 1280, ['base']],
  ['scene-script-sup', 'x^i', false, 1280, ['base']],
  ['scene-script-subsup', 'x_i^j', false, 1280, ['base']],
  ['scene-script-nested', 'x_i^{j_k}', false, 1280, ['base']],
  ['scene-sum-limits-inline', '\\sum_i', false, 1280, ['base']],
  ['scene-sum-limits-display', '\\sum_i', true, 1280, ['base']],
  ['scene-overline-single', '\\overline{x}', false, 1280, ['base']],
  ['scene-widehat-single', '\\widehat{x}', false, 1280, ['base']],
  ['scene-prescript-sup-slot', '\\prescript{a}{}{X}', false, 1280, ['mathtools']],
  ['scene-prescript-sub-slot', '\\prescript{}{b}{X}', false, 1280, ['mathtools']],
  ['scene-prescript-both-slots', '\\prescript{a}{b}{X}', false, 1280, ['mathtools']],
]) cases.push({name, tex, display, width, features});
// Inline break coordination, text-space opportunities and an empty zero-style
// frame are captured as complete Source output, including root geometry.
for (const [name, tex, display, width, features] of [
  ['scene-inline-break-before', 'x\\mmlToken{mo}[linebreak="newline" linebreakstyle="before"]{+}y', false, 1280, ['base']],
  ['scene-inline-break-after', 'x\\mmlToken{mo}[linebreak="newline" linebreakstyle="after"]{+}y', false, 1280, ['base']],
  ['scene-text-wordbreak', '\\text{one two three four}', true, 40, ['base']],
  ['scene-zero-size-frame', '\\bbox[border:0px solid]{}', false, 1280, ['bbox']],
]) cases.push({name, tex, display, width, features});
for (const [name, tex, display, width] of [
  ['scene-vertical-extender-assembly', '\\left(\\frac{a}{\\frac{b}{\\frac{c}{d}}}\\right)', true, 1280],
  ['scene-horizontal-extender-assembly', '\\overbrace{abcdefghij}', true, 1280],
]) cases.push({name, tex, display, width, features: ['base']});
// Phantom keeps the measured box while suppressing its hidden drawing.
// Narrow makeboxes force negative mpadded dw and preserve Source's explicit
// center/right alignment metadata around a raised rule.
for (const [name, tex, display, width, features] of [
  ['scene-simple-phantom', '\\phantom{x}', true, 1280, ['base']],
  ['scene-padding-negative-dw-center', '\\makebox[.25em][c]{\\rule[.2em]{1em}{.4em}}', false, 1280, ['base']],
  ['scene-padding-negative-dw-right', '\\makebox[.25em][r]{\\rule[.2em]{1em}{.4em}}', false, 1280, ['base']],
]) cases.push({name, tex, display, width, features});
// Table-focused inputs are appended after the original 102 records. The first
// five reuse exact TeX inputs from the independent 14-case table witness;
// additions pin dashed row/column lines, labels, nesting and percentage sizing.
for (const item of [
  {name: 'scene-table-array-basic', tex: String.raw`\begin{array}{cc}a&b\\c&d\end{array}`, display: true},
  {name: 'scene-table-array-column-rules', tex: String.raw`\begin{array}{c|c||r}a&b&c\\d&e&f\end{array}`, display: true},
  {name: 'scene-table-array-row-rules', tex: String.raw`\begin{array}{cc}\hline a&b\\\hline c&d\\\hline\end{array}`, display: true},
  {name: 'scene-table-array-cell-padding', tex: String.raw`\begin{array}{c@{\quad}r}x&1\\longer&22\end{array}`, display: true},
  {name: 'scene-table-array-paragraph-column', tex: String.raw`\begin{array}{p{3em}c}a+b+c&x\\d+e+f&y\end{array}`, display: true},
  {name: 'scene-table-row-dashed', tex: String.raw`\begin{array}{cc}\hdashline a&b\\\hdashline c&d\\\hdashline\end{array}`, display: true},
  {name: 'scene-table-column-dashed', tex: String.raw`\begin{array}{c:c}a&b\\c&d\end{array}`, display: true},
  {name: 'scene-table-label-right', tex: String.raw`\begin{align}x&=1\tag{TableSceneRightA}\\y&=2\tag{TableSceneRightB}\end{align}`, display: true, tagSide: 'right'},
  {name: 'scene-table-label-left-metric', tex: String.raw`\begin{align}x&=1\tag{TableSceneLeftA}\\y&=2\tag{TableSceneLeftB}\end{align}`, display: true, em: 20, ex: 7, tagSide: 'left'},
  {name: 'scene-table-nested-label', tex: String.raw`\begin{align}\begin{aligned}x&=1\\y&=2\end{aligned}&=0\tag{TableSceneNestedA}\end{align}`, display: true, tagSide: 'right'},
]) cases.push({...item, width: 1280, features: ['base', 'ams']});
// Source-validated CommonMenclose line/radical cases promoted from the ignored
// enclosure discovery witness. Preserve their exact full38 input metrics.
for (const [name, tex, display, em, ex, width] of [
  ['scene-enclose-all-lines', String.raw`\enclose{top bottom left right horizontalstrike verticalstrike updiagonalstrike downdiagonalstrike}{x}`, false, 16, 8, 1280],
  ['scene-enclose-paired-sides', String.raw`\enclose{actuarial madruwb}{x}`, false, 16, 8, 1280],
  ['scene-enclose-radical-top', String.raw`\enclose{radical top}{x}`, false, 16, 8, 1280],
]) cases.push({name, tex, display, em, ex, width, features: ['enclose']});

function cpp(value) {
  // MSVC rejects a single string literal token beyond 16K; keep each emitted
  // raw token well below the compiler limit. Iterate JS code points so a
  // UTF-8 chunk boundary can never split an astral surrogate pair.
  const chunks = [];
  let chunk = '', bytes = 0;
  for (const codePoint of value) {
    const size = Buffer.byteLength(codePoint, 'utf8');
    if (bytes + size > 3500 && chunk) {
      chunks.push(chunk);
      chunk = '';
      bytes = 0;
    }
    chunk += codePoint;
    bytes += size;
  }
  if (chunk || chunks.length === 0) chunks.push(chunk);
  return chunks.map(part => {
    let delimiter = 'FOLIASCENE';
    for (let suffix = 0; part.includes(`)${delimiter}"`); ++suffix)
      delimiter = `FOLIASCENE${suffix + 1}`;
    return `R"${delimiter}(${part})${delimiter}"`;
  }).join('\n        ');
}
function attr(svg, name) {
  const rootTag = svg.match(/<svg\b[^>]*>/)?.[0] || '';
  const escaped = name.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  return rootTag.match(new RegExp('\\b' + escaped + '="([^"]*)"'))?.[1] ?? '';
}
function nestedAssemblyPieces(svg, axis) {
  const openings = [...svg.matchAll(/<svg\b[^>]*>/g)];
  const transform = axis === 'vertical'
    ? /transform="scale\(1,[^)]+\)"/
    : /transform="scale\([^,]+,1\)"/;
  const pieces = [];
  for (const opening of openings.slice(1)) {
    const close = svg.indexOf('</svg>', opening.index);
    if (close < 0) continue;
    const piece = svg.slice(opening.index, close + '</svg>'.length);
    if (piece.includes('<path data-c=') && transform.test(piece)) pieces.push(piece);
  }
  return pieces;
}

async function capture() {
  const ref = createReference(); // exactly once per process; profile/pins validated there
  const records = [];
  for (const item of cases) {
    let svg = '', error = null;
    try {
      // The profile default is the Source's right-side tag placement. Specific
      // left-label witnesses record their per-input TeX option explicitly.
      ref.input.parseOptions.options.tagSide = item.tagSide ?? 'right';
      const node = ref.document.convert(item.tex, {
        display: item.display, em: item.em ?? 16, ex: item.ex ?? 8,
        containerWidth: item.width
      });
      svg = ref.adaptor.serializeXML(node);
    } catch (e) { error = {name: e?.name || 'Error', message: String(e?.message || e)}; }
    records.push({...item, tagSide: item.tagSide ?? '', em: item.em ?? 16, ex: item.ex ?? 8, svg, error,
      root: {width: attr(svg, 'width'), height: attr(svg, 'height'), viewBox: attr(svg, 'viewBox'),
        xmlns: attr(svg, 'xmlns')},
      geometry: [...svg.matchAll(/<(?:path|rect|line|polygon|g)\b[^>]*>/g)].map(m => m[0])});
  }
  const byName = new Map(records.map(record => [record.name, record]));
  for (const [name, axis] of [['scene-vertical-extender-assembly', 'vertical'],
                              ['scene-horizontal-extender-assembly', 'horizontal']]) {
    const record = byName.get(name);
    assert.ok(record && !record.error, `${name} must convert in pinned Source`);
    const pieces = nestedAssemblyPieces(record.svg, axis);
    assert.ok(pieces.length > 0, `${name} must contain Source nested SVG extender pieces`);
  }
  const phantom = byName.get('scene-simple-phantom');
  assert.ok(phantom && !phantom.error, 'simple phantom must convert in pinned Source');
  assert.ok(phantom.svg.includes('data-mml-node="mphantom"'),
    'simple phantom must preserve its Source wrapper');
  assert.ok(!phantom.svg.includes('data-c='), 'simple phantom must suppress all glyph drawing');
  const [viewX, viewY, viewWidth, viewHeight] = phantom.root.viewBox.split(/\s+/).map(Number);
  assert.ok(Number.isFinite(viewX) && Number.isFinite(viewY) && viewWidth > 0 && viewHeight > 0,
    'simple phantom must retain nonzero completed Source dimensions');
  for (const [name, alignment] of [
    ['scene-padding-negative-dw-center', 'center'],
    ['scene-padding-negative-dw-right', 'right'],
  ]) {
    const record = byName.get(name);
    assert.ok(record && !record.error, `${name} must convert in pinned Source`);
    assert.ok(record.svg.includes(`data-mml-node="mpadded" data-align="${alignment}"`),
      `${name} must retain Source data-align=${alignment}`);
    assert.ok(record.svg.includes('width="1000"') && record.root.viewBox.includes(' 250 '),
      `${name} must demonstrate negative dw against its wider rule content`);
    assert.ok(record.svg.includes('transform="translate(0,200)"'),
      `${name} must retain its positive vertical offset`);
  }
  const requireSourceMarkup = (name, fragments) => {
    const record = byName.get(name);
    assert.ok(record && !record.error, `${name} must convert in pinned Source`);
    for (const fragment of fragments)
      assert.ok(record.svg.includes(fragment), `${name} Source SVG must contain ${fragment}`);
  };
  for (const name of ['scene-table-array-basic', 'scene-table-array-cell-padding',
                      'scene-table-array-paragraph-column'])
    requireSourceMarkup(name, ['data-mml-node="mtable"']);
  requireSourceMarkup('scene-table-array-row-rules', ['data-mml-node="mtable"', 'data-line="h"', 'mjx-solid']);
  requireSourceMarkup('scene-table-array-column-rules', ['data-mml-node="mtable"', 'data-line="v"', 'mjx-solid']);
  requireSourceMarkup('scene-table-row-dashed', ['data-mml-node="mtable"', 'data-frame-styles="dashed', 'data-line="h"', 'mjx-dashed', 'stroke-dasharray=']);
  requireSourceMarkup('scene-table-column-dashed', ['data-mml-node="mtable"', 'data-line="v"', 'mjx-dashed']);
  const sourceEnclosureLines = (name, count) => {
    const record = byName.get(name);
    assert.ok(record && !record.error, `${name} must convert in pinned Source`);
    assert.ok(record.svg.includes('data-mml-node="menclose"'), `${name} must contain Source menclose`);
    assert.equal((record.svg.match(/<line\b/g) || []).length, count,
      `${name} Source enclosure line count`);
  };
  sourceEnclosureLines('scene-enclose-all-lines', 8);
  sourceEnclosureLines('scene-enclose-paired-sides', 4);
  const radicalEnclosure = byName.get('scene-enclose-radical-top');
  assert.ok(radicalEnclosure && !radicalEnclosure.error,
    'scene-enclose-radical-top must convert in pinned Source');
  assert.ok(radicalEnclosure.svg.includes('data-mml-node="menclose"') &&
    radicalEnclosure.svg.includes('data-mml-node="msqrt"') &&
    radicalEnclosure.svg.includes('data-c="221A"') &&
    radicalEnclosure.svg.includes('translate(853,0)') &&
    (radicalEnclosure.svg.match(/<line\b/g) || []).length === 1,
    'radical+top Source must retain nested sqrt, left offset and top line');
  for (const [name, side] of [['scene-table-label-right', 'right'],
                              ['scene-table-label-left-metric', 'left'],
                              ['scene-table-nested-label', 'right']]) {
    const record = byName.get(name);
    assert.ok(record && !record.error, `${name} must convert in pinned Source`);
    assert.ok(record.svg.includes('data-labels="true"'), `${name} must retain Source labels`);
    assert.ok(record.svg.includes('data-mml-node="mlabeledtr"'), `${name} must retain labeled rows`);
    assert.equal(record.tagSide, side, `${name} tag side must be explicit Source input metadata`);
    assert.ok(record.svg.includes('width="100%"') && record.svg.includes('data-mjx-viewBox='),
      `${name} must preserve the Source percentage viewport`);
    assert.ok(record.svg.includes(`mjx-eqn:${name.includes('left') ? 'TableSceneLeftA' :
      name.includes('nested') ? 'TableSceneNestedA' : 'TableSceneRightA'}`),
      `${name} must retain its unique explicit Source tag identity`);
  }
  const nestedTable = byName.get('scene-table-nested-label');
  assert.ok((nestedTable.svg.match(/data-mml-node="mtable"/g) || []).length >= 2,
    'nested label witness must contain nested MathML table structures');
  const metricTable = byName.get('scene-table-label-left-metric');
  assert.equal(metricTable.em, 20);
  assert.equal(metricTable.ex, 7);
  assert.equal(metricTable.width, 1280);
  assert.ok(metricTable.svg.includes('scale(63.14)'),
    'nondefault metric label table must pin Source top-table scale precision=2');
  return {records};
}

function generate({records}) {
  const baseline = profileCases.cases.length + 18;
  let body = `// Pinned MathJax 4.1.3 Source -> Common SVG witness; raw SVG and decoded root/geometry observations.\n`+
    `// Source corpus: original 18 baseline + profile-cases.json (${profileCases.cases.length}); full 75/150 gates remain outstanding.\n`+
    `// scope=${sourceLock.scopeHash}; profile corpus=${sourceLock.corpusSha256}; font/Common dependency files=${sourceLock.dependencies.length}.\n`+
    `// Source is generated only by common-scene-source.cjs through reference-profile.cjs.\n`+
    `struct CommonSceneCase { std::string_view name, tex; bool display; double em, ex; int width; std::string_view tag_side;\n`+
    `    std::string_view svg, root_width, root_height, root_view_box; std::string_view geometry;\n`+
    `    std::string_view error_name, error_message; };\n`+
    `constexpr std::array commonSceneCases{\n`;
  for (const r of records) {
    body += `    CommonSceneCase{${cpp(r.name)}, ${cpp(r.tex)}, ${r.display}, ${r.em}, ${r.ex}, ${r.width}, ${cpp(r.tagSide ?? '')},\n`+
      `        ${cpp(r.svg)}, ${cpp(r.root.width)}, ${cpp(r.root.height)}, ${cpp(r.root.viewBox)},\n`+
      `        ${cpp(JSON.stringify(r.geometry))}, ${cpp(r.error?.name ?? '')}, ${cpp(r.error?.message ?? '')}},\n`;
  }
  body += `};\n// ${records.length} records; canonical 75 corpus=${baseline}; extra focused cases=${records.length-baseline}.\n`;
  return body;
}

(async () => {
  const witness = await capture();
  const next = generate(witness);
  const actualHash = fs.existsSync(out) ? digest(fs.readFileSync(out)) : null;
  const nextHash = digest(next);
  if (process.argv.includes('--write-fixture')) {
    if (actualHash !== nextHash) fs.writeFileSync(out, next);
    process.stdout.write(`wrote ${path.relative(root, out)} sha256=${nextHash} records=${witness.records.length}\n`);
  } else {
    assert.equal(actualHash, nextHash, `Source fixture differs; inspect Source observations, then use --write-fixture. expected fixture hash ${actualHash}, generated ${nextHash}`);
    process.stdout.write(`Source bytecheck ok: ${witness.records.length} records sha256=${nextHash}\n`);
  }
})().catch(error => { process.stderr.write(error.stack + '\n'); process.exitCode = 1; });
