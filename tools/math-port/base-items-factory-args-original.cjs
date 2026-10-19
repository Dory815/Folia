'use strict';
// Additive Source witness for exact StackItemFactory arguments. This does not
// alter the frozen BaseItems corpus; it isolates the FactoryArgs leaf boundary.
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
const load = name => require(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs', name));
const { TeX } = load('input/tex.js');
const { MmlFactory } = load('core/MmlTree/MmlFactory.js');
load('input/tex/base/BaseConfiguration.js');

function value(value) {
  if (value === undefined) return { type: 'undefined' };
  if (value === null) return { type: 'null' };
  if (typeof value === 'object') return { type: 'object', marker: value.marker };
  return { type: typeof value, value };
}

const jax = new TeX({ packages: ['base'], tags: 'base' });
jax.setMmlFactory(new MmlFactory());
const factory = jax.parseOptions.itemFactory;
const object = { marker: 'source-object' };
const rows = [
  ['omitted', []], ['undefined', [undefined]], ['null', [null]],
  ['boolean', [false]], ['number', [7]], ['string', ['source']],
  ['unicode', ['λ𝄞']], ['object', [object]],
];
const cases = rows.map(([name, args]) => {
  const item = factory.create('start', ...args);
  return { name, kind: item.kind, global: value(item.global),
    objectIdentity: name === 'object' && item.global === object };
});
process.stdout.write(JSON.stringify({ cases }, null, 2) + '\n');
