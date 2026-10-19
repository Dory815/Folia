'use strict';
// Bounded MathJax 4.1.3 oracle. A fresh output path is required.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');
const {createReference}=require('../../reference-profile.cjs');
const ref=createReference();
assert.equal(ref.provenance.upstream.version,'4.1.3');
assert.equal(ref.provenance.upstream.commit,'fb987178c2d279a99b0db5ea02e751691703955e');
const eventRegistrations=[];
const elementPrototype=Object.getPrototypeOf(ref.adaptor.node('svg',{}));
elementPrototype.addEventListener=function(type,handler){
  assert.equal(typeof type,'string');
  assert.equal(typeof handler,'function');
  eventRegistrations.push({case:caseName,type,target:ref.adaptor.kind(this)});
};
const cases=[
  {name:'single-tooltip-control',tex:'\\mathtip{x+1}{y^2}',width:1280},
  {name:'two-level-tooltip',tex:'\\mathtip{x}{\\mathtip{y}{z}}',width:1280},
  {name:'sibling-tooltips',tex:'\\mathtip{x}{y}+\\mathtip{a}{b}',width:1280},
  {name:'tooltip-newline',tex:'\\mathtip{x}{a\\newline b}',width:80},
  {name:'sibling-selected-newline',tex:'\\mathtip{x\\newline y}{z}+\\mathtip{a\\newline b}{c}',width:80},
  {name:'nested-tip-and-sibling-newline',tex:'\\mathtip{x}{\\mathtip{y}{z}}+\\mathtip{a\\newline b}{c}',width:80},
];
const events=[];
let depth=0,breakDepth=0,caseName='';
const kind=node=>node?ref.adaptor.kind(node):null;
const processMath=ref.output.processMath;
ref.output.processMath=function(wrapper,parent){
  ++depth;
  const before=ref.adaptor.childNodes(parent).length;
  events.push({case:caseName,op:'processMath.enter',depth,parent:kind(parent),before});
  try{
    return processMath.call(this,wrapper,parent);
  }finally{
    events.push({case:caseName,op:'processMath.exit',depth,parent:kind(parent),
      after:ref.adaptor.childNodes(parent).length});
    --depth;
  }
};
const handleInlineBreaks=ref.output.handleInlineBreaks;
ref.output.handleInlineBreaks=function(wrapper,svg,group){
  events.push({case:caseName,op:'handleInlineBreaks',depth,
    breakCount:wrapper.childNodes[0]?.breakCount??null,
    svgChildren:ref.adaptor.childNodes(svg).map(kind)});
  ++breakDepth;
  try{return handleInlineBreaks.call(this,wrapper,svg,group);}
  finally{--breakDepth;}
};
for(const op of ['append','insert','remove']){
  const original=ref.adaptor[op];
  ref.adaptor[op]=function(...args){
    if((depth>=2||breakDepth)&&events.length<600)
      events.push({case:caseName,op,depth,breakPhase:breakDepth>0,args:args.map(kind)});
    return original.apply(this,args);
  };
}
const records=[];
for(const item of cases){
  caseName=item.name;
  const start=events.length;
  try{
    const node=ref.document.convert(item.tex,{display:false,em:16,ex:8,
      containerWidth:item.width});
    const svg=ref.adaptor.serializeXML(node);
    records.push({...item,error:null,svg,svgLength:svg.length,
      sha256:crypto.createHash('sha256').update(svg).digest('hex'),
      nestedCalls:events.slice(start).filter(e=>e.op==='processMath.enter'&&e.depth>=2).length,
      breakCalls:events.slice(start).filter(e=>e.op==='handleInlineBreaks').map(e=>({depth:e.depth,breakCount:e.breakCount})),
      svgCount:(svg.match(/<svg\b/g)||[]).length,
      tipCount:(svg.match(/<mjx-tip\b/g)||[]).length,
      breakNodeCount:(svg.match(/<mjx-break\b/g)||[]).length});
  }catch(error){
    records.push({...item,error:{name:error.name,message:error.message},
      nestedCalls:events.slice(start).filter(e=>e.op==='processMath.enter'&&e.depth>=2).length,
      breakCalls:events.slice(start).filter(e=>e.op==='handleInlineBreaks').map(e=>({depth:e.depth,breakCount:e.breakCount}))});
  }
}
const byName=name=>records.find(record=>record.name===name);
assert.equal(records.length,6);
assert.ok(records.every(record=>record.error===null));
assert.equal(byName('single-tooltip-control').svgLength,3678);
assert.equal(byName('two-level-tooltip').nestedCalls,2);
assert.equal(byName('two-level-tooltip').tipCount,2);
assert.equal(byName('sibling-tooltips').nestedCalls,2);
assert.equal(byName('sibling-tooltips').tipCount,2);
assert.equal(byName('tooltip-newline').breakCalls.length,0);
assert.equal(byName('sibling-selected-newline').breakNodeCount,5);
assert.deepEqual(byName('nested-tip-and-sibling-newline').breakCalls,
  [{depth:1,breakCount:2}]);
assert.equal(byName('nested-tip-and-sibling-newline').nestedCalls,4);
assert.ok(!events.some(event=>event.op==='handleInlineBreaks'&&event.depth>1));
assert.ok(eventRegistrations.some(event=>event.case==='two-level-tooltip'));
const twoLevelCalls=events.filter(event=>event.case==='two-level-tooltip'&&
  event.op==='processMath.enter').map(event=>({depth:event.depth,parent:event.parent,before:event.before}));
assert.deepEqual(twoLevelCalls,[
  {depth:1,parent:'mjx-container',before:0},
  {depth:2,parent:'mjx-tip',before:0},
  {depth:3,parent:'mjx-tip',before:0},
]);
for(const name of ['sibling-selected-newline','nested-tip-and-sibling-newline']){
  const phases=events.filter(event=>event.case===name&&event.breakPhase);
  assert.deepEqual(phases[0]&&{op:phases[0].op,args:phases[0].args},
    {op:'remove',args:['g']},`${name}: Source detaches g before line creation`);
  assert.ok(phases.some(event=>event.op==='insert'&&event.args[0]==='svg'&&event.args[1]==='svg'));
}
const output={schema:1,source:ref.provenance,eventHost:'passive Lite element listener',
  records,events,eventRegistrations};
if(!process.argv[2])throw Error('A fresh output path is required');
const destination=path.resolve(process.argv[2]);
if(fs.existsSync(destination))throw Error('Refusing to overwrite an earlier capture');
fs.writeFileSync(destination,JSON.stringify(output,null,2)+'\n');
console.log(JSON.stringify({output:destination,records:records.map(({name,error,svgLength,nestedCalls,breakCalls,svgCount,tipCount,breakNodeCount})=>
  ({name,error,svgLength,nestedCalls,breakCalls,svgCount,tipCount,breakNodeCount}))}));
