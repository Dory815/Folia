'use strict';

const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

function repositoryRoot(args) {
  const index = args.indexOf('--repo-root');
  if (index !== -1) {
    assert.ok(args[index + 1], '--repo-root requires a path');
    return path.resolve(args[index + 1]);
  }
  let current = __dirname;
  while (current !== path.dirname(current)) {
    if (fs.existsSync(path.join(current, 'tools/math-port/common-layout-reference/profile-source-lock.json'))) return current;
    current = path.dirname(current);
  }
  throw new Error('Could not locate repository profile-source-lock.json');
}
const root = repositoryRoot(process.argv.slice(2));
const sourceRelative = 'src/cjs/core/Tree/Node.js';
const lockPath = path.join(root, 'tools/math-port/common-layout-reference/profile-source-lock.json');
const lock = JSON.parse(fs.readFileSync(lockPath, 'utf8'));
const sourcePin = lock.dependencies.find(entry => entry.file === sourceRelative);
assert.ok(sourcePin, `No tracked source pin for ${sourceRelative}`);
const sourceRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const sourcePath = path.join(sourceRoot, 'cjs/core/Tree/Node.js');
const actualHash = crypto.createHash('sha256').update(fs.readFileSync(sourcePath)).digest('hex');
assert.equal(actualHash, sourcePin.sha256, 'Pinned Node.js source hash differs');
assert.equal(JSON.parse(fs.readFileSync(path.join(sourceRoot, 'package.json'), 'utf8')).version, '4.1.3');

const { AbstractNode, AbstractEmptyNode } = require(sourcePath);
const { AbstractNodeFactory } = require(path.join(sourceRoot, 'cjs/core/Tree/NodeFactory.js'));
class Row extends AbstractNode {
  get kind() { return 'row'; }
}
class Token extends AbstractNode {
  get kind() { return 'token'; }
}
class Special extends Token {
  get kind() { return 'special'; }
}
class Empty extends AbstractEmptyNode {
  get kind() { return 'empty'; }
}
class Factory extends AbstractNodeFactory {}
const factory = new Factory({ row: Row, token: Token, special: Special, empty: Empty });

const rootNode = factory.create('row');
const first = factory.create('token');
const old = factory.create('token');
const replacement = factory.create('special');
rootNode.setChildren([first, old]);
assert.equal(rootNode.childIndex(old), 1);
assert.equal(first.parent, rootNode);
rootNode.setChildren(rootNode.childNodes);
assert.equal(rootNode.childNodes.length, 2);
assert.equal(first.parent, rootNode);
assert.equal(rootNode.replaceChild(replacement, old), replacement);
assert.equal(old.parent, null);
assert.equal(replacement.parent, rootNode);
const absent = factory.create('token');
assert.equal(rootNode.replaceChild(absent, old), absent);
assert.equal(absent.parent, null);
assert.equal(rootNode.removeChild(old), old);
assert.deepEqual(rootNode.findNodes('special'), [replacement]);
assert.deepEqual(rootNode.findNodes('token'), [first, replacement]);
assert.equal(replacement.isKind('token'), true);
assert.equal(rootNode.toString(), 'row(token(),special())');
const clone = rootNode.copy();
assert.notEqual(clone, rootNode);
assert.equal(clone.childNodes.length, 2);
assert.equal(clone.childNodes[0].parent, clone);
assert.equal(clone.childNodes[1].kind, 'special');

const visited = [];
const late = factory.create('token');
rootNode.walkTree(node => {
  visited.push(node.kind);
  if (node === rootNode) rootNode.appendChild(late);
});
assert.deepEqual(visited, ['row', 'token', 'special', 'token']);

const empty = factory.create('empty');
const ignored = factory.create('token');
empty.setChildren([ignored]);
assert.deepEqual(empty.childNodes, []);
assert.equal(empty.appendChild(ignored), ignored);
assert.equal(ignored.parent, null);
assert.equal(empty.replaceChild(ignored, rootNode), rootNode);
assert.equal(empty.childIndex(ignored), null);
const emptyWalk = [];
empty.walkTree(node => emptyWalk.push(node.kind));
assert.deepEqual(emptyWalk, ['empty']);
assert.equal(empty.toString(), 'empty');

process.stdout.write('verified pinned MathJax 4.1.3 Node source: hierarchy, child links, no-op misses, alias-safe setChildren, copy, mutating preorder walk, and empty overrides\n');
