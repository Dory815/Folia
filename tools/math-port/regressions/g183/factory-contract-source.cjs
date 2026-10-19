const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

const cjsRoot = path.resolve(process.cwd(), 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const packageRoot = path.resolve(cjsRoot, '..');
const pkg = require(path.join(packageRoot, 'package.json'));
const pinnedCommit = 'fb987178c2d279a99b0db5ea02e751691703955e';
const expectedHashes = new Map([
  ['core/Tree/Factory.js', '739DB2E5760C29D7BB4F1BC7BAA08FF8D727F4D117522AFF5EDE017F812CFFEF'],
  ['core/Tree/WrapperFactory.js', 'BBEB7C85E0DFD3C8E437D500D37F083A6B031CD9490B58D41C3285E8F7F679D5'],
]);
for (const [relative, expected] of expectedHashes) {
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(cjsRoot, relative))).digest('hex').toUpperCase();
  assert.equal(actual, expected, `${relative} hash differs from pinned source`);
}
assert.equal(pkg.version, '4.1.3');
const { AbstractFactory } = require(path.join(cjsRoot, 'core/Tree/Factory.js'));
const { AbstractWrapperFactory } = require(path.join(cjsRoot, 'core/Tree/WrapperFactory.js'));

const first = new AbstractFactory({});
const second = new AbstractFactory({});
assert.notStrictEqual(first, second);
assert.notStrictEqual(first.node, second.node);
assert.notStrictEqual(first.nodeMap, second.nodeMap);
const untouchedSecondNode = second.node;
const originalFirstNode = first.node;
assert.equal(AbstractFactory.call(first, {}), undefined);
assert.strictEqual(first, first);
assert.notStrictEqual(first.node, originalFirstNode); // constructor reinitializes fields on the same receiver
assert.strictEqual(second.node, untouchedSecondNode);
assert.strictEqual(AbstractWrapperFactory.call(first, {}), first);

function ExplicitClass(factory) { this.factory = factory; }
first.setNodeClass('explicit', ExplicitClass);
assert.strictEqual(first.getNodeClass('explicit'), ExplicitClass);
assert.notStrictEqual(first.node.explicit, ExplicitClass);
assert.equal(typeof first.node.explicit, 'function');
const product = first.node.explicit();
assert(product instanceof ExplicitClass);
assert.strictEqual(product.factory, first);

process.stdout.write(JSON.stringify({ version: pkg.version, pinnedCommit, checks: [
  'distinct factory receiver state', 'same-receiver base constructor call',
  'wrapper constructor returns this', 'nodeMap stores class, node stores creator'
] }, null, 2) + '\n');
