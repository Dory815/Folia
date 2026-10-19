'use strict';

// Supplemental pinned-CJS observation for the actual LiteAdaptor graph rule:
// nodes are ordinary Source objects and operations do not enforce adaptor
// ownership. This is separate from the frozen 35-row baseline.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const packageVersion = JSON.parse(fs.readFileSync(path.join(packageRoot, 'package.json'))).version;
if (packageVersion !== '4.1.3') throw new Error('Expected pinned MathJax 4.1.3');
const file = path.join(packageRoot, 'cjs/adaptors/liteAdaptor.js');
const cjsSha256 = crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
if (cjsSha256 !== 'e9879de720f356cf8f54c0960102b1e40b4eafc1e34a14481aa896e1e99bd76f')
  throw new Error(`Pinned CJS changed: cjs/adaptors/liteAdaptor.js ${cjsSha256}`);
const { liteAdaptor } = require(file);
const plain = value => value === undefined ? {$:'undefined'} : value;
const a = liteAdaptor(), b = liteAdaptor();
const parent = a.node('parent');
const child = b.node('child');
const appended = a.append(parent, child);
const afterAppend = {
  sameReturnedNode: appended === child,
  childParentIsOtherAdaptorNode: child.parent === parent,
  childPresentInParent: parent.children[0] === child,
  firstAdaptorChildNode: a.childNode(parent, 0) === child,
  secondAdaptorParent: b.parent(child) === parent,
};
const former = b.node('former');
const replacement = a.node('replacement');
b.append(former, replacement);
const replaced = a.replace(replacement, child);
const afterReplace = {
  returnedOldNode: replaced === child,
  oldParentNull: child.parent === null,
  replacementParentIsParent: replacement.parent === parent,
  replacementRetainedInFormerArray: former.children[0] === replacement,
  replacementAlsoInNewParent: parent.children[0] === replacement,
};
const detachedDocument = b.createDocument();
const documentState = {
  defaultViewIsNull: detachedDocument.defaultView === null,
  aHeadAcceptsBDocument: a.head(detachedDocument) === detachedDocument.head,
  aBodyAcceptsBDocument: a.body(detachedDocument) === detachedDocument.body,
  aRootAcceptsBDocument: a.root(detachedDocument) === detachedDocument.root,
  rootParent: plain(detachedDocument.root.parent),
};
process.stdout.write(JSON.stringify({packageVersion,cjsSha256,afterAppend,afterReplace,documentState},null,2)+'\n');
