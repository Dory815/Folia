// Root generated constants and derived fields from pinned table source classes.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,MmlFactory,classes,hashes}=require('./table-reference.cjs');
const ts=require(path.join(root,'build/math-port-tooling/node_modules/typescript'));assert.equal(ts.version,'5.9.3');
const q=JSON.stringify,p=v=>typeof v==='string'?q(v):typeof v==='boolean'?String(v):typeof v==='number'?String(v)+'.0':(()=>{throw Error('Unbound source constant');})();
let fields=0,accessors=0;
for(const item of hashes){const kind=item.source.match(/([^/]+)\.ts$/)[1],source=ts.createSourceFile(kind+'.ts',fs.readFileSync(path.join(root,'.reference/MathJax-src',item.source),'utf8'),ts.ScriptTarget.Latest,true),declarations=source.statements.filter(ts.isClassDeclaration);
 const out=['// Generated MathJax 4.1.3 source bindings; Apache-2.0.','// '+item.source+' SHA256 '+item.sha256,'module folia.math.core.mml_tree.mml_nodes.'+kind+';','import folia.math.core.mml_tree.table_host;','import std;','namespace folia::math::mml {'];
 for(const declaration of declarations){const name=declaration.name.text,base=declaration.heritageClauses[0].types[0].expression.text,sourceKind=name==='MmlMlabeledtr'?'mlabeledtr':kind,T=classes[sourceKind],node=new MmlFactory().create(sourceKind),defaults=declaration.members.find(m=>m.name?.getText(source)==='defaults');
  if(defaults){out.push('namespace {PropertyList '+name+'Defaults=[] {auto value=host::arithmeticCopy('+base+'::defaults());');for(const key of defaults.initializer.properties.filter(ts.isPropertyAssignment).map(x=>x.name.text)){out.push('value.setOwn('+q(key)+','+p(T.defaults[key])+');');fields++;}out.push('return value;}();}','PropertyList& '+name+'::defaults(){return '+name+'Defaults;}');}
  const initialized=declaration.members.filter(m=>ts.isPropertyDeclaration(m)&&!m.modifiers?.some(x=>x.kind===ts.SyntaxKind.StaticKeyword));
  if(initialized.length){assert.equal(kind,'mtable');assert.deepEqual(initialized.map(m=>m.name.text),['properties','texclass']);assert.deepEqual(node.properties,{useHeight:true});assert.equal(node.texClass,0);out.push('void '+name+'::initializeClassFields(){properties=PropertyList{{"useHeight",true}};initializeTexClass(TEXCLASS.ORD);}');}
  for(const member of declaration.members.filter(ts.isGetAccessorDeclaration)){const key=member.name.text,type=key==='kind'||key==='linebreakAlign'?'std::string':key==='arity'?'double':'bool';assert.ok(['kind','arity','linebreakContainer','linebreakAlign'].includes(key));out.push(type+' '+name+'::'+key+'()const{return '+p(node[key])+';}');accessors++;}
 }
 out.push('}');const output=out.join('\n')+'\n',target=path.join(root,'src/math/core/mml_tree/mml_nodes',kind+'_bindings.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
}
console.log(JSON.stringify({nodes:4,extra_default_fields:fields,constant_accessors:accessors,mode:process.argv.includes('--check')?'check':'generate'}));
