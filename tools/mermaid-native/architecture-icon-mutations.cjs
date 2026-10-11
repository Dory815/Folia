// Root isolated mutation checks for the complete frozen Architecture icon helper.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-architecture-icon-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map();function copyTree(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())copyTree(f);else if(e.isFile()){const rel=path.relative(vendor,f),bytes=fs.readFileSync(f),dest=path.join(copy,rel);originals.set(rel,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}}}
copyTree(path.join(vendor,'src'));copyTree(path.join(vendor,'tests'));for(const name of ['Cargo.toml','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name));originals.set(name,bytes);fs.writeFileSync(path.join(copy,name),bytes);}
function run(name){const args=['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','architecture_icons_parity'];const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Mutant must compile: '+name);const result=cp.spawnSync('cargo',args,{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-test.log'),(result.stdout||'')+(result.stderr||''));return result;}
assert.equal(run('baseline').status,0,'Unchanged baseline must pass');console.log('Baseline compiles and passes.');
const mutations=[
  [
    "unicode-lowercase-instead-of-ascii",
    "to_ascii_lowercase()",
    "to_lowercase()"
  ],
  [
    "substring-internet-treated-as-exact",
    "\"internet\" | \"globe\" =>",
    "\"internet\" | \"globe\" | \"internet-server\" =>"
  ],
  [
    "gateway-alias-omitted",
    "name.contains(\"gateway\")",
    "false"
  ],
  [
    "compute-alias-omitted",
    "name.contains(\"compute\")",
    "false"
  ],
  [
    "glacier-alias-omitted",
    "name.contains(\"glacier\")",
    "false"
  ],
  [
    "globe-vertical-line-omitted",
    "            items.push(shape(line((cx, cy - r), (cx, cy + r))?, &outline));",
    ""
  ],
  [
    "server-row-count",
    "let rows = 3;",
    "let rows = 2;"
  ],
  [
    "server-row-height",
    "let row_h = bh / rows as f32;",
    "let row_h = bh / 2.0;"
  ],
  [
    "server-indicator-fill",
    "fill: fill.clone(), ..PaintStyle::default()",
    "fill: Paint::None, ..PaintStyle::default()"
  ],
  [
    "server-shape-order",
    "items.push(shape(rectangle(bx, ry, bw, row_h, 2.0)?, outline));",
    "items.insert(0,shape(rectangle(bx, ry, bw, row_h, 2.0)?, outline));"
  ],
  [
    "server-indicator-position",
    "bw - row_h * 0.35",
    "bw - row_h * 0.5"
  ],
  [
    "server-indicator-radius",
    "row_h * 0.12",
    "row_h * 0.13"
  ],
  [
    "exact-database-lines-omitted",
    "database_icon(cx, cy, r, true, &outline, &mut items)?",
    "database_icon(cx, cy, r, false, &outline, &mut items)?"
  ],
  [
    "substring-database-lines-added",
    "database_icon(cx, cy, r, false, &outline, &mut items)?",
    "database_icon(cx, cy, r, true, &outline, &mut items)?"
  ],
  [
    "database-ellipse-size",
    "let ell_ry = r * 0.3;",
    "let ell_ry = r * 0.2;"
  ],
  [
    "database-painter-order",
    "ellipse(cx, body_top, r, ell_ry)?",
    "ellipse(cx, body_bot, r, ell_ry)?"
  ],
  [
    "cloud-control-position",
    "let y_top = cy - s * 0.8;",
    "let y_top = cy - s * 0.7;"
  ],
  [
    "cloud-unclosed-path",
    "        PathSegment::Close,",
    ""
  ],
  [
    "cloud-wrong-curve-end",
    "end: point(cx, y_top, \"architecture_icon.cloud.top_left_end\")?",
    "end: point(x2, y_top, \"architecture_icon.cloud.top_left_end\")?"
  ],
  [
    "fallback-square-radius",
    "rectangle(x, y, s * 2.0, s * 2.0, 4.0)",
    "rectangle(x, y, s * 2.0, s * 2.0, 3.0)"
  ],
  [
    "fallback-inner-proportion",
    "let inner = s * 0.35;",
    "let inner = s * 0.25;"
  ],
  [
    "outline-has-fill",
    "fill: Paint::None,\n        stroke: Some",
    "fill: fill.clone(),\n        stroke: Some"
  ],
  [
    "outline-cap",
    "cap: LineCap::Round",
    "cap: LineCap::Butt"
  ],
  [
    "outline-join",
    "join: LineJoin::Round",
    "join: LineJoin::Miter"
  ],
  [
    "geometry-decimal-precision",
    "decimal(value, 1, context)",
    "decimal(value, 2, context)"
  ],
  [
    "stroke-decimal-precision",
    "one_decimal(sw, \"architecture_icon.stroke_width\")",
    "decimal(sw, 2, \"architecture_icon.stroke_width\")"
  ],
  [
    "negative-size-clamped",
    "let emitted = one_decimal(value, context)?;",
    "let emitted = one_decimal(value.max(0.0), context)?;"
  ],
  [
    "lambda-font-precision",
    "decimal(value, 0, \"architecture_icon.lambda.font_size\")",
    "decimal(value, 1, \"architecture_icon.lambda.font_size\")"
  ],
  [
    "lambda-anchor",
    "TextValue::Literal(\"middle\".to_owned())",
    "TextValue::Literal(\"start\".to_owned())"
  ],
  [
    "lambda-baseline",
    "Some(\"central\".to_owned())",
    "Some(\"middle\".to_owned())"
  ],
  [
    "lambda-family",
    "TextValue::Literal(\"serif\".to_owned())",
    "TextValue::Literal(\"sans-serif\".to_owned())"
  ],
  [
    "lambda-fill-is-literal",
    "TextValue::AttributeLexeme(fill_lexeme.to_owned())",
    "TextValue::Literal(fill_lexeme.to_owned())"
  ],
  [
    "lambda-glyph",
    "TextContentPresentation::Direct(\"λ\".to_owned())",
    "TextContentPresentation::Direct(\"Λ\".to_owned())"
  ]
];
const file=path.join('src','scene','compile','architecture_icons.rs'),target=path.join(copy,file),original=originals.get(file),source=original.toString().replaceAll('\r\n','\n'),results=[];
for(const[name,from,to,count=1]of mutations){const froms=Array.isArray(from)?from:[from],tos=Array.isArray(to)?to:[to];let mutated=source;for(let i=0;i<froms.length;i++){assert.equal(source.split(froms[i]).length-1,count,'Mutation anchor '+name);mutated=mutated.replaceAll(froms[i],tos[i]);}try{fs.writeFileSync(target,mutated);const r=run(name);assert.equal(r.status,101,'Must fail assertions, not crash: '+name);assert.match(r.stdout||'',/test result: FAILED/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation/);results.push({name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+name);}finally{fs.writeFileSync(target,original);}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working source or expected fixture changed: '+rel);
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');console.log('All '+results.length+' compiling mutants detected; originals unchanged.');
