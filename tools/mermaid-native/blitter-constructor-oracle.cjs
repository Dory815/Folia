// Guarded append-only actual tiny-skia public factory -> unchanged RasterPipelineBlitter constructors.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),source=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0');
const copy=path.join(root,'build/mermaid-blitter-constructor-source'),work=path.join(root,'build/mermaid-blitter-constructor-reference');
const append=require('./blitter-constructor-observer.cjs'),hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const pins={
 'src/blend_mode.rs':'841dcc8eb96fe11fc2316fa0a4ff47c29fd837990ce8c1d8be5fcf08f6c86d19',
 'src/color.rs':'3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d',
 'src/geom.rs':'b93fdae6878e40c4b8b7b0e4f608634c78c6281e30f18e399d093b5b9c945ab9',
 'src/lib.rs':'6c306bab841bd9874d95286fb48c970ace62c4dca41e42f8f1c7fd9a23c4809e',
 'src/pipeline/mod.rs':'fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4',
 'src/pipeline/blitter.rs':'a3189191d6ad4e1e03e738d21f5a26f310b7c00c33bae0021f0961054ad08f06',
 'src/pipeline/highp.rs':'81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416',
 'src/pipeline/lowp.rs':'3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88',
 'src/painter.rs':'40f1d9523ab9875d83496cb7a1b4570aee2d958204dc91424fb6a5bd00c97105',
 'src/shaders/mod.rs':'4f80ecb40d5a82985d97324748779cffd98252fca798c01c3f26969a1c27cbd3',
 'src/shaders/linear_gradient.rs':'d18c92f0762cc5f266241da3c868a769e25fea0c224df4aa8e64cf9a22156704',
 'src/shaders/radial_gradient.rs':'3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c',
 'src/shaders/sweep_gradient.rs':'7e9c4223a5be5b78eafa20b7869d269732bd6c950e72abf70bfd475c4ed58816',
 'src/shaders/pattern.rs':'3a527adb4fad67fe147dec4d041233f35fce9fc985bdf085ab1afc4326323185',
 'src/shaders/gradient.rs':'c8a87c87acb85764d5e13c95fd5f66531d24e2c145513e84adfa372d9a878847',
 'src/pixmap.rs':'484d49d1f4c3c3c8fb3fab3e1b1535292de828c0f258c3cab9bda02c7d9626fc',
 'src/mask.rs':'5c6b8d615f219f78ceebc714a56e9916745eeb93fc9213b87c96b3f00f55792b',
};
for(const[f,p]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(source,f))),p,`pin ${f}`);
if(!fs.existsSync(copy)){fs.cpSync(source,copy,{recursive:true});for(const[f,suffix]of Object.entries(append))fs.appendFileSync(path.join(copy,f),suffix);}
function guard(dir=''){for(const e of fs.readdirSync(path.join(source,dir),{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())guard(f);else if(e.name.endsWith('.rs')){const key=f.split(path.sep).join('/');assert.equal(fs.readFileSync(path.join(copy,f),'utf8'),fs.readFileSync(path.join(source,f),'utf8')+(append[key]??''),`append-only ${key}`);}}}guard();
const b=Buffer.alloc(4),bits=v=>{b.writeFloatLE(v);return b.readUInt32LE();};
const identity=[bits(1),bits(0),bits(0),bits(1),bits(0),bits(0)];
const baseStops=[[bits(0),bits(.1),bits(.2),bits(.3),bits(1)],[bits(1),bits(.8),bits(.6),bits(.4),bits(1)]];
const image=[18,37,56,64,45,90,30,128,100,50,150,192,40,80,120,255,3,7,11,13];
function base(kind,blend=3,color_space=0,label='base'){
 const width=9,height=4;
 return{label,kind,blend,color_space,anti_alias:true,force_hq:false,color:[bits(.2),bits(.4),bits(.7),bits(.5)],
  stops:baseStops.map(s=>s.slice()),geometry:kind===3?[bits(0),bits(0),bits(0),bits(360),bits(0),bits(0)]:[bits(0),bits(0),bits(20),bits(0),bits(0),bits(20)],spread:0,transform:identity.slice(),
  pattern:[2,2,0,bits(1)],image:image.slice(),transforms:[],opacities:[],
  target:Array.from({length:width*height*4},(_,i)=>{const a=64+(Math.floor(i/4)*13)%192;return i%4===3?a:Math.floor(a*(i%4+1)/4);}),target_size:[width,height],target_rect:null,mask:null};
}
const cases=[];
for(let blend=0;blend<29;blend++)for(let color_space=0;color_space<4;color_space++)for(let kind=0;kind<5;kind++)cases.push(base(kind,blend,color_space,`product-${blend}-${color_space}-${kind}`));
assert.equal(cases.length,580);
const edge=(label,kind,edit)=>{const v=base(kind,3,0,label);edit(v);cases.push(v);};
const sameMask=()=>({bytes:Array.from({length:9*4},(_,i)=>[0,1,127,255][i%4]),size:[9,4],rect:null});
for(const aa of [false,true])for(const hq of [false,true])for(const mask of [false,true])edge(`aa-hq-mask-${+aa}-${+hq}-${+mask}`,0,i=>{i.anti_alias=aa;i.force_hq=hq;if(mask)i.mask=sameMask();});
for(const mask of [false,true])for(const kind of [0,1,2,3,4])edge(`opaque-source-over-${kind}-${+mask}`,kind,i=>{i.color=[bits(.2),bits(.4),bits(.7),bits(1)];if(mask)i.mask=sameMask();});
for(const kind of [0,1,2,3,4])edge(`destination-in-opaque-${kind}`,kind,i=>{i.blend=6;i.color[3]=bits(1);});
edge('destination-mask-mismatch-first',0,i=>{i.blend=2;i.mask={bytes:new Array(8*4).fill(255),size:[8,4],rect:null};});
for(const aa of [false,true])for(const mask of [false,true])edge(`clear-aa-mask-${+aa}-${+mask}`,4,i=>{i.blend=0;i.anti_alias=aa;if(mask)i.mask=sameMask();});
for(const c of [0,.5/255,1.5/255,.2,.5,1])edge(`source-memset-${c}`,0,i=>{i.blend=1;i.color=[bits(c),bits(.4),bits(.7),bits(.5)];});
for(const quality of [0,1,2])edge(`pattern-quality-${quality}`,4,i=>{i.pattern[2]=quality;i.transform=[bits(.8),0,0,bits(1.2),bits(2.3),bits(-1.7)];});
edge('pattern-inversion-warning',4,i=>{i.transform=[bits(1),bits(1),bits(1),bits(1),0,0];});
edge('factory-none-solid-invalid-color',0,i=>{i.color[0]=bits(-1);});
edge('factory-none-linear-empty-stops',1,i=>{i.stops=[];});
edge('factory-none-radial-negative-radius',2,i=>{i.geometry[4]=bits(-1);});
edge('factory-none-sweep-reversed-angles',3,i=>{i.geometry[2]=bits(90);i.geometry[3]=bits(10);});
edge('factory-none-pattern-short-image',4,i=>{i.image=[0,0,0,0];});
edge('factory-public-opacity-nan',4,i=>{i.opacities=[bits(NaN)];});
edge('clipped-distinct-strides',4,i=>{const tw=13,th=4,mw=11,mh=4;i.target_size=[tw,th];i.target=Array.from({length:tw*th*4+8},(_,n)=>n%256);i.target_rect={xy:[2,1],wh:[7,2]};i.mask={bytes:Array.from({length:mw*mh},(_,n)=>n%256),size:[mw,mh],rect:{xy:[1,1],wh:[7,2]}};});
edge('new-mask-whole',5,i=>{i.target_size=[9,4];i.target=Array.from({length:36},(_,n)=>n*7%256);});
edge('new-mask-clipped',5,i=>{i.target_size=[11,4];i.target=Array.from({length:44},(_,n)=>n*11%256);i.target_rect={xy:[2,1],wh:[7,2]};});

fs.mkdirSync(work,{recursive:true});const manifest=path.join(__dirname,'blitter-constructor-reference/Cargo.toml');
if(!fs.existsSync(path.join(path.dirname(manifest),'Cargo.lock')))cp.execFileSync('cargo',['generate-lockfile','--offline','--manifest-path',manifest],{stdio:'pipe'});
const build=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest,'--target-dir',path.join(work,'target')],{encoding:'utf8'});
fs.writeFileSync(path.join(work,'build.log'),(build.stdout||'')+(build.stderr||''));assert.equal(build.status,0,'original append-only crate and transport compile: '+(build.stderr||''));
const run=cp.spawnSync(path.join(work,'target/debug/folia-blitter-constructor-source-reference.exe'),[],{input:JSON.stringify(cases),encoding:'utf8',maxBuffer:512*1024*1024});
fs.writeFileSync(path.join(work,'stderr.log'),run.stderr||'');assert.equal(run.status,0,'original constructors execute: '+(run.stderr||''));
const records=JSON.parse(run.stdout);assert.equal(records.length,cases.length);assert.deepEqual(records.map(r=>r.input),cases);guard();
const actualVariants=['SolidColor','LinearGradient','RadialGradient','SweepGradient','Pattern'];
const product=new Set();for(const record of records.slice(0,580)){
 const {blend,color_space,kind}=record.input,key=`${blend}/${color_space}/${kind}`;assert.ok(!product.has(key));product.add(key);
 assert.equal(record.factory,'Some',`public factory ${key}`);assert.equal(record.actual_variant,actualVariants[kind],`actual variant ${key}`);
 assert.equal(record.constructor,blend===2?'None':'Some',`original constructor policy outcome ${key}`);
}assert.equal(product.size,580);
for(const record of records){if(record.constructor==='Some'){assert.ok(record.snapshot);for(const name of ['anti_h','rect','mask']){assert.ok(record.snapshot[name]);assert.equal(record.snapshot[name].context.length,15);}}else assert.equal(record.snapshot,null);}
const out=path.join(work,'candidate.json'),data=JSON.stringify({schema:1,scope:'actual original public Shader/Paint factories, unchanged RasterPipelineBlitter new/new_mask, three actual compiled owners; constructor snapshots only',pins,cases:records})+'\n';
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else fs.writeFileSync(out,data);
const frozen=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/blitter-constructor.json');
if(process.argv.includes('--freeze')){assert.ok(!fs.existsSync(frozen),'Root Source gold may be frozen only once');fs.writeFileSync(frozen,data);}
if(process.argv.includes('--check-frozen'))assert.equal(fs.readFileSync(frozen,'utf8').replaceAll('\r\n','\n'),data,'Unchanged actual original constructor gold');
const tally=(key)=>Object.fromEntries([...new Set(records.map(r=>r[key]))].map(v=>[v,records.filter(r=>r[key]===v).length]));
console.log(JSON.stringify({candidate:out,cases:records.length,product:580,factory:tally('factory'),constructor:tally('constructor'),warnings:records.filter(r=>r.warnings.length).map(r=>[r.input.label,r.warnings]),sha256:hash(data)}));
