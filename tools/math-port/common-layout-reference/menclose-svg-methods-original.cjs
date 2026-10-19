'use strict';
// Pinned Source witness for the SvgMenclose -> SvgWrapper.svg -> Jax.svg
// operation chain, including the nested fill call and post-paint transform.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../../..');
const bundle=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const packageRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax');
const lock=JSON.parse(fs.readFileSync(path.join(__dirname,'profile-source-lock.json'),'utf8'));
const sourceHashes={};
for(const {file,sha256} of lock.dependencies){
  const p=file.startsWith('src/cjs/')?path.join(bundle,'cjs',file.slice('src/cjs/'.length)):path.join(packageRoot,file);
  const actual=crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex');
  if(actual!==sha256)throw new Error(`pinned CJS hash mismatch: ${file}`);
  sourceHashes[file]=actual;
}
const {SvgWrappers}=require(path.join(bundle,'cjs/output/svg/Wrappers.js'));
const {SvgWrapperFactory}=require(path.join(bundle,'cjs/output/svg/WrapperFactory.js'));
const {MmlFactory}=require(path.join(bundle,'cjs/core/MmlTree/MmlFactory.js'));
const events=[];
let nextNode=1;
const jax={
  fixed(value,digits=3){return Number(value).toFixed(digits).replace(/\.?0+$/,'');},
  svg(kind,def={},children=[]){
    const result={id:nextNode++,kind,def,children};
    events.push({op:'jax.svg',receiver:'jax',kind,keys:Object.keys(def),def:{...def},childCount:children.length,result:result.id});
    return result;
  },
  adaptor:{setAttribute(node,name,value){events.push({op:'adaptor.setAttribute',node:node.id,name,value});}},
};
const factory=new SvgWrapperFactory(SvgWrappers);
factory.jax=Object.assign(jax,{font:{params:{rule_thickness:.06},getVariant:()=>null,getChar:()=>null},
  options:{scale:1,htmlHDW:'none'},math:{outputData:{}},pxPerEm:16,nodeMap:new Map(),
  measureText:()=>({w:1,h:.7,d:.2})});
const mml=new MmlFactory();
const parent=new SvgWrappers.math(factory,mml.create('math'),null);
const child=mml.create('mi',{},[mml.create('text').setText('x')]);
const wrapper=new SvgWrappers.menclose(factory,mml.create('menclose',{notation:'box'},[child]),parent);
const notationNames=[...SvgWrappers.menclose.notations.keys()];
const expectedNotationNames=[
  'top','right','bottom','left','actuarial','madruwb','updiagonalstrike',
  'downdiagonalstrike','horizontalstrike','verticalstrike','box','roundedbox',
  'circle','phasorangle','uparrow','downarrow','leftarrow','rightarrow',
  'updownarrow','leftrightarrow','updiagonalarrow','northeastarrow',
  'southeastarrow','northwestarrow','southwestarrow','northeastsouthwestarrow',
  'northwestsoutheastarrow','longdiv','radical'];
assert.deepEqual(notationNames,expectedNotationNames,
  'the actual SvgMenclose constructor must expose the full ordered Source notation Map');
const sourceMethodResults={
  line:wrapper.line([0,.1,1,.1]),
  box:wrapper.box(1,.7,.2,.15),
  ellipse:wrapper.ellipse(1,.7,.2),
  path:wrapper.path('mitre','M',0,.2,'L',1,.2),
  fill:wrapper.fill('M',0,0,'L',1,1,'Z'),
};
const arrow=wrapper.arrow(1,.2,true,'X',.1);
assert.equal(sourceMethodResults.line.kind,'line');
assert.equal(sourceMethodResults.box.kind,'rect');
assert.equal(sourceMethodResults.ellipse.kind,'ellipse');
assert.equal(sourceMethodResults.path.kind,'path');
assert.equal(sourceMethodResults.fill.kind,'path');
assert.ok(events.some(event=>event.op==='adaptor.setAttribute'&&event.node===arrow.id&&event.name==='transform'),
  'arrow must set transform on the exact node returned by fill');

// Source methods perform ordinary dynamic property reads. Check the receiver,
// argument list, return identity, and exception propagation with replacements.
const savedSvg=wrapper.svg;
const replacementResult={kind:'replacement'};
let replacementCall;
wrapper.svg=function(...args){replacementCall={receiver:this,args};return replacementResult;};
assert.equal(wrapper.line([1,2,3,4]),replacementResult);
assert.equal(replacementCall.receiver,wrapper);
assert.equal(replacementCall.args.length,2);
assert.equal(replacementCall.args[0],'line');
assert.deepEqual(Object.keys(replacementCall.args[1]),['x1','y1','x2','y2','stroke-width']);
delete wrapper.svg;
assert.equal(wrapper.svg,savedSvg);
const savedFixed=wrapper.fixed;
let fixedCalls=0;
wrapper.fixed=function(value){fixedCalls++;return `fixed:${value}`;};
const fixedOverrideResult=wrapper.box(1,.7,.2,.15);
assert.equal(fixedCalls,6,'rounded box must dynamically read fixed for each emitted dimension');
assert.equal(fixedOverrideResult.def.x,'fixed:0.0335');
assert.equal(fixedOverrideResult.def['stroke-width'],'fixed:0.067');
assert.equal(fixedOverrideResult.def.rx,'fixed:0.15');
delete wrapper.fixed;
assert.equal(wrapper.fixed,savedFixed);
const sourceError=new Error('source method failure');
wrapper.svg=function(){throw sourceError;};
assert.throws(()=>wrapper.box(1,.7,.2),error=>error===sourceError);
delete wrapper.svg;
const savedSetAttribute=jax.adaptor.setAttribute;
jax.adaptor.setAttribute=function(){throw sourceError;};
assert.throws(()=>wrapper.arrow(1,.2,false,'X',.1),error=>error===sourceError);
jax.adaptor.setAttribute=savedSetAttribute;
const result={schema:1,source:'pinned actual SvgMenclose/SvgWrapper methods with one Jax call receiver fixture',
  sourceHashes,wrapperConstructor:wrapper.constructor.name,notationNames,
  arrowResult:arrow&&arrow.id,events};
const output=process.argv[2]||path.join(root,'build/math-port/menclose-svg-methods-original.json');
fs.mkdirSync(path.dirname(output),{recursive:true});
fs.writeFileSync(output,JSON.stringify(result,null,2)+'\n');
process.stdout.write(`SvgMenclose Source method witness -> ${output}\n`);
