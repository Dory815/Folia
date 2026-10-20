'use strict';
// Original CenternotConfiguration with actual TeX, NodeFactory, and MML classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for(const [file,sha]of [
 ['.reference/MathJax-src/ts/input/tex/centernot/CenternotConfiguration.ts','de494762650f59f0d1e9b9e34483d1c3fc4e16b4d66ecc94f9d9dc1cc1501091'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/centernot/CenternotConfiguration.js','5c147327bf3e1a524bd473b981f18d8b0ea5aea748fd16842ccc6d689a045e58']
])assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),sha,file);
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {NodeFactory}=load('input/tex/NodeFactory.js');
const {CenternotConfiguration,filterCenterOver}=load('input/tex/centernot/CenternotConfiguration.js');
const BaseMethods=load('input/tex/base/BaseMethods.js').default;
assert.deepEqual(CenternotConfiguration.handler.macro,['centernot']);
assert.equal(CenternotConfiguration.postprocessors[0][0]===filterCenterOver,true);
let cases=0;
function fixture(aliases=false){
 let error='';
 const jax=new TeX({packages:['base','centernot',...(aliases?['newcommand']:[])],formatError(j,e){error=e.id;return j.formatError(e)}});
 jax.setMmlFactory(new MmlFactory());
 return {jax,compile(s){++cases;error='';const tree=jax.compile(new AbstractMathItem(s,jax,false),reference.document);return {tree,error}}};
}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n)});return result;}
function ok(f,s){const result=f.compile(s);assert.equal(result.error,'',s);return result.tree;}
function outer(base){let target=base;for(let i=0;i<6;++i)target=target.parent;return target;}
{
 const f=fixture(),map=f.jax.parseOptions.handlers.retrieve('centernot');
 assert.equal(map.lookup('centernot').func===BaseMethods.Macro,true);
 assert.deepEqual(map.lookup('centernot').args,['\\centerOver{#1}{{\u29F8}}',1]);
 for(const [symbol,texClass]of [['=',3],['+',2],['x',0],['\\sum',1],['\\leq',3]]){
  const tree=ok(f,'\\centernot{'+symbol+'}'),bases=f.jax.parseOptions.getList('centerOver');
  assert.equal(bases.length,1);const target=outer(bases[0]);
  assert.equal(target.kind,'TeXAtom');assert.equal(target.texClass,texClass);assert.equal(target.getProperty('texClass'),texClass);
  const pads=nodes(tree,'mpadded');assert.equal(pads.length,2);
  for(const pad of pads){assert.equal(pad.attributes.get('width'),0);assert.equal(pad.attributes.get('lspace'),'-.5width')}
  assert.equal(nodes(tree,'mphantom').length,1);
 }
 const requests=[];
 f.jax.parseOptions.nodeFactory.set('token',(factory,kind,def,text)=>{if(text!==undefined)requests.push(text);return NodeFactory.createToken(factory,kind,def,text)});
 const tree=ok(f,'\\centerOver{xy}{z}');assert.deepEqual(requests,['z','x','y','x','y']);
 assert.equal(nodes(nodes(tree,'mphantom')[0],'mi').length,2);
 const bold=nodes(ok(f,'\\mathbf{\\centernot{x}}'),'mi');assert.equal(bold.length,2);
 for(const token of bold)assert.equal(token.attributes.get('mathvariant'),'bold');
 const nested=ok(f,'\\centernot{\\centernot{=}}'),bases=f.jax.parseOptions.getList('centerOver');
 assert.equal(bases.length,3);for(const base of bases)assert.equal(outer(base).texClass,3);
 assert.equal(nodes(nested,'mphantom').length,3);ok(f,'x');assert.equal(f.jax.parseOptions.getList('centerOver').length,0);
}
{
 const f=fixture(true);ok(f,'\\let\\saved\\centerOver');ok(f,'\\saved{=}{x}');
 assert.equal(outer(f.jax.parseOptions.getList('centerOver')[0]).texClass,3);
 for(const [source,id]of [['\\centernot','MissingArgFor'],['\\centerOver{x}','MissingArgFor'],['\\centerOver{x}{\\unknown}','UndefinedControlSequence']]){
  assert.equal(f.compile(source).error,id);ok(f,'\\centernot{=}');
 }
 assert.throws(()=>f.compile('\\centernot{}'),TypeError);assert.equal(f.jax.parseOptions.parsers.length,0);ok(f,'\\centernot{=}');
}
console.log(JSON.stringify({methods:1,postprocessors:1,commands:2,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
