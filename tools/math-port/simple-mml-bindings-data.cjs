// Root-generated source defaults, scalar getters and source-ordered field phases.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,u,MmlFactory,kinds,classes,hashes}=require('./simple-mml-reference.cjs');
const ts=require(path.join(root,'build/math-port-tooling/node_modules/typescript'));assert.equal(ts.version,'5.9.3');
const q=JSON.stringify,p=v=>typeof v==='string'?q(v):typeof v==='boolean'?String(v):typeof v==='number'?String(v)+'.0':(()=>{throw Error('Unbound source constant');})();
const methods={menclose:['MmlNode* setTeXclass(MmlNode*)override;'],mpadded:['MmlNode* setTeXclass(MmlNode*)override;'],maligngroup:['protected:','void setChildInheritedAttributes(AttributeList,Property,Property,Property)override;'],malignmark:[],mglyph:['void verifyAttributes(PropertyList)override;']};
let fields=0,getters=0,members=0;
for(const [index,kind]of kinds.entries()){
 const {source,sha256}=hashes[index],ast=ts.createSourceFile(source,fs.readFileSync(path.join(root,'.reference/MathJax-src',source),'utf8'),ts.ScriptTarget.Latest,true),decl=ast.statements.filter(ts.isClassDeclaration);assert.equal(decl.length,1);const c=decl[0],name=c.name.text,base=c.heritageClauses[0].types[0].expression.text,node=new MmlFactory().create(kind),T=classes[kind],ds=c.members.find(m=>m.name?.getText(ast)==='defaults');assert.ok(ds);members+=c.members.length;
 const common=['// Generated MathJax 4.1.3 '+source+'; Apache-2.0.','// SHA256 '+sha256];
 const iface=[...common,'export module folia.math.core.mml_tree.mml_nodes.'+kind+';','export import folia.math.core.mml_tree.mml_node;','import std;','export namespace folia::math::mml {','class '+name+' : public '+base+' {','public:','using '+base+'::'+base+';','static PropertyList& defaults();'];
 const body=[...common,'module folia.math.core.mml_tree.mml_nodes.'+kind+';','import folia.math.core.mml_tree.arithmetic_host;','import std;','namespace folia::math::mml {','namespace {PropertyList '+name+'Defaults=[] {auto value=host::arithmeticCopy('+base+'::defaults());'];
 for(const member of ds.initializer.properties.filter(ts.isPropertyAssignment)){const key=member.name.text;body.push('value.setOwn('+q(key)+','+p(T.defaults[key])+');');++fields;}body.push('return value;}();}','PropertyList& '+name+'::defaults(){return '+name+'Defaults;}');
 const own=c.members.filter(m=>ts.isPropertyDeclaration(m)&&!m.modifiers?.some(x=>x.kind===ts.SyntaxKind.StaticKeyword));
 if(own.length){assert.ok(kind==='menclose'||kind==='mglyph');assert.deepEqual(own.map(m=>m.name.text),['texclass']);assert.equal(node.texClass,0);iface.push('void initializeClassFields()override;');body.push('void '+name+'::initializeClassFields(){initializeTexClass(TEXCLASS.ORD);}');}
 for(const m of c.members.filter(ts.isGetAccessorDeclaration)){const key=m.name.text;assert.ok(['kind','arity','linebreakContainer','isSpacelike'].includes(key));const type=key==='kind'?'std::string':key==='arity'?'double':'bool';iface.push(type+' '+key+'()const override;');body.push(type+' '+name+'::'+key+'()const{return '+p(node[key])+';}');++getters;}
 iface.push(...methods[kind],'};','}');body.push('}');
 for(const [suffix,lines]of [['.ixx',iface],['_bindings.cpp',body]]){const output=lines.join('\n')+'\n',file=path.join(root,'src/math/core/mml_tree/mml_nodes',kind+suffix);if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(file,output);}
}
assert.equal(members,22);console.log(JSON.stringify({classes:5,members,extra_default_fields:fields,getters,mode:process.argv.includes('--check')?'check':'generate'}));
