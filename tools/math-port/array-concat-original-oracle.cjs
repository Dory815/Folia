'use strict';
// Observe the platform's actual Array.prototype.concat algorithm.
const fs=require('node:fs');
const path=require('node:path');
const rows=[];
function observe(name,run){
  const trace=[];let result='THREW',error='ok';
  try{result=run(trace);}catch(e){error=e&&e.name||'Error';result=e&&e.message||'';}
  rows.push({name,error,result,trace});
}
function arrayShape(a){return Array.from({length:a.length},(_,i)=>Object.hasOwn(a,i)?String(a[i]):'H').join(',');}
observe('metadata',()=>{const d=Object.getOwnPropertyDescriptor(Array.prototype,'concat');return `${Array.prototype.concat.name}:${Array.prototype.concat.length}:${d.writable}:${d.enumerable}:${d.configurable}`;});
observe('symbol-is-concat-spreadable-metadata',()=>{const d=Object.getOwnPropertyDescriptor(Symbol,'isConcatSpreadable');return `${typeof Symbol.isConcatSpreadable}:${d.writable}:${d.enumerable}:${d.configurable}`;});
observe('one-level-spread',()=>arrayShape([[1,2],3].concat([4,[5]])));
observe('ordinary-object-not-spread',()=>{const x={0:'x',length:1};return [1].concat(x).map(v=>Array.isArray(v)?'array':String(v)).join(',');});
observe('symbol-true-spreads-object',t=>{const x={length:3,0:'a',2:'c',[Symbol.isConcatSpreadable]:true};Object.defineProperty(x,'0',{get(){t.push('get0');return 'a';},configurable:true});return [1].concat(x).map(v=>String(v)).join(',');});
observe('symbol-false-keeps-array-whole',()=>{const x=[2,3];x[Symbol.isConcatSpreadable]=false;const r=[1].concat(x);return `${r.length}:${r[0]}:${r[1]===x}`;});
observe('holes-preserved',()=>{const a=[];a.length=3;a[1]='b';const r=[0].concat(a);return `${r.length}:${[0,1,2,3].map(i=>Object.hasOwn(r,i)?'Y':'N').join('')}:${r[2]}`;});
observe('inherited-hole-gets-live',t=>{const p=Object.create(Array.prototype);Object.defineProperty(p,'0',{get(){t.push('get0');return 'p';},configurable:true});const a=[];a.length=1;Object.setPrototypeOf(a,p);const r=[].concat(a);return `${r.length}:${r[0]}:${Object.hasOwn(r,0)}`;});
observe('species-before-spreadable-get',t=>{const a=[1];Object.defineProperty(a,'constructor',{get(){t.push('constructor');function C(){};Object.defineProperty(C,Symbol.species,{get(){t.push('species');return null;}});return C;}});Object.defineProperty(a,Symbol.isConcatSpreadable,{get(){t.push('spreadable');return true;}});return arrayShape(a.concat(2));});
observe('spreadable-length-before-index-has-get',t=>{const x={get length(){t.push('length');return 2;},[Symbol.isConcatSpreadable]:true};Object.defineProperty(x,'1',{get(){t.push('get1');return 'b';},configurable:true});const p={};Object.setPrototypeOf(x,p);Object.defineProperty(p,'0',{get(){t.push('get0');return 'a';},configurable:true});return arrayShape([].concat(x));});
observe('nonarray-receiver-to-object',()=>{const receiver={0:'x',length:1,[Symbol.isConcatSpreadable]:true};const r=Array.prototype.concat.call(receiver,[2]);return arrayShape(r);});
observe('primitive-string-receiver-boxed',()=>{const r=Array.prototype.concat.call('abc');return `${r.length}:${Object.prototype.toString.call(r[0])}:${r[0].valueOf()}`;});
observe('primitive-number-receiver-boxed',()=>{const r=Array.prototype.concat.call(42);return `${r.length}:${Object.prototype.toString.call(r[0])}:${r[0].valueOf()}`;});
observe('primitive-boolean-receiver-boxed',()=>{const r=Array.prototype.concat.call(false);return `${r.length}:${Object.prototype.toString.call(r[0])}:${r[0].valueOf()}`;});
observe('primitive-symbol-receiver-boxed',()=>{const symbol=Symbol('x'),r=Array.prototype.concat.call(symbol);return `${r.length}:${Object.prototype.toString.call(r[0])}:${r[0].valueOf()===symbol}`;});
observe('receiver-null-throws',()=>Array.prototype.concat.call(null));
observe('spreadability-getter-throws',t=>{const x=[];Object.defineProperty(x,Symbol.isConcatSpreadable,{get(){t.push('spreadable');throw new TypeError('spread');}});return [].concat(x);});
observe('result-identity-retained',()=>{const x={};const r=[1].concat(x);return `${r[1]===x}:${r[0]}`;});
observe('result-define-bypasses-array-prototype-setter',()=>{const old=Object.getOwnPropertyDescriptor(Array.prototype,'0');let setter=0;Object.defineProperty(Array.prototype,'0',{set(){setter++;},configurable:true});try{const r=[].concat([7]);const d=Object.getOwnPropertyDescriptor(r,'0');return `${d.value}:${d.writable}:${d.enumerable}:${d.configurable}:${Object.hasOwn(r,0)}:${setter}`;}finally{if(old)Object.defineProperty(Array.prototype,'0',old);else delete Array.prototype[0];}});
observe('zero-length-still-reads-species',t=>{const a=[];Object.defineProperty(a,'constructor',{get(){t.push('constructor');return {[Symbol.species]:null};}});return a.concat().length;});
observe('number-primitive-not-spread',()=>{Number.prototype[Symbol.isConcatSpreadable]=true;try{const r=[].concat(5);return `${r.length}:${r[0]}`;}finally{delete Number.prototype[Symbol.isConcatSpreadable];}});
observe('nested-array-spreads-one-level',()=>{const r=[1].concat([[2,[3]]]);return `${r.length}:${Array.isArray(r[1])}:${r[1][1][0]}`;});
observe('custom-array-species',()=>{const a=[1];function Species(length){this.length=length;}a.constructor={[Symbol.species]:Species};const result=a.concat([2]);return `${Array.isArray(result)}:${result.length}:${result[0]}:${result[1]}`;});
observe('object-length-to-primitive-order',t=>{const source={[Symbol.isConcatSpreadable]:true,get length(){t.push('length');return {[Symbol.toPrimitive](){t.push('toPrimitive');return 1;}}},get 0(){t.push('get0');return 'x';}};return arrayShape([].concat(source));});
const output={node:process.version,method:'Array.prototype.concat',rows};
const target=path.resolve(process.argv[2]||'build/math-port/array-concat-original.json');
fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,JSON.stringify(output,null,2)+'\n');
console.log(JSON.stringify({observations:rows.length,node:process.version,path:target}));
