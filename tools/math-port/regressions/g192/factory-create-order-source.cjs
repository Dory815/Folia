'use strict';
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../../../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const locks = {
  'ts/core/Tree/Factory.ts': 'e61d2f5868287e2f382503cf690cd241698bf212957c1795d0cd35ce3bef049e',
  'ts/core/Tree/NodeFactory.ts': 'b02c5ac2cabb61755785d35def6b57f8246322c626f548df600ca81af27b4599',
  'ts/core/MmlTree/MmlFactory.ts': '5e7338cd3eed5a4002243b7371dc5098c6433ff4ba4345cb2d50cf0385b2abdb'
};
const cjsLocks = {
  'cjs/core/Tree/NodeFactory.js': '148c9a3a2e469fc464036b88deb7067cd779989a43356812d31365a84ec75c54',
  'cjs/core/MmlTree/MmlFactory.js': 'd0c4e25237d04a9d21d12ee44776e46d30777e7f284db6f4f3ed1cc625b752d3',
  'cjs/core/Tree/Factory.js': '739db2e5760c29d7bb4f1bc7baa08ff8d727f4d117522aff5ede017f812cffef'
};
const digest = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
for (const [file, expected] of Object.entries(locks)) assert.equal(digest(fs.readFileSync(path.join(sourceRoot, file))), expected, `${file} Source hash`);
for (const [file, expected] of Object.entries(cjsLocks)) assert.equal(digest(fs.readFileSync(path.join(packageRoot, file))), expected, `${file} CJS hash`);
assert.equal(JSON.parse(fs.readFileSync(path.join(packageRoot, 'package.json'), 'utf8')).version, '4.1.3');
const { MmlFactory } = require(path.join(packageRoot, 'cjs/core/MmlTree/MmlFactory.js'));
function observe(factory, kind, installKindProperty, expectedEvents) {
  const events = [];
  if (installKindProperty) installKindProperty(factory, events);
  const children = {
    get length() { events.push('children.length'); return 1; },
    get [Symbol.iterator]() {
      events.push('children.[Symbol.iterator] get');
      return function iteratorMethod() {
        events.push('children.[Symbol.iterator] call');
        return { next() { events.push('iterator.next'); return { done: true }; } };
      };
    }
  };
  let result;
  try {
    factory.create(kind, {}, children);
    result = { returned: true };
  } catch (error) {
    result = { returned: false, name: error.name, message: error.message };
  }
  assert.deepEqual(result, {
    returned: false,
    name: 'TypeError',
    message: 'this.node[kind] is not a function'
  });
  assert.deepEqual(events, expectedEvents, `${kind}: unexpected property/child access order`);
  return { kind, error: result, events };
}
const factory = new MmlFactory();
const missingKind = observe(factory, 'missing-kind', null, []);
const nonCallable = observe(factory, 'non-callable-kind', (target, events) => {
  Object.defineProperty(target.node, 'non-callable-kind', {
    configurable: true,
    get() { events.push('node[kind] getter'); return 0; }
  });
}, ['node[kind] getter']);
const inheritedEvents = [];
const inheritedChildren = {
  get length() { inheritedEvents.push('children.length'); return 1; },
  get [Symbol.iterator]() { inheritedEvents.push('children.[Symbol.iterator] get'); return function() { return { next() { inheritedEvents.push('iterator.next'); return { done: true }; } }; }; }
};
const inheritedValue = new MmlFactory().create('toString', {}, inheritedChildren);
assert.equal(inheritedValue, '[object Object]');
assert.deepEqual(inheritedEvents, [], 'inherited callable runs without child conversion');
const customEvents = [];
const customFactory = new MmlFactory();
function CustomNode(_factory, _properties, customChildren) {
  customEvents.push('custom constructor');
  this.childCount = customChildren.length;
}
customFactory.setNodeClass('custom-kind', CustomNode);
const customChildren = { get length() { customEvents.push('children.length'); return 3; } };
const customValue = customFactory.create('custom-kind', {}, customChildren);
assert.equal(customValue.childCount, 3);
assert.deepEqual(customEvents, ['custom constructor', 'children.length']);
const out = {
  schema: 1,
  upstream: { version: '4.1.3', commit: 'fb987178c2d279a99b0db5ea02e751691703955e' },
  packageVersion: '4.1.3',
  sourceLocks: locks,
  cjsLocks,
  observations: [missingKind, nonCallable,
    { kind: 'inherited-toString', value: inheritedValue, events: inheritedEvents },
    { kind: 'setNodeClass-custom-creator', value: { childCount: customValue.childCount }, events: customEvents }],
  output: path.relative(root, path.join(root, 'build/math-port/g190-factory-create-diagnostic/source-witness.json')).replaceAll('\\', '/')
};
const output = path.join(root, 'build/math-port/g190-factory-create-diagnostic/source-witness.json');
fs.mkdirSync(path.dirname(output), { recursive: true });
fs.writeFileSync(output, JSON.stringify(out, null, 2) + '\n');
console.log(JSON.stringify(out, null, 2));
