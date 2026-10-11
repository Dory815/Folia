// Root independent replay of the bounded ignored-copy parser experiment.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),assert=require('node:assert/strict'),{isDeepStrictEqual}=require('node:util');
const root=path.resolve(__dirname,'../..'),source=path.join(root,'.reference/regress'),copy=path.join(root,'build/math-port-regexp-repairs'),work=path.join(root,'build/math-port-regexp-repair-harness');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
assert.equal(hash(fs.readFileSync(path.join(source,'src/parse.rs'))),'591243258a598d58e9ed9c4998bff3d75004ceb5dc15bb40e2f35b9c2e813833');
assert.equal(hash(fs.readFileSync(path.join(copy,'src/parse.rs'))),'c3fa593c45fdd4d617fbfe9ef17114b395cfc3e8289d59d148c98a96e2842218');
for(const name of ['Cargo.toml','src/api.rs','src/indexing.rs','src/unicodetables.rs'])assert.ok(fs.readFileSync(path.join(source,name)).equals(fs.readFileSync(path.join(copy,name))),'Unchanged candidate boundary '+name);
const cases=JSON.parse(fs.readFileSync(path.join(root,'build/math-port-regexp-candidate/node-reference.json'))),reference=JSON.parse(fs.readFileSync(path.join(root,'build/math-port-regexp-candidate/report.json')));
assert.equal(process.version,reference.node);assert.equal(hash(JSON.stringify(cases)),reference.source_case_sha256,'Same independent Node cases');
fs.mkdirSync(path.join(work,'src'),{recursive:true});
fs.writeFileSync(path.join(work,'Cargo.toml'),fs.readFileSync(path.join(__dirname,'regexp-candidate/Cargo.toml'),'utf8').replace('../../../.reference/regress','../math-port-regexp-repairs'));
for(const file of ['Cargo.lock','src/main.rs'])fs.copyFileSync(path.join(__dirname,'regexp-candidate',file),path.join(work,file));
const env={...process.env,CARGO_TARGET_DIR:path.join(root,'build/math-port-regexp-repair-probe-target')};
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',path.join(work,'Cargo.toml')],{env,encoding:'utf8'});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Experiment must compile');
const input=cases.map(({pattern,flags,text,start})=>({pattern,flags,text,start})),exe=path.join(env.CARGO_TARGET_DIR,'debug/folia-regexp-candidate-probe'+(process.platform==='win32'?'.exe':''));
const run=cp.spawnSync(exe,[],{input:JSON.stringify(input),encoding:'utf8',maxBuffer:256*1024*1024,timeout:120000});fs.writeFileSync(path.join(work,'stderr.log'),run.stderr||'');assert.equal(run.status,0);
const outputs=JSON.parse(run.stdout);assert.equal(outputs.length,cases.length);fs.writeFileSync(path.join(work,'native-results.json'),run.stdout);
const totals={};for(const mode of ['literal_flags','v_implies_unicode']){let equal=0;const by_group={};for(let i=0;i<cases.length;++i){const {diagnostic,...actual}=outputs[i][mode],c=cases[i];if(isDeepStrictEqual(actual,c.expected))++equal;else by_group[c.group]=(by_group[c.group]??0)+1;}totals[mode]={cases:cases.length,equal,different:cases.length-equal,by_group};}
const report={stage:'isolated two-parser-repair experiment; no engine or host accepted',candidate_revision:reference.revision,original_parse_sha256:hash(fs.readFileSync(path.join(source,'src/parse.rs'))),repaired_parse_sha256:hash(fs.readFileSync(path.join(copy,'src/parse.rs'))),node:reference.node,unicode:reference.unicode,icu:reference.icu,source_case_sha256:reference.source_case_sha256,totals};
fs.writeFileSync(path.join(root,'docs/math-port/evidence/p19c-regexp-parser-repair-result.json'),JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report));
