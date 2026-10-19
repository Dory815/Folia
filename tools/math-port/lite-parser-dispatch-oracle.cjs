#!/usr/bin/env node
'use strict';

// Supplemental Source witness for virtual method dispatch inside LiteParser.
// The pinned CJS implementation is invoked directly; this file does not patch
// its prototypes, helpers, or parser outputs.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const packageVersion = JSON.parse(fs.readFileSync(path.join(packageRoot, 'package.json'))).version;
if (packageVersion !== '4.1.3') throw new Error(`Expected MathJax 4.1.3, got ${packageVersion}`);
const source = '.reference/MathJax-src/ts/adaptors/lite/Parser.ts';
const sourceSha256 = crypto.createHash('sha256').update(fs.readFileSync(path.join(root, source))).digest('hex');
const cjs = path.join(packageRoot, 'cjs/adaptors/lite/Parser.js');
const cjsSha256 = crypto.createHash('sha256').update(fs.readFileSync(cjs)).digest('hex');
if (sourceSha256 !== 'f3c15592f359cdca1a3c2729c230f8de862e8b67be5ae8f1d2d387e8482fe5d9')
  throw new Error(`Pinned Source changed: ${sourceSha256}`);
if (cjsSha256 !== '38874dadf30ee0bd60184dde9bdf29a1be9d45755893b2a025d8e1b0190ad8c0')
  throw new Error(`Pinned CJS changed: ${cjsSha256}`);
const { LiteParser } = require(cjs);
const { liteAdaptor } = require(path.join(packageRoot, 'cjs/adaptors/liteAdaptor.js'));
const normalize = value => {
  if (value === undefined) return {$:'undefined'};
  if (value === null) return null;
  if (Array.isArray(value)) return Array.from({length:value.length},(_,i) =>
    Object.hasOwn(value,i) ? normalize(value[i]) : {$:'hole'});
  if (value && typeof value === 'object') return Object.fromEntries(Object.keys(value).map(k => [k,normalize(value[k])]));
  return value;
};
const tree = node => !node ? node : node.kind === '#document'
  ? {kind:node.kind,type:node.type,root:tree(node.root)}
  : node.kind === '#text' || node.kind === '#comment' ? {kind:node.kind,value:node.value}
  : {kind:node.kind,attributes:Object.entries(node.attributes),children:node.children.map(tree)};
const records = [];
const observe = (name, fn) => {
  try { records.push({name,result:{value:normalize(fn())}}); }
  catch (e) { records.push({name,result:{error:{name:e?.name || typeof e,message:e?.message || String(e)}}}); }
};
const descriptor = (object,key) => {
  const d=Object.getOwnPropertyDescriptor(object,key);
  return d ? {kind:'value' in d?'data':'accessor',writable:d.writable,enumerable:d.enumerable,configurable:d.configurable,
    arity:typeof d.value==='function'?d.value.length:undefined} : null;
};

observe('runtime-prototype-methods', () => Object.fromEntries([
  'parseFromString','addText','addComment','closeTag','openTag','addAttributes','handlePCDATA',
  'checkDocument','getOnlyChild','serialize','serializeInner','allAttributes','protectAttribute','protectHTML',
].map(name => [name,descriptor(LiteParser.prototype,name)])));

{
  const trace=[];
  class DispatchParser extends LiteParser {
    addText(...args) { trace.push(['addText',args[2]]); return super.addText(...args); }
    addComment(...args) { trace.push(['addComment',args[2]]); return super.addComment(...args); }
    closeTag(...args) { trace.push(['closeTag',args[2]]); return super.closeTag(...args); }
    openTag(...args) { trace.push(['openTag',args[2],args[3].slice()]); return super.openTag(...args); }
    addAttributes(...args) { trace.push(['addAttributes',args[2].slice()]); return super.addAttributes(...args); }
    handlePCDATA(...args) { trace.push(['handlePCDATA',args[2],args[3].slice()]); return super.handlePCDATA(...args); }
    checkDocument(...args) { trace.push(['checkDocument']); return super.checkDocument(...args); }
    getOnlyChild(...args) { trace.push(['getOnlyChild']); return super.getOnlyChild(...args); }
  }
  const parser=new DispatchParser(), adaptor=liteAdaptor();
  observe('parse-internal-virtual-dispatch', () => {
    const result=parser.parseFromString('pre<p a="x">text<script>if(a<b) c()</script><!-- note --><br/></p>post','text/html',adaptor);
    return {trace,tree:tree(result)};
  });
}

{
  const trace=[];
  class SerializeProbe extends LiteParser {
    allAttributes(...args) { trace.push(['allAttributes',args[2]]); return super.allAttributes(...args); }
    protectAttribute(...args) { trace.push(['protectAttribute',args[1]]); return super.protectAttribute(...args); }
    serializeInner(...args) { trace.push(['serializeInner',args[2]]); return super.serializeInner(...args); }
    protectHTML(text) { trace.push(['protectHTML',text]); return `!${super.protectHTML(text)}`; }
    serialize(...args) { trace.push(['serialize',args[2]]); return super.serialize(...args); }
  }
  const parser=new SerializeProbe(), adaptor=liteAdaptor(), node=adaptor.node('p',{title:'a&b'},[adaptor.text('<x>')]);
  observe('serialize-internal-virtual-dispatch', () => ({html:parser.serialize(adaptor,node),trace}));
}

// `parts` is the actual mutable Array returned by split().  The parser passes
// it to openTag and resumes its while loop on that same Array after the
// virtual call.  Also verify that later iterations re-read the method from
// the receiver after an override replaces it during openTag.
{
  const trace=[];
  class LivePartsProbe extends LiteParser {
    openTag(adaptor,node,tag,parts) {
      trace.push(['openTag-before',parts.slice()]);
      parts.splice(0,parts.length,'rewritten','</p>','after');
      this.addText=(a,n,text)=>{
        trace.push(['rebound-addText',text]);
        return super.addText(a,n,`!${text}`);
      };
      const result=super.openTag(adaptor,node,tag,parts);
      trace.push(['openTag-after',parts.slice()]);
      return result;
    }
    closeTag(...args) { trace.push(['closeTag',args[2]]); return super.closeTag(...args); }
  }
  const parser=new LivePartsProbe(),adaptor=liteAdaptor();
  observe('parse-live-parts-and-method-rebind',()=>{
    const result=parser.parseFromString('<p>original</p>tail','text/html',adaptor);
    return {tree:tree(result),trace};
  });
}

{
  const parser=new LiteParser(), adaptor=liteAdaptor();
  const old=LiteParser.SELF_CLOSING;
  try {
    LiteParser.SELF_CLOSING={...old, probe:true};
    observe('live-constructor-static-read-after-instance', () => tree(parser.parseFromString('<probe>x</probe>','text/html',adaptor)));
  } finally { LiteParser.SELF_CLOSING=old; }
}

const out=path.join(root,'build/math-port/lite-parser-dispatch-original.json');
fs.mkdirSync(path.dirname(out),{recursive:true});
fs.writeFileSync(out,JSON.stringify({packageVersion,source:{file:source,sha256:sourceSha256},cjs:{file:path.relative(packageRoot,cjs),sha256:cjsSha256},records},null,2)+'\n');
console.log(`${path.relative(root,out)}: pinned LiteParser internal dispatch observations`);
