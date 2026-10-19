'use strict';
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../../../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const files = {
  'cjs/core/Tree/NodeFactory.js': '148c9a3a2e469fc464036b88deb7067cd779989a43356812d31365a84ec75c54',
  'cjs/core/Tree/Factory.js': '739db2e5760c29d7bb4f1bc7baa08ff8d727f4d117522aff5ede017f812cffef'
};
for (const [file, expected] of Object.entries(files)) {
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(packageRoot, file))).digest('hex');
  assert.equal(actual, expected, `${file} hash`);
}
assert.equal(JSON.parse(fs.readFileSync(path.join(packageRoot, 'package.json'), 'utf8')).version, '4.1.3');
const { AbstractNodeFactory } = require(path.join(packageRoot, 'cjs/core/Tree/NodeFactory.js'));

function run(callable) {
  const events = [];
  const factory = new AbstractNodeFactory();
  Object.defineProperty(factory.node, 'member', {
    configurable: true,
    get() {
      events.push('lookup getter');
      return callable ? function member() { events.push('selected call'); return 'ok'; } : null;
    }
  });
  let result;
  try {
    result = factory.create('member', (events.push('properties expression'), {}),
      (events.push('children expression'), []));
  } catch (error) {
    result = { name: error.name, message: error.message };
  }
  return { events, result };
}

const callable = run(true);
assert.deepEqual(callable, {
  events: ['properties expression', 'children expression', 'lookup getter', 'selected call'],
  result: 'ok'
});
const nonCallable = run(false);
assert.deepEqual(nonCallable, {
  events: ['properties expression', 'children expression', 'lookup getter'],
  result: { name: 'TypeError', message: 'this.node[kind] is not a function' }
});
console.log(JSON.stringify({
  schema: 1,
  upstream: { package: 'MathJax', version: '4.1.3', commit: 'fb987178c2d279a99b0db5ea02e751691703955e' },
  sourceHashes: files,
  observations: { callable, nonCallable }
}, null, 2));
