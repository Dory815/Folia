'use strict';
// Pinned CJS witness for Source Stack.Push(MmlNode) semantics.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');
const cp=require('node:child_process');

const root=path.resolve(__dirname,'../../../..');
const packageRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const cjs=path.join(packageRoot,'cjs');
const sourceRoot=path.join(root,'.reference/MathJax-src');
const load=name=>require(path.join(cjs,name));
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const commit='fb987178c2d279a99b0db5ea02e751691703955e';
assert.equal(cp.execFileSync('git',['rev-parse','HEAD'],{cwd:sourceRoot,encoding:'utf8'}).trim(),commit);
const stackTs='ts/input/tex/Stack.ts';
const stackCjs='cjs/input/tex/Stack.js';
assert.equal(digest(path.join(sourceRoot,stackTs)),
  '36be7583b14111961367e9e3f1dd5911456d235b096a2fcee62f438d37c7e11a');
assert.equal(digest(path.join(packageRoot,stackCjs)),
  '78743f9dfcf1ebf9beb890629d908111be5c1e47f49c16d5ffa0877c5935c385');

const {mathjax}=load('mathjax.js');
const {TeX}=load('input/tex.js');
const {MmlFactory}=load('core/MmlTree/MmlFactory.js');
const {SVG}=load('output/svg.js');
const {liteAdaptor}=load('adaptors/liteAdaptor.js');
const {RegisterHTMLHandler}=load('handlers/html.js');
const {AbstractMathItem}=load('core/MathItem.js');
const Stack=load('input/tex/Stack.js').default;
const adaptor=liteAdaptor({fontSize:16});
RegisterHTMLHandler(adaptor);
const input=new TeX({packages:['base']});
const mmlFactory=new MmlFactory();
input.setMmlFactory(mmlFactory);
const document=mathjax.document('',{InputJax:input,OutputJax:new SVG()});
const mathItem=new AbstractMathItem('x',input,true);
const parsedRoot=input.compile(mathItem,document);
assert.equal(parsedRoot.kind,'math');
const stack=new Stack(input.parseOptions.itemFactory,{},false);
const stackArray=stack.stack;
const initialTop=stack.Top();
assert.equal(initialTop.kind,'start');

// The Source start item receives the MML item's First node and reports
// checkItem failure to the outer loop. Thus Push consumes the MML node into
// the start item's children; it does not append a second Stack entry.
stack.Push(parsedRoot);
const topAfterPush=stack.Top();
assert.equal(stack.stack,stackArray);
assert.equal(stack.stack.length,1);
assert.equal(topAfterPush,initialTop);
assert.equal(topAfterPush.First,parsedRoot);
const output={schema:1,source:{version:'4.1.3',commit,stackTs,stackTsSha256:digest(path.join(sourceRoot,stackTs)),
  stackCjs,stackCjsSha256:digest(path.join(packageRoot,stackCjs))},
  operation:'Stack.Push(compiled MmlNode)',before:{length:1,topKind:'start'},
  after:{length:stack.stack.length,topKind:topAfterPush.kind,topIsInitialTop:topAfterPush===initialTop,
    firstIsExactCompiledRoot:topAfterPush.First===parsedRoot,arrayIsOriginal:stack.stack===stackArray}};
const destination=process.argv[2]||path.join(__dirname,'bussproofs-mml-push-source.json');
fs.writeFileSync(destination,JSON.stringify(output,null,2)+'\n');
console.log(JSON.stringify({destination,after:output.after}));
