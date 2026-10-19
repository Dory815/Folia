const assert = require('node:assert/strict');
const {createReference} = require('../../reference-profile.cjs');

const {adaptor, provenance} = createReference();
assert.equal(provenance.upstream.version, '4.1.3');
assert.equal(provenance.upstream.commit, 'fb987178c2d279a99b0db5ea02e751691703955e');

const parent = adaptor.node('div');
const child = adaptor.node('i');
const base = ['kind', 'attributes', 'children', 'styles'];
assert.deepEqual(Object.keys(child), base);

adaptor.append(parent, child);
assert.deepEqual(Object.keys(child), [...base, 'parent']);
assert.equal(child.parent, parent);

delete child.parent;
assert.deepEqual(Object.keys(child), base);
adaptor.append(parent, child);
assert.deepEqual(Object.keys(child), [...base, 'parent']);
assert.equal(child.parent, parent);

console.log(JSON.stringify({
  source: provenance.upstream,
  observable: 'LiteElement own enumerable keys preserve insertion order across parent assignment, delete, and re-add',
  initial: base,
  appended: [...base, 'parent'],
}));

