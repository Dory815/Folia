'use strict';
// Actual pinned synchronous Source utilities, without a parser/adaptor stub.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const source = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
assert.equal(JSON.parse(fs.readFileSync(path.join(source,'package.json'))).version,'4.1.3');
const locks = {
  '.reference/MathJax-src/ts/util/PrioritizedList.ts': 'a700459f984ba1a5f18bf0e7dc183c242961b94c33587a9aa81bbaacf97180c8',
  '.reference/MathJax-src/ts/util/FunctionList.ts': 'd4106c42268e0d7cb3549f8ffdbeb5bab87eae6daffae7314d5e724045a6918f',
  'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/util/PrioritizedList.js': '787bd21f3634312af666917d8f140145864f98d09ede1bdde3c957d28ec2bc25',
  'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/util/FunctionList.js': '571f57801b863b2d9f01a66dc72f2836b7fd06a1a48a3fe0057e4ec3d51a7278',
};
for(const [file, hash] of Object.entries(locks))
  assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'), hash, file);
const {PrioritizedList} = require(path.join(source,'cjs/util/PrioritizedList.js'));
const {FunctionList} = require(path.join(source,'cjs/util/FunctionList.js'));
const actual = {};
const ordered = list => Array.from(list, x => x.item);
{
  const list = new PrioritizedList();
  list.add('a'); list.add('b',1); list.add('c',5); list.add('d',-2); list.add('e',1);
  actual.stable = ordered(list);
  list.add('a',8); assert.equal(list.remove('a'),list); list.remove('missing');
  actual.removeLast = ordered(list);
  const numeric = new PrioritizedList();
  for(const [item,priority] of [['a',5],['n',NaN],['l',-10],['i',Infinity],['m',-Infinity]]) numeric.add(item,priority);
  actual.numeric = ordered(numeric);
}
for(const mode of ['ordered','stop','append','remove-current','remove-next','insert-before','throw']) {
  const list = new FunctionList(), calls = [];
  const a = () => {calls.push('a');}, b = () => {calls.push('b');}, c = () => {calls.push('c');};
  let result, error;
  if(mode === 'ordered') {
    list.add(a); list.add(() => {calls.push('continue'); return true;},1); list.add(c);
  } else if(mode === 'stop') {
    list.add(a); list.add(() => {calls.push('stop'); return false;},0);
  } else if(mode === 'append') {
    list.add(() => {calls.push('append'); list.add(c,10);},1); list.add(b,5);
  } else if(mode === 'remove-current') {
    const f = () => {calls.push(mode); list.remove(f);};
    list.add(f,1); list.add(b,2); list.add(c,3);
  } else if(mode === 'remove-next') {
    list.add(() => {calls.push(mode); list.remove(b);},1); list.add(b,2); list.add(c,3);
  } else if(mode === 'insert-before') {
    let inserted = false;
    list.add(() => {calls.push(mode); if(!inserted) {inserted=true; list.add(a,0);}},1); list.add(b,5);
  } else {
    list.addList([[a,5],[() => {calls.push('throw'); throw Error('processor failure');},1],[c,5]]);
  }
  try { result = list.execute(); } catch(e) { error=e.message; }
  actual[mode] = {calls, result:result ?? null, error:error ?? null, count:Array.from(list).length};
}
assert.deepEqual(actual, {
  stable:['d','b','e','a','c'], removeLast:['d','b','e','a','c'], numeric:['a','n','m','l','i'],
  ordered:{calls:['continue','a','c'],result:true,error:null,count:3},
  stop:{calls:['stop'],result:false,error:null,count:2},
  append:{calls:['append','b','c'],result:true,error:null,count:3},
  'remove-current':{calls:['remove-current','c'],result:true,error:null,count:2},
  'remove-next':{calls:['remove-next','c'],result:true,error:null,count:2},
  'insert-before':{calls:['insert-before','insert-before','b'],result:true,error:null,count:3},
  throw:{calls:['throw'],result:null,error:'processor failure',count:3}
});
console.log(JSON.stringify({status:'passed',observations:Object.keys(actual).length,sourceFiles:4,
  limit:'Synchronous configuration/filter list behavior. No parser integration or async DOM processing is claimed.'}));
