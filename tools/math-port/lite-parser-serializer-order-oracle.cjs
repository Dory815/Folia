'use strict';

// Focused pinned-CJS observation for LiteParser serialization callbacks.
// This stays independent of native output and does not change the frozen 35-row
// LiteAdaptor baseline.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');

const root = path.resolve(__dirname, '../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const parserPath = path.join(packageRoot, 'cjs/adaptors/lite/Parser.js');
const parserHash = '38874dadf30ee0bd60184dde9bdf29a1be9d45755893b2a025d8e1b0190ad8c0';
const hash = crypto.createHash('sha256').update(fs.readFileSync(parserPath)).digest('hex');
if (hash !== parserHash) throw new Error(`Pinned Parser.js changed: ${hash}`);

const { LiteParser } = require(parserPath);
const trace = [];
const parser = new LiteParser();
let protectMethod = function (value, xml) {
  trace.push(`call:initial:${typeof value}:${String(value)}:${xml}`);
  return String(value);
};
Object.defineProperty(parser, 'protectAttribute', {
  configurable: true,
  get() { trace.push('getMethod'); return protectMethod; },
  set(value) { trace.push('setMethod'); protectMethod = value; }
});
const rows = [
  Object.defineProperties({}, {
    name: { enumerable: true, get() { trace.push('name:first'); return 'a'; } },
    value: { enumerable: true, get() {
      trace.push('value:first');
      parser.protectAttribute = function (value, xml) {
        trace.push(`call:replacement:${typeof value}:${String(value)}:${xml}`);
        return String(value);
      };
      return { marker: 'raw-one' };
    } }
  }),
  Object.defineProperties({}, {
    name: { enumerable: true, get() { trace.push('name:second'); return 'b'; } },
    value: { enumerable: true, get() { trace.push('value:second'); return 7; } }
  })
];
parser.allAttributes = function () { trace.push('allAttributes'); return rows; };
parser.serializeInner = function () { trace.push('serializeInner'); return ''; };
parser.constructor = { SELF_CLOSING: {} };
const result = parser.serialize({ kind: () => 'section' }, {});

process.stdout.write(JSON.stringify({ parserHash: hash, result, trace }, null, 2) + '\n');
