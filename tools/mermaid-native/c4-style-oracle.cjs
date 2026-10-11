// Root actual-source oracle, independent of the native C4 scene selectors.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),reference=path.join(root,'.reference/mermaid-registry/mermaid-rs-renderer-0.3.1'),work=path.join(root,'build/mermaid-c4-style-reference');fs.mkdirSync(work,{recursive:true});
const hash=b=>crypto.createHash('sha256').update(b).digest('hex'),source=fs.readFileSync(path.join(reference,'src/render.rs'));
assert.equal(hash(source),'026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4');
const sourceText=source.toString(),start=sourceText.indexOf('fn c4_shape_colors('),end=sourceText.indexOf('\nfn text_block_svg_class(',start),extracted=sourceText.slice(start,end);
assert.deepEqual([...extracted.matchAll(/^fn (\w+)\(/gm)].map(m=>m[1]),['c4_shape_colors','c4_shape_font_family','c4_shape_font_size','c4_shape_font_weight']);
for(const f of ['src/config.rs','src/ir.rs'])assert.ok(fs.readFileSync(path.join(reference,f)).equals(fs.readFileSync(path.join(root,'third_party/mermaid-rs-renderer',f))),f+' must remain pinned');
const kinds=['Person','ExternalPerson','System','SystemDb','SystemQueue','ExternalSystem','ExternalSystemDb','ExternalSystemQueue','Container','ContainerDb','ContainerQueue','ExternalContainer','ExternalContainerDb','ExternalContainerQueue','Component','ComponentDb','ComponentQueue','ExternalComponent','ExternalComponentDb','ExternalComponentQueue'];
const prefixes=kinds.map(k=>k.replaceAll(/([a-z])([A-Z])/g,'$1_$2').toLowerCase());
const stringFields=prefixes.flatMap(k=>['bg_color','border_color','font_family','font_weight'].map(s=>k+'_'+s)),sizeFields=prefixes.map(k=>k+'_font_size');
const helper=`// Root-owned stimuli and config-address lookup; no selector expectations.
const STRING_FIELDS:&[&str]=&[${stringFields.map(JSON.stringify).join(',')}];
fn kind(name:&str)->crate::ir::C4ShapeKind{match name{${kinds.map(k=>JSON.stringify(k)+'=>crate::ir::C4ShapeKind::'+k+',').join('')} _=>panic!("unknown test variant")}}
fn string_field<'a>(conf:&'a crate::config::C4Config,key:&str)->&'a str{match key{${stringFields.map(k=>JSON.stringify(k)+'=>&conf.'+k+',').join('')} _=>panic!("unknown string field")}}
fn set_size(conf:&mut crate::config::C4Config,key:&str,bits:u32){let value=f32::from_bits(bits);match key{${sizeFields.map(k=>JSON.stringify(k)+'=>conf.'+k+'=value,').join('')} _=>panic!("unknown size field")}}
fn stimulus(input:&serde_json::Value)->crate::config::C4Config{
 let preset=input["preset"].as_str().unwrap();let mut values=serde_json::to_value(crate::config::C4Config::default()).unwrap();
 if preset=="unique" {for key in STRING_FIELDS{values[*key]=serde_json::Value::String(format!("{key}| <>&\\\"\\t"));}}
 if preset=="raw" {for key in STRING_FIELDS{values[*key]=input["raw"].clone();}}
 if let Some(key)=input["field"].as_str(){if !key.ends_with("font_size"){values[key]=input["raw"].clone();}}
 let mut conf:crate::config::C4Config=serde_json::from_value(values).unwrap();
 if preset=="unique" {${sizeFields.map((k,i)=>'conf.'+k+'='+(100+i)+'.25;').join('')}}
 if let Some(bits)=input["all_bits"].as_u64(){${sizeFields.map(k=>'conf.'+k+'=f32::from_bits(bits as u32);').join('')}}
 if let Some(key)=input["field"].as_str(){if key.ends_with("font_size"){set_size(&mut conf,key,input["bits"].as_u64().unwrap() as u32);}}
 conf
}
`;
const cases=[];for(const preset of ['default','unique'])for(const kind of kinds)cases.push({kind,preset});
for(const field of [...stringFields,...sizeFields])for(const kind of kinds)cases.push({kind,preset:'unique',field,...(field.endsWith('font_size')?{bits:0x80000000}:{raw:field+'|mutated\0&";color:red'})});
for(const raw of ['',' \t\n','&amp;','x" y="9','Arial; fill:red','数学 🧮','مرحبا','\0'])for(const kind of kinds)cases.push({kind,preset:'raw',raw});
for(const all_bits of [0,0x80000000,1,0x80000001,0x7f800000,0xff800000,0x7fc00019,0xffc00031,0x7f7fffff])for(const kind of kinds)cases.push({kind,preset:'unique',all_bits});
const exact=path.join(work,'source-functions.rs'),stimuli=path.join(work,'stimuli.rs'),inputs=path.join(work,'inputs.json'),actual=path.join(work,'actual.json');fs.writeFileSync(exact,extracted);fs.writeFileSync(stimuli,helper);fs.writeFileSync(inputs,JSON.stringify(cases));
const result=cp.spawnSync('cargo',['run','--locked','--offline','--manifest-path',path.join(__dirname,'c4-style-reference/Cargo.toml'),'--target-dir',path.join(root,'build/mermaid-node-shape-reference/target'),'--',inputs,actual],{encoding:'utf8',env:{...process.env,FOLIA_C4_STYLE_SOURCE:exact,FOLIA_C4_STYLE_STIMULI:stimuli}});fs.writeFileSync(path.join(work,'run.log'),(result.stdout||'')+(result.stderr||''));assert.equal(result.status,0,'Unchanged source C4 selectors must compile/execute');
const output=JSON.stringify({source_sha256:hash(source),extracted_sha256:hash(extracted),cases:JSON.parse(fs.readFileSync(actual))})+'\n';
const fixture=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/c4-style.json'),nativeHelper=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/c4-style-stimuli.rs');
if(process.argv.includes('--check')){assert.equal(fs.readFileSync(fixture,'utf8').replaceAll('\r\n','\n'),output);assert.equal(fs.readFileSync(nativeHelper,'utf8').replaceAll('\r\n','\n'),helper);}else{fs.writeFileSync(fixture,output);fs.writeFileSync(nativeHelper,helper);}
console.log(JSON.stringify({source_functions:4,variants:kinds.length,cases:cases.length,mode:process.argv.includes('--check')?'check':'generate'}));
