// Root actual-Source private-field observer, never linked into native output.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),manifest=path.join(__dirname,'radial-geometry-reference/Cargo.toml');
const registry=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f'),source=path.join(registry,'tiny-skia-0.12.0'),copy=path.join(root,'build/mermaid-radial-geometry-source');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const pins={'src/shaders/radial_gradient.rs':'3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c','src/shaders/gradient.rs':'c8a87c87acb85764d5e13c95fd5f66531d24e2c145513e84adfa372d9a878847'};
for(const [file,sha]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(source,file))),sha);
assert.equal(hash(fs.readFileSync(path.join(registry,'tiny-skia-path-0.12.0/src/transform.rs'))),'03f7e45c2708a416c35983ab6ad800211d5ceef92ba881b6025f2e8bc91a550f');
const append={
 'src/shaders/gradient.rs':`\n// Read-only root observation shim, outside production native modules.\nimpl Gradient {\n pub(crate) fn folia_source_geometry(&self) -> (Transform,bool) {\n  (self.points_to_unit,self.stops.len()==3 && self.stops[0].position.get()==0.0 && self.stops[1].position.get()==1.0 && self.stops[2].position.get()==1.0 && self.stops[0].color==self.stops[1].color)\n }\n}\n`,
 'src/shaders/radial_gradient.rs':`\n// Read-only root observation shim; constructor and geometry bodies unchanged.\nimpl RadialGradient {\n #[allow(missing_docs)]\n pub fn folia_source_snapshot(&self) -> (u8,[u32;6],[u32;3],bool,bool) {\n  let (m,thin_ring)=self.base.folia_source_geometry();\n  let matrix=[m.sx,m.ky,m.kx,m.sy,m.tx,m.ty].map(f32::to_bits);\n  let (tag,parameters,swapped)=match self.gradient_type {\n   GradientType::Radial {radius1,radius2}=>(2,[radius1,radius2,0.0],false),\n   GradientType::Strip {scaled_r0}=>(3,[scaled_r0,0.0,0.0],false),\n   GradientType::Focal(fd)=>(4,[fd.r1,fd.focal_x,0.0],fd.is_swapped),\n  };\n  (tag,matrix,parameters.map(f32::to_bits),swapped,thin_ring)\n }\n}\n`
};
if(!fs.existsSync(copy)){fs.cpSync(source,copy,{recursive:true});for(const[file,text]of Object.entries(append))fs.appendFileSync(path.join(copy,file),text);}
for(const[file,text]of Object.entries(append))assert.equal(fs.readFileSync(path.join(copy,file),'utf8'),fs.readFileSync(path.join(source,file),'utf8')+text,'Only read-only append shim permitted');
const buf=Buffer.alloc(4),bits=n=>{buf.writeFloatLE(n);return buf.readUInt32LE();},E=1/32768,N=1/4096;
const shapes=[
 [0,0,0,0,0,10],[3,4,0,3,4,5],[0,0,2,0,0,10],[0,0,10,0,0,2],[0,0,2,0,0,2],[0,0,0,0,0,0],
 [0,0,-1,0,0,5],[0,0,1,0,0,-1],[0,0,-0,0,0,10],
 ...[E*(1-2**-23),E,E*(1+2**-23),N*(1-2**-23),N,N*(1+2**-23)].flatMap(d=>[[0,0,0,d,0,10],[0,0,2,d,0,2],[0,0,2,d,0,3]]),
 ...[0,E*(1-2**-23),E,E*(1+2**-23)].map(r=>[0,0,r,0,0,10]),
 ...[[0,10],[2,10],[10,2],[2,2],[10,0],[2,0],[0,2],[1,2],[1,3],[3,1],[2,3],[3,2]].map(([r0,r1])=>[0,0,r0,4,0,r1]),
 [3,-2,2,-4,7,10],[0,0,2,1e30,1e30,10],[0,0,2,Infinity,0,10],[NaN,0,2,0,0,10],
 [0,0,NaN,4,0,10],[0,0,2,4,0,NaN],[0,0,Infinity,4,0,10],[0,0,2,4,0,Infinity]
];
const transforms=[[1,0,0,1,0,0],[0,0,0,0,0,0],[2,0,0,3,7,-2],[1,.25,-.5,2,13,9],[-1,0,0,1,3,4],[1,2,2,4,0,0],[1,0,0,1,Infinity,0]];
const input=shapes.flatMap(circles=>transforms.flatMap(transform=>[0,1,2,3].flatMap(stops=>[0,1,2].map(spread=>({circles:circles.map(bits),transform:transform.map(bits),stops,spread})))));
const env={...process.env,CARGO_TARGET_DIR:path.join(root,'build/mermaid-radial-geometry-reference-target')},work=path.join(root,'build/mermaid-radial-geometry-reference');fs.mkdirSync(work,{recursive:true});
if(!fs.existsSync(path.join(path.dirname(manifest),'Cargo.lock')))cp.execFileSync('cargo',['generate-lockfile','--offline','--manifest-path',manifest],{env,stdio:'pipe'});
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest],{env,encoding:'utf8',timeout:180000});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Source oracle compiles');
const run=cp.spawnSync(path.join(env.CARGO_TARGET_DIR,'debug/folia-radial-geometry-source-reference'+(process.platform==='win32'?'.exe':'')),[],{input:JSON.stringify(input),encoding:'utf8',maxBuffer:32*1024*1024,timeout:30000});assert.equal(run.status,0,'Actual Source completes without panic');
const cases=JSON.parse(run.stdout);assert.deepEqual(cases.map(c=>c.input),input);assert.equal(cases.length,input.length);
const text='{"schema":1,"stage":"actual Source geometry projection; no native output used","cases":[\n'+cases.map(c=>JSON.stringify(c)).join(',\n')+'\n]}\n',target=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/radial-geometry.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),text);else fs.writeFileSync(target,text);
for(const[file,sha]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(source,file))),sha);
console.log(JSON.stringify({cases:cases.length,by_tag:Object.fromEntries([0,1,2,3,4].map(tag=>[tag,cases.filter(c=>c.tag===tag).length])),thin_rings:cases.filter(c=>c.thin_ring).length,fixture_sha256:hash(text),mode:process.argv.includes('--check')?'check':'generate'}));
