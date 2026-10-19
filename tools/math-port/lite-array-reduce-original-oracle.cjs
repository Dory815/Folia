'use strict';

// Source observations for the real Array.prototype.reduce operation used by
// LiteAdaptor.textContent. The runtime is pinned by the repository's Node
// package; no replacement reduce implementation is used in this oracle.
const rows = [];
const capture = (name, run) => {
  try {
    rows.push({name, ...run()});
  } catch (error) {
    rows.push({name, error: {name: error && error.name, message: error && error.message}});
  }
};

capture('sparse_live_has_get_and_captured_length', () => {
  const trace = [];
  const values = [1, 2, 3];
  const result = values.reduce((acc, value, index, receiver) => {
    trace.push(['call', acc, value, index, receiver === values]);
    if (index === 0) {
      delete values[1];
      values[2] = 7;
      values[3] = 11;
    }
    return acc + value;
  }, 0);
  return {result, trace, length: values.length};
});

capture('inherited_initial_accumulator_without_callback', () => {
  const trace = [];
  const proto = Object.create(Array.prototype);
  Object.defineProperty(proto, '0', {get() { trace.push('get inherited 0'); return 4; }});
  const values = new Array(2);
  Object.setPrototypeOf(values, proto);
  values[1] = 3;
  const result = values.reduce((acc, value, index) => {
    trace.push(['call', acc, value, index]);
    return acc + value;
  });
  return {result, trace};
});

capture('length_get_precedes_callback_validation', () => {
  const trace = [];
  const receiver = {};
  Object.defineProperty(receiver, 'length', {get() { trace.push('length'); return 1; }});
  try { Array.prototype.reduce.call(receiver, 3, 0); }
  catch (error) { return {trace, error: {name: error.name, message: error.message}}; }
  return {trace};
});

capture('array_like_string_length_and_index', () => {
  const receiver = {0: 'x', length: '1'};
  const result = Array.prototype.reduce.call(receiver,
    (acc, value, index, object) => `${acc}${value}${index}${object === receiver}`, '');
  return {result};
});

capture('all_holes_without_initial_value', () => {
  return {result: new Array(3).reduce((a, b) => a + b)};
});

capture('callback_exception_stops_later_gets', () => {
  const trace = [];
  const proto = Object.create(Array.prototype);
  Object.defineProperty(proto, '2', {get() { trace.push('get 2'); return 9; }});
  const values = [1, 2, 3];
  Object.setPrototypeOf(values, proto);
  try {
    values.reduce((acc, value, index) => {
      trace.push(['call', index]);
      if (index === 1) throw new RangeError('stop');
      return acc + value;
    }, 0);
  } catch (error) {
    return {trace, error: {name: error.name, message: error.message}};
  }
  return {trace};
});

process.stdout.write(JSON.stringify({runtime: process.version, rows}, null, 2) + '\n');
