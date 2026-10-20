'use strict';
// Pinned original font configurations and actual MathJax TeX/MML, development only.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for(const [file,sha]of [
 ['.reference/MathJax-src/ts/input/tex/bbm/BbmConfiguration.ts','96854e83b013a943bae6affa31d76b2234e38f3d235c0efd541345a27fc65b28'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/bbm/BbmConfiguration.js','14c57a633454cd1c558fe22c45a90c15c86e69c4960b79375a9737311f8f8901'],
 ['.reference/MathJax-src/ts/input/tex/dsfont/DsfontConfiguration.ts','d3d4c807f5fea64159945edc972b296335fae494a970cee491f319ef71d8c50f'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/dsfont/DsfontConfiguration.js','98decbf37503163e30e0906b30bc825c0cf750db5286c527c399d013d8b39f5a']
])assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),sha,file);
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {BbmMethods,BbmConfiguration}=load('input/tex/bbm/BbmConfiguration.js');
const {DsfontConfiguration}=load('input/tex/dsfont/DsfontConfiguration.js');
const {default:BaseMethods}=load('input/tex/base/BaseMethods.js');
assert.deepEqual(Object.keys(BbmMethods),['ChooseFont','ChangeBold','MathFont']);
assert.equal(BbmMethods.MathFont===BaseMethods.MathFont,true);
assert.deepEqual(BbmConfiguration.handler.macro,['bbm']);
assert.deepEqual(BbmConfiguration.options.bbm,{bold:false});
assert.deepEqual(DsfontConfiguration.handler.macro,['dsfont']);
assert.deepEqual(DsfontConfiguration.options.dsfont,{sans:false});
let cases=0;
function fixture(packageName,option=false){
 let error='';const jax=new TeX({packages:['base',packageName,'newcommand'],[packageName]:{[packageName==='bbm'?'bold':'sans']:option},formatError(j,e){error=e.id;return j.formatError(e)}});
 jax.setMmlFactory(new MmlFactory());
 return {jax,compile(s){++cases;error='';const tree=jax.compile(new AbstractMathItem(s,jax,false),reference.document);if(!error)assert.equal(jax.parseOptions.parsers.length,0);return {tree,error}}};
}
function tokens(tree){const result=[];tree.walkTree(n=>{if(n.isToken)result.push([n.getText(),n.attributes.get('mathvariant')])});return result;}
function ok(f,s,expected){const {tree,error}=f.compile(s);assert.equal(error,'',s);if(expected)assert.deepEqual(tokens(tree),expected,s);return tree;}
for(const bold of [false,true]){
 const f=fixture('bbm',bold),map=f.jax.parseOptions.handlers.retrieve('bbm');
 for(const [name,handler,args]of [
  ['mathversion','ChangeBold',[]],['mathbbm','ChooseFont',['-bbm-normal','-bbm-bold']],
  ['mathbbmss','ChooseFont',['-bbm-sans-serif','-bbm-sans-serif-bold']],['mathbbmtt','MathFont',['-bbm-monospace']]
 ]){const macro=map.lookup(name);assert.equal(macro.func===BbmMethods[handler],true);assert.deepEqual(macro.args,args);}
 for(const [name,font]of [['mathbbm',bold?'-bbm-bold':'-bbm-normal'],['mathbbmss',bold?'-bbm-sans-serif-bold':'-bbm-sans-serif'],['mathbbmtt','-bbm-monospace']]){
  ok(f,'\\'+name+'{x}y',[['x',font],['y','italic']]);
  ok(f,'\\'+name+' x',[['x',font]]);
  ok(f,'\\'+name+'{\\mathbf{x}y}z',[['x','bold'],['y',font],['z','italic']]);
  ok(f,'\\'+name+'{}');
 }
}
{
 const f=fixture('bbm');
 ok(f,'{\\mathversion{bold}}\\mathbbm{x}',[['x','-bbm-bold']]);
 ok(f,'\\mathbbmss{x}',[['x','-bbm-sans-serif-bold']]);
 assert.equal(f.compile('\\mathversion{normal}\\unknown').error,'UndefinedControlSequence');
 ok(f,'\\mathbbm{x}',[['x','-bbm-normal']]);
 ok(f,'\\mathversion{bold}');
 assert.equal(f.compile('\\mathbbm{\\unknown}').error,'UndefinedControlSequence');
 ok(f,'\\mathbbm{x}',[['x','-bbm-bold']]);
 ok(f,'\\mathversion{other}\\mathbbm{x}',[['x','-bbm-normal']]);
 for(const source of ['\\mathversion','\\mathbbm','\\mathbbmss','\\mathbbmtt']){
  assert.equal(f.compile(source).error,'MissingArgFor');ok(f,'\\mathbbm{x}',[['x','-bbm-normal']]);
 }
 ok(f,'\\let\\savedFont\\mathbbm\\let\\savedVersion\\mathversion\\let\\savedMono\\mathbbmtt');
 ok(f,'\\savedVersion{bold}\\savedFont{x}\\savedMono{y}',[['x','-bbm-bold'],['y','-bbm-monospace']]);
}
for(const sans of [false,true]){
 const f=fixture('dsfont',sans),font=sans?'-ds-ss':'-ds-rm',map=f.jax.parseOptions.handlers.retrieve('dsfont');
 assert.deepEqual(map.lookup('mathds').args,[]);
 for(const source of ['\\mathds{x}y','\\mathds x y'])ok(f,source,[['x',font],['y','italic']]);
 ok(f,'\\mathds{\\mathbf{x}y}z',[['x','bold'],['y',font],['z','italic']]);
 ok(f,'\\mathds{}');
 assert.equal(f.compile('\\mathds').error,'MissingArgFor');
 assert.equal(f.compile('\\mathds{\\unknown}').error,'UndefinedControlSequence');
 ok(f,'\\mathds{x}',[['x',font]]);
 ok(f,'\\let\\savedDs\\mathds');ok(f,'\\savedDs{x}',[['x',font]]);
 f.jax.parseOptions.options.dsfont.sans=!sans;
 ok(f,'\\mathds{x}',[['x',sans?'-ds-rm':'-ds-ss']]);
}
console.log(JSON.stringify({packages:['bbm','dsfont'],commands:5,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
