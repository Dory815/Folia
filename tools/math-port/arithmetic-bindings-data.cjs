// Root generated static records and constant getters from actual source classes.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,MmlFactory,kinds,classes,hashes}=require('./arithmetic-reference.cjs');
const ts=require(path.join(root,'build/math-port-tooling/node_modules/typescript'));assert.equal(ts.version,'5.9.3');
const q=JSON.stringify,p=v=>typeof v==='string'?q(v):typeof v==='boolean'?String(v):typeof v==='number'?String(v)+'.0':(()=>{throw Error('Unbound static source value');})();
let fields=0,accessors=0;
for(const kind of kinds){const hash=hashes.find(x=>x.source.endsWith('/'+kind+'.ts')),source=ts.createSourceFile(kind+'.ts',fs.readFileSync(path.join(root,'.reference/MathJax-src',hash.source),'utf8'),ts.ScriptTarget.Latest,true),cls=source.statements.find(ts.isClassDeclaration),name=cls.name.text,base=cls.heritageClauses[0].types[0].expression.text;
 const defaults=cls.members.find(x=>x.name?.getText(source)==='defaults').initializer.properties.filter(ts.isPropertyAssignment).map(x=>x.name.text);
 const n=new MmlFactory().create(kind);
 const out=['// Generated root source bindings; MathJax 4.1.3, Apache-2.0.','// '+hash.source+' SHA256 '+hash.sha256,'module folia.math.core.mml_tree.mml_nodes.'+kind+';','import folia.math.core.mml_tree.arithmetic_host;','import std;','namespace folia::math::mml {namespace {','PropertyList sourceDefaults=[] {auto value=host::arithmeticCopy('+base+'::defaults());'];
 for(const key of defaults)out.push('value.setOwn('+q(key)+','+p(classes[kind].defaults[key])+');');fields+=defaults.length;
 out.push('return value;}();','}','PropertyList& '+name+'::defaults(){return sourceDefaults;}');
 if(cls.members.some(x=>ts.isPropertyDeclaration(x)&&x.name?.getText(source)==='texclass')){assert.equal(n.texClass,0);out.push('void '+name+'::initializeClassFields(){initializeTexClass(TEXCLASS.ORD);}');}
 for(const member of cls.members.filter(ts.isGetAccessorDeclaration)){const key=member.name.text,cppType=key==='kind'||key==='linebreakAlign'?'std::string':key==='arity'?'double':'bool';assert.ok(['kind','arity','linebreakContainer','linebreakAlign'].includes(key));out.push(cppType+' '+name+'::'+key+'()const{return '+p(n[key])+';}');accessors++;}
 out.push('}');const output=out.join('\n')+'\n',target=path.join(root,'src/math/core/mml_tree/mml_nodes',kind+'_bindings.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
}
console.log(JSON.stringify({nodes:3,extra_default_fields:fields,constant_accessors:accessors,mode:process.argv.includes('--check')?'check':'generate'}));
