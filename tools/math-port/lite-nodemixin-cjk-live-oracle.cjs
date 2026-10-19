'use strict';

// Supplemental pinned-CJS observation for NodeMixin's live static RegExp.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const parserPath = path.join(packageRoot, 'cjs/adaptors/NodeMixin.js');
const hash = crypto.createHash('sha256').update(fs.readFileSync(parserPath)).digest('hex');
if (hash !== 'd331d73f054c46dfd3122c41bd6762389935b68c2e89bf1a4011cc70ea42ef3a')
  throw new Error(`Pinned NodeMixin.js changed: ${hash}`);
const { NodeMixin } = require(parserPath);
const { LiteBase, LiteAdaptor } = require(path.join(packageRoot, 'cjs/adaptors/liteAdaptor.js'));

const mixinClass = Object.getPrototypeOf(LiteAdaptor);
const original = mixinClass.cjkPattern;
try {
  const pattern = /a/gu;
  pattern.lastIndex = 2;
  mixinClass.cjkPattern = pattern;
  const adaptor = new LiteAdaptor(null, { cjkCharWidth: 2, unknownCharWidth: 0.5 });
  const node = adaptor.node('span', {}, [adaptor.text('aaab')]);
  const live = { size: adaptor.nodeSize(node), lastIndex: pattern.lastIndex };

  const Mixed = NodeMixin(LiteBase, {});
  const prior = Mixed.cjkPattern;
  Mixed.cjkPattern = /x/gu;
  const separateMixin = new Mixed(null, {});
  const separateNode = separateMixin.node('span', {}, [separateMixin.text('aaab')]);
  const lexical = { size: separateMixin.nodeSize(separateNode), localStaticSource: Mixed.cjkPattern.source };
  Mixed.cjkPattern = prior;
  process.stdout.write(JSON.stringify({ hash, live, lexical }, null, 2) + '\n');
} finally {
  mixinClass.cjkPattern = original;
}
