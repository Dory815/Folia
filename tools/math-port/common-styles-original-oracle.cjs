'use strict';

// Root-owned observations of the real pinned classes. No native candidate
// participates in producing these records; this is development evidence only.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const manifest = JSON.parse(fs.readFileSync(path.join(packageRoot, 'package.json'), 'utf8'));
if (manifest.version !== '4.1.3') throw new Error('Expected @mathjax/src 4.1.3');
const {Styles, TRBL, WSC} = require(path.join(packageRoot, 'cjs/util/Styles.js'));
const {StyleJsonSheet} = require(path.join(packageRoot, 'cjs/util/StyleJson.js'));

function plain(value) {
  if (value === undefined) return {$: 'undefined'};
  if (typeof value === 'number' && !Number.isFinite(value))
    return {$: Number.isNaN(value) ? 'NaN' : value < 0 ? '-Infinity' : 'Infinity'};
  if (typeof value === 'number' && Object.is(value, -0)) return {$: '-0'};
  if (Array.isArray(value)) return Array.from({length: value.length}, (_, i) =>
    Object.hasOwn(value, i) ? plain(value[i]) : {$: 'hole'});
  if (value && typeof value === 'object') return Object.fromEntries(
    Object.keys(value).map(key => [key, plain(value[key])]));
  return value;
}
function result(action) {
  try { return {value: plain(action())}; }
  catch (error) { return {error: {name: error.name, message: error.message}}; }
}
function snapshot(style) {
  return {cssText: style.cssText, styleList: plain(style.styleList),
    keys: Object.keys(style.styleList), missing: style.get('missing'),
    padding: TRBL.map(side => style.get('padding-' + side)),
    border: TRBL.map(side => WSC.map(part => style.get('border-' + side + '-' + part)))};
}
const constructors = [
  ['', 'empty'], ['color: red; background-color: blue;', 'ordinary'],
  ['padding: 1em;', 'padding-one'], ['padding: 1em 2em;', 'padding-two'],
  ['padding: 1em 2em 3em;', 'padding-three'], ['padding: 1em 2em 3em 4em;', 'padding-four'],
  ['margin: 1px 2px 3px 4px;', 'margin-four'],
  ['border: 1px solid red;', 'border'], ['border: solid red;', 'border-no-width'],
  ['border-top: thin dashed rgb(1, 2, 3);', 'border-top'],
  ['border-width: 1px 2px 3px 4px; border-style: solid; border-color: red blue;', 'border-parts'],
  ['font: italic bold 12px/1.5 "Times New Roman", serif;', 'font'],
  ['font: normal small-caps 700 semi-condensed 1.2em/120% serif;', 'font-variants'],
  ['font-family: "A; B"; color: green;', 'quoted-semicolon'],
  ["font-family: 'A\\\'B'; color: red;", 'escaped-quote'],
  ['font-family: "unclosed; color: red;', 'unclosed-quote'],
  ['color: red; /* removed */ padding: 1em;', 'comment'],
  ['color: red; /* multiline\ncomment */ color: blue;', 'multiline-comment'],
  ['color: red; @invalid; color: blue;', 'invalid-prefix'],
  ['color: red; Color: blue; padding: 2em;', 'uppercase-parser-name'],
  ['font-family: "数学😀"; color: red;', 'unicode-value'],
  ['padding: 1em; padding-left: 4em; padding-top: 2em;', 'sequential-child-overrides'],
  ['border: 2px solid blue; border-left-width: 1px;', 'border-child-overrides'],
  ['color: red; color: blue; color:;', 'delete-value'],
];
const records = constructors.map(([css, name]) => ({name, input: css,
  actual: result(() => snapshot(new Styles(css)))}));
const mutations = [
  ['set-parent-and-child', [['padding', '1em 2em'], ['paddingLeft', '3em'], ['paddingTop', '4em']]],
  ['combine-border', [['borderTop', '1px solid red'], ['borderRight', '1px solid red'],
    ['borderBottom', '1px solid red'], ['borderLeft', '1px solid red']]],
  ['delete-border-child', [['border', '1px solid red'], ['borderLeftColor', '']]],
  ['set-font-parts', [['fontSize', '12px'], ['fontWeight', 'bold'], ['fontFamily', 'serif']]],
  ['sanitize-semicolon', [['color', 'red; background: blue']]],
  ['sanitize-unclosed-quote', [['fontFamily', '"unclosed']]],
  ['primitive-values', [['opacity', 0.5], ['zIndex', -0], ['flag', true], ['color', false]]],
  ['ordered-delete-reinsert', [['color', 'red'], ['padding', '1em'], ['color', ''], ['color', 'blue']]],
];
for (const [name, operations] of mutations) records.push({name, operations,
  actual: result(() => {
    const style = new Styles();
    return operations.map(([key, value]) => { style.set(key, value); return snapshot(style); });
  })});
records.push({name: 'style-list-is-copy', actual: result(() => {
  const style = new Styles('color: red;');
  const first = style.styleList, second = style.styleList;
  first.color = 'blue';
  return {distinct: first !== second, untouched: style.get('color'), actual: snapshot(style)};
})});
records.push({name: 'protected-name-and-sanitizing-methods', actual: result(() => {
  const style = new Styles();
  return {
    normalize: ['fontSize', 'MozTransform', 'borderTopWidth', 'COLOR', '--X', 'x😀Y']
      .map(name => style.normalizeName(name)),
    parent: ['', 'color', 'border-top-width', '--x'].map(name => style.parentName(name)),
    child: [['border-width', 'top'], ['font', 'line-height'], ['border-top', 'color']]
      .map(([parent, child]) => style.childName(parent, child)),
    sanitize: ['red', 'red; blue', '"unclosed', "'a; b'", '"a\\"b"', 'a\nb']
      .map(value => style.sanitizeValue(value)),
  };
})});
records.push({name: 'subclass-pattern-lookup-and-base-connection-lookup', actual: result(() => {
  class DerivedStyles extends Styles {}
  DerivedStyles.pattern = {...Styles.pattern,
    style: /([a-z]+)\s*=\s*([^;]*);?/g, sanitize: /$^/};
  DerivedStyles.connect = {...Styles.connect, padding: {children: ['fake'],
    split() { throw new Error('Subclass connection must not be selected'); },
    combine() { throw new Error('Subclass connection must not be selected'); }}};
  const style = new DerivedStyles('padding=1em 2em;color=red;');
  style.set('color', 'red; blue');
  return snapshot(style);
})});
records.push({name: 'style-sheet-lifecycle-and-nested-alias', actual: result(() => {
  const nested = {color: 'red'};
  const input = {'.a': {color: 'blue', opacity: 0.5, '@media test': {'.child': nested}}};
  const sheet = new StyleJsonSheet(input);
  const trace = [{cssText: sheet.cssText, rules: sheet.getStyleRules()}];
  input['.a'].color = 'green'; nested.color = 'purple';
  trace.push({cssText: sheet.cssText, shallowPropertyCopy: sheet.styles['.a'].color === 'blue',
    nestedAlias: sheet.styles['.a']['@media test'] === input['.a']['@media test']});
  sheet.addStyles({'.a': {padding: '1em', color: 'black'}, '.b': {display: 'inline'}});
  trace.push({cssText: sheet.cssText, indented: sheet.getStyleRules(sheet.styles, '  ')});
  sheet.removeStyles('.missing', '.a'); trace.push({cssText: sheet.cssText});
  sheet.clear(); trace.push({cssText: sheet.cssText, rules: sheet.getStyleRules()});
  sheet.addStyles(null); trace.push({cssText: sheet.cssText});
  return trace;
})});
records.push({name: 'static-patterns-and-connections', actual: result(() => ({
  TRBL, WSC, patterns: Object.fromEntries(Object.entries(Styles.pattern)
    .map(([name, pattern]) => [name, {source: pattern.source, flags: pattern.flags}])),
  connections: Object.fromEntries(Object.entries(Styles.connect).map(([name, connection]) =>
    [name, {children: connection.children, parts: plain(connection.parts),
      subPart: plain(connection.subPart), split: connection.split.name, combine: connection.combine.name}])),
}))});
const files = ['ts/util/Styles.ts', 'ts/util/StyleJson.ts'];
const hashes = Object.fromEntries(files.map(file => [file,
  crypto.createHash('sha256').update(fs.readFileSync(path.join(sourceRoot, file))).digest('hex')]));
const cjsFiles = ['cjs/util/Styles.js', 'cjs/util/StyleJson.js'];
const cjsHashes = Object.fromEntries(cjsFiles.map(file => [file,
  crypto.createHash('sha256').update(fs.readFileSync(path.join(packageRoot, file))).digest('hex')]));
const output = {version: manifest.version, hashes, cjsHashes, records};
if (process.argv[2]) fs.writeFileSync(path.resolve(process.argv[2]), JSON.stringify(output, null, 2) + '\n');
else process.stdout.write(JSON.stringify(output, null, 2) + '\n');
