// Root freezes original MathChoice calls and their complete lossless value table.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),Module=require('node:module'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file=path.join(__dirname,'mathchoice-oracle.cjs');let code=fs.readFileSync(file,'utf8').replaceAll('\r\n','\n');
function encode(v){if(v===undefined)return['undefined'];if(v===null)return['null'];if(typeof v==='number'){const b=Buffer.alloc(8);b.writeDoubleLE(v);return['number',b.toString('hex')];}if(typeof v!=='object')return[typeof v,v];return[Array.isArray(v)?'array':'object',Array.isArray(v)?v.length:null,Object.keys(v).map(k=>[k,encode(v[k])])];}
function inject(from,to){assert.equal(code.split(from).length,2,'Original observer anchor');code=code.replace(from,to);}
inject('const cases=[];function add(',`const sourceProfile=[];const encode=${encode.toString()};const cases=[];function add(`);
inject('const wanted=observe(s,[raw,display,level,prime],count);','const wanted=observe(s,[raw,display,level,prime],count);sourceProfile.push(["call",name,count,shape,scenario,encode(wanted)]);');
inject('const wanted=tree(math);','const wanted=tree(math);sourceProfile.push(["real",display,level,encode(wanted)]);');
code+='\nmodule.exports={assertions:(output.match(/\\bexpect\\(/g)||[]).length,cases:cases.length,names:cases.map(c=>c.match(/^"([^"]+)"_test/)[1]),sourceProfile,expectedValues:v.declarations()};\n';
const compiled=new Module(file,module);compiled.filename=file;compiled.paths=Module._nodeModulePaths(path.dirname(file));const args=process.argv;process.argv=[...args.filter(x=>x!=='--check'),'--check'];try{compiled._compile(code,file);}finally{process.argv=args;}
const data=JSON.stringify(compiled.exports,null,2)+'\n',out=path.join(root,'docs/math-port/evidence/p23n-mathchoice-source-profile.json');if(args.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data,'Frozen original MathChoice observations changed');else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}
console.log(JSON.stringify({assertions:compiled.exports.assertions,cases:compiled.exports.cases,source_calls:compiled.exports.sourceProfile.length,sha256:crypto.createHash('sha256').update(data).digest('hex')}));
