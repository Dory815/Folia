#!/usr/bin/env node
'use strict';

// Focused Source-only observation for checkDocument's virtual calls, iterator
// order, and document-field writes.  No upstream methods are patched.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const rootDir = path.resolve(__dirname, '../..');
const packageRoot = path.join(rootDir, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const parserPath = path.join(packageRoot, 'cjs/adaptors/lite/Parser.js');
const adaptorPath = path.join(packageRoot, 'cjs/adaptors/liteAdaptor.js');
const sha = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const parserHash = sha(parserPath), adaptorHash = sha(adaptorPath);
if (parserHash !== '38874dadf30ee0bd60184dde9bdf29a1be9d45755893b2a025d8e1b0190ad8c0')
  throw new Error(`Pinned Parser.js changed: ${parserHash}`);
if (adaptorHash !== 'e9879de720f356cf8f54c0960102b1e40b4eafc1e34a14481aa896e1e99bd76f')
  throw new Error(`Pinned liteAdaptor.js changed: ${adaptorHash}`);
const {LiteParser} = require(parserPath);
const {liteAdaptor} = require(adaptorPath);
const {LiteComment} = require(path.join(packageRoot,'cjs/adaptors/lite/Text.js'));

const trace = [];
const adaptor = liteAdaptor();
const parser = new LiteParser();
const document = adaptor.createDocument();
const head = adaptor.node('head'), body = adaptor.node('body');
const html = adaptor.node('html',{},[head,body]);
const doctype = new LiteComment('<!DOCTYPE html>');
document.body.children = [doctype,html];

for (const name of ['body','childNodes','kind','remove','parent','append','insert']) {
  const original = adaptor[name];
  adaptor[name] = function (...args) {
    trace.push(`adaptor.${name}`);
    return original.apply(this,args);
  };
}
const originalOnly = parser.getOnlyChild;
parser.getOnlyChild = function (...args) {
  trace.push('parser.getOnlyChild');
  return originalOnly.apply(this,args);
};
for (const field of ['type','head','body','root']) {
  let value = document[field];
  Object.defineProperty(document,field,{
    configurable:true,enumerable:true,
    get(){trace.push(`document.get:${field}`);return value;},
    set(next){trace.push(`document.set:${field}`);value=next;}
  });
}
parser.checkDocument(adaptor,document);

const output = {parserHash,adaptorHash,trace,
  document:{type:document.type,head:document.head.kind,body:document.body.kind,root:document.root.kind}};
const file = path.join(rootDir,'build/math-port/lite-parser-check-document-order-original.json');
fs.mkdirSync(path.dirname(file),{recursive:true});
fs.writeFileSync(file,JSON.stringify(output,null,2)+'\n');
process.stdout.write(JSON.stringify(output,null,2)+'\n');
