// Root original object -> original producer -> same builder -> original run.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),source=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0'),copy=path.join(root,'build/mermaid-shader-run-source'),work=path.join(root,'build/mermaid-shader-run-reference'),append=require('./shader-run-observer.cjs'),hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const pins={
 'src/pipeline/mod.rs':'fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4',
 'src/pipeline/highp.rs':'81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416',
 'src/pipeline/lowp.rs':'3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88',
 'src/pixmap.rs':'484d49d1f4c3c3c8fb3fab3e1b1535292de828c0f258c3cab9bda02c7d9626fc',
 'src/shaders/gradient.rs':'c8a87c87acb85764d5e13c95fd5f66531d24e2c145513e84adfa372d9a878847',
 'src/shaders/linear_gradient.rs':'d18c92f0762cc5f266241da3c868a769e25fea0c224df4aa8e64cf9a22156704',
 'src/shaders/radial_gradient.rs':'3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c',
 'src/shaders/pattern.rs':'3a527adb4fad67fe147dec4d041233f35fce9fc985bdf085ab1afc4326323185',
};for(const[f,p]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(source,f))),p);
if(!fs.existsSync(copy)){fs.cpSync(source,copy,{recursive:true});for(const[f,a]of Object.entries(append))fs.appendFileSync(path.join(copy,f),a);}
function guard(dir=''){for(const e of fs.readdirSync(path.join(source,dir),{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())guard(f);else if(e.name.endsWith('.rs'))assert.equal(fs.readFileSync(path.join(copy,f),'utf8'),fs.readFileSync(path.join(source,f),'utf8')+(append[f.split(path.sep).join('/')]??''),'append-only '+f);}}guard();
const fixtureDir=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures'),prior={},priorHashes={};
for(const family of ['gradient','pattern']){const name=family==='gradient'?'gradient-shader.json':'pattern-producer.json',bytes=fs.readFileSync(path.join(fixtureDir,name));prior[family]=JSON.parse(bytes).cases;priorHashes[name]=hash(bytes);}
assert.equal(prior.gradient.length,53862);assert.equal(prior.pattern.length,13284);
const names=fs.readFileSync(path.join(source,'src/pipeline/mod.rs'),'utf8').match(/pub enum Stage \{([\s\S]*?)\n\}/)[1].split('\n').map(s=>s.trim().replace(/ = 0/,'').replace(/,$/,'')).filter(Boolean),id=n=>{const i=names.indexOf(n);assert.ok(i>=0);return i;};
const terminals=[['Store'],['SourceOverRgba'],['LoadDestination','SourceOver','Store'],['MaskU8','SourceOverRgba']].map(a=>a.map(id)),inputs=[],orders={},bands={};
const samplingBoundary=process.argv.includes('--sampling-boundary');
function add(family,index,width,terminal,shift){const p=prior[family][index].input,real_width=40,origin=[3,1];
 const image=family==='pattern'?Array.from({length:p.width*p.height*4},(_,i)=>{const pixel=Math.floor(i/4),alpha=[63,127,191,255][pixel%4];return i%4===3?alpha:Math.floor(alpha*([1,3,2][i%4]+(pixel%3))/6);}):[];
 inputs.push({family,source_index:index,producer:p,image,ids:terminals[terminal],destination:Array.from({length:real_width*4*4},(_,i)=>{const a=(Math.floor(i/4)*31+shift*7)%256;return i%4===3?a:Math.floor(a*(i%4+1)/4);}),real_width,mask:Array.from({length:real_width*4},(_,i)=>shift%3===0?0:shift%3===1?(i<real_width+19?0:255):[0,1,127,128,254,255][(i+shift)%6]),mask_width:real_width,aa_pixels:[31,223],aa_stride:0,aa_shift:origin[0],origin,size:[width,2]});
}
if(!samplingBoundary)for(const family of ['gradient','pattern']){
 const byOrder=new Map(),byBand=new Map(),stageIndex=family==='gradient'?1:2;
 for(const[index,c]of prior[family].entries()){if(c.input.prefix!==0||c.result?.[0]!=='true'||c.result[stageIndex].length>24)continue;
  const order=JSON.stringify(c.result[stageIndex]);if(!byOrder.has(order))byOrder.set(order,index);
  const band=JSON.stringify([order,c.input.force,c.input.spread,c.input.color_space,c.input.kind?.[0]??null,c.input.quality??null,c.input.sentinel]);if(!byBand.has(band))byBand.set(band,index);
 }
 orders[family]=byOrder.size;bands[family]=byBand.size;
 for(const[index,sourceIndex]of [...byOrder.values()].entries())for(const[wi,width]of [7,8,9,15,16,17].entries())for(let terminal=0;terminal<terminals.length;terminal++)add(family,sourceIndex,width,terminal,index+wi+terminal);
 // Additional force/spread/color-space/kind/sentinel bands, retaining all
 // complete old producer records; run widths and terminal suffixes rotate.
 for(const[index,sourceIndex]of [...byBand.values()].entries())add(family,sourceIndex,[7,8,9,15,16,17][index%6],index%terminals.length,index);
}
if(samplingBoundary){
 const selected=new Map();
 for(const[index,c]of prior.pattern.entries()){const p=c.input,r=c.result;
  // Positive, finite normalized opacity and every actual dimension/transform,
  // rather than using only the stage order or zero opacity representative.
  if(p.prefix!==0||p.color_space!==0||r[0]!=='true'||r[2].length>24||!r[1][4]||r[1][4]>=2139095040)continue;
  const key=JSON.stringify([p.width,p.height,p.spread,p.quality,r[1].slice(4),p.force]);if(!selected.has(key))selected.set(key,index);
 }
 bands.pattern_sampling=selected.size;
 for(const[index,sourceIndex]of [...selected.values()].entries())for(const[wi,width]of [7,8,9,15,16,17].entries())add('pattern',sourceIndex,width,wi%3,index+wi);
}
fs.mkdirSync(work,{recursive:true});const manifest=path.join(__dirname,'shader-run-reference/Cargo.toml');if(!fs.existsSync(path.join(path.dirname(manifest),'Cargo.lock')))cp.execFileSync('cargo',['generate-lockfile','--offline','--manifest-path',manifest],{stdio:'pipe'});
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest,'--target-dir',path.join(work,'target')],{encoding:'utf8'});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Original producer + original run compiles');
const run=cp.spawnSync(path.join(work,'target/debug/folia-shader-run-source-reference.exe'),[],{input:JSON.stringify(inputs),encoding:'utf8',maxBuffer:512*1024*1024});fs.writeFileSync(path.join(work,'stderr.log'),run.stderr||'');assert.equal(run.status,0,'Original full shader execution');const cases=JSON.parse(run.stdout);assert.deepEqual(cases.map(c=>c.input),inputs);guard();
for(const c of cases){assert.deepEqual(c.result[0],prior[c.input.family][c.input.source_index].result,'same original producer record');assert.deepEqual(c.warnings,prior[c.input.family][c.input.source_index].warnings,'same producer warnings');assert.ok(c.result[1]);}
const data=JSON.stringify({schema:1,scope:'controlled original Gradient/Pattern producer once, same live builder compiled and run unchanged; nonzero borrowed images, complete destination and compiled context; public Gradient constructors and blitter ownership excluded',pins,priorHashes,orders,bands,cases})+'\n',out=path.join(fixtureDir,samplingBoundary?'shader-run-sampling-boundary.json':'shader-run.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}
console.log(JSON.stringify({cases:cases.length,orders,bands,panics:cases.filter(c=>c.result[1][4]).length,context_mutations:cases.filter(c=>JSON.stringify(c.result[1][6])!==JSON.stringify(c.result[1][7])).length,sha256:hash(data)}));
