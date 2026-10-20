'use strict';
// Development witness: original package map and actual TeX/MML classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
const packageName='upgreek',className='UpgreekConfiguration',mapName='upgreek';
const ts='.reference/MathJax-src/ts/input/tex/'+packageName+'/'+className+'.ts';
assert.equal(digest(ts),'af2272ec1be6855d63b41e98c563331caf584b4e88a44671075abb75ff5a209b');
assert.equal(digest('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/'+packageName+'/'+className+'.js'),'04a57e9122035b332654b358597c1b93ee6c5903c8c5d65044ab682c2c24102c');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {Token}=load('input/tex/Token.js'),{NodeFactory}=load('input/tex/NodeFactory.js');
const configuration=load('input/tex/'+packageName+'/'+className+'.js')[className];
const characters=Array.from(fs.readFileSync(path.join(root,ts),'utf8').matchAll(/^  (\w+): '(\\u[0-9A-Fa-f]{4})',/gm),m=>[m[1],JSON.parse('"'+m[2]+'"')]);
assert.equal(characters.length,41);assert.deepEqual(configuration.handler.macro,[mapName]);
let cases=0;
function fixture(extensions=false,selected=true) {
  const jax=new TeX({packages:['base',...(selected?[packageName]:[]),...(extensions?['newcommand','boldsymbol']:[])]});jax.setMmlFactory(new MmlFactory());
  return {jax,map:jax.parseOptions.handlers.retrieve(mapName),compile(source){++cases;const root=jax.compile(new AbstractMathItem(source,jax,false),reference.document),tokens=[];root.walkTree(node=>{if(node.isToken)tokens.push(node);});return {tokens,error:jax.parseOptions.error};}};
}
function variant(node,value){assert.equal(node.attributes.get('mathvariant'),value);}
{
  const f=fixture();
  for(const [name,text]of characters)for(const source of ['\\'+name,'\\mathbf{\\'+name+'}','\\mathit{\\'+name+'}']) {
    const result=f.compile(source);assert.equal(result.error,false,source);assert.equal(result.tokens.length,1);const node=result.tokens[0];assert.equal(node.kind,'mi');assert.equal(node.getText(),text);variant(node,'normal');
  }
}
{
  const f=fixture(),requests=[];
  for(const [name,text]of [['probeText','α'],['probeNull',null],['probeEmpty','']])f.map.add(name,new Token(name,text,{mathvariant:'italic',mathcolor:'red',class:'kept'}));
  f.jax.parseOptions.nodeFactory.set('token',(factory,kind,def,text)=>{requests.push({kind,text,variant:def.mathvariant,color:def.mathcolor,css:def.class});return NodeFactory.createToken(factory,kind,def,text);});
  const result=f.compile('\\probeText\\probeNull\\probeEmpty');assert.equal(result.error,false);assert.equal(result.tokens.length,3);assert.equal(requests.length,3);
  for(const request of requests){assert.equal(request.kind,'mi');assert.equal(request.variant,'normal');assert.equal(request.color,'red');assert.equal(request.css,'kept');}
  assert.deepEqual(requests.map(r=>r.text),['α',null,'']);assert.deepEqual(result.tokens.map(n=>n.childNodes.length),[1,0,1]);
}
{
  const f=fixture(true),[first,second]=characters;
  let result=f.compile('\\let\\saved\\'+first[0]+'\\let\\'+first[0]+'\\'+second[0]+'\\saved\\'+first[0]);
  assert.equal(result.error,false);assert.deepEqual(result.tokens.map(n=>n.getText()),[first[1],second[1]]);for(const node of result.tokens)variant(node,'normal');
  result=f.compile('\\boldsymbol{\\saved+\\Upomega}\\upbeta');assert.equal(result.error,false);assert.equal(result.tokens.length,4);
  assert.deepEqual(result.tokens.map(n=>n.attributes.get('mathvariant')),['bold','bold','bold','normal']);
}
{
  const selected=fixture(),notSelected=fixture(false,false),name=characters[0][0];
  assert.equal(notSelected.compile('\\'+name).error,true);assert.equal(selected.compile('\\'+name).error,false);
}
console.log(JSON.stringify({symbols:characters.length,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
