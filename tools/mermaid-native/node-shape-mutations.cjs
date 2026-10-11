// Root isolated mutation checks for the complete frozen node-shape helper.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-node-shape-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map();function copyTree(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())copyTree(f);else if(e.isFile()){const rel=path.relative(vendor,f),bytes=fs.readFileSync(f),dest=path.join(copy,rel);originals.set(rel,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}}}
copyTree(path.join(vendor,'src'));copyTree(path.join(vendor,'tests'));for(const name of ['Cargo.toml','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name));originals.set(name,bytes);fs.writeFileSync(path.join(copy,name),bytes);}
function run(name){const args=['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','node_shapes_parity'];const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Mutant must compile: '+name);const result=cp.spawnSync('cargo',args,{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-test.log'),(result.stdout||'')+(result.stderr||''));return result;}
assert.equal(run('baseline').status,0,'Unchanged baseline must pass');console.log('Baseline compiles and passes.');
const mutations=[
  [
    "rectangle-radius",
    "rectangle(x, y, w, h, 3.0, 3.0)",
    "rectangle(x, y, w, h, 4.0, 4.0)"
  ],
  [
    "fork-radius",
    "rectangle(x, y, w, h, 2.0, 2.0)",
    "rectangle(x, y, w, h, 3.0, 3.0)"
  ],
  [
    "circle-state-start-role",
    "(&paints.line, &paints.line)",
    "(&paints.primary_text, &paints.primary_text)"
  ],
  [
    "circle-state-end-role",
    "(&paints.primary_border, &paints.primary_border)",
    "(&paints.fill, &paints.stroke)"
  ],
  [
    "double-circle-inner-omitted",
    "if r2 > 0.0",
    "if false"
  ],
  [
    "inner-threshold-after-quantization",
    "if r2 > 0.0",
    "if two_decimals(r2, \"mutant\")? > 0.0"
  ],
  [
    "inner-dash-inherited",
    "&[], true, 1.0)))",
    "dash, true, 1.0)))"
  ],
  [
    "inner-width-flattened",
    "let inner_width = if label_empty || is_state_end { 1.2 } else { 1.0 };",
    "let inner_width = 1.0;"
  ],
  [
    "cylinder-painter-order",
    "ellipse(cx, y + ry, rx, ry)?",
    "ellipse(cx, y + h - ry, rx, ry)?"
  ],
  [
    "cylinder-body-clamp",
    "(h - 2.0 * ry).max(0.0)",
    "(h - 2.0 * ry).max(1.0)"
  ],
  [
    "subroutine-line-dash",
    "let line_style = style(&Paint::None, &paints.stroke, width, &[], true, 1.0);",
    "let line_style = style(&Paint::None, &paints.stroke, width, dash, true, 1.0);"
  ],
  [
    "polygon-slant",
    "let slant = w * 0.22;",
    "let slant = w * 0.18;"
  ],
  [
    "mindmap-divider-opacity",
    "&[], false, 0.35",
    "&[], false, 1.0"
  ],
  [
    "mindmap-divider-raw-width",
    "raw_width.max(0.8)",
    "raw_width.max(1.0)"
  ],
  [
    "mindmap-relative-first-corner",
    "relative(current, radius, neg_radius)?",
    "relative(current, radius, radius)?"
  ],
  [
    "mindmap-close-omitted",
    "segments.push(PathSegment::Close);",
    "/* omitted */"
  ],
  [
    "default-shape-radius",
    "rectangle(x, y, w, h, 6.0, 6.0)",
    "rectangle(x, y, w, h, 5.0, 5.0)",
    2
  ],
  [
    "round-line-caps",
    "if round { LineCap::Round }",
    "if round { LineCap::Square }"
  ],
  [
    "width-binary-widening",
    "parse_decimal(&value.to_string(), context)?",
    "f64::from(value)"
  ],
  [
    "decimal-precision",
    "format!(\"{value:.2}\")",
    "format!(\"{value:.3}\")"
  ]
];
const file=path.join('src','scene','compile','node_shapes.rs'),target=path.join(copy,file),original=originals.get(file),source=original.toString().replaceAll('\r\n','\n'),results=[];
for(const[name,from,to,count=1]of mutations){const froms=Array.isArray(from)?from:[from],tos=Array.isArray(to)?to:[to];let mutated=source;for(let i=0;i<froms.length;i++){assert.equal(source.split(froms[i]).length-1,count,'Mutation anchor '+name);mutated=mutated.replaceAll(froms[i],tos[i]);}try{fs.writeFileSync(target,mutated);const r=run(name);assert.equal(r.status,101,'Must fail assertions, not crash: '+name);assert.match(r.stdout||'',/test result: FAILED/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation/);results.push({name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+name);}finally{fs.writeFileSync(target,original);}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working source or expected fixture changed: '+rel);
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');console.log('All '+results.length+' compiling mutants detected; originals unchanged.');
