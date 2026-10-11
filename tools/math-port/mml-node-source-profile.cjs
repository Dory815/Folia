// Root records actual original Source helper observations independently of native expressions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),Module=require('node:module'),crypto=require('node:crypto');const root=path.resolve(__dirname,'../..'),file=path.join(__dirname,'mml-node-oracle.cjs');
let code=fs.readFileSync(file,'utf8').replaceAll('\r\n','\n');
function inject(from,to){assert.equal(code.split(from).length,2);code=code.replace(from,to);}
inject('let assertions=0,cases=0;','const sourceProfile=[];let assertions=0,cases=0;');
inject('function begin(name){','function begin(name){sourceProfile.push(["case",name]);');
inject('function eq(v,expression){','function eq(v,expression){sourceProfile.push(["eq",p(v)]);');
inject('function pp(v,e){','function pp(v,e){sourceProfile.push(["property",p(v)]);');
inject('function list(v,e){','function list(v,e){sourceProfile.push(["list",v]);');
inject("assert.ok(err,'Expected source exception');","assert.ok(err,'Expected source exception');sourceProfile.push(['exception',err.name]);");
code+='\nmodule.exports={assertions,cases,sourceProfile};\n';
const compiled=new Module(file,module);compiled.filename=file;compiled.paths=Module._nodeModulePaths(path.dirname(file));
const originalArgs=process.argv;process.argv=[...originalArgs.filter(x=>x!=='--check'),'--check'];try{compiled._compile(code,file);}finally{process.argv=originalArgs;}
const data=JSON.stringify(compiled.exports,null,2)+'\n',out=path.join(root,'docs/math-port/evidence/p23k-mml-node-source-profile.json');
if(originalArgs.includes('--check')){const expected=fs.readFileSync(out,'utf8').replaceAll('\r\n','\n');if(expected!==data){const old=JSON.parse(expected),now=compiled.exports,index=now.sourceProfile.findIndex((r,i)=>JSON.stringify(r)!==JSON.stringify(old.sourceProfile[i]));assert.fail('Frozen Source profile changed: first record '+index+' / '+JSON.stringify(old.sourceProfile[index])+' -> '+JSON.stringify(now.sourceProfile[index]));}}else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}
console.log(JSON.stringify({assertions:compiled.exports.assertions,cases:compiled.exports.cases,helper_observations:compiled.exports.sourceProfile.length-42,sha256:crypto.createHash('sha256').update(data).digest('hex')}));
