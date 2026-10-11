// Root actual tiny-skia Source execution. No native renderer output is read.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),registry=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f'),source=path.join(registry,'tiny-skia-0.12.0'),copy=path.join(root,'build/mermaid-gradient-program-source'),work=path.join(root,'build/mermaid-gradient-program-reference'),manifest=path.join(__dirname,'gradient-program-reference/Cargo.toml');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex'),append=require('./gradient-program-observer.cjs');
for(const[file,pin]of Object.entries({
 'tiny-skia-0.12.0/src/shaders/gradient.rs':'c8a87c87acb85764d5e13c95fd5f66531d24e2c145513e84adfa372d9a878847',
 'tiny-skia-0.12.0/src/color.rs':'3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d',
 'tiny-skia-0.12.0/src/math.rs':'9bdc3fc49895b286a082f457ce65ace24e1fdb9f5d648c5c078ba646c517e341',
 'tiny-skia-0.12.0/src/pipeline/mod.rs':'fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4',
 'tiny-skia-path-0.12.0/src/scalar.rs':'ffec71494c2c1c281300fab24efe4a33e900e770a836ea4c85aa4665e45b9e23',
 'tiny-skia-path-0.12.0/src/transform.rs':'03f7e45c2708a416c35983ab6ad800211d5ceef92ba881b6025f2e8bc91a550f',
 'strict-num-0.1.1/src/lib.rs':'03f39058c3ce3b67e65acedd5cef0eddaa61d732303b61cee9238cdf245ff854'
}))assert.equal(hash(fs.readFileSync(path.join(registry,file))),pin);
if(!fs.existsSync(copy)){fs.cpSync(source,copy,{recursive:true});for(const[file,text]of Object.entries(append))fs.appendFileSync(path.join(copy,file),text);}
function guard(relative=''){for(const entry of fs.readdirSync(path.join(source,relative),{withFileTypes:true})){const name=path.join(relative,entry.name);if(entry.isDirectory())guard(name);else if(entry.name.endsWith('.rs'))assert.equal(fs.readFileSync(path.join(copy,name),'utf8'),fs.readFileSync(path.join(source,name),'utf8')+(append[name.split(path.sep).join('/')]??''),'Append-only observer '+name);}}
guard();
function fixture(file,pin){const bytes=fs.readFileSync(path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures',file),'utf8').replaceAll('\r\n','\n');assert.equal(hash(bytes),pin);return JSON.parse(bytes);}
const stops=fixture('gradient-stops.json','db620fb7db4ddfdde7cf47357ed8f9375be0f63d52c8576265be4058e3f6bd77').cases;
const radial=fixture('radial-stages.json','71cc0ab0cbab59eaee266a1261c8b5e2bee1aa109a6152cd3b5a0bb70e831e23').cases;
const chosen=[...new Map(radial.map(c=>[JSON.stringify(c.stages),c])).values()],buf=Buffer.alloc(4),bits=n=>{buf.writeFloatLE(n);return buf.readUInt32LE();};
const programs=[];
for(const c of stops)for(let color_space=0;color_space<4;color_space++){
 const i=programs.length,r=chosen[i%chosen.length];programs.push({...c.input,color_space,radial:i%5===0?null:[r.p,r.stages]});
}
// Additional wrapper-only failure/omission/identity transforms. Stops come from Source stimuli.
const matrices=[[0,0,0,0,0,0],[1,2,2,4,0,0],[Infinity,0,0,1,0,0],[NaN,0,0,1,0,0],[1,0,0,1,Infinity,0],[2,0,0,3,-4,5],[1,0,0,1,0,-0]].map(a=>a.map(bits));
for(let i=0;i<matrices.length;i++)for(let j=0;j<matrices.length;j++)for(let color_space=0;color_space<4;color_space++)for(let spread=0;spread<3;spread++){const r=chosen[(i+j)%chosen.length],c=stops[0].input;programs.push({...c,spread,transform:matrices[i],points_to_unit:matrices[j],color_space,radial:[r.p,r.stages]});}
const cutoff=bits(.04045),rawChannels=[...[-Infinity,-1,-0,0,2**-149,2**-126,.003,.25,.5,.75,1-2**-24,1,1+2**-23,Infinity,NaN].map(bits),cutoff-1,cutoff,cutoff+1],colors=[];
for(const x of rawChannels)for(const y of rawChannels)for(let mode=0;mode<4;mode++)colors.push([[x,y,rawChannels[(colors.length+3)%rawChannels.length],rawChannels[(colors.length+7)%rawChannels.length]],mode]);
const powers=[],xs=[...rawChannels,...[-3,3,Number.MAX_VALUE,-Number.MAX_VALUE].map(bits)],ys=[...[-Infinity,-3,-1,-0,0,.5,1,2,2.2,2.4,3,128,1e30,Infinity,NaN].map(bits)];
let seed=0x23456789;for(let i=0;i<512;i++){seed=(Math.imul(seed,1664525)+1013904223)>>>0;xs.push(bits(seed/0xffffffff));}
for(const x of xs)for(const y of ys)powers.push([x,y]);
const request={programs,colors,powers},env={...process.env,CARGO_TARGET_DIR:path.join(root,'build/mermaid-gradient-program-reference-target')};fs.mkdirSync(work,{recursive:true});
if(!fs.existsSync(path.join(path.dirname(manifest),'Cargo.lock')))cp.execFileSync('cargo',['generate-lockfile','--offline','--manifest-path',manifest],{env,stdio:'pipe'});
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest],{env,encoding:'utf8',timeout:180000});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Actual Source compiles');
const run=cp.spawnSync(path.join(env.CARGO_TARGET_DIR,'debug/folia-gradient-program-source-reference'+(process.platform==='win32'?'.exe':'')),[],{input:JSON.stringify(request),encoding:'utf8',maxBuffer:256*1024*1024,timeout:60000});fs.writeFileSync(path.join(work,'stderr.log'),run.stderr||'');assert.equal(run.status,0,'Actual Source completes normally');
const observed=JSON.parse(run.stdout);for(const key of Object.keys(request))assert.deepEqual(observed[key].map(c=>c.input),request[key]);guard();
const data=JSON.stringify({schema:1,stage:'actual Source full gradient wrapper and color expansion; no native output',...observed})+'\n',out=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/gradient-program.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else fs.writeFileSync(out,data);
console.log(JSON.stringify({programs:programs.length,colors:colors.length,powers:powers.length,radial_orders:chosen.length,accepted:observed.programs.filter(c=>c.accepted).length,returned:observed.programs.filter(c=>c.returned).length,fixture_sha256:hash(data)}));
