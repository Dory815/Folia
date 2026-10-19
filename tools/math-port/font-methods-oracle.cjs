#!/usr/bin/env node
'use strict';

// Supplemental pinned-source probes for FontData operations not fully
// distinguished by the whole-table hash. This is evidence only; the Root-owned
// full table/lifecycle golds remain unchanged.
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'../..');
const packageRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax');
const {MathJaxNewcmFont:Font}=require(path.join(packageRoot,'mathjax-newcm-font/cjs/svg.js'));
const {AddPaths}=require(path.join(packageRoot,'src/cjs/output/svg/FontData.js'));
const {DIRECTION,V,H}=require(path.join(packageRoot,'src/cjs/output/common/Direction.js'));
const mathjax=require(path.join(packageRoot,'src/cjs/mathjax.js')).mathjax;
const normalize=(v)=>v===undefined?{$:'undefined'}:v===null?null:Array.isArray(v)?Array.from({length:v.length},(_,i)=>Object.hasOwn(v,i)?normalize(v[i]):{$:'hole'}):typeof v==='function'?{$:'function'}:typeof v==='number'&&!Number.isFinite(v)?{$:Number.isNaN(v)?'NaN':v<0?'-Infinity':'Infinity'}:typeof v==='object'?Object.fromEntries(Object.keys(v).sort().map(k=>[k,normalize(v[k])])):v;
const observe=(fn)=>{try{return {value:normalize(fn())};}catch(e){return {error:{name:e?.name||typeof e,message:e?.message||String(e)}};}};
const results={};

results.direction={none:DIRECTION.None,vertical:DIRECTION.Vertical,horizontal:DIRECTION.Horizontal,
  vAlias:V===DIRECTION.Vertical,hAlias:H===DIRECTION.Horizontal};

{
  const map={65:[1,0,.5],66:[1,0,.5,{keep:'old'}]};
  const paths={65:'M65',66:'M66'},content={65:'A',66:'B'};
  const returned=AddPaths(map,paths,content);
  results.addPaths={sameMap:returned===map,chars:normalize(map),sameExistingOptions:map[66][3].keep==='old'};
  const bad={65:{p:'not-a-tuple'}};
  results.addPathsMalformed=observe(()=>AddPaths(bad,{65:'M'},{}));
}

{
  const font=new Font();
  font.createVariant('linked-probe','normal','bold');
  const tuple=[2,0,.75,{p:'linked-probe'}];
  font.defineChars('bold',{0xE000:tuple});
  const linked=font.getChar('linked-probe',0xE000);
  results.linkedVariant={record:normalize(linked),sameRecord:linked===tuple,
    normalBoldZero:font.getChar('normal',48)===font.getChar('bold',48),
    boldItalicZero:font.getChar('bold',48)===font.getChar('italic',48)};
  results.missingGlyph=normalize(font.getChar('normal',0x10ffff));
}

{
  const previousFiles=Font.dynamicFiles;
  const previousSync=mathjax.asyncIsSynchronous,previousLoad=mathjax.asyncLoad;
  const file='__font_method_probe__.js';
  const defs=Font.defineDynamicFiles([[file,{normal:[0xE100]},[0x28]]]);
  Font.dynamicFiles={...previousFiles,...defs};
  const paths=[];
  mathjax.asyncIsSynchronous=true;
  mathjax.asyncLoad=(url)=>{paths.push(String(url));Font.dynamicSetup('',file,
    {normal:{0xE100:[1,0,.5,{p:'M1 0'}]}},{0x28:{dir:'v',variants:[0],stretchv:[0]}});};
  try{
    const font=new Font({dynamicPrefix:'probe-prefix'});
    const dynamic=font.getChar('normal',0xE100);
    results.dynamicSync={paths,record:normalize(dynamic),failed:Font.dynamicFiles[file].failed,
      setupInstalled:typeof Font.dynamicFiles[file].setup,
      delimiter:normalize(font.getDelimiter(0x28)),sizeVariant:normalize(font.getSizeVariant(0x28,0)),
      stretchVariant:normalize(font.getStretchVariant(0x28,0))};
  }finally{Font.dynamicFiles=previousFiles;mathjax.asyncIsSynchronous=previousSync;mathjax.asyncLoad=previousLoad;}
}

{
  const font=new Font();
  results.invalidQueries={missingSize:normalize(font.getSizeVariant(0x10ffff,0)),
    missingStretch:observe(()=>font.getStretchVariant(0x10ffff,0)),
    missingVariant:observe(()=>font.getChar('no-such-variant',65)),
    invalidRemap:normalize(font.getRemappedChar('not-a-map',65))};
}

const destination=path.join(root,'build/math-port/font-methods-original.json');
fs.mkdirSync(path.dirname(destination),{recursive:true});
fs.writeFileSync(destination,JSON.stringify({source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e',results},null,2)+'\n');
console.log(`${path.relative(root,destination)}: supplemental FontData/Direction source observations`);
