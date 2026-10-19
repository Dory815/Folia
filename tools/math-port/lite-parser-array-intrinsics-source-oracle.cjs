'use strict';

// Pinned Node observations for the Array intrinsics used by LiteParser.
// Native output is never an input to this script.
const observe = (name, body) => {
  const trace = [];
  let value;
  let error;
  try { value = body(trace); }
  catch (e) { error = { name: e && e.name, message: e && e.message }; }
  return { name, value, error, trace };
};

const slots = (array) => Array.from({ length: array.length }, (_, i) =>
  Object.prototype.hasOwnProperty.call(array, i) ? array[i] : '<hole>');

const rows = [
  observe('splice-zero-arguments-is-noop', () => {
    const a = ['a', 'b'];
    const removed = a.splice();
    return { removed: slots(removed), source: slots(a) };
  }),
  observe('splice-one-argument-removes-tail', () => {
    const a = ['a', 'b', 'c'];
    const removed = a.splice(1);
    return { removed: slots(removed), source: slots(a) };
  }),
  observe('splice-explicit-undefined-deletes-none', () => {
    const a = ['a', 'b'];
    const removed = a.splice(1, undefined);
    return { removed: slots(removed), source: slots(a) };
  }),
  observe('splice-preserves-hole-in-removed-array', () => {
    const a = ['a', , 'c', 'd'];
    const removed = a.splice(0, 3);
    return { removed: slots(removed), source: slots(a) };
  }),
  observe('shift-uses-inherited-get-and-live-has', (trace) => {
    const proto = Object.create(Array.prototype);
    Object.defineProperty(proto, '1', { configurable: true, get() {
      trace.push('get:1');
      this[2] = 'changed';
      return 'inherited';
    } });
    const a = ['first', , 'old'];
    Object.setPrototypeOf(a, proto);
    const first = a.shift();
    return { first, source: slots(a), own0: Object.hasOwn(a, 0), own1: Object.hasOwn(a, 1) };
  }),
  observe('splice-observes-inherited-source-slot', (trace) => {
    const proto = Object.create(Array.prototype);
    Object.defineProperty(proto, '1', { configurable: true, get() {
      trace.push('get:1');
      return 'inherited';
    } });
    const a = ['a', , 'c', 'd'];
    Object.setPrototypeOf(a, proto);
    const removed = a.splice(0, 3, 'x');
    return { removed: slots(removed), removedOwn: [0, 1, 2].map(i => Object.hasOwn(removed, i)), source: slots(a) };
  }),
];

console.log(JSON.stringify(rows, null, 2));
