// Root original public PixmapRef storage/borrow observation, independent of native.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),source=path.join(root,'build/mermaid-pattern-stage-source'),work=path.join(root,'build/mermaid-pattern-pixmap-reference');fs.mkdirSync(work,{recursive:true});
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const pin='484d49d1f4c3c3c8fb3fab3e1b1535292de828c0f258c3cab9bda02c7d9626fc';
assert.equal(hash(fs.readFileSync(path.join(source,'src/pixmap.rs'))),pin,'Run --pattern --check to prepare the guarded original Source copy first');
const manifest=path.join(__dirname,'pattern-stage-reference/Cargo.toml');
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest,'--bin','pixmap','--target-dir',path.join(work,'target')],{encoding:'utf8'});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0);
const inputs=[];for(const width of [0,1,2,3,7,17,0x1fffffff,0x20000000,0x7fffffff,0x80000000,0xffffffff])for(const height of [0,1,2,3,0x7fffffff,0x80000000,0xffffffff])for(const len of [0,1,3,4,7,8,11,12,15,16,23,24,27,28,31,32,67,68,203,204,205,512])inputs.push({width,height,data:Array.from({length:len},(_,i)=>(i*73+width+height)&255)});
const r=cp.spawnSync(path.join(work,'target/debug/pixmap.exe'),[],{input:JSON.stringify(inputs),encoding:'utf8',maxBuffer:8*1024*1024});assert.equal(r.status,0);const cases=JSON.parse(r.stdout);assert.equal(cases.length,1694);
const data=JSON.stringify({schema:1,scope:'actual original PixmapRef construction, dimensions, full data and borrow identity; bounded allocation with extreme dimension rejection',pins:{'src/pixmap.rs':pin},cases})+'\n',out=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/pattern-pixmap.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}
console.log(JSON.stringify({cases:cases.length,accepted:cases.filter(c=>c.result).length,sha256:hash(data)}));
