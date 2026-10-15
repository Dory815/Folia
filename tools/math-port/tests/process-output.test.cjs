'use strict';

const assert = require('node:assert/strict');
const {Readable} = require('node:stream');
const {once} = require('node:events');
const {captureOutput} = require('../process-output.cjs');

async function read(chunks) {
  const stream = Readable.from(chunks);
  const output = captureOutput(stream);
  await once(stream, 'end');
  return output;
}

(async () => {
  const record = {text: 'aµ⁢𝒙中文🙂z', nested: {linebreakmultchar: '⁢'}};
  const text = JSON.stringify(record) + '\n';
  const bytes = Buffer.from(text, 'utf8');
  // Every split includes positions inside all multi-byte characters, including
  // the invisible multiplication character from the Mhchem observation.
  for (let split = 0; split <= bytes.length; ++split) {
    const output = await read([bytes.subarray(0, split), bytes.subarray(split)]);
    assert.deepStrictEqual(output.bytes(), bytes);
    assert.equal(output.text(), text);
    assert.deepStrictEqual(JSON.parse(output.text()), record);
  }
  const bytewise = await read([...bytes].map(byte => Buffer.from([byte])));
  assert.equal(bytewise.text(), text);
  const invalid = await read([Buffer.from([0xb5])]);
  assert.deepStrictEqual(invalid.bytes(), Buffer.from([0xb5]));
  assert.throws(() => invalid.text(), /not valid UTF-8/);
  console.log(`Process output: ${bytes.length + 3} byte-boundary checks passed`);
})().catch(error => { console.error(error); process.exitCode = 1; });
