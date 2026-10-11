// Root actual-source oracle, independent of native label helper implementations.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/mermaid-label-data-reference');fs.mkdirSync(work,{recursive:true});
const hash=b=>crypto.createHash('sha256').update(b).digest('hex'),bytes=fs.readFileSync(path.join(root,'.reference/mermaid-registry/mermaid-rs-renderer-0.3.1/src/render.rs'));
assert.equal(hash(bytes),'026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4');
const text=bytes.toString();function functionBody(name){const start=text.indexOf('fn '+name+'(');assert.ok(start>=0);const open=text.indexOf('{',start);let depth=1,i=open+1;while(depth){if(text[i]==='{')depth++;else if(text[i]==='}')depth--;i++;assert.ok(i<=text.length);}return text.slice(start,i);}
const structStart=text.lastIndexOf('#[derive',text.indexOf('struct ErAttribute {')),structEnd=text.indexOf('\nfn parse_er_attributes(',structStart);
const extracted=[functionBody('format_sankey_value'),functionBody('is_divider_line'),text.slice(structStart,structEnd),functionBody('parse_er_attributes')].join('\n\n');
const cases=[],bits=new Set([0,0x80000000,1,0x80000001,0x007fffff,0x807fffff,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00019,0xffc00031]);
const b=Buffer.alloc(4);function neighbors(v){b.writeFloatLE(v);const center=b.readUInt32LE();for(let d=-3;d<=3;d++)bits.add((center+d)>>>0);}
for(const v of [-1234.567,-10.005,-2.999,-1.995,-1.101,-.105,-.015,-.005,-.001,0,.001,.005,.015,.095,.105,.995,1,1.005,1.095,1.1,1.101,1.995,2.999,10.005,1234.567,1e20,3.4028235e36])neighbors(v);
let seed=0x2319ac37;for(let i=0;i<2048;i++){seed=(Math.imul(seed,1664525)+1013904223)>>>0;bits.add(seed);}
for(const value of bits)cases.push({kind:'sankey',bits:value});
const tokens=['','id','int id','INT user name','PK','FK','UK','pk','Fk','uK','PK FK UK','PK PK FK','int id PK FK UK','int id pk,fk,uk','PK,other','other,FK,remaining','PK;FK','PK,;FK',';PK;','pk,,FK,','int id ;PK,FK;','PK,FK,UK,other','int id PK,FK;','x,pk,y,uk,z','int ,;id;,','---',' --- ','----','--- PK','数学 名称 PK','İNT İD pK','ｐｋ ｆｋ','pk\u200bFK','int\u00a0id\u2003PK','int\tname\nFK','\0raw <>&" UK','int e\u0301 PK','name; PK'];
const prefixes=[[],['title'],[' title ',''],['first','second','third'],['---'],[' --- ','body before next divider'],['title',' --- '],['title','---',''],['title','not a divider','---'],['title','---','---'],['title','---','replacement title','---']];
tokens.push('int id PK PK FK','float user name FK PK','int id UK FK PK UK','name PK PK','int id ;PK; ;PK;','int id PK,PK,FK','int id PK,junk,PK');
for(const prefix of prefixes)for(const token of tokens)cases.push({kind:'er',lines:[...prefix,token]});
for(let i=0;i<tokens.length;i++)cases.push({kind:'er',lines:[' t ',' sub ','---',tokens[i],tokens[(i+7)%tokens.length],tokens[(i+13)%tokens.length],'---',tokens[(i+19)%tokens.length]]});
cases.push({kind:'er',lines:[]},{kind:'er',lines:['']},{kind:'er',lines:['\u00a0\u2003TITLE\u3000',' \u2028 ',' OTHER ','---','int\u202fname PK FK PK']});
const source=path.join(work,'source-functions.rs'),input=path.join(work,'inputs.json'),actual=path.join(work,'actual.json');fs.writeFileSync(source,extracted);fs.writeFileSync(input,JSON.stringify(cases));
const args=['run','--locked','--offline','--manifest-path',path.join(__dirname,'label-data-reference/Cargo.toml'),'--target-dir',path.join(work,'target'),'--',input,actual];
const result=cp.spawnSync('cargo',args,{encoding:'utf8',env:{...process.env,FOLIA_LABEL_DATA_SOURCE:source}});fs.writeFileSync(path.join(work,'run.log'),(result.stdout||'')+(result.stderr||''));assert.equal(result.status,0,'Unchanged Source functions must compile/execute');
const output=JSON.stringify({source_sha256:hash(bytes),extracted_sha256:hash(extracted),cases:JSON.parse(fs.readFileSync(actual))})+'\n';
const fixture=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/label-data.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(fixture,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(fixture,output);
console.log(JSON.stringify({source_functions:2,sankey_cases:bits.size,er_cases:cases.length-bits.size,mode:process.argv.includes('--check')?'check':'generate'}));
