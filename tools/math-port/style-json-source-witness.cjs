'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

const root = path.resolve(__dirname, '../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const manifest = require(path.join(packageRoot, 'package.json'));
assert.equal(manifest.version, '4.1.3');
const sourcePath = path.join(packageRoot, 'cjs/util/StyleJson.js');
const sourceHash = crypto.createHash('sha256').update(fs.readFileSync(sourcePath)).digest('hex');
assert.equal(sourceHash, 'dfd45381fc15fa9b8a038e8ac782550934b5327871d10534df52af02aa43f11f');
const {StyleJsonSheet} = require(sourcePath);
let passed = 0;
function check(name, actual, expected) {
  assert.deepStrictEqual(actual, expected, name);
  ++passed;
}

const nested = new StyleJsonSheet({
  '.x': {color: 'red', '@media test': {
    '.child': {'font-size': 12, '@font-face': {font: 'MathJax'}}
  }}
});
check('nested media/font-face CSS and indentation', nested.cssText,
  '.x {\n  color: red;\n  @media test {\n      .child {\n      font-size: 12;\n              @font-face {\n        font: MathJax;\n      }\n    }\n  }\n}');
check('nested groups preserve rule/property order', nested.getStyleRules(), [nested.cssText]);

const sheet = new StyleJsonSheet({'.a': {color: 'red', opacity: 0.5}, '.b': {display: 'block'}});
sheet.addStyles({'.a': {color: 'blue', padding: '1em'}, '.c': {display: 'inline'}});
check('replacement keeps old key order and appends new keys/selectors', sheet.cssText,
  '.a {\n  color: blue;\n  opacity: 0.5;\n  padding: 1em;\n}\n\n.b {\n  display: block;\n}\n\n.c {\n  display: inline;\n}');
sheet.removeStyles('.a', '.missing');
sheet.addStyles({'.a': {color: 'green'}});
check('remove then reinsert appends selector', sheet.getStyleRules(), [
  '.b {\n  display: block;\n}',
  '.c {\n  display: inline;\n}',
  '.a {\n  color: green;\n}'
]);

const shallow = new StyleJsonSheet({'.a': {color: 'red', '@media': {
  '.child': {color: 'old', padding: '2em'}
}}});
shallow.addStyles({'.a': {'@media': {'.child': {color: 'new'}}}});
check('nested group replacement is shallow and keeps sibling declarations', shallow.cssText,
  '.a {\n  color: red;\n  @media {\n      .child {\n      color: new;\n    }\n  }\n}');
check('explicit rules honor requested indentation', shallow.getStyleRules({
  '.custom': {padding: '1em'}
}, '  '), ['  .custom {\n    padding: 1em;\n  }']);
shallow.clear();
check('clear removes all rules', [shallow.cssText, shallow.getStyleRules()], ['', []]);

check('empty root and nested groups', new StyleJsonSheet({empty: {}, '@media empty': {}}).cssText,
  'empty {\n\n}\n\n@media empty {\n\n}');
check('number stringification follows JavaScript Number', new StyleJsonSheet({n: {
  fraction: 0.5, negativeZero: -0, exponent: 1e21, smallExponent: 1e-7,
  notANumber: NaN, infinity: Infinity
}}).cssText,
  'n {\n  fraction: 0.5;\n  negativeZero: 0;\n  exponent: 1e+21;\n  smallExponent: 1e-7;\n  notANumber: NaN;\n  infinity: Infinity;\n}');
check('UTF-16 names and values survive CSS serialization', new StyleJsonSheet({
  '😀 selector': {'字体': '数学😀'}
}).cssText, '😀 selector {\n  字体: 数学😀;\n}');
check('isolated UTF-16 surrogates remain source code units',
  new StyleJsonSheet({'\uD800': {'\uDC00': '\uD801'}}).cssText,
  '\uD800 {\n  \uDC00: \uD801;\n}');

process.stdout.write(JSON.stringify({packageVersion: manifest.version,
  sourcePath: path.relative(root, sourcePath).replaceAll('\\', '/'), sourceSha256: sourceHash,
  passed}, null, 2) + '\n');
