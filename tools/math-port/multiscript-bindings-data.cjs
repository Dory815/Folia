// Root-generated source defaults and accessors; no node algorithm generation.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,MmlFactory,kinds,source,sha256}=require('./multiscript-reference.cjs');
const ts=require(path.join(root,'build/math-port-tooling/node_modules/typescript'));
assert.equal(ts.version,'5.9.3');
const ast=ts.createSourceFile(source,fs.readFileSync(path.join(root,'.reference/MathJax-src',source),'utf8'),ts.ScriptTarget.Latest,true);
const declarations=ast.statements.filter(ts.isClassDeclaration);assert.equal(declarations.length,3);
const out=['// Generated MathJax 4.1.3 source bindings; Apache-2.0.','// '+source+' SHA256 '+sha256,'module folia.math.core.mml_tree.mml_nodes.mmultiscripts;','import folia.math.core.mml_tree.multiscript_host;','import std;','namespace folia::math::mml {'];
for(let i=0;i<3;i++){
 const d=declarations[i],name=d.name.text,base=d.heritageClauses[0].types[0].expression.text,node=new MmlFactory().create(kinds[i]);
 const defaults=d.members.find(m=>m.name?.getText(ast)==='defaults');assert.equal(defaults.initializer.properties.length,1);assert.ok(ts.isSpreadAssignment(defaults.initializer.properties[0]));
 out.push('namespace {PropertyList '+name+'Defaults=host::arithmeticCopy('+base+'::defaults());}','PropertyList& '+name+'::defaults(){return '+name+'Defaults;}');
 for(const m of d.members.filter(ts.isGetAccessorDeclaration)){const key=m.name.text;assert.ok(['kind','arity'].includes(key));out.push((key==='kind'?'std::string':'double')+' '+name+'::'+key+'()const{return '+(key==='kind'?JSON.stringify(node[key]):node[key]+'.0')+';}');}
}
out.push('}');const output=out.join('\n')+'\n',target=path.join(root,'src/math/core/mml_tree/mml_nodes/mmultiscripts_bindings.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
console.log(JSON.stringify({default_copies:3,accessors:6,mode:process.argv.includes('--check')?'check':'generate'}));
