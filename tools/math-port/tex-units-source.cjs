'use strict';
// Original UnitsConfiguration and actual TeX/MML/NodeFactory, development only.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for(const [file,sha]of [
 ['.reference/MathJax-src/ts/input/tex/units/UnitsConfiguration.ts','d9c2f2523181801959e628bc30f20b7bce42870f782f34e60cae954bcbe1980d'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/units/UnitsConfiguration.js','f4e1cf73b7249c87fa767ea640dcab108766c552155227ca92cd791da1fd5a6e']
])assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),sha,file);
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {NodeFactory}=load('input/tex/NodeFactory.js');
const {UnitsMethods,UnitsConfiguration}=load('input/tex/units/UnitsConfiguration.js');
assert.deepEqual(Object.keys(UnitsMethods),['Unit','UnitFrac','NiceFrac']);
assert.deepEqual(UnitsConfiguration.handler.macro,['units']);assert.deepEqual(UnitsConfiguration.options.units,{loose:false,ugly:false});
let cases=0;
function fixture(options={},aliases=false){
 let error='';const jax=new TeX({packages:['base','units',...(aliases?['newcommand']:[])],units:options,formatError(j,e){error=e.id;return j.formatError(e)}});
 jax.setMmlFactory(new MmlFactory());
 return {jax,compile(s){++cases;error='';const tree=jax.compile(new AbstractMathItem(s,jax,false),reference.document);return {tree,error}}};
}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n)});return result;}
function text(root){const result=[];root.walkTree(n=>{if(n.isToken)result.push(n.getText())});return result.join('');}
function ok(f,s){const result=f.compile(s);assert.equal(result.error,'',s);assert.equal(f.jax.parseOptions.parsers.length,0);return result.tree;}
const f=fixture(),map=f.jax.parseOptions.handlers.retrieve('units');
for(const [command,method]of [['units','Unit'],['unitfrac','UnitFrac'],['nicefrac','NiceFrac']])assert.equal(map.lookup(command).func===UnitsMethods[method],true);
for(const source of ['\\units{kg}','\\units[]{kg}','\\units k']){
 const tree=ok(f,source),mi=nodes(tree,'mi');assert.equal(mi.length,1);assert.equal(text(tree),source==='\\units k'?'k':'kg');
 assert.equal(nodes(tree,'mspace').length,0);for(const token of mi)assert.equal(token.attributes.get('mathvariant'),'normal');
}
for(const [loose,width]of [[false,'0.167em'],[true,'0.333em']]){
 const instance=fixture({loose});
 for(const source of ['\\units[2]{kg}+z','\\units[0]{kg}+z','\\unitfrac[2]{m}{s}+z']){
  const tree=ok(instance,source),spaces=nodes(tree,'mspace');assert.equal(spaces.length,loose?0:1);
  if(loose)assert.equal(nodes(tree,'mtext')[0].getText(),'\u00A0');else assert.equal(spaces[0].attributes.get('width'),width);
  assert.equal(text(tree).endsWith('+z'),true);
 }
}
for(const ugly of [false,true]){
 const instance=fixture({ugly});
 for(const source of ['\\unitfrac{m}{s}','\\nicefrac{m}{s}','\\nicefrac{}{s}']){
  const frac=nodes(ok(instance,source),'mfrac');assert.equal(frac.length,1);assert.equal(frac[0].attributes.get('bevelled'),!ugly);
  assert.equal(Object.hasOwn(frac[0].attributes.getAllAttributes(),'bevelled'),!ugly);
 }
}
{
 const tree=ok(f,'\\mathbf{\\nicefrac[\\mathit]{a}{b}c}'),tokens=nodes(tree,'mi');
 assert.deepEqual(tokens.map(n=>[n.getText(),n.attributes.get('mathvariant')]),[['a','-tex-mathit'],['b','-tex-mathit'],['c','bold']]);
 const emptyFont=nodes(ok(f,'\\mathbf{\\nicefrac[]{a}{b}}'),'mi');assert.deepEqual(emptyFont.map(n=>n.attributes.get('mathvariant')),['bold','bold']);
 const nested=nodes(ok(f,'\\nicefrac{\\nicefrac{a}{b}}{c}'),'mfrac');assert.equal(nested.length,2);
 const requests=[];
 f.jax.parseOptions.nodeFactory.set('node',(factory,kind,children,def,txt)=>{if(kind==='mfrac')requests.push({children:[...children],def:{...def}});return NodeFactory.createNode(factory,kind,children,def,txt)});
 const frac=nodes(ok(f,'\\nicefrac{a}{b}'),'mfrac')[0];assert.equal(requests.length,1);assert.deepEqual(requests[0].def,{bevelled:true});
 for(let i=0;i<2;++i)assert.equal(requests[0].children[i]===frac.childNodes[i],true);
}
const errors=[['\\units','MissingArgFor'],['\\unitfrac{x}','MissingArgFor'],['\\nicefrac{x}','MissingArgFor'],['\\nicefrac{\\unknown}{b}','UndefinedControlSequence'],['\\nicefrac{a}{\\unknown}','UndefinedControlSequence'],['\\units{\\unknown}','UndefinedControlSequence']];
for(const [source,id]of errors){assert.equal(f.compile(source).error,id);ok(f,'\\units{kg}')}
{
 const alias=fixture({},true);ok(alias,'\\let\\savedUnit\\units\\let\\savedFrac\\unitfrac\\let\\savedNice\\nicefrac');
 assert.equal(text(ok(alias,'\\savedUnit{kg}')),'kg');
 for(const source of ['\\savedFrac{m}{s}','\\savedNice{m}{s}'])assert.equal(nodes(ok(alias,source),'mfrac').length,1);
}
console.log(JSON.stringify({methods:3,commands:3,cases,errorCases:errors.length,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
