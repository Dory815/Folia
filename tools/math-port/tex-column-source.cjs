#!/usr/bin/env node
'use strict';

const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

function findRoot(start) {
  for (let dir = start; ; dir = path.dirname(dir)) {
    if (fs.existsSync(path.join(dir, '.reference/MathJax-src/ts/input/tex/ColumnParser.ts'))) return dir;
    const parent = path.dirname(dir);
    if (parent === dir) throw new Error('repository root not found');
  }
}
const root = findRoot(__dirname);
const source = path.join(root, '.reference/MathJax-src/ts');
const cjs = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const lock = JSON.parse(fs.readFileSync(path.join(root, 'tests/math/fixtures/tex-source/column-source-lock.json'), 'utf8'));
const digest = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
assert.equal(JSON.parse(fs.readFileSync(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/package.json'), 'utf8')).version, lock.version);
for (const [rel, hash] of Object.entries(lock.ts)) assert.equal(digest(path.join(source, rel)).toUpperCase(), hash.toUpperCase(), `TS source changed: ${rel}`);
for (const [rel, hash] of Object.entries(lock.cjs)) assert.equal(digest(path.join(cjs, rel)).toUpperCase(), hash.toUpperCase(), `CJS source changed: ${rel}`);

const { TeX } = require(path.join(cjs, 'input/tex.js'));
const BaseMethods = require(path.join(cjs, 'input/tex/base/BaseMethods.js')).default;
const ColumnParser = require(path.join(cjs, 'input/tex/ColumnParser.js')).ColumnParser;
const jax = new TeX({ packages: ['base'] });
const columnParser = jax.parseOptions.columnParser;
assert.ok(columnParser instanceof ColumnParser);

function defineColumn(char, count, replacement) {
  const args = [char, String(count), replacement];
  const parser = {
    configuration: { columnParser },
    itemFactory: { create(kind) { assert.equal(kind, 'null'); return { kind }; } },
    GetArgument() { return args.shift(); },
    GetBrackets(_name, fallback) { return args.shift() ?? fallback; },
    pushed: [],
    Push(item) { this.pushed.push(item); },
  };
  BaseMethods.NewColumnType(parser, 'newcolumntype');
  assert.equal(parser.pushed.length, 1);
  return parser;
}
function arrayRecord() {
  return { cstart: [], cend: [], cextra: [], ralign: [], frame: [], arraydef: {} };
}
function parseTemplate(template, parser = { configuration: { options: { maxBuffer: 10000 } } }) {
  const array = arrayRecord();
  columnParser.process(parser, template, array);
  return { arraydef: array.arraydef, cstart: array.cstart, cend: array.cend,
    cextra: array.cextra, ralign: array.ralign, frame: array.frame };
}
function errorId(fn) {
  try { fn(); } catch (error) { return error.id; }
  throw new Error('expected Source TexError');
}

// Real NewColumnType and ColumnParser methods; only the token-argument and
// ArrayItem-shaped input are bounded stubs.  The exercised result is the
// actual handler -> macroColumn -> getColumn/UnitUtil path.
defineColumn('Q', 2, 'p{#1}{#2}');
const beforeReset = columnParser;
jax.reset();
assert.equal(jax.parseOptions.columnParser, beforeReset);
const expandedAfterReset = parseTemplate('Q{2em}{r}');
assert.deepEqual(expandedAfterReset.arraydef, { columnalign: 'left right', columnwidth: '2em auto' });
assert.deepEqual(expandedAfterReset.ralign, [['top', '2em', 'left']]);

assert.equal(errorId(() => defineColumn('XY', 0, 'l')), 'BadColumnName');
assert.equal(errorId(() => defineColumn('Z', 'x', 'l')), 'PositiveIntegerArg');
defineColumn('R', 0, 'r'); // zero arguments are accepted and parseInt('0') is used
assert.deepEqual(parseTemplate('R').arraydef, { columnalign: 'right' });
defineColumn('S', '01', 'c'); // decimal-looking leading zero is accepted by the regex
assert.deepEqual(parseTemplate('S{unused}').arraydef, { columnalign: 'center' });

defineColumn('T', 1, 'p{#2}');
assert.equal(errorId(() => parseTemplate('T{2em}')), 'IllegalMacroParam');
defineColumn('L', 0, 'L'); // bounded recursive expansion reaches the actual MAXCOLUMNS guard
const oldLimit = columnParser.MAXCOLUMNS;
columnParser.MAXCOLUMNS = 2;
assert.equal(errorId(() => parseTemplate('L')), 'MaxColumns');
columnParser.MAXCOLUMNS = oldLimit;

console.log(JSON.stringify({
  version: lock.version,
  persistedAcrossReset: jax.parseOptions.columnParser === beforeReset,
  expandedAfterReset,
  validationErrors: ['BadColumnName', 'PositiveIntegerArg'],
  zeroAndLeadingZeroArguments: 'accepted; integer count follows parseInt',
  deferredTemplateArgumentError: 'IllegalMacroParam',
  recursiveDefinitionBound: 'MaxColumns',
  limit: 'Source classes/callbacks are actual; parser argument scanner and ArrayItem are bounded typed-shape stubs; no full TeX expression or MML finalization is claimed.',
}, null, 2));
