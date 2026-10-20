'use strict';
// Original shared helpers exercised with the actual base TeX parser and MML factory.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),hash=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
assert.equal(hash('.reference/MathJax-src/ts/input/tex/ParseUtil.ts'),'b43646071997f88f27df51610c7972a8cb5791ea76194e7318d17d1a2f81f76a');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js');
const TexParser=load('input/tex/TexParser.js').default,{ParseUtil}=load('input/tex/ParseUtil.js');
const NodeUtil=load('input/tex/NodeUtil.js').default;
const {NodeFactory}=load('input/tex/NodeFactory.js');
function fixture() {const jax=new TeX({packages:['base']});jax.setMmlFactory(new MmlFactory());return jax.parseOptions;}
function shape(node) {return node.kind==='text'?'text('+node.getText()+')':node.kind+'('+node.childNodes.map(n=>n?shape(n):'_').join(',')+')';}
{
  const options=fixture(),p=new TexParser('\\mathbb{R}\\frac{x}{y}\\sqrt[3]{x}',{},options),tree=p.mml();
  assert.equal(tree.findNodes('mi').some(n=>n.getText()==='R'&&n.attributes.getExplicit('mathvariant')==='double-struck'),true);
  assert.equal(tree.findNodes('mfrac').length,1);assert.equal(tree.findNodes('mroot').length,1);
  const symbols=new TexParser('\\hbar\\sum\\langle',{},fixture()).mml();
  const ids=symbols.findNodes('mi'),ops=symbols.findNodes('mo');
  assert.equal(ids[0].getProperty('variantForm'),true);assert.equal(ids[0].attributes.getExplicit('variantForm'),undefined);
  assert.equal(ops[0].getProperty('movesupsub'),true);assert.equal(ops[0].getProperty('fixStretchy'),true);
  assert.equal(ops[1].getText(),'⟨');assert.equal(ops[1].attributes.getExplicit('stretchy'),false);
}
const observations=[];
for(const text of ['', '  a  b\n\n c  ','a $x$ b','a \\(x\\) b','\\$\\{x\\}\\\\','\\U{1F600}','\\U4A']) {
  const options=fixture(),parser=new TexParser('',{font:'bold'},options);
  const nodes=ParseUtil.internalMath(parser,text);observations.push([text,nodes.map(shape)]);parser.mml();
}
assert.deepEqual(observations.map(([,nodes])=>nodes),[
  [],['mtext(text(\u00a0a b  c\u00a0))'],
  ['mrow(mtext(text(a\u00a0)),TeXAtom(inferredMrow(mi(text(x)))),mtext(text(\u00a0b)))'],
  ['mrow(mtext(text(a\u00a0)),TeXAtom(inferredMrow(mi(text(x)))),mtext(text(\u00a0b)))'],
  ['mtext(text(${x}\\))'],['mtext(text(😀))'],['mtext(text(\u0004A))']
]);
{
  const options=fixture(),parser=new TexParser('',{},options),calls=[];
  options.nodeFactory.setCreators({text:(f,t)=>{calls.push('text');return NodeFactory.createText(f,t);},node:(f,...args)=>{calls.push('node');return NodeFactory.createNode(f,...args);},token:()=>{throw new Error('Unexpected token route');}});
  const node=ParseUtil.internalText(parser,'  a\n\n b  ',{mathvariant:'-tex-calligraphic'});
  assert.deepEqual(calls,['text','node']);assert.equal(node.getText(),'\u00a0a  b\u00a0');assert.equal(node.getProperty('ignore-variant'),true);parser.mml();
}
{
  const options=fixture(),parser=new TexParser('',{},options);
  for(const [text,id] of [['$x','MathNotTerminated'],['\\(x','MathNotTerminated'],['\\U{xyz}','BadRawUnicode']])assert.throws(()=>ParseUtil.internalMath(parser,text),e=>e.id===id);
  assert.throws(()=>ParseUtil.internalMath(parser,'\\U{110000}'),RangeError);
  const level=ParseUtil.internalMath(parser,'',0);
  assert.equal(level[0].kind,'mstyle');
  parser.mml();
}
{
  const options=fixture(),parser=new TexParser('',{},options),nf=options.nodeFactory;
  const base=nf.create('token','mi',{},'x'),sup=nf.create('token','mi',{},'n'),script=nf.create('node','msubsup',[base],{id:'drop'});
  NodeUtil.setChild(script,2,sup);
  options.addNode('custom',script);const copy=ParseUtil.copyNode(script,parser);
  assert.equal(copy.childNodes.length,3);assert.equal(copy.childNodes[1],null);assert.equal(copy.attributes.getExplicit('id'),undefined);
  assert.equal(copy.getProperty('in-lists'),'custom,custom');assert.equal(options.nodeLists.custom.includes(copy),true);parser.mml();
}
{
  const options=fixture(),parser=new TexParser('',{},options),nf=options.nodeFactory;
  const base=nf.create('token','mo',{},'∑'),script=nf.create('token','mi',{},'a');
  const node=ParseUtil.underOver(parser,base,script,'over',true);
  assert.equal(base.getProperty('movablelimits'),false);assert.equal(base.attributes.getExplicit('movablelimits'),false);
  assert.equal(node.kind,'TeXAtom');assert.equal(node.getProperty('subsupOK'),true);assert.equal(node.getProperty('movesupsub'),true);
  const inner=node.findNodes('munderover')[0];assert.equal(inner.kind,'munderover');assert.equal(inner.childNodes[1],undefined);assert.equal(inner.childNodes[2]===script,true);
  parser.mml();
}
{
  const options=fixture(),parser=new TexParser('',{},options),array=parser.itemFactory.create('array');
  ParseUtil.setArrayAlign(array,' t ');assert.equal(array.arraydef.align,'baseline 1');
  parser.string='oldtail';parser.i=3;ParseUtil.setArrayAlign(array,' l ',parser);assert.equal(parser.string,'[ l ]tail');assert.equal(parser.i,0);
  parser.stack.Pop();parser.stack.Push(parser.itemFactory.create('start',parser.stack.global));
  ParseUtil.checkEqnEnv(parser);parser.stack.Push(array);array.setProperty('nestable',true);ParseUtil.checkEqnEnv(parser);
  assert.throws(()=>ParseUtil.checkEqnEnv(parser,false),e=>e.id==='ErroneousNestingEq');parser.mml();
}
{
  const options=fixture(),parser=new TexParser('',{},options),nf=options.nodeFactory;
  const node=nf.create('token','mo',{variantForm:true,useHeight:false,texClass:3,movablelimits:true,autoOP:false,open:'(',inferred:true},'∑');
  assert.equal(node.getProperty('variantForm'),true);assert.equal(node.getProperty('useHeight'),false);assert.equal(node.getProperty('texClass'),3);
  assert.equal(node.attributes.getExplicit('variantForm'),undefined);assert.equal(node.attributes.getExplicit('movablelimits'),true);
  assert.equal(NodeUtil.getMoAttribute(node,'movablelimits'),true);node.attributes.set('movablelimits',false);assert.equal(NodeUtil.getMoAttribute(node,'movablelimits'),false);
  options.options.maxMacros=0;assert.throws(()=>ParseUtil.checkMaxMacros(parser,false),e=>e.id==='MaxMacroSub2');assert.equal(parser.macroCount,1);parser.mml();
}
console.log(JSON.stringify({status:'passed',observations,limit:'Original shared helpers and base parser only; full native configuration/filters/38 packages/SVG/Folia remain separate.'}));
