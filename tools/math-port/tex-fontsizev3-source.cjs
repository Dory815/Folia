'use strict';
// Original FontSizeV3Configuration map; all sizes use the original Base SetSize.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for(const [file,sha]of [
 ['.reference/MathJax-src/ts/input/tex/fontsizev3/FontSizeV3Configuration.ts','11231c4f2a01616db4fe7c3dbb96ba1549d364fd2670f7f153d84113b9f5ff0f'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/fontsizev3/FontSizeV3Configuration.js','a38d1ba7d6200dc42d76c8bf7401b2daaae3b957a08ca9329b54591920961d29']
])assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),sha,file);
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {NodeFactory}=load('input/tex/NodeFactory.js');
const {FontSizeV3Configuration}=load('input/tex/fontsizev3/FontSizeV3Configuration.js');
const BaseMethods=load('input/tex/base/BaseMethods.js').default;
assert.deepEqual(FontSizeV3Configuration.handler.macro,['fontsizev3-macros']);
let cases=0;
function fixture(selected=true,aliases=false,reversed=false){
 const packages=selected?(reversed?['fontsizev3','base']:['base','fontsizev3']):['base'];
 if(aliases)packages.push('newcommand');
 let error='';const jax=new TeX({packages,formatError(j,e){error=e.id;return j.formatError(e)}});
 jax.setMmlFactory(new MmlFactory());
 return {jax,compile(s){++cases;error='';const tree=jax.compile(new AbstractMathItem(s,jax,false),reference.document);assert.equal(error,'',s);return tree}};
}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n)});return result;}
function inheritedSize(node){for(let n=node;n;n=n.parent){const value=n.attributes.getExplicit('mathsize');if(value!==undefined)return value}return '1em'}
const f=fixture(),map=f.jax.parseOptions.handlers.retrieve('fontsizev3-macros');
const sizes=[['tiny',.5],['Tiny',.6],['scriptsize',.7],['small',.85],['normalsize',1],['large',1.2],['Large',1.44],['LARGE',1.73],['huge',2.07],['Huge',2.49]];
for(const [name,size]of sizes){
 assert.equal(map.lookup(name).func===BaseMethods.SetSize,true);assert.deepEqual(map.lookup(name).args,[size]);
 const styles=nodes(f.compile('\\'+name+' x'),'mstyle');assert.equal(styles.length,1);assert.equal(styles[0].attributes.get('mathsize'),size+'em');
}
for(const [instance,size]of [[fixture(false),'0.6em'],[fixture(),'0.5em'],[fixture(true,false,true),'0.6em']]){
 const styles=nodes(instance.compile('\\tiny x'),'mstyle');assert.equal(styles.length,1);assert.equal(styles[0].attributes.get('mathsize'),size);
}
assert.deepEqual(nodes(f.compile('\\small a{\\Huge b}c'),'mi').map(inheritedSize),['0.85em','2.49em','0.85em']);
assert.deepEqual(nodes(f.compile('{\\Huge x}y'),'mi').map(inheritedSize),['2.49em','1em']);
const requests=[];
f.jax.parseOptions.nodeFactory.set('node',(factory,kind,children,def,text)=>{if(kind==='mstyle')requests.push(def.mathsize);return NodeFactory.createNode(factory,kind,children,def,text)});
f.compile('\\tiny\\large x');assert.deepEqual(requests,['1.2em','0.5em']);
const alias=fixture(true,true);alias.compile('\\let\\saved\\Huge');const styles=nodes(alias.compile('\\saved x'),'mstyle');assert.equal(styles.length,1);assert.equal(styles[0].attributes.get('mathsize'),'2.49em');
console.log(JSON.stringify({methods:0,baseCallbacks:1,commands:10,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
