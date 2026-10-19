'use strict';
// Actual pinned CommonMencloseMixin/SvgMenclose observation. The MML factory,
// wrappers, notation Map, child wrapper and BBox calculations are real Source;
// only font measurement is the explicitly bounded measurement fixture.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
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
const loadedFiles=[];
for(const filename of Object.keys(require.cache)){
  const relative=path.relative(path.join(bundle,'cjs'),filename);let file;
  if(!relative.startsWith('..')&&!path.isAbsolute(relative))file=`src/cjs/${relative.split(path.sep).join('/')}`;
  else {const p=path.relative(packageRoot,filename);if(p.startsWith('..')||path.isAbsolute(p))continue;file=p.split(path.sep).join('/');}
  const expected=lock.dependencies.find(x=>x.file===file);
  if(!expected)throw new Error(`loaded source is absent from lock: ${file}`);
  loadedFiles.push({file,sha256:sourceHashes[file]});
}
const factory=new SvgWrapperFactory(SvgWrappers);
factory.jax={font:{params:{rule_thickness:.06},getVariant:()=>null,getChar:()=>null},
  options:{scale:1,htmlHDW:'none'},math:{outputData:{}},pxPerEm:16,nodeMap:new Map(),
  measureText:()=>({w:1,h:.7,d:.2})};
const mml=new MmlFactory();
const parent=new SvgWrappers.math(factory,mml.create('math'),null);
function observe(name, attributes) {
  const child=mml.create('mi',{},[mml.create('text').setText('x')]);
  const sourceNode=mml.create('menclose',attributes,[child]);
  const wrapper=new SvgWrappers.menclose(factory,sourceNode,parent);
  const bbox=wrapper.getBBox();
  return {name,actualConstructor:wrapper.constructor.name,
    notations:Object.keys(wrapper.notations),padding:wrapper.padding,thickness:wrapper.thickness,
    arrowhead:{...wrapper.arrowhead},trbl:[...wrapper.TRBL],
    paddingEntries:wrapper.getPadding(),
    childConstructor:wrapper.childNodes[0].constructor.name,
    bbox:{w:bbox.w,h:bbox.h,d:bbox.d}};
}
const cases=[
  observe('menclose-box-construction-and-bbox',{notation:'box'}),
  observe('menclose-multiple-border-redundancy',{notation:'left right top bottom box'}),
  observe('menclose-strike',{notation:'horizontalstrike'}),
  observe('menclose-unknown-notation-is-ignored',{notation:'not-a-source-notation'}),
  observe('menclose-arrow-parameters',{notation:'updiagonalarrow','data-padding':'.3em',
    'data-thickness':'.08em','data-arrowhead':'5 3 2'}),
];
// Add an ephemeral entry to the real Source Map to observe constructor call
// order, callback receivers, and mutation visibility without substituting a
// native notation registry.
const notationMap=SvgWrappers.menclose.notations;
const savedProbe=notationMap.get('probe');
const callbackTrace=[];
notationMap.set('probe',{
  init(receiver){callbackTrace.push(`init:${receiver.constructor.name}:${receiver.padding}`);receiver.padding+=.05;},
  bbox(receiver){callbackTrace.push(`bbox:${receiver.constructor.name}:${receiver.padding}`);return [0,.2,0,.4];},
  border(receiver){callbackTrace.push(`border:${receiver.constructor.name}:${receiver.padding}`);return [0,.1,0,.1];},
  renderer(){},
});
try {
  const probe=observe('menclose-dynamic-notation-callback-order',{notation:'probe'});
  probe.callbackTrace=[...callbackTrace];
  cases.push(probe);
} finally {
  if(savedProbe===undefined)notationMap.delete('probe');else notationMap.set('probe',savedProbe);
}
const result={schema:1,source:'pinned CJS actual SvgMenclose, MmlFactory and Notation Map; bounded font measurement fixture',sourceHashes,
  loadedFiles:loadedFiles.sort((a,b)=>a.file.localeCompare(b.file)),
  cases};
const output=process.argv[2]||path.join(root,'build/math-port/menclose-original.json');
fs.mkdirSync(path.dirname(output),{recursive:true});fs.writeFileSync(output,JSON.stringify(result,null,2)+'\n');
process.stdout.write(`${cases.length} actual menclose Source cases -> ${output}\n`);
