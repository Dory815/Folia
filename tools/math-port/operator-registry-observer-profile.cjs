// Root freezes original generated expectations before narrowly scoped binding repairs.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),Module=require('node:module'),crypto=require('node:crypto');const root=path.resolve(__dirname,'../..'),packet=process.argv.find(x=>x.startsWith('--packet='))?.slice(9);assert.ok(['mo-values','mml-registry','mo'].includes(packet));
const file=path.join(__dirname,packet+'-oracle.cjs');let code=fs.readFileSync(file,'utf8').replaceAll('\r\n','\n');function inject(from,to){assert.equal(code.split(from).length,2,'Original anchor');code=code.replace(from,to);}
if(packet==='mo-values')code+='\nmodule.exports={assertions:count,cases,expectedBody:output.split("\\n").filter(s=>!s.startsWith("m::Children children(")).join("\\n")};\n';
if(packet==='mml-registry')code+='\nmodule.exports={assertion_slots:(cases.join("\\n").match(/\\bexpect\\(/g)||[]).length,cases:cases.length,names:cases.map(c=>c.match(/^"([^"]+)"_test/)[1]),expectedBody:v.declarations().join("\\n")+"\\n"+cases.join("\\n")};\n';
if(packet==='mo'){
 inject('let assertions=0,cases=0;', 'const sourceProfile=[];let assertions=0,cases=0;');
 inject('function begin(n){','function begin(n){sourceProfile.push(["case",n]);');
 inject('function eq(x,e){','function eq(x,e){sourceProfile.push(["eq",p(x)]);');
 inject('function pp(x,e){','function pp(x,e){sourceProfile.push(["property",p(x)]);');
 inject('if(error){emit(\'{bool caught=false;', 'sourceProfile.push(["attempt",error?error.name:"ok"]);if(error){emit(\'{bool caught=false;');
 code+='\nmodule.exports={assertions,cases,sourceProfile,states:states.map(s=>s.map(p)),rows:rows.map(r=>[r.key,r.form,r.id,r.extra.map(([k,v])=>[k,p(v)])])};\n';
}
const compiled=new Module(file,module);compiled.filename=file;compiled.paths=Module._nodeModulePaths(path.dirname(file));const args=process.argv;process.argv=[...args.filter(x=>x!=='--check'),'--check'];try{compiled._compile(code,file);}finally{process.argv=args;}
const data=JSON.stringify(compiled.exports,null,2)+'\n',out=path.join(root,'docs/math-port/evidence/p24-'+packet+'-observer-profile.json');if(args.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data,'Frozen original expectations changed');else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}
console.log(JSON.stringify({packet,assertions:compiled.exports.assertions??compiled.exports.assertion_slots,cases:compiled.exports.cases,sha256:crypto.createHash('sha256').update(data).digest('hex')}));
