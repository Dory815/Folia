'use strict';
// Actual JavaScript host observations for the Unicode RegExp dependency used
// by pinned MathJax LiteParser/NodeMixin. Native output is never an input.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const pinned={
  'ts/adaptors/lite/Parser.ts':'f3c15592f359cdca1a3c2729c230f8de862e8b67be5ae8f1d2d387e8482fe5d9',
  'ts/adaptors/NodeMixin.ts':'c28ccea622471176271cf688de41ae0b3c83f8acb8c44c66fbdd66517d101872'
};
const sourceLocks=Object.entries(pinned).map(([file,expected])=>{
  const sha256=digest(path.join(root,'.reference/MathJax-src',file));
  assert.equal(sha256,expected,file);return{file,sha256};
});
const plain=value=>value===undefined?{$:'undefined'}:typeof value==='number'&&Object.is(value,-0)?{$:'-0'}:
  Array.isArray(value)?Array.from({length:value.length},(_,i)=>Object.hasOwn(value,i)?plain(value[i]):{$:'hole'}):
  value&&typeof value==='object'?Object.fromEntries(Object.entries(value).map(([key,v])=>[key,plain(v)])):value;
function match(value){return value===null?null:{captures:plain(Array.from(value)),index:value.index,input:value.input,groups:plain(value.groups)};}
const records=[];
function observe(name,body){try{records.push({name,value:plain(body())});}catch(error){records.push({name,error:{name:error.name,message:error.message}});}}
function execute(pattern,flags,input,lastIndex=0,count=1){
  const expression=new RegExp(pattern,flags);expression.lastIndex=lastIndex;const calls=[];
  for(let i=0;i<count;i++){calls.push({match:match(expression.exec(input)),lastIndex:expression.lastIndex});}
  return{source:expression.source,flags:expression.flags,calls};
}
observe('dot-codepoint',()=>execute('.','gu','A\uD83D\uDE00B',0,4));
observe('dot-legacy-units',()=>execute('.','g','A\uD83D\uDE00B',0,5));
observe('sticky-inside-pair',()=>execute('.','uy','\uD83D\uDE00x',1,2));
observe('global-inside-pair',()=>execute('.','gu','\uD83D\uDE00x',1,2));
observe('nonglobal-lastindex',()=>execute('.','u','\uD83D\uDE00',1,2));
observe('capture-offsets',()=>execute('(\u{1F600})(.)','gu','x\uD83D\uDE00y',0,2));
observe('named-capture',()=>execute('(?<face>\u{1F600})(?<next>.)','u','\uD83D\uDE00x'));
observe('lone-surrogates',()=>execute('.','gu','\uD800A\uDC00',0,4));
observe('surrogate-class',()=>execute('[\\uD800-\\uDFFF]','gu','\uD83D\uDE00\uD800\uDC00\uD800',0,2));
observe('escaped-surrogate-pair',()=>execute('\\uD83D\\uDE00','u','\uD83D\uDE00'));
observe('escaped-codepoint',()=>execute('\\u{1F600}','u','x\uD83D\uDE00'));
observe('astral-range',()=>execute('[\\u{1F600}-\\u{1F64F}]+','u','a\uD83D\uDE00\uD83D\uDE4Fx'));
observe('quantified-codepoint',()=>execute('.{2}','u','\uD83D\uDE00x'));
observe('negated-class',()=>execute('[^x]+','u','\uD83D\uDE00x'));
observe('lookahead-capture-offset',()=>execute('(?=(.))','gu','\uD83D\uDE00x',1));
observe('empty-exec-keeps-index',()=>execute('(?:)','gu','\uD83D\uDE00x',1,2));
observe('split-empty-advances-codepoint',()=>({unicode:'\uD83D\uDE00x'.split(/(?:)/u),legacy:'\uD83D\uDE00x'.split(/(?:)/)}));
observe('split-captures',()=>({unicode:'A\uD83D\uDE00B'.split(/(.)/u)}));
observe('unicode-ignore-case',()=>execute('[a-z]+','iu','\u212A\u017Fx'));
observe('unicode-word',()=>execute('\\w+','iu','\u212A\u017Fx'));
observe('unicode-word-boundary',()=>execute('\\b\\u212A\\b','iu',' \u212A '));
observe('unicode-backreference',()=>execute('(.)\\1','iu','\u017FS'));
observe('dot-line-separators',()=>execute('.','gu','\r\n\u2028\u2029x',0,2));
observe('lastindex-out-of-range',()=>execute('.','uy','\uD83D\uDE00',3));
observe('lastindex-negative',()=>execute('.','gu','\uD83D\uDE00',-1));
observe('lastindex-fraction',()=>execute('.','gu','\uD83D\uDE00x',1.9));
for(const [name,pattern] of [['identity-escape','\\q'],['class-identity-escape','[\\-a]'],['decimal-without-capture','\\1'],['octal','\\01'],['out-of-range-codepoint','\\u{110000}'],['reversed-range','[z-a]'],['incomplete-escape','\\u{']])
  observe('syntax-'+name,()=>execute(pattern,'u','q'));
const packageRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
assert.equal(digest(path.join(packageRoot,'cjs/adaptors/NodeMixin.js')),'d331d73f054c46dfd3122c41bd6762389935b68c2e89bf1a4011cc70ea42ef3a');
assert.equal(digest(path.join(packageRoot,'cjs/adaptors/liteAdaptor.js')),'e9879de720f356cf8f54c0960102b1e40b4eafc1e34a14481aa896e1e99bd76f');
const adaptor=require(path.join(packageRoot,'cjs/adaptors/liteAdaptor.js')).liteAdaptor();
observe('actual-cjk-class',()=>execute(adaptor.constructor.cjkPattern.source,adaptor.constructor.cjkPattern.flags,'A\u{20000}\u{3FFFD}\u{3FFFE}\uD800',0,3));
assert.equal(digest(path.join(packageRoot,'cjs/adaptors/lite/Parser.js')),'38874dadf30ee0bd60184dde9bdf29a1be9d45755893b2a025d8e1b0190ad8c0');
const {PATTERNS}=require(path.join(packageRoot,'cjs/adaptors/lite/Parser.js'));
observe('actual-parser-tag-split',()=>({source:PATTERNS.tag.source,flags:PATTERNS.tag.flags,
  parts:'A<mi data-\u{1F600}="v">\u{1F600}</mi><!--x--><broken'.split(PATTERNS.tag)}));
observe('actual-parser-attr-match',()=>execute(PATTERNS.attr.source,PATTERNS.attr.flags,'data-\u{1F600}="v" id=two'));
observe('actual-parser-attrsplit',()=>({source:PATTERNS.attrsplit.source,flags:PATTERNS.attrsplit.flags,
  parts:'id="x" data-\u{1F600}="value"'.split(PATTERNS.attrsplit)}));
process.stdout.write(JSON.stringify({schema:1,node:process.version,v8:process.versions.v8,unicode:process.versions.unicode,sourceLocks,records},null,2)+'\n');
