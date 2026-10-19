const assert = require('node:assert/strict');
const {createReference} = require('../../reference-profile.cjs');

const {adaptor, provenance} = createReference();
assert.equal(provenance.upstream.version, '4.1.3');
assert.equal(provenance.upstream.commit, 'fb987178c2d279a99b0db5ea02e751691703955e');

const node = adaptor.node('span');
const base = ['kind', 'attributes', 'children', 'styles'];
adaptor.setProperty(node, 'foo', 'native-initial');
assert.equal(adaptor.getProperty(node, 'foo'), 'native-initial');
assert.deepEqual(Object.keys(node), [...base, 'foo']);

node.foo = 'context-write';
assert.equal(adaptor.getProperty(node, 'foo'), 'context-write');
node.bar = 'second';
delete node.foo;
assert.equal(adaptor.getProperty(node, 'foo'), undefined);
adaptor.setProperty(node, 'foo', 'native-readd');
assert.equal(node.foo, 'native-readd');
assert.deepEqual(Object.keys(node), [...base, 'bar', 'foo']);

const parent = adaptor.node('div');
const child = adaptor.node('i');
adaptor.append(parent, child);
assert.deepEqual(Object.keys(child), [...base, 'parent']);
const clonedParent = adaptor.clone(parent);
assert.deepEqual(Object.keys(clonedParent.children[0]), [...base, 'parent']);

console.log(JSON.stringify({
  source: provenance.upstream,
  observable: 'LiteAdaptor generic properties share ordinary own properties, current values, deletion, and insertion order',
  initial: [...base, 'foo'],
  afterDeleteAndReadd: [...base, 'bar', 'foo'],
  appendedChild: [...base, 'parent'],
  clonedChild: [...base, 'parent'],
}));

