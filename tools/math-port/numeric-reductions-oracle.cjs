'use strict';
// Root independent actual pinned Source numeric reductions.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),base=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/util/numeric.js');
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(base)).digest('hex'),'ea8a2810b86707438361be533cc80b9566eb685619abff81990befb3f0fb5b50');
const{sum,max}=require(base),buffer=new ArrayBuffer(8),view=new DataView(buffer);
function bits(x){view.setFloat64(0,x,false);return view.getBigUint64(0,false).toString(16).padStart(16,'0');}
function number(s){view.setBigUint64(0,BigInt('0x'+s),false);return view.getFloat64(0,false);}
function value(x){return Number.isNaN(x)?{nan:true}:{bits:bits(x)};}
const arrays=[[],[-0],[0],[-0,-0],[-1,-2],[-Infinity],[Infinity],[-Infinity,Infinity],[1,NaN,2],[NaN,Infinity],[1e16,-1e16,1],[1,1e16,-1e16],[1e308,1e308,-1e308],[Number.MIN_VALUE,Number.MIN_VALUE,-Number.MIN_VALUE],[1,-1,-0],[-1,1,-0],[1.1,2.2,3.3]];
let seed=0x6e756d65;function next(){seed^=seed<<13;seed^=seed>>>17;seed^=seed<<5;return seed>>>0;}
for(let i=0;i<128;++i){const a=[];for(let j=0,n=next()%24;j<n;++j){let word=(BigInt(next())<<32n)|BigInt(next());a.push(number(word.toString(16).padStart(16,'0')));}arrays.push(a);}
const cases=arrays.map((a,id)=>({id,input:a.map(bits),sum:value(sum(a)),max:value(max(a))}));
const report={stage:'actual Source dense numeric reductions',source:'a2004a9888f5c85f3cf81126e43d04dfe66b64c55e3b5f7d97366631809d7c15',cases};
const code='// Generated actual Source binary64 observations; do not hand edit.\nconst std::vector<NumericCase> source_cases{\n'+cases.map(c=>`{{${c.input.map(b=>'0x'+b+'ULL').join(',')}},${c.sum.nan?'true,0ULL':'false,0x'+c.sum.bits+'ULL'},${c.max.nan?'true,0ULL':'false,0x'+c.max.bits+'ULL'}},`).join('\n')+'\n};\n';
for(const[f,text]of [['docs/math-port/evidence/p20c-numeric-reductions-source.json',JSON.stringify(report,null,2)+'\n'],['tests/math/numeric_reductions_source.inc',code]]){
 const target=path.join(root,f);if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),text);else fs.writeFileSync(target,text);
}
console.log(JSON.stringify({cases:cases.length,source_assertions:cases.length*2,mode:process.argv.includes('--check')?'check':'freeze'}));
