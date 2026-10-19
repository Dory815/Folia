'use strict';
// Pinned Source witness for CommonMenclose's constructor dispatch order. The
// class uses `this.method()` calls during construction, so prototype edits
// must be observed and the exact returned TRBL array must be retained.
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
const constructor=SvgWrappers.menclose, prototype=constructor.prototype;
const saved={};
for(const name of ['getParameters','getNotations','removeRedundantNotations','initializeNotations','getBBoxExtenders'])
  saved[name]=prototype[name];
const trace=[],trbl=[.1,.2,.3,.4];let instance;
prototype.getParameters=function(){instance=this;trace.push(['getParameters',this===instance]);this.padding=.45;};
prototype.getNotations=function(){trace.push(['getNotations',this===instance,this.padding]);};
prototype.removeRedundantNotations=function(){trace.push(['removeRedundantNotations',this===instance]);};
prototype.initializeNotations=function(){trace.push(['initializeNotations',this===instance]);};
prototype.getBBoxExtenders=function(){trace.push(['getBBoxExtenders',this===instance]);return trbl;};
try{
  const child=mml.create('mi',{},[mml.create('text').setText('x')]);
  instance=new SvgWrappers.menclose(factory,mml.create('menclose',{notation:'box'},[child]),parent);
}finally{
  for(const name of Object.keys(saved))prototype[name]=saved[name];
}
const result={schema:1,source:'pinned CJS actual SvgMenclose constructor with temporary prototype methods',sourceHashes,
  loadedFiles:loadedFiles.sort((a,b)=>a.file.localeCompare(b.file)),
  observation:{trace,returnedArrayRetained:instance.TRBL===trbl,trbl:[...instance.TRBL],padding:instance.padding}};
try{
  const badChild=mml.create('mi',{},[mml.create('text').setText('x')]);
  new SvgWrappers.menclose(factory,mml.create('menclose',
    {notation:'box','data-arrowhead':42},[badChild]),parent);
  result.invalidArrowhead={threw:false};
}catch(error){result.invalidArrowhead={threw:true,name:error.name,message:error.message};}
const output=process.argv[2]||path.join(root,'build/math-port/menclose-constructor-dispatch-original.json');
fs.mkdirSync(path.dirname(output),{recursive:true});fs.writeFileSync(output,JSON.stringify(result,null,2)+'\n');
process.stdout.write(`Menclose constructor dispatch witness -> ${output}\n`);
