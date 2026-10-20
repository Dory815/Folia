'use strict';
// Independent proof-frame component witness, using only pinned MathJax Source.
const fs = require('node:fs'), path = require('node:path');
const crypto = require('node:crypto'), assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const lock = JSON.parse(fs.readFileSync(path.join(root, 'docs/math-port/evidence/bussproofs-source-lock.json'), 'utf8'));
for (const f of lock.files) for (const [file, hash] of [
  [path.join(root, '.reference/MathJax-src', f.ts), f.tsSha256],
  [path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src', f.cjs), f.cjsSha256]
]) assert.equal(crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex'), hash, file);
const reference = require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256, 'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load = f => require(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs', f));
const {TeX} = load('input/tex.js'), {MmlFactory} = load('core/MmlTree/MmlFactory.js');
const {ProofTreeItem} = load('input/tex/bussproofs/BussproofsItems.js');
const Stack = load('input/tex/Stack.js').default;
const trees = [], errors = [];
let transfers = 0;
function fixture() {
  const jax = new TeX({packages: ['base', 'bussproofs']});
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const proof = new ProofTreeItem(factory).setProperty('name', 'prooftree');
  return {jax, factory, proof, token: text => jax.parseOptions.nodeFactory.create('token', 'mi', {}, text)};
}
// Length-prefixed UTF-16 fields: kind, text, optional bspr_proof, child count.
// These direct frame cases introduce no attributes; assert that invariant.
function snapshot(root) {
  let result = '';
  const add = value => { value = String(value); result += value.length + ':' + value; };
  const visit = n => {
    add(n.kind); add(n.isToken ? n.getText() : '');
    assert.deepEqual(n.attributes?.getAllAttributes?.() || {}, {});
    const proof = n.getProperty('bspr_proof');
    add(proof === undefined ? '' : String(proof));
    const children = n.isToken ? [] : n.childNodes;
    add(children.length); children.forEach(visit);
  };
  visit(root); return result;
}
function tree(name, action) {
  const f = fixture();
  trees.push({name, snapshot: snapshot(action(f))});
}
function bad(name, action) {
  const f = fixture();
  assert.throws(() => action(f), error => {
    assert.equal(typeof error.id, 'string');
    errors.push({name, id: error.id, message: error.message});
    return true;
  });
}
tree('empty', f => f.proof.toMml());
tree('one', f => { f.proof.Push(f.token('x')); return f.proof.toMml(); });
tree('multiple', f => { f.proof.Push(f.token('x'), f.token('y')); return f.proof.toMml(); });
tree('ignored-flags', f => { f.proof.Push(f.token('x'), f.token('y')); return f.proof.toMml(false, true); });
tree('ignored-force-single', f => { f.proof.Push(f.token('x')); return f.proof.toMml(false, true); });
tree('prefix', f => {
  const frame = f.factory.create('mml', f.token('p'));
  assert.deepEqual(f.proof.checkItem(frame), [null, false]);
  f.proof.Push(f.token('x')); return f.proof.toMml();
});
tree('grouped-prefix', f => {
  for (const frame of [f.factory.create('open'), f.factory.create('mml', f.token('p')), f.factory.create('close')])
    assert.deepEqual(f.proof.checkItem(frame), [null, false]);
  f.proof.Push(f.token('x')); return f.proof.toMml();
});
tree('prefix-empty-proof', f => {
  f.proof.checkItem(f.factory.create('mml', f.token('p'))); return f.proof.toMml();
});
tree('matching-end', f => {
  f.proof.Push(f.token('x'));
  const end = f.factory.create('end').setProperty('name', 'prooftree');
  const [replay, success] = f.proof.checkItem(end);
  assert.equal(success, true); assert.equal(replay.length, 2);
  assert.equal(replay[0].kind, 'mml'); assert.equal(replay[1] === end, true);
  return replay[0].First;
});
tree('matching-end-prefix', f => {
  f.proof.Push(f.token('x')); f.proof.checkItem(f.factory.create('mml', f.token('p')));
  return f.proof.checkItem(f.factory.create('end').setProperty('name', 'prooftree'))[0][0].First;
});
tree('outer-replay', f => {
  const outer = new Stack(f.factory, {}, false);
  outer.Push(f.factory.create('begin').setProperty('name', 'prooftree'), f.proof);
  const open = f.factory.create('open');
  outer.Push(open);
  assert.equal(outer.Top() === f.proof, true);
  // Direct access is confined to the development oracle, to establish identity.
  assert.equal(f.proof.innerStack.Top() === open, true); ++transfers;
  outer.Push(f.token('p'), f.factory.create('close'));
  f.proof.Push(f.token('x'));
  outer.Push(f.factory.create('end').setProperty('name', 'prooftree'));
  assert.equal(outer.height, 1);
  outer.Push(f.factory.create('stop'));
  assert.equal(outer.Top().kind, 'mml');
  return outer.Top().First;
});
bad('stop', f => f.proof.checkItem(f.factory.create('stop')));
bad('stop-unnamed', f => new ProofTreeItem(f.factory).checkItem(f.factory.create('stop')));
bad('mismatched-end', f => f.proof.checkItem(f.factory.create('end').setProperty('name', 'other')));
bad('extra-close', f => f.proof.checkItem(f.factory.create('close')));
bad('unfinished-group', f => { f.proof.checkItem(f.factory.create('open')); f.proof.toMml(); });
bad('unfinished-group-end', f => {
  f.proof.checkItem(f.factory.create('open'));
  f.proof.checkItem(f.factory.create('end').setProperty('name', 'prooftree'));
});
function cpp(value) {
  return 'u"' + Array.from(value, c => {
    if (c === '\\') return '\\\\'; if (c === '"') return '\\"';
    const n = c.codePointAt(0);
    return n < 32 || n > 126 ? (n > 65535 ? '\\U' + n.toString(16).padStart(8, '0') : '\\u' + n.toString(16).padStart(4, '0')) : c;
  }).join('') + '"';
}
let output = '// Generated from pinned Source: tex-bussproofs-items-source.cjs --write-fixture\n';
output += 'const std::array proofFrameTrees{\n' + trees.map(c => '    ProofFrameTree{' + cpp(c.name) + ', ' + cpp(c.snapshot) + '},').join('\n') + '\n};\n';
output += 'const std::array proofFrameErrors{\n' + errors.map(c => '    ProofFrameError{' + cpp(c.name) + ', "' + c.id + '", ' + cpp(c.message) + '},').join('\n') + '\n};\n';
const target = path.join(root, 'tests/math/fixtures/bussproofs_items.inc');
if (process.argv.includes('--write-fixture')) fs.writeFileSync(target, output);
else assert.equal(fs.readFileSync(target, 'utf8').replaceAll('\r\n', '\n'), output, 'Proof frame Source fixture changed');
console.log(JSON.stringify({trees: trees.length, errors: errors.length, transfers,
  compiled_source_sha256: reference.provenance.compiled_source_sha256}));
