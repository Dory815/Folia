// Root observes original Context defaults; no native-derived expected constants.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),registry=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f'),source=path.join(registry,'tiny-skia-0.12.0'),copy=path.join(root,'build/mermaid-context-source'),work=path.join(root,'build/mermaid-context-default-reference');fs.mkdirSync(work,{recursive:true});
const hash=b=>crypto.createHash('sha256').update(b).digest('hex'),pins={'src/pipeline/mod.rs':'fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4'};for(const[f,p]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(source,f))),p);
const append={
 'src/lib.rs':'\n#[allow(missing_docs)]\npub use pipeline::folia_context_default;\n',
 'src/pipeline/mod.rs':`
pub fn folia_context_default()->alloc::vec::Vec<alloc::vec::Vec<u32>>{
 let c=Context::default();let color=|v:GradientColor|[v.r,v.g,v.b,v.a].map(f32::to_bits).to_vec();
 let mask:[u32;8]=bytemuck::cast(c.two_point_conical_gradient.mask);
 alloc::vec![alloc::vec![c.current_coverage.to_bits()],alloc::vec![match c.sampler.spread_mode{crate::SpreadMode::Pad=>0,crate::SpreadMode::Reflect=>1,crate::SpreadMode::Repeat=>2},c.sampler.inv_width.to_bits(),c.sampler.inv_height.to_bits()],
 [c.uniform_color.r,c.uniform_color.g,c.uniform_color.b,c.uniform_color.a].map(f32::to_bits).to_vec(),c.uniform_color.rgba.map(u32::from).to_vec(),
 color(c.evenly_spaced_2_stop_gradient.factor),color(c.evenly_spaced_2_stop_gradient.bias),alloc::vec![c.gradient.len as u32,c.gradient.factors.len()as u32,c.gradient.biases.len()as u32,c.gradient.t_values.len()as u32],
 c.gradient.factors.into_iter().flat_map(color).collect(),c.gradient.biases.into_iter().flat_map(color).collect(),c.gradient.t_values.into_iter().map(|v|v.get().to_bits()).collect(),
 [c.two_point_conical_gradient.p0,c.two_point_conical_gradient.p1].map(f32::to_bits).to_vec(),mask.to_vec(),[c.limit_x.scale,c.limit_x.inv_scale].map(f32::to_bits).to_vec(),[c.limit_y.scale,c.limit_y.inv_scale].map(f32::to_bits).to_vec(),
 [c.transform.sx,c.transform.ky,c.transform.kx,c.transform.sy,c.transform.tx,c.transform.ty].map(f32::to_bits).to_vec()]
}
`};
if(!fs.existsSync(copy)){fs.cpSync(source,copy,{recursive:true});for(const[f,a]of Object.entries(append))fs.appendFileSync(path.join(copy,f),a);}
function guard(dir=''){for(const e of fs.readdirSync(path.join(source,dir),{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())guard(f);else if(e.name.endsWith('.rs'))assert.equal(fs.readFileSync(path.join(copy,f),'utf8'),fs.readFileSync(path.join(source,f),'utf8')+(append[f.split(path.sep).join('/')]??''),'append-only '+f);}}guard();
const manifest=path.join(__dirname,'context-reference/Cargo.toml');if(!fs.existsSync(path.join(__dirname,'context-reference/Cargo.lock')))cp.execFileSync('cargo',['generate-lockfile','--offline','--manifest-path',manifest],{stdio:'pipe'});
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest,'--target-dir',path.join(work,'target')],{encoding:'utf8'});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Actual Source must compile');
const r=cp.spawnSync(path.join(work,'target/debug/folia-context-source-reference.exe'),[],{encoding:'utf8'});assert.equal(r.status,0);const groups=JSON.parse(r.stdout);assert.equal(groups.length,15);const data=JSON.stringify({schema:1,scope:'actual original pipeline Context::default; raw persistent-field projection',pins,groups})+'\n',out=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/context-default.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}console.log(JSON.stringify({groups:groups.length,sha256:hash(data)}));
