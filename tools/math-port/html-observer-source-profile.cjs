// Root freezes original HtmlNode observations before repairing native bindings.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),Module=require('node:module'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file=path.join(__dirname,'html-node-oracle.cjs');
let code=fs.readFileSync(file,'utf8').replaceAll('\r\n','\n');
function inject(from,to){assert.equal(code.split(from).length,2,'Original observer anchor');code=code.replace(from,to);}
inject('const tests=[];let cases=0,assertions=0;','const sourceProfile=[];const tests=[];let cases=0,assertions=0;');
inject('function begin(name){','function begin(name){sourceProfile.push(["case",name]);');
inject('function eq(expected,actual){','function eq(expected,actual){sourceProfile.push(["eq",typeof expected,expected]);');
inject("function prop(expected,actual){++assertions;emit('expect(deep('+actual+','+p(expected)+'));');}","function prop(expected,actual){++assertions;const rendered=p(expected);sourceProfile.push(['property',rendered]);emit('expect(deep('+actual+','+rendered+'));');}");
inject('const result=caught(fn);emit(', 'const result=caught(fn);sourceProfile.push(["result",result.error,typeof result.result,typeof result.result===\'object\'?null:result.result]);emit(');
code+='\nmodule.exports={assertions,cases,sourceProfile,expectedValues:values.declarations()};\n';
const compiled=new Module(file,module);compiled.filename=file;compiled.paths=Module._nodeModulePaths(path.dirname(file));
const args=process.argv;process.argv=[...args.filter(x=>x!=='--check'),'--check'];try{compiled._compile(code,file);}finally{process.argv=args;}
const data=JSON.stringify(compiled.exports,null,2)+'\n',out=path.join(root,'docs/math-port/evidence/p23m-html-source-profile.json');
if(args.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data,'Frozen original HtmlNode observations changed');else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}
console.log(JSON.stringify({assertions:compiled.exports.assertions,cases:compiled.exports.cases,records:compiled.exports.sourceProfile.length,sha256:crypto.createHash('sha256').update(data).digest('hex')}));
