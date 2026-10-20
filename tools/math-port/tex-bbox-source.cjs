'use strict';
// Original BboxConfiguration methods and actual TeX/NodeFactory/MML classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for(const [file,sha] of [
 ['.reference/MathJax-src/ts/input/tex/bbox/BboxConfiguration.ts','0e004e3b4c22c5935c12fc4609962e84dcba41fdb6935cb7d1685553aab18867'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/bbox/BboxConfiguration.js','f85374d99b29c894624c4ae7e5775ec1290afad514c89fe86499762f59456355']
]) assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),sha);
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {NodeFactory}=load('input/tex/NodeFactory.js');
const {BboxConfiguration}=load('input/tex/bbox/BboxConfiguration.js');
assert.deepEqual(BboxConfiguration.handler.macro,['bbox']);
let cases=0;
function fixture(aliases=false) {
 let error='',message='';
 const jax=new TeX({packages:['base','bbox',...(aliases?['newcommand']:[])],formatError(j,e){error=e.id;message=e.message;return j.formatError(e);}});
 jax.setMmlFactory(new MmlFactory());
 return {jax,compile(s){++cases;error='';message='';const tree=jax.compile(new AbstractMathItem(s,jax,false),reference.document);return {tree,error,message}}};
}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n);});return result;}
function ok(f,s){const r=f.compile(s);assert.equal(r.error,'',s);return r.tree;}
const f=fixture();
for (const source of ['\\bbox{x}','\\bbox[]{x}','\\bbox[,,,]{x}','\\bbox[\u00A0, \uFEFF]{x}']) {
 const tree=ok(f,source);assert.equal(nodes(tree,'mpadded').length,0);assert.equal(nodes(tree,'mstyle').length,0);
 assert.deepEqual(nodes(tree,'mi').map(n=>n.getText()),['x']);
}
for(const unit of ['pt','em','ex','mu','px','in','cm','mm']) {
 const pad='001.50'+unit,tree=ok(f,'\\bbox['+pad+']{x}'),padded=nodes(tree,'mpadded');assert.equal(padded.length,1);
 assert.deepEqual(['height','depth','lspace','width'].map(k=>padded[0].attributes.get(k)),['+'+pad,'+'+pad,pad,'+3'+unit]);
}
for(const [padding,width]of [['.25em','+0.5em'],['0em','+0em'],['1.em','+2em'],['00000000000000000001px','+2px']]) {
 const padded=nodes(ok(f,'\\bbox['+padding+']{x}'),'mpadded');assert.equal(padded.length,1);assert.equal(padded[0].attributes.get('width'),width);
}
for(const color of ['Red','123','#aF3','#AbC123','2EM','1pc']) {
 const styles=nodes(ok(f,'\\bbox['+color+']{x}'),'mstyle');assert.equal(styles.length,1);assert.equal(styles[0].attributes.get('mathbackground'),color);
}
for(const style of ['border:1px solid red','-moz-border: 1em','foo:']) {
 const styles=nodes(ok(f,'\\bbox['+style+']{x}'),'mstyle');assert.equal(styles.length,1);assert.equal(styles[0].attributes.get('style'),style);
}
{
 const tree=ok(f,'\\bbox[\u00A0 red \uFEFF, .25em , border:1px solid blue]{\\frac{a}{b}}');
 const styles=nodes(tree,'mstyle'),pads=nodes(tree,'mpadded');assert.equal(styles.length,1);assert.equal(pads.length,1);
 assert.equal(styles[0].attributes.get('mathbackground'),'red');assert.equal(styles[0].attributes.get('style'),'border:1px solid blue');
 assert.equal(pads[0].attributes.get('width'),'+0.5em');assert.equal(pads[0].parent?.parent===styles[0],true);assert.equal(nodes(pads[0],'mfrac').length,1);
}
{
 const f=fixture(),requests=[];
 f.jax.parseOptions.nodeFactory.set('node',(factory,kind,children,def,text)=>{
  if(kind==='mpadded'||kind==='mstyle')requests.push([kind,{...def}]);
  return NodeFactory.createNode(factory,kind,children,def,text);
 });
 ok(f,'\\bbox[red,.25em,border:1px solid blue]{x}');
 assert.deepEqual(requests,[['mpadded',{height:'+.25em',depth:'+.25em',lspace:'.25em',width:'+0.5em'}],['mstyle',{mathbackground:'red',style:'border:1px solid blue'}]]);
}
for(const [options,message]of [['1em,2em','Padding specified twice in \\bbox'],['red,blue','Background specified twice in \\bbox'],['border:red,foo:bar','Style specified twice in \\bbox']]) {
 const r=f.compile('\\bbox['+options+']{x}');assert.equal(r.error,'MultipleBBoxProperty');assert.equal(r.message,message);
}
for(const part of ['-1em','+1em','#12','#ggg','border :red','foo_bar:red','red blue']) {
 const r=f.compile('\\bbox['+part+']{x}');assert.equal(r.error,'InvalidBBoxProperty');
 assert.equal(r.message,'"'+part+'" doesn\'t look like a color, a padding dimension, or a style');ok(f,'\\bbox[red]{x}');
}
assert.equal(f.compile('\\bbox[-1em]{\\unknown}').error,'UndefinedControlSequence');
assert.equal(f.compile('\\bbox[-1em]').error,'MissingArgFor');
{
 const f=fixture(true);ok(f,'\\let\\saved\\bbox');
 const pads=nodes(ok(f,'\\saved[red,1em]{x}'),'mpadded');assert.equal(pads.length,1);assert.equal(pads[0].attributes.get('width'),'+2em');
 const r=f.compile('\\saved[1em,2em]{x}');assert.equal(r.error,'MultipleBBoxProperty');assert.equal(r.message,'Padding specified twice in \\bbox');
 ok(f,'\\saved[blue]{x}');
}
console.log(JSON.stringify({methods:1,maps:1,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
