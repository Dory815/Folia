'use strict';
// Development witness: original package map and actual TeX/MML classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
const packageName='gensymb',className='GensymbConfiguration',mapName='gensymb-symbols';
const ts='.reference/MathJax-src/ts/input/tex/'+packageName+'/'+className+'.ts';
assert.equal(digest(ts),'3ea78b42e813eb0e087076afde8ddb350282806e211254c4b8583126045d6413');
assert.equal(digest('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/'+packageName+'/'+className+'.js'),'28899f27e9e56a9f5371f713bf57b5671e7f448397d04760a192b4b2f6d76834');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {Token}=load('input/tex/Token.js'),{NodeFactory}=load('input/tex/NodeFactory.js');
const configuration=load('input/tex/'+packageName+'/'+className+'.js')[className];
const characters=Array.from(fs.readFileSync(path.join(root,ts),'utf8').matchAll(/^  (\w+): '(\\u[0-9A-Fa-f]{4})',/gm),m=>[m[1],JSON.parse('"'+m[2]+'"')]);
assert.equal(characters.length,5);assert.deepEqual(configuration.handler.macro,[mapName]);
let cases=0;
function fixture(extensions=false,selected=true) {
  const jax=new TeX({packages:['base',...(selected?[packageName]:[]),...(extensions?['newcommand','boldsymbol']:[])]});jax.setMmlFactory(new MmlFactory());
  return {jax,map:jax.parseOptions.handlers.retrieve(mapName),compile(source){++cases;const root=jax.compile(new AbstractMathItem(source,jax,false),reference.document),tokens=[];root.walkTree(node=>{if(node.isToken)tokens.push(node);});return {tokens,error:jax.parseOptions.error};}};
}
function variant(node,value){assert.equal(node.attributes.get('mathvariant'),value);}
function unit(node){assert.equal(node.attributes.get('class'),'MathML-Unit');}
{
  const f=fixture();
  for(const [name,text]of characters)for(const source of ['\\'+name,'\\mathbf{\\'+name+'}','\\mathit{\\'+name+'}']) {
    const result=f.compile(source);assert.equal(result.error,false,source);assert.equal(result.tokens.length,1);const node=result.tokens[0];assert.equal(node.kind,'mi');assert.equal(node.getText(),text);variant(node,'normal');unit(node);
  }
}
{
  const f=fixture(),requests=[];
  for(const [name,text]of [['probeText','Ω'],['probeNull',null],['probeEmpty','']])f.map.add(name,new Token(name,text,{mathvariant:'italic',mathcolor:'red',class:'replaced'}));
  f.jax.parseOptions.nodeFactory.set('token',(factory,kind,def,text)=>{requests.push({kind,text,variant:def.mathvariant,color:def.mathcolor,css:def.class});return NodeFactory.createToken(factory,kind,def,text);});
  const result=f.compile('\\probeText\\probeNull\\probeEmpty');assert.equal(result.error,false);assert.equal(result.tokens.length,3);assert.equal(requests.length,3);
  for(const request of requests){assert.equal(request.kind,'mi');assert.equal(request.variant,'normal');assert.equal(request.color,'red');assert.equal(request.css,'MathML-Unit');}
  assert.deepEqual(requests.map(r=>r.text),['Ω',null,'']);assert.deepEqual(result.tokens.map(n=>n.childNodes.length),[1,0,1]);
}
{
  const f=fixture(true),first=characters[0],second=characters.find(([name])=>name==='micro');
  let result=f.compile('\\let\\saved\\'+first[0]+'\\let\\'+first[0]+'\\'+second[0]+'\\saved\\'+first[0]);
  assert.equal(result.error,false);assert.deepEqual(result.tokens.map(n=>n.getText()),[first[1],second[1]]);for(const node of result.tokens){variant(node,'normal');unit(node);}
  result=f.compile('\\boldsymbol{\\saved+\\micro}\\degree');assert.equal(result.error,false);assert.equal(result.tokens.length,4);
  assert.deepEqual(result.tokens.map(n=>n.attributes.get('mathvariant')),['bold','bold','bold','normal']);for(const i of [0,2,3])unit(result.tokens[i]);
}
{
  const selected=fixture(),notSelected=fixture(false,false),name=characters[0][0];
  assert.equal(notSelected.compile('\\'+name).error,true);assert.equal(selected.compile('\\'+name).error,false);
}
console.log(JSON.stringify({symbols:characters.length,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
