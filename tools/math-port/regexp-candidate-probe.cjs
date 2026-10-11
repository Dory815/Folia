// Root independent Node/candidate differential. This is feasibility, not acceptance.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-regexp-candidate');fs.mkdirSync(work,{recursive:true});
const {isDeepStrictEqual}=require('node:util');
const candidate=path.join(root,'.reference/regress'),revision='771e6c8fa1e472ef86c45b5206b37fa4e4494e25';
assert.equal(cp.execFileSync('git',['-C',candidate,'rev-parse','HEAD'],{encoding:'utf8'}).trim(),revision);
const hash=bytes=>crypto.createHash('sha256').update(bytes).digest('hex'),sources=new Map();
const pins={'src/api.rs':'b748e71e56e40c46108b86842dc0face872ff9b0d4f36f5612aa0f61731c424a','src/parse.rs':'591243258a598d58e9ed9c4998bff3d75004ceb5dc15bb40e2f35b9c2e813833','src/indexing.rs':'8df75d23395147e16a06112aba619d8305eba6a0455046c2fe3291ed303ab5f4','src/unicodetables.rs':'e2f63d08fbe2f0b523986ad29dd87e5e9c24a3e4389632f043b2e1185d859849','Cargo.toml':'c78ec2e4f0ca4f84c005db377926c7d38d5d9a35cf5267906f897b23d1dc3e5c'};
for(const name of ['src/api.rs','src/parse.rs','src/indexing.rs','src/unicodetables.rs','Cargo.toml'])sources.set(name,fs.readFileSync(path.join(candidate,name)));
for(const[name,bytes]of sources)assert.equal(hash(bytes),pins[name],'Pinned candidate source '+name);
const sourceBase=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/base/BaseConfiguration.js');
const baseBytes=fs.readFileSync(sourceBase),baseHash=hash(baseBytes);
assert.equal(baseHash,'0feedc60ac600b647cf2a1437c7244196a5aec04715f4a301fe6e9ec09722c57','Pinned BaseConfiguration reference');
const {BaseConfiguration}=require(sourceBase);
const units=s=>Array.from({length:s.length},(_,i)=>s.charCodeAt(i));
function outcome(pattern,flags,text,start){let re;try{re=new RegExp(pattern,flags);}catch(e){assert.equal(e.name,'SyntaxError');return {constructor:e.name};}
 // Add d only after independently validating the original constructor flags.
 if(!flags.includes('d'))re=new RegExp(pattern,flags+'d');re.lastIndex=start;
 const m=re.exec(text);return {constructor:'ok',match:m?{ranges:m.indices.map(r=>r??null),named:Object.keys(m.indices.groups??{}).map(name=>({name,range:m.indices.groups[name]??null}))}:null,last_index:re.lastIndex};}
const cases=[],keys=new Set();function add(group,pattern,flags,text,start){const key=JSON.stringify([pattern,flags,text,start]);if(keys.has(key))return;keys.add(key);cases.push({id:cases.length,group,pattern:units(pattern),flags,text:units(text),start,expected:outcome(pattern,flags,text,start)});}
const texts=['','a','aa','abc','baab','ab\ncd','\r\n','123.45','12{,}345.6','.5','Kſİı','αβ','中文','\ud83d\ude00','a\ud83d\ude00b','\ud800x\udfff','\ud800\udc00','\u2028\u2029','👩‍👩‍👧‍👦','🇨🇳'];
const starts=text=>[...new Set([0,1,2,text.length,text.length+1])];
for(const [name,value]of Object.entries(BaseConfiguration.options))if(value instanceof RegExp){for(const text of [...texts,'0','1,234','1{,}234{,}567.89','-.5','ab12'])for(const start of starts(text))for(const flags of [value.flags,'g','y','u','gu','yu','v','gv','yv'])add('base/'+name,value.source,flags,text,start);}
const patterns=['','.', '^.$','^','$', '^.*$', '\\d+', '\\D+', '\\s+', '\\S+', '\\w+', '\\W+', '\\b', '\\B', '(a)?(b)?', '(a|ab)+', '(a*)*', '(a+?)', '(?:a|)', 'a{0,2}', 'a{2,}', 'a{', 'a}', ']', '[^]', '[a-z]', '[^a-z]', '[a-z-]', '[a--b]', '[\\d-a]', '\\1(a)', '(a)\\1', '(a|(b))\\2', '(?<x>a)|(?<y>b)', '(?<x>a)|(?<x>b)', '(?<x>a)\\k<x>', '(?=(a+))a*b\\1', '(?<=([ab]+)([bc]+))$', '(?<!a)b', '(?<=([ab])+)c', '(?<=\\1(a))b', '(?<=a|bc)d', '(?=a)*', '\\cA', '\\c_', '\\a', '\\8', '\\01', '\\077', '\\u{1F600}', '\\uD83D\\uDE00', '\\uD800', '\\uDC00', '\\xZZ', '[', '(', '(?:)', '(?x:a)', '(?i:a)', 'a++', '(?>a)', '(?<1>a)', '(?<x>a)(?<x>b)', '\\p{Letter}', '\\P{Letter}', '\\p{Script=Greek}', '\\p{Assigned}', '\\p{ASCII}', '\\p{Script=Beria_Erfe}', '\\p{Script=Tolong_Siki}', '[\\p{ASCII}&&\\p{Letter}]', '[\\p{ASCII}--[a-z]]', '[[a-z]&&[^b]]', '[\\q{ab|a|}]', '\\p{RGI_Emoji}', '[\\p{RGI_Emoji}--\\q{😀}]', '[()]', '[a&&b]', '[a--b]', '\\P{Lowercase_Letter}', '[^\\p{Lowercase_Letter}]', '\\p{Uppercase_Letter}', '😀', '[😀]', '\ud800', '\udc00'];
for(const pattern of patterns)for(const flags of ['','i','u','ui','v','vi','g','gu','gv','y','yu','yv','m','s','ims'])for(const text of texts)for(const start of starts(text))add('syntax-capture-unicode',pattern,flags,text,start);
for(let bits=0;bits<256;++bits){const flags=[...'dgimsuvy'].filter((_,i)=>bits&(1<<i)).join('');for(const text of ['a','ba','😀'])for(const start of starts(text))add('flag-power-set','(?<x>.)?',flags,text,start);}
for(const flags of ['gg','ii','uu','vv','dd','yy','uv','vu','x','a','G','😀','gug','dgyg'])add('invalid-flags','a',flags,'a',0);
for(const cpValue of [0x11db0,0x16ea0,0x1e6c0,0x105c0,0x1c89,0x1c8a,0x10d40,0x10d50])for(const pattern of ['\\p{Assigned}','\\p{Letter}','\\p{Script=Beria_Erfe}','\\p{Script=Tolong_Siki}','\\p{Script=Tai_Yo}','.'])for(const flags of ['u','ui','v','vi'])add('unicode-version',pattern,flags,String.fromCodePoint(cpValue),0);
const input=cases.map(({pattern,flags,text,start})=>({pattern,flags,text,start}));
fs.writeFileSync(path.join(work,'input.json'),JSON.stringify(input));fs.writeFileSync(path.join(work,'node-reference.json'),JSON.stringify(cases));
const manifest=path.join(__dirname,'regexp-candidate/Cargo.toml'),env={...process.env,CARGO_TARGET_DIR:path.join(root,'build/math-port-regexp-candidate-target')};
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest],{env,encoding:'utf8'});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Candidate harness must build');
const exe=path.join(env.CARGO_TARGET_DIR,'debug/folia-regexp-candidate-probe'+(process.platform==='win32'?'.exe':''));
const run=cp.spawnSync(exe,[],{input:JSON.stringify(input),encoding:'utf8',maxBuffer:256*1024*1024,timeout:120000});fs.writeFileSync(path.join(work,'native-stderr.log'),run.stderr||'');assert.equal(run.status,0,'Candidate must finish normally');
const outputs=JSON.parse(run.stdout);assert.equal(outputs.length,cases.length);fs.writeFileSync(path.join(work,'native-results.json'),run.stdout);
const totals={},mismatches={};for(const mode of ['literal_flags','v_implies_unicode']){let equal=0;const byGroup={},byPattern={},samples=[];for(let i=0;i<cases.length;++i){const {diagnostic,...actual}=outputs[i][mode],c=cases[i];if(isDeepStrictEqual(actual,c.expected)){++equal;continue;}byGroup[c.group]=(byGroup[c.group]??0)+1;const key=JSON.stringify([String.fromCharCode(...c.pattern),c.flags]);byPattern[key]=(byPattern[key]??0)+1;if(samples.length<60)samples.push({...c,actual,diagnostic});}totals[mode]={cases:cases.length,equal,different:cases.length-equal,by_group:byGroup,by_pattern:byPattern};mismatches[mode]=samples;}
for(const[name,bytes]of sources)assert.ok(bytes.equals(fs.readFileSync(path.join(candidate,name))),'Candidate source changed: '+name);assert.ok(baseBytes.equals(fs.readFileSync(sourceBase)),'Reference source changed');
const report={stage:'candidate experiment; no engine or host accepted',revision,node:process.version,unicode:process.versions.unicode,icu:process.versions.icu,source_base_sha256:baseHash,candidate_hashes:Object.fromEntries([...sources].map(([name,bytes])=>[name,hash(bytes)])),source_case_sha256:hash(Buffer.from(JSON.stringify(cases))),totals,samples:mismatches};
fs.writeFileSync(path.join(work,'report.json'),JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify({cases:cases.length,totals:Object.fromEntries(Object.entries(totals).map(([mode,{equal,different,by_group}])=>[mode,{equal,different,by_group}])),report:'build/math-port-regexp-candidate/report.json'}));
