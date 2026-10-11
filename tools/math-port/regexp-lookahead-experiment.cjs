// Root isolated replay; complete engine adoption is deliberately still open.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),assert=require('node:assert/strict'),{isDeepStrictEqual}=require('node:util');
const root=path.resolve(__dirname,'../..'),source=path.join(root,'.reference/regress'),copy=path.join(root,'build/math-port-regexp-lookahead-source'),work=path.join(root,'build/math-port-regexp-lookahead-source-harness');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const pins={'src/unicode.rs':['df896bd48e36ba4d08812bc68a378b8e1ad32a069ad67f5cf68bcc2d6ba26e12','4aebcc04392ddf6820b74bc470e3e9bfc7ea7bf486e0b203e3f589136c741635'],'src/parse.rs':['591243258a598d58e9ed9c4998bff3d75004ceb5dc15bb40e2f35b9c2e813833','3c2a911d8b65389d1a657f371784777393b2d374f7b4de4b0c3c5f2f31f2967e']};
for(const[name,[original,changed]]of Object.entries(pins)){assert.equal(hash(fs.readFileSync(path.join(source,name))),original);assert.equal(hash(fs.readFileSync(path.join(copy,name),'utf8').replaceAll('\r\n','\n')),changed);}
const protectedBytes=new Map();for(const dir of [source,copy])for(const name of ['Cargo.toml','src/api.rs','src/indexing.rs','src/unicodetables.rs']){const bytes=fs.readFileSync(path.join(dir,name));assert.ok(bytes.equals(fs.readFileSync(path.join(source,name))),'Unchanged engine boundary '+name);protectedBytes.set(path.join(dir,name),bytes);}
cp.execFileSync(process.execPath,[path.join(__dirname,'regexp-legacy-data.cjs'),'--check']);
cp.execFileSync(process.execPath,[path.join(__dirname,'regexp-lookahead-oracle.cjs'),'--check']);
cp.execFileSync(process.execPath,[path.join(__dirname,'regexp-legacy-oracle.cjs'),'--check']);
assert.equal(hash(fs.readFileSync(path.join(copy,'src/lib.rs'))),'1bcbdb8e6890f51bcb56ba3a8431900e1c44124fb215f2d974a3938904431a23');
fs.mkdirSync(path.join(work,'src'),{recursive:true});
fs.writeFileSync(path.join(work,'Cargo.toml'),fs.readFileSync(path.join(__dirname,'regexp-candidate/Cargo.toml'),'utf8').replace('../../../.reference/regress','../math-port-regexp-lookahead-source'));
for(const file of ['Cargo.lock','src/main.rs'])fs.copyFileSync(path.join(__dirname,'regexp-candidate',file),path.join(work,file));
const env={...process.env,CARGO_TARGET_DIR:path.join(root,'build/math-port-regexp-lookahead-source-probe-target')};
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',path.join(work,'Cargo.toml')],{env,encoding:'utf8',timeout:180000});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Experiment must compile');
const exe=path.join(env.CARGO_TARGET_DIR,'debug/folia-regexp-candidate-probe'+(process.platform==='win32'?'.exe':''));
const baseline=path.join(root,'build/math-port-regexp-casefold-probe-target/debug/folia-regexp-candidate-probe'+(process.platform==='win32'?'.exe':''));
const reports=[];
for(const [name,file,sha,previousFile]of [
 ['original','build/math-port-regexp-candidate/node-reference.json','5b03d68995413da826ebb481d5e8057fa1f248eaf240913ffeacc7087e1d1195','build/math-port-regexp-casefold-harness/original-native-results.json'],
 ['legacy','build/math-port-regexp-legacy-oracle/node-reference.json','ac9fbadad5d6da06de36de3458df1912bdebc9bf2d0810013d5c6edada7d7376','build/math-port-regexp-casefold-harness/legacy-native-results.json'],
 ['lookahead','build/math-port-regexp-lookahead/node-reference.json','9bedc9186f90a3f4eddfcd01fbdbe8cee6a097cc3d229639743d8cb1a801323a',null]]){
 const text=fs.readFileSync(path.join(root,file),'utf8');assert.equal(hash(text),sha,'Same independent '+name+' corpus');const cases=JSON.parse(text),input=JSON.stringify(cases.map(({pattern,flags,text,start})=>({pattern,flags,text,start})));
 function run(binary,label){const p=cp.spawnSync(binary,[],{input,encoding:'utf8',maxBuffer:512*1024*1024,timeout:120000});fs.writeFileSync(path.join(work,name+'-'+label+'-stderr.log'),p.stderr||'');assert.equal(p.status,0,name+' '+label+' must finish normally');return JSON.parse(p.stdout);}
 const actual=run(exe,'changed'),previous=previousFile?JSON.parse(fs.readFileSync(path.join(root,previousFile))):run(baseline,'baseline');assert.equal(actual.length,cases.length);assert.equal(previous.length,cases.length);
 fs.writeFileSync(path.join(work,name+'-native-results.json'),JSON.stringify(actual));
 const totals={};for(const mode of ['literal_flags','v_implies_unicode']){let equal=0,priorEqual=0,fixed=0,regressed=0;const by_group={},samples=[];
  for(let i=0;i<cases.length;++i){const {diagnostic,...result}=actual[i][mode],{diagnostic:priorDiagnostic,...prior}=previous[i][mode],c=cases[i],ok=isDeepStrictEqual(result,c.expected),wasOk=isDeepStrictEqual(prior,c.expected);if(ok)++equal;else{by_group[c.group]=(by_group[c.group]??0)+1;if(samples.length<12)samples.push({...c,actual:result,diagnostic});}if(wasOk)++priorEqual;if(ok&&!wasOk)++fixed;if(!ok&&wasOk)++regressed;}
  totals[mode]={cases:cases.length,previous_equal:priorEqual,equal,different:cases.length-equal,fixed,regressed,by_group,samples};
 }reports.push({corpus:name,source_case_sha256:sha,totals});console.log(JSON.stringify({corpus:name,totals:Object.fromEntries(Object.entries(totals).map(([k,{samples,...v}])=>[k,v]))}));
}
for(const[file,bytes]of protectedBytes)assert.ok(bytes.equals(fs.readFileSync(file)),'Protected boundary changed');
const report={stage:'isolated speculative Unicode escape rollback repair; no engine or host accepted',node:process.version,unicode:process.versions.unicode,icu:process.versions.icu,changed_source_hashes:Object.fromEntries(Object.entries(pins).map(([k,v])=>[k,hash(fs.readFileSync(path.join(copy,k)))])),data_sha256:hash(fs.readFileSync(path.join(copy,'src/legacy_case.rs'))),reports};
fs.writeFileSync(path.join(root,'docs/math-port/evidence/p20d-lookahead-regexp-repair-result.json'),JSON.stringify(report,null,2)+'\n');
