'use strict';
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const root=path.resolve(__dirname,'../../../mathjax-bundle/node_modules/@mathjax/src');
const pkg=JSON.parse(fs.readFileSync(path.join(root,'package.json'),'utf8'));
assert.equal(pkg.version,'4.1.3');
const rel='input/tex/StackItem.js';
const expected='53baa91d7c27475a91a9b6993235071ab32b04a29736c5920bdda2836ab5a91b';
const source=fs.readFileSync(path.join(root,'cjs',rel));
assert.equal(crypto.createHash('sha256').update(source).digest('hex'),expected);
const {MmlStack}=require(path.join(root,'cjs',rel));
const first=Object.getOwnPropertyDescriptor(MmlStack.prototype,'First');
const last=Object.getOwnPropertyDescriptor(MmlStack.prototype,'Last');
const stack={_nodes:[],trace:[],get nodes(){this.trace.push('nodes');throw Error('unexpected nodes accessor');},
  Size(){this.trace.push('Size');return MmlStack.prototype.Size.call(this);},
  create(...args){this.trace.push('create:'+args[0]+':'+args[1]);this.created=args;return 'created';}};
Object.defineProperty(stack,'First',{get(){this.trace.push('First');return first.get.call(this);},set(value){this.trace.push('setFirst');return first.set.call(this,value);}});
MmlStack.prototype.Push.call(stack,'mi','mn');
first.set.call(stack,'op');
last.set.call(stack,'mi');
const top=first.get.call(stack);
const bottom=last.get.call(stack);
const peek=MmlStack.prototype.Peek.call(stack,2);
const popped=MmlStack.prototype.Pop.call(stack);
const size=stack.Size();
const row=MmlStack.prototype.toMml.call(stack);
const forced=MmlStack.prototype.toMml.call(stack,false,true);
const old=stack._nodes;
MmlStack.prototype.Clear.call(stack);
assert.deepEqual({top,bottom,peek,popped,size,row,forced,old,current:stack._nodes,trace:stack.trace},
  {top:'op',bottom:'mi',peek:['mi','op'],popped:'op',size:1,row:'mi',forced:'created',old:['mi'],current:[],
   trace:['Size','Size','Size','Size','First','Size','create:node:mrow']});
assert.equal(stack.trace.includes('nodes'),false,'Source methods read _nodes without consulting nodes getter');
assert.deepEqual(stack.created.slice(0,3),['node','mrow',['mi']]);
process.stdout.write(JSON.stringify({version:pkg.version,hashes:{[rel]:expected},methods:['Push','Pop','First','Last','Peek','Size','Clear','toMml'],nodesGetterReads:0,oldAlias:old,current:stack._nodes,ok:true})+'\n');
