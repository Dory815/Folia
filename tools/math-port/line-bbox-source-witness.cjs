'use strict';
const path=require('node:path');
const fs=require('node:fs');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');
const args=process.argv.slice(2);
const rootIndex=args.indexOf('--repo-root');
if(rootIndex>=0&&(!args[rootIndex+1]||args.length!==rootIndex+2))
  throw Error('usage: node line-bbox-source-witness.cjs [--repo-root <repository> ]');
const repo=rootIndex>=0?path.resolve(args[rootIndex+1]):path.resolve(__dirname,'../..');
const bundle=path.join(repo,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const files={
  'output/common/LineBBox.js':'5e6464e467ef82116d6933af020450bacb354d518848049188cd0dde0b2eadd2',
  'util/BBox.js':'792428ce9802696fe669aa8a13b1a92f5f2eaadd07e05fc28bd359acd8d2062d',
  'core/MmlTree/Attributes.js':'efa567c0c573f13f136f1d37b0af6bdc1a675f70cc13a9ae1cd7334321f30749',
};
for(const [name,expected] of Object.entries(files)) {
  const bytes=fs.readFileSync(path.join(bundle,name));
  if(crypto.createHash('sha256').update(bytes).digest('hex')!==expected) throw Error(`Pinned Source hash mismatch: ${name}`);
}
const {LineBBox}=require(path.join(bundle,'output/common/LineBBox.js'));
const {BBox}=require(path.join(bundle,'util/BBox.js'));
const {Attributes}=require(path.join(bundle,'core/MmlTree/Attributes.js'));
const box=new BBox({w:2.4,h:1.2,d:.3});
Object.assign(box,{L:.25,R:.15,rscale:.75,scale:1.3,ic:.1,pwidth:'40%'});
const from=LineBBox.from(box,.2);
const child=new LineBBox({w:2,h:.8,d:.2});
Object.assign(child,{L:.3,R:.2,rscale:.5,lineLeading:.4,indentData:[['first','0'],['normal','1'],['last','2']]});
const parent=new LineBBox({w:1,h:.2,d:.1});
parent.isFirst=true; parent.append(child);
const cloned=parent.copy();
const copiedIndent=parent.copyIndentData(parent);
const inherited={indentalign:'right',indentshift:'1em',indentalignfirst:'indentalign',
  indentshiftfirst:'indentshift',indentalignlast:'indentalign',indentshiftlast:'indentshift'};
const node={attributes:{getAllAttributes:()=>inherited,getInherited:name=>
  ({indentalign:'center',indentshift:'2em'})[name]}};
const indentBox=new LineBBox(); indentBox.getIndentData(node);
const absent=new LineBBox(); absent.getIndentData({attributes:{getAllAttributes:()=>({}),getInherited:()=>undefined}});
const witness={
  source:{package:'@mathjax/src 4.1.3',hashes:files},
  from:{w:from.w,h:from.h,d:from.d,L:from.L,R:from.R,scale:from.scale,rscale:from.rscale,
    ic:from.ic,pwidth:from.pwidth,lineLeading:from.lineLeading,originalL:from.originalL},
  append:{w:parent.w,h:parent.h,d:parent.d,childL:child.L,childOriginalL:child.originalL,
    lineLeading:parent.lineLeading,isFirst:parent.isFirst,sharedIndent:parent.indentData===child.indentData},
  copy:{w:cloned.w,h:cloned.h,d:cloned.d,sharedIndent:cloned.indentData===parent.indentData,
    copiedIndentIndependent:copiedIndent!==parent.indentData},
  indent:indentBox.indentData,absent:absent.indentData,
};
if(JSON.stringify(witness.from)!==JSON.stringify({w:2.4,h:1.2,d:.3,L:.25,R:.15,scale:1.3,rscale:.75,ic:.1,pwidth:'40%',lineLeading:.2,originalL:0}))
  throw Error('Unexpected Source from result');
if(witness.append.w!==2.1||witness.append.h!==.4||witness.append.d!==.1||witness.append.childOriginalL!==.3||!witness.append.sharedIndent)
  throw Error('Unexpected Source append result');
assert.deepStrictEqual(witness.copy,{w:2.1,h:.4,d:.1,sharedIndent:true,copiedIndentIndependent:true});
assert.notStrictEqual(copiedIndent,parent.indentData);
assert.notStrictEqual(copiedIndent[0],parent.indentData[0]);
assert.deepStrictEqual(witness.indent,[['center','2em'],['right','1em'],['right','1em']]);
assert.deepStrictEqual(witness.absent,[[undefined,undefined],[undefined,undefined],[undefined,undefined]]);
const sentinelAttributes=new Attributes({}, {indentalign:'center'});
sentinelAttributes.set('indentalign','_inherit_');
const sentinelLine=new LineBBox();
sentinelLine.getIndentData({attributes:sentinelAttributes});
assert.equal(sentinelLine.indentData[1][0],'_inherit_');
assert.equal(sentinelAttributes.get('indentalign'),'center');
witness.sentinel={raw:sentinelLine.indentData[1][0],resolved:sentinelAttributes.get('indentalign')};
process.stdout.write(JSON.stringify(witness,null,2)+'\n');
