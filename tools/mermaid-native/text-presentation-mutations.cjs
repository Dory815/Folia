// Root isolated mutation checks for the frozen complete text presentation helpers.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-text-presentation-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map();function copyTree(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())copyTree(f);else if(e.isFile()){const rel=path.relative(vendor,f),bytes=fs.readFileSync(f),dest=path.join(copy,rel);originals.set(rel,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}}}
copyTree(path.join(vendor,'src'));copyTree(path.join(vendor,'tests'));for(const name of ['Cargo.toml','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name));originals.set(name,bytes);fs.writeFileSync(path.join(copy,name),bytes);}
function run(name){const args=['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','text_presentation_parity'];const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Mutant must compile: '+name);const result=cp.spawnSync('cargo',args,{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-test.log'),(result.stdout||'')+(result.stderr||''));return result;}
assert.equal(run('baseline').status,0,'Unchanged baseline must pass');console.log('Baseline compiles and passes.');
const mutations=[
  [
    "family-fallback",
    "\"sans-serif\".to_string()",
    "\"serif\".to_string()",
    1
  ],
  [
    "family-trim-order",
    ".trim_matches('\\'').trim_matches('\"')",
    ".trim_matches('\"').trim_matches('\\'')",
    1
  ],
  [
    "divider-trim",
    "line.trim() == \"---\"",
    "line == \"---\"",
    1
  ],
  [
    "git-trailing-empty-lines",
    "text.split('\\n')",
    "text.lines()",
    1
  ],
  [
    "git-first-y",
    "let start_y = y + font_size;",
    "let start_y = y;",
    1
  ],
  [
    "git-dy-size",
    "let line_height_px = font_size * line_height;",
    "let line_height_px = line_height;",
    1
  ],
  [
    "git-anchor",
    "TextValue::Literal(\"start\".to_string())",
    "TextValue::Literal(\"middle\".to_string())",
    1
  ],
  [
    "git-color-provenance",
    "TextValue::Literal(color.to_string())",
    "TextValue::AttributeLexeme(color.to_string())",
    1
  ],
  [
    "block-theme-size",
    "config, theme.font_size, \"middle\"",
    "config, 10.0, \"middle\"",
    1
  ],
  [
    "block-anchor-forward",
    "config, theme.font_size, anchor",
    "config, theme.font_size, \"end\"",
    1
  ],
  [
    "block-baseline-forward",
    "override_color, None, baseline,",
    "override_color, None, false,",
    1
  ],
  [
    "weight-empty-omission",
    ".filter(|weight| !weight.trim().is_empty())",
    ".filter(|_| true)",
    1
  ],
  [
    "weight-trim-retention",
    "TextValue::AttributeLexeme(weight.to_string())",
    "TextValue::AttributeLexeme(weight.trim().to_string())",
    1
  ],
  [
    "line-explicit-size",
    "x, y, &theme.font_family, font_size, None,",
    "x, y, &theme.font_family, theme.font_size, None,",
    1
  ],
  [
    "line-default-size",
    "theme, theme.font_size, fill",
    "theme, 10.0, fill",
    1
  ],
  [
    "c4-dy-offset",
    "font_size * (line_count - 1.0) / 2.0",
    "font_size * line_count / 2.0",
    1
  ],
  [
    "c4-parent-baseline",
    "dominant_baseline: Some(\"middle\".to_string())",
    "dominant_baseline: None",
    1
  ],
  [
    "c4-child-baseline",
    "alignment_baseline: Some(\"mathematical\".to_string())",
    "alignment_baseline: None",
    1
  ],
  [
    "c4-italic-presence",
    "if italic { Some(true) } else { None }",
    "Some(italic)",
    1
  ],
  [
    "c4-color-provenance",
    "TextValue::Literal(fill.to_string())",
    "TextValue::AttributeLexeme(fill.to_string())",
    1
  ],
  [
    "ordinary-zero-lines",
    "// The source has no zero-line guard: preserve its empty parent element.\n    Ok(vec![presentation])",
    "// The source has no zero-line guard: preserve its empty parent element.\n    if label.lines.is_empty() { Ok(Vec::new()) } else { Ok(vec![presentation]) }",
    1
  ],
  [
    "block-unused-intermediate",
    "let start_y = if baseline {",
    "finite_emitted(total_height, \"mutant-unused\")?;\n    let start_y = if baseline {",
    1
  ],
  [
    "font-binary-widening",
    "parse_decimal(&value.to_string(), context)",
    "Ok(f64::from(value))",
    1
  ],
  [
    "numeric-precision",
    "format!(\"{value:.2}\")",
    "format!(\"{value:.3}\")",
    1
  ]
];
const file=path.join('src','scene','compile','text_helpers.rs'),target=path.join(copy,file),original=originals.get(file),source=original.toString().replaceAll('\r\n','\n'),results=[];
for(const[name,from,to,count=1]of mutations){const froms=Array.isArray(from)?from:[from],tos=Array.isArray(to)?to:[to];let mutated=source;for(let i=0;i<froms.length;i++){assert.equal(source.split(froms[i]).length-1,count,'Mutation anchor '+name);mutated=mutated.replaceAll(froms[i],tos[i]);}try{fs.writeFileSync(target,mutated);const r=run(name);assert.equal(r.status,101,'Must fail assertions, not crash: '+name);assert.match(r.stdout||'',/test result: FAILED/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation/);results.push({name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+name);}finally{fs.writeFileSync(target,original);}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working source or expected fixture changed: '+rel);
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');console.log('All '+results.length+' compiling mutants detected; originals unchanged.');
