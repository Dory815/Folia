// Source witness for the native Context Map host. Uses the engine's actual
// ECMAScript Map and records observable identities/order without a substitute.
const out = [];
const summary = (map) => ({size: map.size, entries: [...map.entries()].map(([k, v]) => [
  typeof k === 'number' && Number.isNaN(k) ? 'NaN' :
    Object.is(k, -0) ? '-0' : k,
  v
])});

{
  const map = new Map();
  const objectKey = {};
  map.set(NaN, 'first');
  map.set(Number.NaN, 'updated');
  map.set(-0, 'zero');
  map.set(+0, 'positive-zero-updated');
  map.set(objectKey, 'object');
  out.push({case: 'same-value-zero-and-order', ...summary(map),
    nan: map.get(NaN), zero: map.get(0), objectIdentity: map.get(objectKey),
    distinctObject: map.has({})});
}

{
  const map = new Map([['a', 1], ['b', 2]]);
  const iterator = map.entries();
  const first = iterator.next().value;
  map.delete('b');
  map.set('c', 3);
  const second = iterator.next().value;
  const done = iterator.next();
  map.set('d', 4);
  out.push({case: 'iterator-live-delete-and-append', first, second,
    done: done.done, afterDone: iterator.next().done, ...summary(map)});
}

{
  const map = new Map([['a', 1], ['b', 2]]);
  const iterator = map.entries();
  const first = iterator.next().value;
  map.delete('b');
  map.set('c', 3);
  const second = iterator.next().value;
  map.set('d', 4);
  const appended = iterator.next().value;
  out.push({case: 'iterator-live-append-before-exhaustion', first, second,
    appended, done: iterator.next().done, ...summary(map)});
}

{
  const map = new Map([['a', 1], ['b', 2]]);
  const iterator = map.entries();
  const first = iterator.next().value;
  map.clear();
  map.set('c', 3);
  out.push({case: 'iterator-live-clear-and-reinsert', first,
    next: iterator.next().value, size: map.size});
}

{
  const map = new Map([['a', 1], ['b', 2]]);
  const seen = [];
  map.forEach((value, key, receiver) => {
    seen.push([key, value, receiver === map]);
    if (key === 'a') {
      map.delete('b');
      map.set('c', 3);
    }
  });
  out.push({case: 'for-each-live-mutation', seen, ...summary(map)});
}

{
  const proto = Map.prototype;
  const set = Object.getOwnPropertyDescriptor(proto, 'set');
  const constructorPrototype = Object.getOwnPropertyDescriptor(Map, 'prototype');
  const size = Object.getOwnPropertyDescriptor(proto, 'size');
  const iterator = Object.getOwnPropertyDescriptor(proto, Symbol.iterator);
  const map = new Map([['x', 1]]);
  out.push({case: 'constructor-and-descriptors', entries: [...map],
    set: {writable: set.writable, enumerable: set.enumerable, configurable: set.configurable,
      length: set.value.length},
    constructorPrototype: {writable: constructorPrototype.writable,
      enumerable: constructorPrototype.enumerable, configurable: constructorPrototype.configurable},
    size: {get: typeof size.get, set: size.set === undefined, enumerable: size.enumerable,
      configurable: size.configurable},
    iteratorAliasesEntries: iterator.value === proto.entries});
}

{
  const map = new Map();
  const setReturnsReceiver = map.set('a', 1) === map;
  map.set('b', 2);
  const has = map.has('a');
  const firstDelete = map.delete('a');
  const secondDelete = map.delete('a');
  const keys = [...map.keys()];
  const values = [...map.values()];
  const clearResult = map.clear();
  out.push({case: 'complete-method-surface', setReturnsReceiver, has,
    firstDelete, secondDelete, keys, values, clearReturnsUndefined: clearResult === undefined,
    sizeAfterClear: map.size});
}

{
  const trace = [];
  const builtinSet = Map.prototype.set;
  const originalSet = function (key, value) {
    trace.push(['captured-set', key, value]);
    return builtinSet.call(this, key, value);
  };
  const replacementSet = function (key, value) {
    trace.push(['replacement-set', key, value]);
    return builtinSet.call(this, key, value);
  };
  Map.prototype.set = originalSet;
  let map;
  try {
    const iterable = {
      get [Symbol.iterator]() {
        trace.push(['get-iterator']);
        Map.prototype.set = replacementSet;
        return function* () {
          trace.push(['call-iterator']);
          yield {0: 'key', 1: 17, [Symbol.iterator]() {
            trace.push(['entry-iterator-should-not-run']);
            return [][Symbol.iterator]();
          }};
        };
      }
    };
    map = new Map(iterable);
    trace.push(['value', map.get('key')]);
  } finally {
    Map.prototype.set = builtinSet;
  }
  out.push({case: 'constructor-captures-adder-before-iterable-and-reads-entry-indices', trace});
}

{
  const trace = [];
  const iterator = {
    next() { trace.push(['next']); return {value: 1, done: false}; },
    return() { trace.push(['return']); return {}; }
  };
  const iterable = {[Symbol.iterator]() { trace.push(['iterator']); return iterator; }};
  let error;
  try { new Map(iterable); }
  catch (exception) { error = {name: exception.name, message: exception.message}; }
  out.push({case: 'constructor-closes-outer-iterator-on-invalid-entry', trace, error});
}

for (const stage of ['next', 'done', 'value']) {
  const trace = [];
  const iterator = {
    next() {
      trace.push('next');
      if (stage === 'next') throw new TypeError('next failed');
      const result = {};
      Object.defineProperty(result, 'done', {get() {
        trace.push('done');
        if (stage === 'done') throw new TypeError('done failed');
        return false;
      }});
      Object.defineProperty(result, 'value', {get() {
        trace.push('value');
        if (stage === 'value') throw new TypeError('value failed');
        return ['key', 1];
      }});
      return result;
    },
    return() { trace.push('return'); return {}; }
  };
  const iterable = {[Symbol.iterator]() { trace.push('iterator'); return iterator; }};
  let error;
  try { new Map(iterable); }
  catch (exception) { error = {name: exception.name, message: exception.message}; }
  out.push({case: `constructor-does-not-close-on-${stage}-abrupt`, trace, error});
}

for (const stage of ['key', 'adder']) {
  const trace = [];
  const entry = {};
  if (stage === 'key') {
    Object.defineProperty(entry, '0', {get() {
      trace.push('key');
      throw new TypeError('key failed');
    }});
    entry[1] = 1;
  } else {
    entry[0] = 'key';
    entry[1] = 1;
  }
  const iterator = {
    next() {
      trace.push('next');
      return {done: false, value: entry};
    },
    return() { trace.push('return'); return {}; }
  };
  const iterable = {[Symbol.iterator]() { trace.push('iterator'); return iterator; }};
  const priorSet = Map.prototype.set;
  if (stage === 'adder') Map.prototype.set = function () {
    trace.push('adder');
    throw new TypeError('adder failed');
  };
  let error;
  try { new Map(iterable); }
  catch (exception) { error = {name: exception.name, message: exception.message}; }
  finally { Map.prototype.set = priorSet; }
  out.push({case: `constructor-closes-on-${stage}-abrupt`, trace, error});
}

process.stdout.write(JSON.stringify({runtime: process.version, cases: out}) + '\n');
