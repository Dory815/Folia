'use strict';
// Compare the public native Renderer against the pinned six-case Source capture.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const {spawnSync}=require('node:child_process');
const assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../../../..');
const gate=Number(process.argv[2]),output=process.argv[3]&&path.resolve(process.argv[3]);
assert.ok(Number.isSafeInteger(gate)&&output,'Gate and fresh output path required');
for(const suffix of ['', '.stdin', '.stdout', '.stderr', '.provenance.json', '.comparison.json'])
  assert.ok(!fs.existsSync(output+suffix),'Refusing to overwrite '+output+suffix);
const hash=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');
const sourcePath=path.join(__dirname,'source-oracle.json');
assert.equal(hash(fs.readFileSync(sourcePath)),
  '7e33b437f03e4a9240d86fbd25d4957d6bc7d9587208aa7ba544f48302de568e');
const source=JSON.parse(fs.readFileSync(sourcePath));
assert.equal(source.source.upstream.commit,'fb987178c2d279a99b0db5ea02e751691703955e');
assert.equal(source.records.length,6);
const groups=['continuous','fresh-session-per-case'];
const b64=value=>Buffer.from(value,'utf8').toString('base64');
const input=groups.map(group=>'@group\t'+b64(group)+'\n'+source.records.map((row,index)=>
  [index,b64(row.name),b64(row.tex),'0','16',row.width].join('\t')).join('\n')+'\n').join('');
fs.writeFileSync(output+'.stdin',input);
const capture=spawnSync(process.execPath,[path.join(root,'tools/math-port/capture-native-observation.cjs'),
  String(gate),'FoliaMathProfile75FullSvgDump',output],{cwd:root,windowsHide:true,
  env:{...process.env,MATH_OBSERVATION_STDIN:output+'.stdin',MATH_OBSERVATION_TIMEOUT_MS:'120000'},
  encoding:'utf8',timeout:135000,maxBuffer:4*1024*1024});
if(capture.status!==0||capture.error){
  process.stderr.write(capture.stderr||capture.error?.message||'Native capture failed');
  process.exitCode=1;
}else{
  const native=JSON.parse(fs.readFileSync(output));
  assert.deepEqual(native.groups,groups);
  assert.equal(native.records.length,12);
  const records=native.records.map((row,index)=>{
    const expected=source.records[index%6];
    const identity=row.group===groups[Math.floor(index/6)]&&row.index===index%6&&
      row.name===expected.name&&row.tex===expected.tex&&row.display===false&&
      row.em===16&&row.ex===8&&row.width===expected.width;
    return {group:row.group,name:row.name,identity,error:row.error,
      expectedSha256:expected.sha256,actualSha256:row.svg===null?null:hash(row.svg),
      equal:identity&&row.error===null&&row.svg===expected.svg};
  });
  const accepted=records.every(row=>row.equal);
  const report={gate,sourceSha256:hash(fs.readFileSync(sourcePath)),accepted,records};
  fs.writeFileSync(output+'.comparison.json',JSON.stringify(report,null,2)+'\n');
  console.log(JSON.stringify({gate,accepted,matches:records.filter(row=>row.equal).length,total:12,
    report:output+'.comparison.json'}));
  process.exitCode=accepted?0:1;
}
