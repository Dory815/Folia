'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const cp = require('node:child_process');
const {createReference} = require('../../reference-profile.cjs');

const root = path.resolve(__dirname, '../../../../');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const commit = 'fb987178c2d279a99b0db5ea02e751691703955e';
assert.equal(cp.execFileSync('git', ['rev-parse', 'HEAD'], {
  cwd: sourceRoot, encoding: 'utf8'
}).trim(), commit);
const pinned = new Map([
  ['ts/input/tex/texhtml/TexHtmlConfiguration.ts',
    'e31f1539452eba33048d8e7bcf87fcf9690f23be1412799ddf2a6f43beab4a3b'],
  ['ts/core/MmlTree/MmlNode.ts',
    'cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656'],
  ['ts/core/MmlTree/MmlNodes/mtext.ts',
    '66e24a47505714c0d67b334a8c73c6e85dbfcac1f920913eef9f664546f2403f'],
  ['ts/core/MmlTree/MmlNodes/HtmlNode.ts',
    '1bf0ba981fc72b63a525424fb51e8bc45231fcf768d618fb8c75517bd72ae665'],
  ['ts/output/svg/Wrappers/semantics.ts',
    '6a68dd39649fcf05407dd12a700a294c78e9be140b7ceefe518523efae4bdb6e'],
  ['ts/adaptors/lite/Element.ts',
    'd3fc9147fa7bf658fd2eb523784edc0940f66c6223d61bdd3b9d7f6b5e53bfeb'],
  ['ts/adaptors/liteAdaptor.ts',
    '6aded576671ae8cf4b1146cc82c793628fa593abdc00d05e3e8880c53fe6a214'],
]);
const sha = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const files = new Map();
for (const [relative, expected] of pinned) {
  const bytes = fs.readFileSync(path.join(sourceRoot, relative));
  const actual = sha(bytes);
  if (expected) assert.equal(actual, expected, `${relative} hash changed`);
  files.set(relative, bytes.toString('utf8'));
}
assert.match(files.get('ts/input/tex/texhtml/TexHtmlConfiguration.ts'),
  /parser\.Push\(parser\.create\('node', 'mtext', \[node\]\)\)/);
assert.match(files.get('ts/core/MmlTree/MmlNodes/mtext.ts'),
  /class MmlMtext extends AbstractMmlTokenNode/);
assert.match(files.get('ts/core/MmlTree/MmlNode.ts'),
  /public setTeXclass\(prev: MmlNode\): MmlNode \{\s*this\.getPrevClass\(prev\);\s*return this\.texClass != null \? this : prev;/);
assert.match(files.get('ts/output/svg/Wrappers/semantics.ts'),
  /this\.adaptor\.append\(\s*parents\[0\],\s*this\.svg\(\s*'foreignObject'/);
assert.match(files.get('ts/output/svg/Wrappers/semantics.ts'),
  /\[this\.getHTML\(\)\]/);
assert.match(files.get('ts/adaptors/lite/Element.ts'),
  /for \(const child of this\.children\) \{\s*child\.parent = this;/);
assert.match(files.get('ts/adaptors/liteAdaptor.ts'),
  /node\.children\.push\(child\);\s*child\.parent = node;/);

const reference = createReference();
const {output, adaptor} = reference;
output.options.htmlHDW = 'use';
const {TeX} = reference.require('@mathjax/src/cjs/input/tex.js');
const {MmlFactory} = reference.require('@mathjax/src/cjs/core/MmlTree/MmlFactory.js');
const input = new TeX({packages: ['base', 'html', 'setoptions', 'texhtml']});
input.setMmlFactory(new MmlFactory());
const document = reference.mathjax.document('', {InputJax: input, OutputJax: output});
const mathItem = new (reference.require('@mathjax/src/cjs/core/MathItem.js').AbstractMathItem)(
  '\\setOptions{allowTexHTML=true}<tex-html><section data-mjx-hdw="1em .25em 2em"><b>source</b><!--source comment--></section></tex-html>',
  input,
  true
);
const parsed = input.compile(mathItem, document);
function shape(node) {
  return {kind: node.kind, children: node.childNodes?.map(shape) ?? []};
}
assert.equal(parsed.kind, 'math');
assert.equal(parsed.childNodes.length, 1);
let mtext = parsed.childNodes[0];
if (mtext.kind === 'inferredMrow') {
  assert.equal(mtext.childNodes.length, 1);
  mtext = mtext.childNodes[0];
}
assert.equal(mtext.kind, 'mtext', 'pinned texhtml parser wraps HtmlNode in mtext');
assert.equal(mtext.childNodes.length, 1);
assert.equal(mtext.childNodes[0].kind, 'html');

const originalSvg = output.svg;
let helperCalls = 0;
let returned = null;
let decoy = null;
let rect = null;
let helperChild = null;
output.svg = function(type, attributes, children) {
  const first = originalSvg.call(this, type, attributes, children);
  if (type === 'foreignObject') {
    helperCalls++;
    returned = first;
    helperChild = children[0];
    helperChild.attributes['data-helper'] = 'source-helper';
    decoy = originalSvg.call(this, 'g', {}, []);
    rect = originalSvg.call(this, 'rect', {width: '12', height: '4'}, []);
    adaptor.append(first, rect);
  }
  return first;
};
const converted = document.convert(mathItem.math, {
  display: true, em: 16, ex: 8, containerWidth: 640
});
assert.equal(helperCalls, 1);
assert.ok(returned);
assert.ok(helperChild);
assert.ok(decoy);
assert.ok(rect);
assert.equal(adaptor.parent(rect), returned);
assert.notEqual(adaptor.parent(decoy), returned);
assert.equal(adaptor.parent(helperChild), returned,
  'LiteElement constructor assigns parent for the foreignObject child');
assert.equal(adaptor.kind(helperChild), 'mjx-html');
assert.equal(returned.children.length, 2);
assert.equal(returned.children[0], helperChild);
assert.equal(returned.children[1], rect);
assert.equal(adaptor.parent(decoy) ?? null, null);
assert.equal(helperChild.attributes['data-helper'], 'source-helper');
assert.equal(adaptor.kind(returned), 'foreignObject');
assert.equal(adaptor.kind(decoy), 'g');
const findSvg = node => adaptor.kind(node) === 'svg' ? node :
  adaptor.childNodes(node).map(findSvg).find(Boolean) ?? null;
const svg = findSvg(converted);
assert.ok(svg, 'MathDocument.convert returned an SVG subtree');
const section = adaptor.firstChild(helperChild);
assert.equal(adaptor.kind(section), 'section');
const bold = adaptor.firstChild(section);
const comment = adaptor.next(bold);
assert.equal(adaptor.kind(comment), '#comment');
assert.equal(comment.parent, undefined,
  'LiteAdaptor.clone copies comments without restoring parent');
assert.equal(helperChild.parent, returned);
const sourceAttributes = helperChild.attributes;
sourceAttributes['data-after-import'] = 'same owner';
assert.equal(helperChild.attributes['data-after-import'], 'same owner');
const oldAttributes = helperChild.attributes;
const replacementAttributes = {'data-replacement': 'initial'};
helperChild.attributes = replacementAttributes;
assert.equal(helperChild.attributes, replacementAttributes);
assert.notEqual(helperChild.attributes, oldAttributes);
sourceAttributes['data-old-alias'] = 'detached';
assert.equal(replacementAttributes['data-old-alias'], undefined);
const oldChildren = helperChild.children;
const replacementChildren = [comment];
helperChild.children = replacementChildren;
replacementChildren[1] = adaptor.firstChild(bold);
assert.equal(helperChild.children, replacementChildren);
assert.equal(replacementChildren.length, 2);
const sectionChildren = section.children;
sectionChildren[0] = comment;
assert.equal(bold.parent, section);
assert.equal(comment.parent, undefined);
bold.parent = helperChild;
assert.equal(bold.parent, helperChild);
assert.equal(section.children[0], comment);
adaptor.append(helperChild, comment);
assert.equal(comment.parent, helperChild);
assert.equal(section.children.length, 2,
  'append does not remove a comment whose cloned parent field is absent');
delete comment.parent;
assert.equal(adaptor.parent(comment) ?? null, null);
Object.defineProperty(comment, 'parent', {
  value: helperChild, writable: false, enumerable: true, configurable: true
});
const childCountBeforeFailure = helperChild.children.length;
assert.throws(() => adaptor.append(helperChild, comment), TypeError);
assert.equal(helperChild.children.length, childCountBeforeFailure - 1);
assert.equal(comment.parent, helperChild);
console.log(JSON.stringify({
  source: {version: '4.1.3', commit, pinnedFiles: Object.fromEntries(
    [...pinned.keys()].map(file => [file, sha(fs.readFileSync(path.join(sourceRoot, file)))])
  )},
  mathml: ['math', 'mtext', 'html'],
  typeset: 'MathDocument.convert -> MathItem.typeset -> SvgXmlNode.toSVG',
  helper: {calls: helperCalls, returnsFirst: true, decoyDetached: true,
    appendedRectParentIsReturn: true, childParentIsReturn: true,
    sourceAssignedHelperAttribute: true},
  sourceOwners: {htmlChildHasParentAfterConstruction: true,
    clonedCommentStartsWithoutParent: true,
    directChildrenAndParentWritesAreIndependent: true,
    appendMovesAndUpdatesParent: true,
    readonlyParentFailureRemovesOldSlotFirst: true}
}, null, 2));
