'use strict';
// Pinned Source observations for CommonXmlNode over the real MmlFactory,
// SvgWrapperFactory, LiteAdaptor, and XML node classes. The Jax object only
// supplies bounded metrics/options; the XML/HTML node graph is actual Source.
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
const {LiteAdaptor}=require(path.join(bundle,'cjs/adaptors/liteAdaptor.js'));
const adaptor=new LiteAdaptor();
const loadedFiles=[];
for(const filename of Object.keys(require.cache)){
  const relative=path.relative(path.join(bundle,'cjs'),filename);let file;
  if(!relative.startsWith('..')&&!path.isAbsolute(relative))file=`src/cjs/${relative.split(path.sep).join('/')}`;
  else{const p=path.relative(packageRoot,filename);if(p.startsWith('..')||path.isAbsolute(p))continue;file=p.split(path.sep).join('/');}
  const expected=lock.dependencies.find(x=>x.file===file);
  if(!expected)throw new Error(`loaded source is absent from lock: ${file}`);
  loadedFiles.push({file,sha256:sourceHashes[file]});
}
const mml=new MmlFactory();
const factory=new SvgWrapperFactory(SvgWrappers);
const start=adaptor.node('div');
const jax={
  font:{params:{rule_thickness:.06},getVariant:()=>null,getChar:()=>null},
  adaptor,options:{htmlHDW:'use',scale:1},math:{metrics:{em:16,scale:2,family:'SourceFont'},start:{node:start}},
  pxPerEm:16,nodeMap:new Map(),fixed:(n)=>Number(n).toFixed(3),container:adaptor.node('div'),
  html:(kind,definition={},children=[])=>adaptor.node(kind,definition,children),
};
factory.jax=jax;
const parentNode=mml.create('math');
const parent=new SvgWrappers.math(factory,parentNode,null);
parent.variant='normal';
parent.styles={get:()=>null};
const xml=adaptor.node('section',{'data-mjx-hdw':'1em .25em 2em'},[adaptor.text('xml')]);
const xmlNode=mml.create('XML').setXML(xml,adaptor);
const wrapper=new SvgWrappers.XML(factory,xmlNode,parent);
const bbox=wrapper.getBBox();
const html=wrapper.getHTML();
const result={schema:1,source:'pinned CJS actual SvgXmlNode/MmlFactory/LiteAdaptor; bounded Jax metrics',sourceHashes,
  loadedFiles:loadedFiles.sort((a,b)=>a.file.localeCompare(b.file)),
  cases:[{
    name:'xml-node-hdw-use-and-html-clone',actualConstructor:wrapper.constructor.name,
    xmlKind:adaptor.kind(xml),bbox:{w:bbox.w,h:bbox.h,d:bbox.d},rscale:wrapper.rscale,
    htmlKind:adaptor.kind(html),htmlSerialized:adaptor.outerHTML(html),
    clonedChildKind:adaptor.kind(adaptor.firstChild(html)),
    originalChildText:adaptor.textContent(xml),
    clonedChildText:adaptor.textContent(adaptor.firstChild(html)),
  }]};
const output=process.argv[2]||path.join(root,'build/math-port/xml-node-original.json');
fs.mkdirSync(path.dirname(output),{recursive:true});fs.writeFileSync(output,JSON.stringify(result,null,2)+'\n');
process.stdout.write(`1 actual XmlNode Source case -> ${output}\n`);
