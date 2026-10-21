'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { spawnSync } = require('node:child_process');

const packet = __dirname;
const comparator = path.join(packet, 'common-menclose-compare-svg.cjs');
const { sourceGeometry, sourcePath, parseSvg, decimal, key, add } = require('./common-menclose-compare-svg.cjs');
function findRoot(start = packet) {
  for (let current = path.resolve(start); current !== path.dirname(current); current = path.dirname(current))
    if (fs.existsSync(path.join(current, 'docs/math-port/scope.json'))) return current;
  return null;
}
const rootIndex = process.argv.indexOf('--repo-root');
assert.ok(rootIndex < 0 ? process.argv.length === 2 : (process.argv[rootIndex + 1] && process.argv.length === rootIndex + 2),
  'usage: node common-menclose-comparator-selftest.cjs [--repo-root <repository>]');
const root = rootIndex < 0 ? findRoot() : path.resolve(process.argv[rootIndex + 1]);
assert.ok(root, 'pass --repo-root <repository> or run from the repository');
const expectedPath = path.join(root, 'tests/math/fixtures/common-source/menclose-geometry-source-expected.json');
const expected = JSON.parse(fs.readFileSync(expectedPath, 'utf8'));
const testDir = path.join(root, 'build/math-port/common-source/menclose-comparator-selftest');
fs.mkdirSync(testDir, { recursive: true });
let positionalCliResult;

function decimalText(k) {
  const [ns, ds] = k.split('/'); let n=BigInt(ns), d=BigInt(ds);
  let twos=0, fives=0; while(d%2n===0n){d/=2n;++twos;} while(d%5n===0n){d/=5n;++fives;}
  assert.equal(d,1n,'all test geometry should have finite base-ten expansions');
  const scale=Math.max(twos,fives); n*=2n**BigInt(scale-twos)*5n**BigInt(scale-fives);
  const neg=n<0n; if(neg)n=-n; const digits=n.toString().padStart(scale+1,'0');
  const s=scale?`${digits.slice(0,-scale)}.${digits.slice(-scale).replace(/0+$/,'')}`:digits;
  return `${neg?'-':''}${s.endsWith('.')?s.slice(0,-1):s}`;
}
function scaledText(k, scale) {
  const [n,d]=k.split('/').map(BigInt); return decimalText(`${n*BigInt(scale)}/${d}`);
}
function serializePath(segments) {
  return segments.map(([cmd, ...v]) => {
    if (cmd === 'Z') return 'Z';
    if (cmd === 'M' || cmd === 'L') return `${cmd} ${scaledText(v[0],1000)} ${scaledText(v[1],1000)}`;
    if (cmd === 'A') return `A ${scaledText(v[0],1000)} ${scaledText(v[1],1000)} ${decimalText(v[2])} ${+v[3]} ${+v[4]} ${scaledText(v[5],1000)} ${scaledText(v[6],1000)}`;
    throw new Error(`unexpected source segment ${cmd}`);
  }).join(' ');
}
function serializeTransform(ops) {
  return ops.map(op => op[0] === 'T'
    ? `translate(${scaledText(op[1],1000)},${scaledText(op[2],1000)})`
    : `rotate(${decimalText(op[1])},${scaledText(op[2],1000)},${scaledText(op[3],1000)})`).join(' ');
}
function serializeShape(shape) {
  const attrs = { d: serializePath(shape.segments) };
  if (shape.transform.length) attrs.transform = serializeTransform(shape.transform);
  const paint = shape.paint;
  if (paint.fill !== 'inherited') attrs.fill = paint.fill;
  if (paint.stroke !== 'inherited') attrs.stroke = paint.stroke;
  if (paint.stroke_width !== '0/1') attrs['stroke-width'] = scaledText(paint.stroke_width,1000);
  if (paint.opacity !== '1/1') attrs.opacity = decimalText(paint.opacity);
  if (paint.fill_rule !== 'nonzero') attrs['fill-rule'] = paint.fill_rule;
  if (paint.fill_opacity !== null) attrs['fill-opacity'] = decimalText(paint.fill_opacity);
  if (paint.stroke_opacity !== null) attrs['stroke-opacity'] = decimalText(paint.stroke_opacity);
  if (paint.line_cap !== null) attrs['stroke-linecap'] = paint.line_cap;
  if (paint.line_join !== null) attrs['stroke-linejoin'] = paint.line_join;
  const transformText = attrs.transform;
  if (transformText) delete attrs.transform;
  const pathMarkup = `<path ${Object.entries(attrs).map(([k,v]) => `${k}="${v}"`).join(' ')} />`;
  return transformText ? `<g data-mml-node="mrow" transform="${transformText}">${pathMarkup}</g>`
    : `<g data-mml-node="mrow">${pathMarkup}</g>`;
}
function render(row) {
  const rootGroup = '<g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)">';
  const inner = `<g data-mml-node="mrow">${sourceGeometry(row.source_svg).map(serializeShape).join('')}</g>`;
  return `<svg style="vertical-align: -1.25ex;" xmlns="http://www.w3.org/2000/svg" width="7.5ex" height="3.75ex" role="img" focusable="false" viewBox="0 -1000 3000 1500">${rootGroup}${inner}</g></svg>`;
}
function dump(rows) {
  return rows.map(row => `${row.case}\t${row.notation}\t${render(row)}`).join('\n') + '\n';
}
function run(name, rows, shouldPass) {
  const file = path.join(testDir, `${name}.tsv`);
  fs.writeFileSync(file, dump(rows));
  return runText(name, fs.readFileSync(file, 'utf8'), shouldPass, file);
}
function runText(name, text, shouldPass, file = path.join(testDir, `${name}.tsv`)) {
  fs.writeFileSync(file, text);
  const dumpFromRepo = path.relative(root, file);
  const expectedFromRepo = path.relative(root, expectedPath);
  const result = spawnSync(process.execPath,
    [comparator, '--repo-root', root, '--expected', expectedFromRepo, '--dump', dumpFromRepo],
    { cwd: path.parse(root).root, encoding: 'utf8' });
  assert.equal(result.error, undefined, result.error?.message);
  assert.equal(result.status === 0, shouldPass,
    `${name}: expected ${shouldPass ? 'pass' : 'reject'}, status=${result.status}\n${result.stdout}\n${result.stderr}`);
  return shouldPass ? 'accepted expected baseline' : 'rejected as expected';
}

assert.equal(expected.rows.length, 56);
const svgPrefix='<svg style="x" xmlns="http://www.w3.org/2000/svg" width="1" height="1" role="img" focusable="false" viewBox="0 0 1 1"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="mrow"><path d="';
const svgSuffix='" /></g></g></svg>';
assert.throws(()=>parseSvg(`${svgPrefix}M 0 0 A -1 1 0 0 1 2 2${svgSuffix}`), /radii must be nonnegative/,
  'native SVG parser must directly reject negative arc radii');
assert.throws(()=>parseSvg(`${svgPrefix}M 1e999 0 L 1 1${svgSuffix}`), /finite SVG number range/,
  'native SVG parser must directly reject values outside finite binary64 SVG range');
assert.equal(sourceGeometry([{tag:'rect',attrs:{x:'0',y:'0',width:'1',height:'1','stroke-width':'1'}}])[0].paint.fill,
  'inherited', 'a raw rect without fill retains SVG inherited fill semantics');
assert.equal(sourceGeometry([{tag:'ellipse',attrs:{rx:'1',ry:'1',cx:'0',cy:'0',fill:'none','stroke-width':'1'}}])[0].paint.fill,
  'none', 'an explicit raw ellipse fill is preserved');
assert.equal(sourceGeometry([{tag:'path',attrs:{d:'M 0 0 L 1 1',style:{'stroke-width':'1'},fill:'red'}}])[0].paint.fill,
  'red', 'style stroke-width does not override an explicit path fill');
assert.equal(sourceGeometry([{tag:'path',attrs:{d:'M 0 0 L 1 1',style:{'stroke-width':'1'}}}])[0].paint.fill,
  'inherited', 'style stroke-width alone leaves path fill inherited');
assert.deepEqual(sourcePath('M 0 0 l -30.1 408.5 l 67 100.5'),
  sourcePath('M 0 0 L -30.1 408.5 L 36.9 509'),
  'relative and absolute base-ten paths normalize to identical exact coordinates');
const primitiveSvg = '<svg style="x" xmlns="http://www.w3.org/2000/svg" width="1" height="1" role="img" focusable="false" viewBox="0 0 1 1">' +
  '<g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="mrow">' +
  '<line x1="1" y1="2" x2="5" y2="7" stroke-width="2" />' +
  '<rect x="0" y="0" width="10" height="8" rx="8" fill="none" stroke-width="1" />' +
  '<ellipse rx="3" ry="4" cx="10" cy="11" fill="none" stroke-width="2" />' +
  '<path style="stroke-width: 3;" d="M 0 0 l 2 3 a 4 5 0 0,1 2 3 L 9 9" fill="none" ' +
  'stroke-linecap="round" stroke-linejoin="mitre" transform="rotate(20,0,0)" />' +
  '</g></g></svg>';
const primitiveExpected = sourceGeometry([
  {tag:'line',attrs:{x1:'1',y1:'2',x2:'5',y2:'7','stroke-width':'2'}},
  {tag:'rect',attrs:{x:'0',y:'0',width:'10',height:'8',rx:'8',fill:'none','stroke-width':'1'}},
  {tag:'ellipse',attrs:{rx:'3',ry:'4',cx:'10',cy:'11',fill:'none','stroke-width':'2'}},
  {tag:'path',attrs:{d:'M 0 0 l 2 3 a 4 5 0 0,1 2 3 L 9 9',
    style:{'stroke-width':'3'},fill:'none','stroke-linecap':'round','stroke-linejoin':'mitre',transform:'rotate(20,0,0)'}},
]);
const primitiveParsed = parseSvg(primitiveSvg);
assert.deepEqual(primitiveParsed, primitiveExpected,
  'native line, clipped rounded rect, ellipse, relative path/style width, and mitre normalize to Source geometry');
assert.deepEqual(primitiveParsed[1].segments[0], ['M','1/250','0/1'],
  'rounded rectangle radius is clamped to half of its shortest side');
assert.notDeepEqual(parseSvg(primitiveSvg.replace('width="10"','width="11"')), primitiveParsed,
  'native primitive geometry changes remain observable');
assert.notDeepEqual(parseSvg(primitiveSvg.replace('fill="none"','fill="red"')), primitiveParsed,
  'explicit primitive paint changes remain observable');
assert.notDeepEqual(parseSvg(primitiveSvg.replace('rotate(20,0,0)','rotate(21,0,0)')), primitiveParsed,
  'primitive path transform changes remain observable');
assert.throws(() => parseSvg(primitiveSvg.replace('<line ', '<line data-extra="x" ')), /unsupported attribute/,
  'unknown primitive attributes are rejected');
assert.throws(() => parseSvg(primitiveSvg.replace('stroke-width: 3;', 'stroke-width: 3; fill: red;')), /only supports/,
  'unmodeled style declarations are rejected');
assert.throws(() => parseSvg(primitiveSvg.replace('x1="1"', 'x1="1e999"')), /finite SVG number range/,
  'primitive coordinates must fit the finite SVG number range');
assert.throws(() => parseSvg(primitiveSvg.replace('width="10"', 'width="-10"')), /rect width must be nonnegative/,
  'negative rectangle width is rejected');
assert.throws(() => parseSvg(primitiveSvg.replace('rx="8"', 'rx="-1"')), /rect rx must be nonnegative/,
  'negative rectangle radius is rejected');
assert.throws(() => parseSvg(primitiveSvg.replace('rx="3"', 'rx="-3"')), /ellipse rx must be nonnegative/,
  'negative ellipse radius is rejected');
const baseline = expected.rows.map(row => structuredClone(row));
const baselineResult = run('equivalent-baseline', baseline, true);
const positional = spawnSync(process.execPath,
  [comparator, path.join(testDir, 'equivalent-baseline.tsv')], { cwd: root, encoding: 'utf8' });
assert.equal(positional.status, 0, positional.stderr);
positionalCliResult = 'accepted positional baseline';
const baselineText = dump(baseline);
const coordinateResult = runText('mutated-coordinate', baselineText.replace(/d="M (-?\d+(?:\.\d+)?)/,
  (_m, x) => `d="M ${decimalText(key(add(decimal(x), decimal('0.001'))))}`), false);
const tinyCoordinateResult = runText('tiny-nonzero-coordinate-difference',
  baselineText.replace(/d="M [^ ]+/, 'd="M 0.000000000000001'), false);
const transformResult = runText('mutated-transform', baselineText.replace(/rotate\(([^,]+),/, 'rotate(99,'), false);
const paintResult = runText('mutated-paint', baselineText.replace('stroke-linecap="round"', 'stroke-linecap="butt"'), false);
const firstPath = baselineText.indexOf('<path ');
const firstPathEnd = baselineText.indexOf(' />', firstPath) + 3;
const extraFlipText = baselineText.slice(0, firstPath) + '<g data-mml-node="mrow" transform="scale(1,-1)">' +
  baselineText.slice(firstPath, firstPathEnd) + '</g>' + baselineText.slice(firstPathEnd);
const extraFlipResult = runText('extra-nested-root-flip', extraFlipText, false);
const duplicateRootGroupText = baselineText.replace('</g></svg>',
  '</g><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"></g></svg>');
const duplicateRootGroupResult = runText('duplicate-root-coordinate-group', duplicateRootGroupText, false);
const inheritedPaintText = baselineText.slice(0, firstPath) + '<g fill="none" stroke-linecap="butt">' +
  baselineText.slice(firstPath, firstPathEnd) + '</g>' + baselineText.slice(firstPathEnd);
const inheritedPaintResult = runText('mutated-inherited-paint', inheritedPaintText, false);
const unknownAttributeResult = runText('unknown-path-attribute',
  baselineText.replace('<path ', '<path data-unmodeled="changed" '), false);
const unknownShapeResult = runText('unmodeled-svg-shape',
  baselineText.replace('</svg>', '<rect x="0" y="0" width="1" height="1" /></svg>'), false);
const ignoredCharacterResult = runText('unparsed-path-character',
  baselineText.replace(' d="', ' d="@'), false);
const invalidArcFlagText = baselineText.replace(/(A [^ ]+ [^ ]+ [^ ]+ [01] )1(?= )/,
  (_match, prefix) => `${prefix}2`);
assert.notEqual(invalidArcFlagText, baselineText, 'the baseline needs an arc segment');
const invalidArcFlagResult = runText('invalid-arc-flag', invalidArcFlagText, false);
const wrongArityResult = runText('wrong-command-arity', baselineText.replace('L 3000 1483.5', 'L 3000'), false);
const nonFiniteResult = runText('non-finite-coordinate',
  baselineText.replace(/d="M [^ ]+ /, 'd="M 1e999 '), false);
process.stdout.write(JSON.stringify({baselineResult, positionalCliResult, exactRelativeAbsolute: 'equivalent', coordinateResult,
  tinyCoordinateResult, transformResult, paintResult,
  extraFlipRejected: extraFlipResult, duplicateRootRejected: duplicateRootGroupResult,
  inheritedPaintRejected: inheritedPaintResult,
  unknownAttributeRejected: unknownAttributeResult,
  unknownShapeRejected: unknownShapeResult, invalidPathCharacterRejected: ignoredCharacterResult,
  invalidArcFlagRejected: invalidArcFlagResult, wrongArityRejected: wrongArityResult,
  nonFiniteRejected: nonFiniteResult}, null, 2) + '\n');
