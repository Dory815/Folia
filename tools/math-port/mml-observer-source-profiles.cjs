// Root records locked original helper observations before changing native bindings.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),Module=require('node:module'),crypto=require('node:crypto');const root=path.resolve(__dirname,'../..'),packet=process.argv.find(x=>x.startsWith('--packet='))?.slice(9);assert.ok(['script','leaf'].includes(packet),'Select frozen script or leaf packet');
const file=path.join(__dirname,packet+'-oracle.cjs');let code=fs.readFileSync(file,'utf8').replaceAll('\r\n','\n');function inject(from,to){assert.equal(code.split(from).length,2,'Original observer anchor');code=code.replace(from,to);}
inject('let assertions=0,cases=0;','const sourceProfile=[];let assertions=0,cases=0;');
inject('function begin(name){','function begin(name){sourceProfile.push(["case",name]);');
if(packet==='script'){
 inject('function eq(x,e){','function eq(x,e){sourceProfile.push(["eq",p(x)]);');
 inject('function pp(x,e){','function pp(x,e){sourceProfile.push(["property",p(x)]);');
 inject('catch(x){error=x;}if(error){','catch(x){error=x;}sourceProfile.push(["result",kind,error?[error.name,error.message]:Array.isArray(v)?v.map(p):p(v)]);if(error){');
 inject('assert.ok(error);assertions+=2;','assert.ok(error);sourceProfile.push(["exception",error.name]);assertions+=2;');
}else{
 inject('function eq(v,e){','function eq(v,e){sourceProfile.push(["eq",p(v)]);');
 inject('function pp(v,e){','function pp(v,e){sourceProfile.push(["property",p(v)]);');
 inject('function list(v,e){','function list(v,e){sourceProfile.push(["list",v]);');
 inject("assert.ok(error,'Reference must throw');","assert.ok(error,'Reference must throw');sourceProfile.push(['exception',error.name]);");
}
code+='\nmodule.exports={assertions,cases,referenceAssertions:typeof referenceAssertions===\'undefined\'?null:referenceAssertions,referenceCases:typeof referenceCases===\'undefined\'?null:referenceCases,sourceProfile};\n';
const compiled=new Module(file,module);compiled.filename=file;compiled.paths=Module._nodeModulePaths(path.dirname(file));const originalArgs=process.argv;process.argv=[...originalArgs.filter(x=>x!=='--check'),'--check'];try{compiled._compile(code,file);}finally{process.argv=originalArgs;}
const data=JSON.stringify(compiled.exports,null,2)+'\n',out=path.join(root,'docs/math-port/evidence/p23l-'+packet+'-source-profile.json');
if(originalArgs.includes('--check')){const expected=fs.readFileSync(out,'utf8').replaceAll('\r\n','\n');if(expected!==data){const old=JSON.parse(expected),now=compiled.exports,index=now.sourceProfile.findIndex((r,i)=>JSON.stringify(r)!==JSON.stringify(old.sourceProfile[i]));assert.fail('Frozen Source profile changed: first record '+index+' / '+JSON.stringify(old.sourceProfile[index])+' -> '+JSON.stringify(now.sourceProfile[index]));}}else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}
console.log(JSON.stringify({packet,assertions:compiled.exports.assertions,cases:compiled.exports.cases,helper_observations:compiled.exports.sourceProfile.filter(r=>r[0]!=='case').length,sha256:crypto.createHash('sha256').update(data).digest('hex')}));
