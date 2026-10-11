// Root isolated mutation checks for the frozen complete text presentation helpers.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-edge-label-geometry-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map();function copyTree(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())copyTree(f);else if(e.isFile()){const rel=path.relative(vendor,f),bytes=fs.readFileSync(f),dest=path.join(copy,rel);originals.set(rel,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}}}
copyTree(path.join(vendor,'src'));copyTree(path.join(vendor,'tests'));for(const name of ['Cargo.toml','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name));originals.set(name,bytes);fs.writeFileSync(path.join(copy,name),bytes);}
function run(name){const args=['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','edge_label_geometry_parity'];const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Mutant must compile: '+name);const result=cp.spawnSync('cargo',args,{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-test.log'),(result.stdout||'')+(result.stderr||''));return result;}
assert.equal(run('baseline').status,0,'Unchanged baseline must pass');console.log('Baseline compiles and passes.');
const mutations=[
  [
    "center-padding-x",
    "pad_x * 2.0",
    "pad_x * 1.0"
  ],
  [
    "center-nonnegative-width",
    "(label_w + pad_x * 2.0).max(0.0)",
    "label_w + pad_x * 2.0"
  ],
  [
    "center-horizontal-half",
    "center_x - width * 0.5",
    "center_x - width"
  ],
  [
    "short-polyline-background",
    "edge_points.len() < 2",
    "edge_points.len() < 1"
  ],
  [
    "flowchart-center-threshold",
    "crate::ir::DiagramKind::Flowchart => 1.2",
    "crate::ir::DiagramKind::Flowchart => 1.0"
  ],
  [
    "sequence-center-height-factor",
    "rect.height * 0.16",
    "rect.height * 0.12"
  ],
  [
    "requirement-center-threshold",
    "crate::ir::DiagramKind::Requirement => 1.0",
    "crate::ir::DiagramKind::Requirement => 0.9"
  ],
  [
    "other-center-threshold",
    "_ => 0.9",
    "_ => 0.35"
  ],
  [
    "sequence-endpoint-height-factor",
    "rect.height * 0.12",
    "rect.height * 0.16"
  ],
  [
    "flowchart-endpoint-threshold",
    "gap <= 0.35",
    "gap <= 0.9"
  ],
  [
    "other-endpoints-visible",
    "_ => false",
    "_ => true"
  ],
  [
    "polyline-only-first-segment",
    "points.windows(2)",
    "points.windows(2).take(1)"
  ],
  [
    "polyline-near-zero-not-collapsed",
    "best <= 1e-6",
    "best <= 0.0"
  ],
  [
    "segment-rectangle-corners-omitted",
    "for corner in corners {",
    "for corner in corners.into_iter().take(0) {"
  ],
  [
    "point-rectangle-dy-omitted",
    "(dx * dx + dy * dy).sqrt()",
    "(dx * dx).sqrt()",
    3
  ],
  [
    "segment-degenerate-everywhere",
    "len_sq <= 1e-9",
    "true"
  ],
  [
    "point-segment-projection-unclamped",
    "let t = t.clamp(0.0, 1.0);",
    "let t = t;"
  ],
  [
    "rectangle-only-top-edge",
    "edges\n        .iter()",
    "edges\n        .iter().take(1)"
  ],
  [
    "rectangle-left-exclusive",
    "point.0 >= rect.x",
    "point.0 > rect.x"
  ],
  [
    "collinearity-epsilon-inclusive",
    "o1.abs() < eps",
    "o1.abs() <= eps"
  ],
  [
    "orientation-inverted",
    "(b.0 - a.0) * (c.1 - a.1) - (b.1 - a.1) * (c.0 - a.0)",
    "(b.1 - a.1) * (c.0 - a.0) - (b.0 - a.0) * (c.1 - a.1)"
  ],
  [
    "segment-bound-box-epsilon-omitted",
    "c.1 >= a.1.min(b.1) - eps",
    "c.1 >= a.1.min(b.1)"
  ]
];
const file=path.join('src','scene','compile','edge_label_geometry.rs'),target=path.join(copy,file),original=originals.get(file),source=original.toString().replaceAll('\r\n','\n'),results=[];
for(const[name,from,to,count=1]of mutations){const froms=Array.isArray(from)?from:[from],tos=Array.isArray(to)?to:[to];let mutated=source;for(let i=0;i<froms.length;i++){assert.equal(source.split(froms[i]).length-1,count,'Mutation anchor '+name);mutated=mutated.replaceAll(froms[i],tos[i]);}try{fs.writeFileSync(target,mutated);const r=run(name);assert.equal(r.status,101,'Must fail assertions, not crash: '+name);assert.match(r.stdout||'',/test result: FAILED/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation/);results.push({name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+name);}finally{fs.writeFileSync(target,original);}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working source or expected fixture changed: '+rel);
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');console.log('All '+results.length+' compiling mutants detected; originals unchanged.');
