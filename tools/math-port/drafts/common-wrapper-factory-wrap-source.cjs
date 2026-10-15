'use strict';
// Draft focused witness for the canonical factory.wrap edge used by the
// pinned SVG tooltip path. Uses the actual Common/SVG factory and MML nodes.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');

const root = path.resolve(__dirname, '../../..');
const bundle = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const lock = JSON.parse(fs.readFileSync(
  path.join(root, 'tools/math-port/common-layout-reference/profile-source-lock.json'), 'utf8'));
function requireLocked(file) {
  const entry = lock.dependencies.find(item => item.file === file);
  assert.ok(entry, `missing source lock: ${file}`);
  const filename = path.join(bundle, 'cjs', file.slice('src/cjs/'.length));
  assert.equal(crypto.createHash('sha256').update(fs.readFileSync(filename)).digest('hex'),
    entry.sha256, `pinned source hash: ${file}`);
  return require(filename);
}

const { MmlFactory } = requireLocked('src/cjs/core/MmlTree/MmlFactory.js');
const { SvgWrapperFactory } = requireLocked('src/cjs/output/svg/WrapperFactory.js');
const { SvgWrappers } = requireLocked('src/cjs/output/svg/Wrappers.js');
const { AbstractWrapperFactory } = requireLocked('src/cjs/core/Tree/WrapperFactory.js');

const factory = new SvgWrapperFactory(SvgWrappers);
factory.jax = {
  font: { params: { rule_thickness: 0 }, getVariant: () => null },
  options: {}, math: { outputData: {} }, pxPerEm: 16, nodeMap: new Map()
};
const mml = new MmlFactory();
const node = mml.create('math');
const wrapDescriptor = Object.getOwnPropertyDescriptor(AbstractWrapperFactory.prototype, 'wrap');
assert.ok(wrapDescriptor && typeof wrapDescriptor.value === 'function');
assert.equal(factory.wrap, wrapDescriptor.value);
assert.equal(wrapDescriptor.value.length, 1);
const wrapper = factory.wrap(node);
assert.equal(wrapper.node, node);
assert.equal(wrapper.constructor, SvgWrappers.math);

process.stdout.write(JSON.stringify({
  method: { name: wrapDescriptor.value.name, length: wrapDescriptor.value.length,
    inherited: !Object.hasOwn(factory, 'wrap') },
  result: { constructor: wrapper.constructor.name, sameMmlNode: wrapper.node === node }
}) + '\n');
