'use strict';
const assert = require('node:assert/strict');
const path = require('node:path');
const fs = require('node:fs');
const crypto = require('node:crypto');
const repo = path.resolve(__dirname, '../..');
const sourcePath = path.join(repo, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/util/Styles.js');
const sourcePackage = require(path.join(repo, 'tools/mathjax-bundle/node_modules/@mathjax/src/package.json'));
assert.equal(sourcePackage.version, '4.1.3');
const { Styles } = require(sourcePath);
const result = { sourcePath: path.relative(repo, sourcePath).replaceAll('\\', '/'),
  version: sourcePackage.version,
  sha256: crypto.createHash('sha256').update(fs.readFileSync(sourcePath)).digest('hex'), cases: [] };
function check(name, actual, expected) {
  assert.deepStrictEqual(actual, expected, name);
  result.cases.push({ name, actual });
}
const parses = [
  ['', 'color', '', ''],
  ['font-size: 150%', 'font-size', '150%', 'font-size: 150%;'],
  ['abc-def: 0', 'abc-def', '0', 'abc-def: 0;'],
  ['a: 0; b: 1px', 'b', '1px', 'a: 0; b: 1px;'],
  ['a: 0; /*b: 1px*/ c: bold', 'c', 'bold', 'a: 0; c: bold;'],
  ['abc: ; xyz: 0', 'xyz', '0', 'xyz: 0;'],
  ['abc xyz: 1px', 'abc', '', ''],
  ["abc: xy 'pqr", 'abc', "xy 'pqr'", "abc: xy 'pqr';"],
  ["abc: ';", 'abc', "';'", "abc: ';';"],
  ['background: red; background-clip: none', 'background-clip', 'none', 'background: red; background-clip: none;'],
];
for (const [css, key, expectedValue, expectedText] of parses) {
  const style = new Styles(css);
  check(`parse ${JSON.stringify(css)}`, { value: style.get(key), cssText: style.cssText },
    { value: expectedValue, cssText: expectedText });
}
const margin = new Styles('margin: 3px; margin-right: 1px');
check('margin shorthand', { margin: margin.get('margin'), bottom: margin.get('margin-bottom'), cssText: margin.cssText },
  { margin: '3px 1px 3px 3px', bottom: '3px', cssText: 'margin: 3px 1px 3px 3px;' });
const border = new Styles('border: 3px solid red; border-top-color: blue');
check('border shorthand/recombine', {
  top: border.get('border-top'), right: border.get('border-right'), width: border.get('border-top-width'),
  style: border.get('border-top-style'), color: border.get('border-top-color'), cssText: border.cssText,
  keys: Object.keys(border.styleList),
}, {
  top: '3px solid blue', right: '3px solid red', width: '3px', style: 'solid', color: 'blue',
  cssText: 'border-top: 3px solid blue; border-right: 3px solid red; border-bottom: 3px solid red; border-left: 3px solid red;',
  keys: ['border-top', 'border-top-width', 'border-top-style', 'border-top-color', 'border-right', 'border-right-width', 'border-right-style', 'border-right-color', 'border-bottom', 'border-bottom-width', 'border-bottom-style', 'border-bottom-color', 'border-left', 'border-left-width', 'border-left-style', 'border-left-color'],
});
const font = new Styles('font: arial bold italic 120%');
check('font shorthand', { family: font.get('font-family'), weight: font.get('font-weight'), style: font.get('font-style'), size: font.get('font-size'), cssText: font.cssText },
  { family: 'arial', weight: 'bold', style: 'italic', size: '120%', cssText: 'font-style: italic; font-weight: bold; font-size: 120%; font-family: arial;' });
const lineHeight = new Styles('font: arial small/12px');
check('font slash line-height', { family: lineHeight.get('font-family'), size: lineHeight.get('font-size'), lineHeight: lineHeight.get('line-height'), cssText: lineHeight.cssText },
  { family: 'arial', size: 'small', lineHeight: '12px', cssText: 'line-height: 12px; font-size: small; font-family: arial;' });
const padding = new Styles('padding: 0; padding-left: 1px');
const before = { padding: padding.get('padding'), top: padding.get('padding-top'), left: padding.get('padding-left') };
padding.set('padding-left', '3px');
const after = padding.get('padding');
padding.set('padding', '');
check('get/set/empty deletion', { before, after, cleared: padding.cssText, keys: Object.keys(padding.styleList) },
  { before: { padding: '0 0 0 1px', top: '0', left: '1px' }, after: '0 0 0 3px', cleared: '', keys: [] });
const order = new Styles('a: 1; b: 2; c: 3');
const originalOrder = Object.keys(order.styleList);
order.set('b', '');
order.set('b', '4');
check('delete and reinsert moves key to the end', { originalOrder, finalOrder: Object.keys(order.styleList) },
  { originalOrder: ['a', 'b', 'c'], finalOrder: ['a', 'c', 'b'] });
const copied = new Styles('color: red; padding: 1em 2em');
const returned = copied.styleList;
returned.color = 'blue';
returned.extra = 'ignored';
check('styleList returns independent copy', { color: copied.get('color'), extra: copied.get('extra'), keys: Object.keys(copied.styleList) },
  { color: 'red', extra: '', keys: ['color', 'padding', 'padding-top', 'padding-right', 'padding-bottom', 'padding-left'] });
const primitives = new Styles();
primitives.set('opacity', 2);
primitives.set('display', false);
check('numeric and boolean StyleValue conversion', { number: primitives.get('opacity'), boolean: primitives.get('display'), cssText: primitives.cssText },
  { number: '2', boolean: 'false', cssText: 'opacity: 2; display: false;' });
const lone = 'x\uD800Y';
const unicode = new Styles();
unicode.set(lone, 'kept');
check('lone-surrogate name normalization and roundtrip', { value: unicode.get(lone), text: unicode.cssText, key: Object.keys(unicode.styleList)[0] },
  { value: 'kept', text: 'x\uD800-y: kept;', key: 'x\uD800-y' });
result.passed = result.cases.length;
console.log(JSON.stringify(result, null, 2));
