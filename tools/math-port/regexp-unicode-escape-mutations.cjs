// Root isolated grammar mutations over actual Node observations, no adoption claim.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),{isDeepStrictEqual}=require('node:util');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'build/math-port-regexp-unicode-escape-source'),work=path.join(root,'build/math-port-regexp-unicode-escape-mutations'),copy=path.join(work,'source'),harness=path.join(work,'harness');
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map(),originalNames=cp.execFileSync('git',['-C',path.join(root,'.reference/regress'),'ls-files','-z'],{encoding:'utf8'}).split('\0').filter(Boolean);
for(const name of [...originalNames,'src/legacy_case.rs','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name)),dest=path.join(copy,name);originals.set(name,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});fs.writeFileSync(dest,bytes);}
fs.mkdirSync(path.join(harness,'src'),{recursive:true});fs.writeFileSync(path.join(harness,'Cargo.toml'),fs.readFileSync(path.join(__dirname,'regexp-candidate/Cargo.toml'),'utf8').replace('../../../.reference/regress','../source'));
for(const file of ['Cargo.lock','src/main.rs'])fs.copyFileSync(path.join(__dirname,'regexp-candidate',file),path.join(harness,file));
const sourceText=fs.readFileSync(path.join(root,'build/math-port-regexp-unicode-escape/node-reference.json'),'utf8'),lookaheadText=fs.readFileSync(path.join(root,'build/math-port-regexp-lookahead/node-reference.json'),'utf8');
const cases=[...JSON.parse(sourceText),...JSON.parse(lookaheadText)],input=JSON.stringify(cases.map(({pattern,flags,text,start})=>({pattern,flags,text,start}))),env={...process.env,CARGO_TARGET_DIR:path.join(work,'target')};
function run(name){const p=cp.spawnSync('cargo',['build','--manifest-path',path.join(harness,'Cargo.toml'),'--locked','--offline'],{env,encoding:'utf8',timeout:180000});fs.writeFileSync(path.join(work,name+'-build.log'),(p.stdout||'')+(p.stderr||''));assert.equal(p.status,0,'Mutant must compile '+name);const r=cp.spawnSync(path.join(env.CARGO_TARGET_DIR,'debug/folia-regexp-candidate-probe'+(process.platform==='win32'?'.exe':'')),[],{input,encoding:'utf8',maxBuffer:256*1024*1024,timeout:120000});fs.writeFileSync(path.join(work,name+'-stderr.log'),r.stderr||'');assert.equal(r.status,0,'Normal completion, no crash '+name);const values=JSON.parse(r.stdout);assert.equal(values.length,cases.length);let different=0;for(let i=0;i<cases.length;i++){const {diagnostic,...observed}=values[i].v_implies_unicode;assert.notEqual(observed.constructor,'native-panic','Panic cannot count as detection');if(!isDeepStrictEqual(observed,cases[i].expected))++different;}return different;}
assert.equal(run('baseline'),0,'Frozen complete grammar baseline');console.log('Baseline '+cases.length+' cases equal.');
const mutations=[
 ['braced-gate-dropped','allow_braced && self.try_consume(\'{\')','self.try_consume(\'{\')'],
 ['ordinary-always-braced','try_escape_unicode_sequence(unicode_mode, unicode_mode)','try_escape_unicode_sequence(true, unicode_mode)'],
 ['ordinary-pairs-disabled','try_escape_unicode_sequence(unicode_mode, unicode_mode)','try_escape_unicode_sequence(unicode_mode, false)'],
 ['braced-signs-accepted','Some(c) if c.is_ascii_hexdigit() => s.push(c),','Some(c) => s.push(c),'],
 ['hex4-signs-accepted','if let Some(c) = self.next().and_then(char::from_u32)\n                    && c.is_ascii_hexdigit()\n                {','if let Some(c) = self.next().and_then(char::from_u32) {'],
 ['named-grammar-follows-flags','try_escape_unicode_sequence(true, true)','try_escape_unicode_sequence(self.flags.unicode, true)',true],
 ['named-pairs-disabled','try_escape_unicode_sequence(true, true)','try_escape_unicode_sequence(true, false)',true],
 ['code-point-range-narrowed','if u > 0x10_FFFF {','if u > 0xFFFF {'],
];
const results=[],file=path.join(copy,'src/parse.rs'),original=originals.get('src/parse.rs'),text=original.toString('utf8').replaceAll('\r\n','\n');
for(const[name,from,to,all]of mutations){assert.equal(text.split(from).length-1,all?2:1,'Mutation anchor '+name);try{fs.writeFileSync(file,all?text.replaceAll(from,to):text.replace(from,to));const different=run(name);assert.ok(different>0,'Independent observations must detect '+name);results.push({mutation:name,compiles:true,process_exit:0,different,detected:true});console.log('Detected '+name+': '+different+' differences');}finally{fs.writeFileSync(file,original);}}
for(const[file,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,file)).equals(bytes),'Original source changed '+file);
assert.equal(fs.readFileSync(path.join(root,'build/math-port-regexp-unicode-escape/node-reference.json'),'utf8'),sourceText);assert.equal(fs.readFileSync(path.join(root,'build/math-port-regexp-lookahead/node-reference.json'),'utf8'),lookaheadText);
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');console.log('All '+results.length+' compiling mutations independently detected; Source and expectations unchanged.');
