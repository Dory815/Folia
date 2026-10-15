// Pinned MathJax 4.1.3 CJS witness. Run from the repository root with:
// node tools/math-port/drafts/parser-configuration-function-lists-source.cjs
const { ParserConfiguration } = require(
  '../../mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/Configuration.js',
);
const fs = require('node:fs');
const crypto = require('node:crypto');
const path = require('node:path');

const packageRoot = path.resolve('tools/mathjax-bundle/node_modules/@mathjax/src');
const pkg = require(path.join(packageRoot, 'package.json'));
if (pkg.version !== '4.1.3') throw new Error(`Expected @mathjax/src 4.1.3, got ${pkg.version}`);
const pinned = {
  'cjs/input/tex/Configuration.js': '86f62a6f6e44fec20156546b7e8d424cac493c7465c2c8ec621c604332a276f3',
  'cjs/util/FunctionList.js': '571f57801b863b2d9f01a66dc72f2836b7fd06a1a48a3fe0057e4ec3d51a7278',
  'cjs/util/PrioritizedList.js': '787bd21f3634312af666917d8f140145864f98d09ede1bdde3c957d28ec2bc25',
};
for (const [relative, expected] of Object.entries(pinned)) {
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(packageRoot, relative))).digest('hex');
  if (actual !== expected) throw new Error(`${relative} hash mismatch: ${actual}`);
}

const parser = new ParserConfiguration([]);
const list = parser.configMethod;
const trace = [];
const callback = function () { trace.push(this.priority); };
list.add(callback, 3);
list.add(callback, 1);
const iterator = list[Symbol.iterator]();
const first = iterator.next().value;
const report = {
  sourceVersion: pkg.version,
  sourceHashes: pinned,
  ownKeys: Object.keys(list),
  constructorLength: list.constructor.length,
  addLength: list.add.length,
  itemsIsArray: Array.isArray(list.items),
  itemsDescriptor: Object.getOwnPropertyDescriptor(list, 'items'),
  orderedPriorities: list.items.map(({ priority }) => priority),
  iteratorRecordKeys: Object.keys(first),
  iteratorItemIdentity: first.item === callback,
  iteratorFirstPriority: first.priority,
  executeResult: list.execute(),
  callbackPriorities: trace,
};
process.stdout.write(`${JSON.stringify(report, null, 2)}\n`);
