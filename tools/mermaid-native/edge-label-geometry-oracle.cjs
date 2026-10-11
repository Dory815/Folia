// Root executes unchanged pinned geometry functions independently of the port.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict'),cp=require('node:child_process');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/mermaid-edge-label-geometry-reference'),sha=x=>crypto.createHash('sha256').update(x).digest('hex');
// Preserve the established reference dependency pins, including packages that
// have more than one simultaneously locked version.
const packageIdentities=file=>fs.readFileSync(file,'utf8').split('[[package]]').slice(1).map(block=>{
    const fields=['name','version','source','checksum'].map(k=>block.match(new RegExp('^'+k+' = "([^"\\n]+)"','m'))?.[1]||'');return {name:fields[0],identity:fields.join('|')};
});
const pinned=new Set(packageIdentities(path.join(__dirname,'text-presentation-reference/Cargo.lock')).map(p=>p.identity));
for(const p of packageIdentities(path.join(__dirname,'edge-label-geometry-reference/Cargo.lock')))if(!p.name.startsWith('folia-'))assert.ok(pinned.has(p.identity),'Reference dependency drift: '+p.identity);
const bytes=fs.readFileSync(path.join(root,'.reference/mermaid-registry/mermaid-rs-renderer-0.3.1/src/render.rs'));
assert.equal(sha(bytes),'026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4');const source=bytes.toString();
const names=['from_center','edge_label_background_visible','polyline_rect_gap','segment_rect_gap','point_rect_distance','point_segment_distance','segment_intersects_rect','point_in_rect','segments_intersect','orient','on_segment'];
const functions=names.map(name=>{const start=source.indexOf('fn '+name+'('),tail=name==='from_center'?'\n    }':'\n}',end=source.indexOf(tail,start)+tail.length;assert.ok(start>0&&end>start);return {name,sha256:sha(source.slice(start,end))};});
const start=source.lastIndexOf('#[derive',source.indexOf('struct LabelRect {')),end=source.indexOf('\nfn format_sankey_value(',start);
const exact=source.slice(start,end);assert.deepEqual([...exact.matchAll(/\bfn (\w+)\(/g)].map(m=>m[1]),names);
fs.mkdirSync(work,{recursive:true});const extracted=path.join(work,'source-functions.rs');fs.writeFileSync(extracted,exact);
const scalar=x=>Object.is(x,-0)?'-0':String(x),strings=x=>Array.isArray(x)?x.map(strings):scalar(x);
const base={a:['-1','5'],b:['11','5'],c:['0','0'],d:['10','10'],rect:['0','0','10','10'],points:[['-1','5'],['11','5']],eps:'0.000001',center:['10','20','12','4','2','3'],diagram:'Flowchart',label:'Center'};
const cases=[],add=(method,change={})=>cases.push({...base,...change,method});
function adjacent(x,offset){const buffer=new ArrayBuffer(4),v=new DataView(buffer);v.setFloat32(0,x);v.setUint32(0,v.getUint32(0)+offset);return v.getFloat32(0);}
const gaps=[0,-0,0.1,...[0.35,0.6,0.9,1,1.2,1.4,2.4].flatMap(x=>[-1,0,1].map(n=>adjacent(x,n))),10];
const diagrams=JSON.parse(fs.readFileSync(path.join(root,'docs/mermaid-native/source-inventory.json'))).diagram_kinds;assert.equal(diagrams.length,23);
for(const diagram of diagrams)for(const label of ['Center','Start','End'])for(const height of [1,10,30])for(const gap of gaps)add('visible',{diagram,label,rect:strings([0,0,10,height]),points:strings([[-10,-gap],[20,-gap]])});
const geometry=[{},...['NaN','Infinity','-Infinity','-0','0','-3','3.4028234663852886e38'].map(x=>({a:[x,x],c:[x,x],center:[x,x,x,x,x,x],rect:[x,x,x,x]})),...[[0,0],[10,10],[5,0],[5,10],[-1,5],[11,5],[5,-1],[5,11]].map(c=>({c:strings(c),a:strings(c)})),{a:['0','0'],b:['0','0']},{a:['0','0'],b:['0','0.00001']},{a:['0','0'],b:['0','0.0001']},{a:['-1','-1'],b:['0','0']},{a:['0','0'],b:['10','0'],c:['5','0'],d:['20','0']},{a:['0','0'],b:['10','0'],c:['11','0'],d:['20','0']},{rect:['0','0','-10','-10']},{rect:['0','0','0','0']},{rect:['-10','-10','20','20']},...[-1,0,1].map(n=>({a:['0','0'],b:['1','0'],c:['0.5',scalar(adjacent(1e-6,n))],d:['0.5','2']})),...[-1,0,1].map(n=>({a:['0','0'],b:[scalar(adjacent(Math.sqrt(1e-9),n)),'0'],c:['1','1']})),...[-1,0,1].map(n=>({points:strings([[-10,-adjacent(1e-6,n)],[20,-adjacent(1e-6,n)]])}))];
let seed=0x615702;const rand=()=>{seed=(Math.imul(seed,1664525)+1013904223)>>>0;return Math.fround((seed/4294967296-0.5)*40);};
for(const n of [-1,0,1])geometry.push({a:['0','0'],b:['1','0'],c:['0.5',scalar(-adjacent(1e-6,n))],d:['0.5','-2']});
for(let i=0;i<96;i++)geometry.push({a:strings([rand(),rand()]),b:strings([rand(),rand()]),c:strings([rand(),rand()]),d:strings([rand(),rand()]),rect:strings([rand(),rand(),rand(),rand()]),points:strings([[rand(),rand()],[rand(),rand()],[rand(),rand()]])});
for(const input of geometry)for(const method of ['center','visible','polyline','segment_rect','point_rect','point_segment','segment_intersects','point_in','segments','orient','on_segment'])add(method,input);
for(const points of [[],[[0,0]],[[0,0],[0,0]],[[20,20],[30,30],[0,5],[10,5]],[[20,20],[30,30],[40,40]],[[NaN,NaN],[1,1]]])for(const method of ['polyline','visible'])add(method,{points:strings(points)});
const inputs=path.join(work,'inputs.json'),actual=path.join(work,'actual.json');fs.writeFileSync(inputs,JSON.stringify(cases));
const run=cp.spawnSync('cargo',['run','--locked','--offline','--manifest-path',path.join(__dirname,'edge-label-geometry-reference/Cargo.toml'),'--target-dir',path.join(work,'target'),'--',inputs,actual],{encoding:'utf8',env:{...process.env,FOLIA_EDGE_LABEL_GEOMETRY_REFERENCE:extracted}});
fs.writeFileSync(path.join(work,'run.log'),(run.stdout||'')+(run.stderr||''));assert.equal(run.status,0,'Pristine source reference must compile and run; inspect ignored log');
const observed=JSON.parse(fs.readFileSync(actual));assert.equal(observed.length,cases.length);
const output=JSON.stringify({source_sha256:sha(bytes),functions}).slice(0,-1)+',"cases":[\n'+observed.map(x=>JSON.stringify(x)).join(',\n')+'\n]}\n',target=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/edge-label-geometry.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
console.log(JSON.stringify({source_functions:10,source_struct_methods:1,cases:cases.length,diagram_kinds:diagrams.length,mode:process.argv.includes('--check')?'check':'generate'}));
