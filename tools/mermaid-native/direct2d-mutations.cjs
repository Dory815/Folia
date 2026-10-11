// Root-owned isolated compiling mutations. A panic/driver crash is never parity evidence.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-direct2d-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map();function copyTree(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())copyTree(f);else if(e.isFile()){const rel=path.relative(vendor,f),bytes=fs.readFileSync(f),dest=path.join(copy,rel);originals.set(rel,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}}}
copyTree(path.join(vendor,'src'));copyTree(path.join(vendor,'tests'));for(const name of ['Cargo.toml','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name));originals.set(name,bytes);fs.writeFileSync(path.join(copy,name),bytes);}
function run(name){const args=['test','--locked','--offline','--no-default-features','--features','direct2d','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','direct2d_warp'];const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Mutant must compile: '+name);const result=cp.spawnSync('cargo',args,{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-test.log'),(result.stdout||'')+(result.stderr||''));return result;}
assert.equal(run('baseline').status,0,'Unchanged baseline must pass');console.log('Baseline compiles and passes.');
const mutations=[
 ['fill-rule','geometry.rs','FillRule::EvenOdd => D2D1_FILL_MODE_ALTERNATE','FillRule::EvenOdd => D2D1_FILL_MODE_WINDING'],
 ['arc-sweep','geometry.rs','sweepDirection: if *sweep','sweepDirection: if !*sweep'],
 ['quadratic-control','geometry.rs','point1: point(*control, &location)?','point1: point(*end, &location)?'],
 ['cubic-control','geometry.rs','point1: point(*control1, &location)?','point1: point(*end, &location)?'],
 ['rounded-corner','geometry.rs','if *rx == 0.0 || *ry == 0.0','if *rx >= 0.0 || *ry >= 0.0'],
 ['gradient-transform-order','paint.rs','}.compose(gradient.transform)','}.compose(Affine::IDENTITY)'],
 ['gradient-spread','paint.rs','SpreadMode::Repeat => D2D1_EXTEND_MODE_WRAP','SpreadMode::Repeat => D2D1_EXTEND_MODE_CLAMP'],
 ['absolute-dash-units','paint.rs','*value / stroke.width','*value'],
 // Native Direct2D already repeats odd arrays; removing explicit duplication
 // survived both one- and three-element pixel checks and is an equivalent
 // mutation in this environment. Omit an actual odd-array component instead.
 ['odd-dash-component-omitted','paint.rs','cycle().take(count)','cycle().take(stroke.dash.len().min(2))'],
 ['multiply-mode','paint.rs','D2D1_BLEND_MODE_MULTIPLY.0','D2D1_BLEND_MODE_SCREEN.0'],
 ['run-origin','text.rs','run.origin.x + advance + glyph.offset.x','text.origin.x + advance + glyph.offset.x'],
 ['run-em-size','text.rs','let scale = run.em_size /','let scale = text.font.size /'],
 ['glyph-y-direction','text.rs','a: scale, d: -scale','a: scale, d: scale'],
 ['raster-clip','image.rs','let _clip = AxisClipGuard::push(context, &self.destination);','// omitted image clip'],
 ['raster-mapping','image.rs','geometry::affine(original).compose(self.mapping)','geometry::affine(original).compose(Affine::IDENTITY)'],
 ['group-opacity','../direct2d.rs','let opacity = geometry::number(group.layer.opacity,','let opacity = geometry::number(1.0,'],
 ['group-clip-transform','../direct2d.rs','clip.transform, &format!("{context}.clip.transform")','Affine::IDENTITY, &format!("{context}.clip.transform")'],
 ['device-preflight','../direct2d.rs','if identity != self.device_identity','if false'],
];
const results=[];
for(const[name,file,from,to]of mutations){const rel=path.normalize(path.join('src','backends','direct2d',file)),target=path.join(copy,rel),original=originals.get(rel),source=original.toString().replaceAll('\r\n','\n');assert.equal(source.split(from).length-1,1,'Unique mutation anchor '+name);try{fs.writeFileSync(target,source.replace(from,to));const r=run(name);assert.equal(r.status,101,'Must fail assertions, not crash: '+name);assert.match(r.stdout||'',/test result: FAILED/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation/);results.push({name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+name);}finally{fs.writeFileSync(target,original);}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working source or expected fixture changed: '+rel);
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');console.log('All '+results.length+' compiling mutants detected; originals unchanged.');
