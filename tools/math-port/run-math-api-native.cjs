'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),cp=require('node:child_process');
const {captureOutput}=require('./process-output.cjs');
const gate=Number(process.argv[2]);
if(!Number.isSafeInteger(gate)||gate<1)throw Error('A completed build number is required');
const state=JSON.parse(fs.readFileSync('build/math-port/shared-build-state.json'));
if(state.gate!==gate||state.status!=='terminal'||state.exitCode!==0||state.changedInputs!==0)
 throw Error('A successful unchanged shared build is required');
const hash=p=>crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex');
const inputPath=`build/math-port/gate-${gate}-inputs-before.json`;
const auditPath=`build/math-port/gate-${gate}-input-changes.json`;
const inputs=JSON.parse(fs.readFileSync(inputPath));
const auditInputs=()=>{for(const item of inputs)if(hash(item.path)!==item.sha256)throw Error(`Build input changed: ${item.path}`);};
auditInputs();
const pin=file=>({path:file,sha256:hash(file)});
const casesPath='tools/math-port/math-api-cases.json';
const cases=JSON.parse(fs.readFileSync(casesPath)).cases;
const base64=s=>Buffer.from(s,'utf8').toString('base64');
const input=cases.map(item=>[base64(item.name),base64(item.tex),item.display?1:0,
 item.em??16,item.width??1280].join(' ')).join('\n')+'\n';
const executable=path.resolve('build/core/FoliaMathApiDump.exe');
const executableSha256Before=hash(executable);
const destination=`build/math-port/math-api-native-g${gate}.json`;
const record={gate,executable,sha256:executableSha256Before,corpusSha256:hash(casesPath),
 stdinSha256:crypto.createHash('sha256').update(input).digest('hex'),
 build:{gate,inputs:pin(inputPath),audit:pin(auditPath),log:pin(state.log)},startedAt:new Date().toISOString()};
const child=cp.spawn(executable,[],{windowsHide:true});record.pid=child.pid;
const stdoutCapture=captureOutput(child.stdout),stderrCapture=captureOutput(child.stderr);
child.on('error',error=>{record.error=error.message;});
child.stdin.on('error',error=>{record.stdinError=error.message;});
child.stdin.end(input);
child.on('close',(exitCode,signal)=>{
 Object.assign(record,{endedAt:new Date().toISOString(),exitCode,signal});
 try{record.sha256After=hash(executable);}catch(error){record.executableHashError=error.message;}
 record.executableUnchanged=record.sha256After===record.sha256;
 try{auditInputs();record.build.inputsUnchangedAfter=true;}catch(error){record.build.inputAuditError=error.message;}
 fs.writeFileSync(destination+'.stdout',stdoutCapture.bytes());fs.writeFileSync(destination+'.stderr',stderrCapture.bytes());
 record.stdoutSha256=hash(destination+'.stdout');record.stderrSha256=hash(destination+'.stderr');
 if(exitCode===0){
  try{const value=JSON.parse(stdoutCapture.text());fs.writeFileSync(destination,JSON.stringify(value,null,2)+'\n');record.outputSha256=hash(destination);}
  catch(error){record.parseError=error.message;}
 }
 fs.writeFileSync(destination+'.provenance.json',JSON.stringify(record,null,2)+'\n');
 console.log(JSON.stringify(record));process.exitCode=exitCode===0&&!record.parseError&&!record.error&&record.executableUnchanged&&!record.build.inputAuditError?0:1;
});
