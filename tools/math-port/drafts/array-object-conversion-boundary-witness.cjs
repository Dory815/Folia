const trace = [];
const concatSource = {
  [Symbol.isConcatSpreadable]: true,
  get length() { trace.push('concat:length'); return {}; },
  get 0() { trace.push('concat:get0'); return 'unexpected'; }
};
const concatenated = [].concat(concatSource);
const sliced = [].slice({});
const included = [2].includes(2, {});
const filled = [].fill(7, {});
const joinTrace = [];
const joinedArray = [{
  [Symbol.toPrimitive](hint) {
    joinTrace.push(`element:${hint}`);
    joinedArray.length = 0;
    return 'value';
  }
}, undefined];
const joined = joinedArray.join();
const actual = {
  concat: [concatenated.length, concatenated[0], trace],
  slice: sliced.length,
  includes: included,
  fill: filled.length,
  joinAfterShrink: [joined, joinTrace, joinedArray.length]
};
const expected = {
  concat: [0, undefined, ['concat:length']],
  slice: 0,
  includes: true,
  fill: 0,
  joinAfterShrink: ['value,', ['element:string'], 0]
};
if (JSON.stringify(actual) !== JSON.stringify(expected)) throw new Error(JSON.stringify(actual));
console.log(JSON.stringify(actual));
