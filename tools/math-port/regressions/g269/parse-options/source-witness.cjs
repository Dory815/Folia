'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../../../../..');
const sr=path.join(root,'.reference/MathJax-src'),pr=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const source='ts/input/tex/ParseOptions.ts', bytes=fs.readFileSync(path.join(sr,source));
const sourceSha=crypto.createHash('sha256').update(bytes).digest('hex');
assert.equal(sourceSha,'ab60a07dbb24040c0fb9bd67b942755ffc5b0b18980abf1938736d8d90b22209');
assert.equal(bytes.toString('utf8').replaceAll('\r\n','\n'),fs.readFileSync(path.join(pr,source),'utf8').replaceAll('\r\n','\n'));
const cjs=path.join(pr,'cjs');
const {TeX}=require(path.join(cjs,'input/tex.js'));
const {MmlFactory}=require(path.join(cjs,'core/MmlTree/MmlFactory.js'));
const TexParser=require(path.join(cjs,'input/tex/TexParser.js')).default;
function make(){const jax=new TeX({packages:['base']});jax.setMmlFactory(new MmlFactory());return jax.parseOptions;}
function node(kind){return make().nodeFactory.mmlFactory.create(kind);}
const p=make();
const parser=new TexParser('',{},p);const parserArray=p.parsers;const constructorPushedSelf=p.parser===parser;parser.mml();const emptyAfterMml=p.parsers.length===0;p.pushParser(parser);const parserHead=p.parser===parser;p.popParser();
const rootNode=node('mrow'), live=node('mi'), detached=node('mn');rootNode.appendChild(live);p.root=rootNode;
p.addNode('custom',live);p.addNode('custom',detached);p.addNode('custom',live);
const afterAdd={keys:Object.keys(p.nodeLists),customKinds:p.nodeLists.custom.map(n=>n.kind),liveProperty:live.getProperty('in-lists'),detachedProperty:detached.getProperty('in-lists')};
const filtered=p.getList('custom');
const filteredRecord={length:filtered.length,allLive:filtered.every(n=>n===live),nodeListsSameArray:p.nodeLists.custom===filtered};
p.removeFromList('custom',[live]);
const duplicateRemoval={length:p.nodeLists.custom.length,remainingIsLive:p.nodeLists.custom[0]===live};
const replacement=[detached,live];p.nodeLists.custom=replacement;p.removeFromList('custom',[detached]);
const replacementRemoval={sameArray:p.nodeLists.custom===replacement,length:replacement.length,remainingIsLive:replacement[0]===live};
const parentCases={rootIsInTree:p.inTree(rootNode),descendantIsInTree:p.inTree(live),detachedIsInTree:p.inTree(detached)};
const c=make(), keepOptions=c.options, keepPackageData=c.packageData, keepNodeFactory=c.nodeFactory, keepHandlers=c.handlers;
const beforeClear={root:c.root,parsers:c.parsers,nodeLists:c.nodeLists,error:c.error};
const clearParser=new TexParser('',{},c);clearParser.mml();assert.equal(c.parsers.length,0);c.pushParser(clearParser);c.root=rootNode;c.nodeLists={stale:[live]};c.error=true;c.clear();
const clearRetains={options:c.options===keepOptions,packageData:c.packageData===keepPackageData,nodeFactory:c.nodeFactory===keepNodeFactory,handlers:c.handlers===keepHandlers};
const clearResets={parserArrayReplaced:c.parsers!==beforeClear.parsers,parsersEmpty:c.parsers.length===0,rootNull:c.root===null,nodeListsEmpty:Object.keys(c.nodeLists).length===0,errorFalse:c.error===false,clearRetains};
const failure=make();const failureParser=new TexParser('',{},failure);failureParser.mml();assert.equal(failure.parsers.length,0);failure.pushParser(failureParser);failure.root=rootNode;failure.nodeLists={stale:[live]};failure.error=true;
const tags=failure.tags;let resetGetterReads=0;Object.defineProperty(tags,'resetTag',{configurable:true,get(){resetGetterReads++;throw new Error('reset-witness');}});
let clearError;try{failure.clear();}catch(e){clearError={name:e.name,message:e.message};}
const clearThrowAfterWrites={error:clearError,resetGetterReads,parsersEmpty:failure.parsers.length===0,rootNull:failure.root===null,nodeListsEmpty:Object.keys(failure.nodeLists).length===0,errorFalse:failure.error===false};
const result={source:{path:source,sha256:sourceSha},observed:{parser:{constructorPushedSelf,emptyAfterMml,sameArray: p.parsers===parserArray,headWasPushed:parserHead,emptyAfterPop:p.parsers.length===0},afterAdd,filtered:filteredRecord,duplicateRemoval,replacementRemoval,parentCases,clearResets,clearThrowAfterWrites}};
assert.deepEqual(result.observed,{parser:{constructorPushedSelf:true,emptyAfterMml:true,sameArray:true,headWasPushed:true,emptyAfterPop:true},afterAdd:{keys:['inferredMrow','custom'],customKinds:['mi','mn','mi'],liveProperty:'custom,custom',detachedProperty:'custom'},filtered:{length:2,allLive:true,nodeListsSameArray:true},duplicateRemoval:{length:1,remainingIsLive:true},replacementRemoval:{sameArray:true,length:1,remainingIsLive:true},parentCases:{rootIsInTree:true,descendantIsInTree:true,detachedIsInTree:false},clearResets:{parserArrayReplaced:true,parsersEmpty:true,rootNull:true,nodeListsEmpty:true,errorFalse:true,clearRetains:{options:true,packageData:true,nodeFactory:true,handlers:true}},clearThrowAfterWrites:{error:{name:'Error',message:'reset-witness'},resetGetterReads:1,parsersEmpty:true,rootNull:true,nodeListsEmpty:true,errorFalse:true}});
fs.writeFileSync(path.join(__dirname,'source-witness.json'),JSON.stringify(result,null,2)+'\n');
process.stdout.write(JSON.stringify(result,null,2)+'\n');


// A separate JSON artifact is retained beside the source witness for review.
