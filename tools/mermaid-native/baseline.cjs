// Root independent SVG baseline. Never imports or executes the modified vendor.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict'),cp=require('node:child_process');
const root=path.resolve(__dirname,'../..'),inventory=JSON.parse(fs.readFileSync(path.join(root,'docs/mermaid-native/source-inventory.json')));
const {fingerprint}=require('./font-environment.cjs'),fonts=fingerprint();
const reference=path.join(root,'.reference/mermaid-registry/mermaid-rs-renderer-0.3.1'),work=path.join(root,'build/mermaid-native-reference');
const hash=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');
function verify(){for(const f of inventory.source_files)assert.equal(hash(fs.readFileSync(path.join(reference,f.path))),f.sha256,f.path);}
verify();fs.mkdirSync(work,{recursive:true});
const inputs=inventory.fixture_files.filter(p=>p.endsWith('.mmd')).map(p=>({name:p.replace(/^tests\/fixtures\//,'').replace(/\.mmd$/,''),path:p,source_sha256:hash(fs.readFileSync(path.join(reference,p)))}));
assert.equal(inputs.length,47);fs.writeFileSync(path.join(work,'inputs.json'),JSON.stringify(inputs,null,2)+'\n');
const resultFile=path.join(work,'actual.json');
const built=cp.spawnSync('cargo',['run','--locked','--offline','--bin','folia-mermaid-pinned-reference','--manifest-path',path.join(__dirname,'reference/Cargo.toml'),'--target-dir',path.join(work,'target'),'--',path.join(work,'inputs.json'),reference,resultFile],{encoding:'utf8',env:{...process.env,XDG_CACHE_HOME:path.join(work,'font-cache')}});
fs.writeFileSync(path.join(work,'run.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Pinned oracle must build/run, see ignored run.log');verify();
const actual=JSON.parse(fs.readFileSync(resultFile));assert.equal(actual.length,inputs.length*10);
const results=actual.map(v=>{if(!v.svg)return v;const {svg,...rest}=v;return{...rest,svg_bytes:Buffer.byteLength(svg),svg_sha256:hash(svg)};});
const families=[...new Set(results.filter(r=>!r.error).map(r=>r.kind))].sort();assert.deepEqual(families,[...inventory.diagram_kinds].sort(),'All 23 families must have successful source observations');
assert.deepEqual(fingerprint(),fonts,'Source font environment changed during oracle execution');
const environment={source_lock_sha256:hash(fs.readFileSync(path.join(__dirname,'reference/Cargo.lock'))),folia_lock_sha256:hash(fs.readFileSync(path.join(root,'src/app-winui/third_party/usvg-normalizer/Cargo.lock'))),node:process.versions.node,fonts,cache:'ignored dedicated XDG_CACHE_HOME; original HOME unchanged'};
const output=JSON.stringify({schema:1,source:'unmodified pinned registry source',source_sha256:hash(JSON.stringify(inventory.source_files)),stage:'baseline only; no geometry/pixel/native acceptance',environment,inputs,observations:results},null,2)+'\n',file=path.join(root,'docs/mermaid-native/evidence/svg-baseline.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),output);else{fs.mkdirSync(path.dirname(file),{recursive:true});fs.writeFileSync(file,output);}
console.log(JSON.stringify({cases:results.length,families:families.length,errors:results.filter(v=>v.error).length,mode:process.argv.includes('--check')?'check':'generate'}));
