// Pinned MathJax 4.1.3 witness for ParserConfiguration.configurations' actual
// PrioritizedList owner and Source-visible field/record behavior.
// Run from the repository root with:
// node tools/math-port/parser-configuration-prioritized-list-source.cjs
const fs = require('node:fs');
const crypto = require('node:crypto');
const path = require('node:path');

const packageRoot = path.resolve('tools/mathjax-bundle/node_modules/@mathjax/src');
const pkg = require(path.join(packageRoot, 'package.json'));
if (pkg.version !== '4.1.3') throw new Error(`Expected @mathjax/src 4.1.3, got ${pkg.version}`);
const pinned = {
  'cjs/input/tex/Configuration.js': '86f62a6f6e44fec20156546b7e8d424cac493c7465c2c8ec621c604332a276f3',
  'cjs/util/PrioritizedList.js': '787bd21f3634312af666917d8f140145864f98d09ede1bdde3c957d28ec2bc25',
};
for (const [relative, expected] of Object.entries(pinned)) {
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(packageRoot, relative))).digest('hex');
  if (actual !== expected) throw new Error(`${relative} hash mismatch: ${actual}`);
}

const { ParserConfiguration, Configuration } = require(path.join(packageRoot, 'cjs/input/tex/Configuration.js'));
const { PrioritizedList } = require(path.join(packageRoot, 'cjs/util/PrioritizedList.js'));
const parser = new ParserConfiguration([]);
const list = parser.configurations;
const originalItems = list.items;
const low = Configuration.local({ priority: 1 });
const high = Configuration.local({ priority: 7 });
list.add(low, 1);
list.add(high, 7);
const first = list[Symbol.iterator]().next().value;
const rawPriorities = new PrioritizedList();
rawPriorities.add(low, 'named-priority');
rawPriorities.add(high, true);
const lockedLength = new PrioritizedList();
lockedLength.add(low, 1);
Object.defineProperty(lockedLength.items, 'length', { writable: false });
let lockedAddError = null;
try { lockedLength.add(high, 2); }
catch (error) { lockedAddError = { name: error.name, message: error.message }; }
const listDescriptor = Object.getOwnPropertyDescriptor(list, 'items');
if (JSON.stringify(Object.keys(list)) !== JSON.stringify(['items'])) throw new Error('PrioritizedList own-key order changed');
if (Object.getPrototypeOf(list) !== PrioritizedList.prototype) throw new Error('PrioritizedList prototype identity changed');
if (!listDescriptor.writable || !listDescriptor.enumerable || !listDescriptor.configurable)
  throw new Error('PrioritizedList.items descriptor changed');
if (first.item !== low || first.priority !== 1 || list.items[0] !== first)
  throw new Error('PrioritizedList record/iterator identity changed');
if (Object.keys(first).join(',') !== 'item,priority') throw new Error('PrioritizedList record key order changed');

const report = {
  sourceVersion: pkg.version,
  sourceHashes: pinned,
  parserOwnKeys: Object.keys(parser),
  listOwnKeys: Object.keys(list),
  listPrototypeIsPrioritizedListPrototype: Object.getPrototypeOf(list) === PrioritizedList.prototype,
  itemsDescriptor: listDescriptor,
  itemsIsOriginalArray: list.items === originalItems,
  firstRecordKeys: Object.keys(first),
  firstRecordItemIsActualConfiguration: first.item === low,
  firstRecordItemName: first.item.name,
  orderedRecords: list.items.map(({ item, priority }) => ({
    itemIsHigh: item === high,
    itemIsLow: item === low,
    priority,
  })),
  iteratorReturnsRecordIdentity: list[Symbol.iterator]().next().value === list.items[0],
  rawPriorityValues: rawPriorities.items.map(({ priority }) => ({ value: priority, type: typeof priority })),
  rawPrioritySourceValuesPreserved:
    rawPriorities.items[0].priority === 'named-priority' && rawPriorities.items[1].priority === true,
  nativeAddAfterLengthLockedError: lockedAddError,
};
if (!report.rawPrioritySourceValuesPreserved) throw new Error('PrioritizedList did not preserve raw priority values');
if (!lockedAddError || lockedAddError.name !== 'TypeError') throw new Error('PrioritizedList.add did not honor a non-writable array length');
process.stdout.write(`${JSON.stringify(report, null, 2)}\n`);
