'use strict';
// Executes the pinned SvgWrapper method directly from an ordinary profile
// wrapper instance and records the actual adaptor attributes it creates.
const {createReference}=require('./reference-profile.cjs');
const ref=createReference();
const {SvgWrapper}=ref.require('@mathjax/src/js/output/svg/Wrapper.js');
const original=SvgWrapper.prototype.addBorderBroken;
const observations=[];
let captured=false;
SvgWrapper.prototype.addBorderBroken=function(...args){
  if(!captured){
    captured=true;
    const cases=[
      {name:'dotted-horizontal',style:'dotted',t:.1,i:0,path:[[0,0],[1,0]]},
      {name:'dashed-horizontal',style:'dashed',t:.1,i:0,path:[[0,0],[1,0]]},
      {name:'dotted-vertical',style:'dotted',t:.1,i:1,path:[[0,0],[0,1]]},
      {name:'dashed-vertical',style:'dashed',t:.1,i:1,path:[[0,0],[0,1]]},
      {name:'dotted-zero-geometric-extent',style:'dotted',t:.1,i:0,path:[[0,0],[0,0]]},
      {name:'dashed-zero-geometric-extent',style:'dashed',t:.1,i:1,path:[[0,0],[0,0]]},
      {name:'dotted-short-thickness',style:'dotted',t:.001,i:0,path:[[0,0],[0,0]]},
      {name:'dashed-short-thickness',style:'dashed',t:.001,i:0,path:[[0,0],[0,0]]},
      {name:'dotted-zero-thickness',style:'dotted',t:0,i:0,path:[[0,0],[0,0]]},
      {name:'dashed-zero-thickness',style:'dashed',t:0,i:0,path:[[0,0],[0,0]]},
    ];
    for(const item of cases){
      const parent=this.svg('g',{});
      original.call(this,item.path,'red',item.style,item.t,item.i,parent,0);
      const line=ref.adaptor.firstChild(parent);
      observations.push({name:item.name,style:item.style,thickness:item.t,side:item.i,
        path:item.path,attributes:ref.adaptor.allAttributes(line),xml:ref.adaptor.serializeXML(line)});
    }
  }
  return original.apply(this,args);
};
ref.document.convert('\\bbox[border:2px dotted red]{x+y}',{em:16,ex:8,containerWidth:1280});
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const expected = JSON.parse(fs.readFileSync(path.resolve(__dirname,
  '../../tests/math/fixtures/common-source/border-dash-source-expected.json')));
assert.equal(ref.provenance.compiled_source_sha256, expected.compiled_source_sha256);
assert.deepEqual(observations, expected.observations);
process.stdout.write(JSON.stringify({status:'passed',observations:observations.length,
  limit:'Actual SvgWrapper.addBorderBroken, including direct zero-thickness calls. handleBorder skips zero thickness; native finite Scene paint remains its established precondition. No full Common emitter acceptance.'})+'\n');