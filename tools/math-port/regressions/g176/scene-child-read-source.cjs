'use strict';
const assert = require('node:assert/strict');
const { createReference } = require('../../reference-profile.cjs');
const reference = createReference();
const { adaptor } = reference;

const parent = adaptor.node('div');
const previous = adaptor.node('i');
const node = adaptor.node('b');
const tail = adaptor.node('u');
const slots = [previous, node, , tail];
parent.children = slots;
node.parent = parent;

// Direct LiteAdaptor reads use the original raw array and preserve its holes.
assert.equal(adaptor.firstChild(parent), previous);
assert.equal(adaptor.lastChild(parent), tail);
assert.equal(adaptor.childNode(parent, 2), undefined);
const spread = adaptor.childNodes(parent);
assert.equal(spread.length, 4);
assert.equal(2 in spread, true);
assert.equal(spread[2], undefined);
assert.equal(adaptor.next(node), undefined);
assert.equal(adaptor.previous(node), previous);

// Own undefined and an absent slot both read as undefined, but own presence
// remains observable and the raw array is still the authoritative object.
const ownUndefined = [undefined];
assert.equal(ownUndefined[0], undefined);
assert.equal(0 in ownUndefined, true);
const hole = Array(1);
assert.equal(hole[0], undefined);
assert.equal(0 in hole, false);

// A prototype index is returned by ordinary indexed reads even though it is
// not an own slot. `childNodes` spread uses the same indexed Get semantics.
const inherited = adaptor.node('em');
const inheritedPrototype = Object.assign(Object.create(Array.prototype), { 0: inherited });
const inheritedSlots = [];
Object.setPrototypeOf(inheritedSlots, inheritedPrototype);
inheritedSlots.length = 1;
const inheritedParent = adaptor.node('section');
inheritedParent.children = inheritedSlots;
assert.equal(Object.hasOwn(inheritedSlots, '0'), false);
assert.equal(adaptor.firstChild(inheritedParent), inherited);
assert.equal(adaptor.childNodes(inheritedParent)[0], inherited);

// Last-child evaluation reads `children` twice: once for the base array, then
// again to compute `children.length - 1`. The base array supplies the result.
const readTrace = [];
const base = ['base-0', 'base-1'];
const keySource = ['key-0'];
const changing = {};
Object.defineProperty(changing, 'children', {
  configurable: true,
  get() { readTrace.push('children'); return readTrace.length === 1 ? base : keySource; },
});
assert.equal(adaptor.lastChild(changing), 'base-0');
assert.deepEqual(readTrace, ['children', 'children']);

const singleReadTrace = [];
const singleReadArray = ['first', 'second'];
const singleRead = {};
Object.defineProperty(singleRead, 'children', {
  configurable: true,
  get() { singleReadTrace.push('children'); return singleReadArray; },
});
assert.equal(adaptor.firstChild(singleRead), 'first');
assert.deepEqual(singleReadTrace, ['children']);
singleReadTrace.length = 0;
assert.equal(adaptor.childNode(singleRead, 1), 'second');
assert.deepEqual(singleReadTrace, ['children']);

// `childNodes` gets the owner once, then reads its iterator and consumes that
// iterator. Its values can therefore differ from raw indexed slots.
const iteratorTrace = [];
const iterableSlots = ['raw-slot'];
Object.defineProperty(iterableSlots, Symbol.iterator, {
  configurable: true,
  get() {
    iteratorTrace.push('iterator');
    return function* () { yield 'iterator-value'; };
  },
});
const iterableParent = {};
Object.defineProperty(iterableParent, 'children', {
  configurable: true,
  get() { iteratorTrace.push('children'); return iterableSlots; },
});
const customSpread = adaptor.childNodes(iterableParent);
assert.deepEqual(customSpread, ['iterator-value']);
assert.deepEqual(iteratorTrace, ['children', 'iterator']);

// next()/previous() read node.parent three times (the local parent and both
// reads in childIndex), then read parent.children for findIndex and for the
// final length/slot (next) or slot (previous). A hole is returned as undefined.
const navigationTrace = [];
const navigationSlots = [previous, node, , tail];
const navigationParent = {};
Object.defineProperty(navigationParent, 'children', {
  configurable: true,
  get() { navigationTrace.push('children'); return navigationSlots; },
});
Object.defineProperty(node, 'parent', {
  configurable: true,
  get() { navigationTrace.push('parent'); return navigationParent; },
});
assert.equal(adaptor.next(node), undefined);
const nextTrace = [...navigationTrace];
assert.deepEqual(nextTrace, ['parent', 'parent', 'parent', 'children', 'children', 'children']);
navigationTrace.length = 0;
assert.equal(adaptor.previous(node), previous);
const previousTrace = [...navigationTrace];
assert.deepEqual(previousTrace, ['parent', 'parent', 'parent', 'children', 'children']);

process.stdout.write(JSON.stringify({
  provenance: reference.provenance.upstream,
  rawHole: { first: 'previous', last: 'tail', childNode2: 'undefined',
    childNodesLength: spread.length, spreadHoleIsOwn: 2 in spread,
    next: 'undefined', previous: 'previous' },
  ownUndefined: { own: 0 in ownUndefined, value: 'undefined' },
  absentHole: { own: 0 in hole, value: 'undefined' },
  inheritedIndex: { own: Object.hasOwn(inheritedSlots, '0'),
    firstChildIsInherited: adaptor.firstChild(inheritedParent) === inherited,
    spreadChildIsInherited: adaptor.childNodes(inheritedParent)[0] === inherited },
  lastChildReads: readTrace.length,
  firstChildAndChildNodeReads: ['children', 'children'],
  customIterator: { reads: iteratorTrace, rawIndex: iterableSlots[0], spread: customSpread },
  nextPreviousReadCounts: { next: nextTrace, previous: previousTrace },
}, null, 2) + '\n');
