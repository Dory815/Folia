'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),cp=require('node:child_process');
const {captureOutput}=require('./process-output.cjs');
const [gateArg,target,output,...args]=process.argv.slice(2),gate=Number(gateArg);
if(!Number.isSafeInteger(gate)||!/^Folia[A-Za-z][A-Za-z0-9]*$/.test(target)||!output)throw Error('Gate, target and output required');
const hash=p=>crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex');
const state=JSON.parse(fs.readFileSync('build/math-port/shared-build-state.json'));
if(state.gate!==gate||state.status!=='terminal'||state.exitCode!==0||state.changedInputs!==0)
 throw Error('A successful unchanged shared build is required');
const inputPath=`build/math-port/gate-${gate}-inputs-before.json`;
const auditPath=`build/math-port/gate-${gate}-input-changes.json`;
const inputs=JSON.parse(fs.readFileSync(inputPath));
const auditInputs=()=>{for(const input of inputs)if(hash(input.path)!==input.sha256)throw Error(`Build input changed: ${input.path}`);};
auditInputs();
const pin=path=>({path,sha256:hash(path)});
const executable=path.resolve(`build/core/${target}.exe`);
const executableSha256Before=hash(executable);
const record={gate,executable,args,sha256:executableSha256Before,build:{gate,
 inputs:pin(inputPath),audit:pin(auditPath),log:pin(state.log)},startedAt:new Date().toISOString()};
record.format=process.env.MATH_OBSERVATION_FORMAT||'json';
if(!['json','text'].includes(record.format))throw Error('Observation format must be json or text');
const timeoutMs=Number(process.env.MATH_OBSERVATION_TIMEOUT_MS||60000);
if(!Number.isSafeInteger(timeoutMs)||timeoutMs<1)throw Error('Positive observation timeout required');
record.runner=pin(__filename);
record.outputCapture=pin(path.join(__dirname,'process-output.cjs'));
const child=cp.spawn(executable,args,{windowsHide:true});record.pid=child.pid;
record.timeoutMs=timeoutMs;
const timer=setTimeout(()=>{record.timedOut=true;child.kill();},timeoutMs);
const inputPathOverride=process.env.MATH_OBSERVATION_STDIN;
if(inputPathOverride){record.stdin={path:inputPathOverride,sha256:hash(inputPathOverride)};
 child.stdin.end(fs.readFileSync(inputPathOverride));
}else child.stdin.end();
const stdoutCapture=captureOutput(child.stdout),stderrCapture=captureOutput(child.stderr);
child.on('error',error=>{record.error=error.message;});
child.on('close',(exitCode,signal)=>{
 clearTimeout(timer);
 Object.assign(record,{endedAt:new Date().toISOString(),exitCode,signal});
 try{record.sha256After=hash(executable);}catch(error){record.executableHashError=error.message;}
 record.executableUnchanged=record.sha256After===record.sha256;
 try{auditInputs();record.build.inputsUnchangedAfter=true;}catch(error){record.build.inputAuditError=error.message;}
 const stdoutBytes=stdoutCapture.bytes(),stderrBytes=stderrCapture.bytes();
 record.stdoutBytes=stdoutBytes.length;record.stderrBytes=stderrBytes.length;
 fs.writeFileSync(output+'.stdout',stdoutBytes);fs.writeFileSync(output+'.stderr',stderrBytes);
 record.stdoutSha256=hash(output+'.stdout');record.stderrSha256=hash(output+'.stderr');
 if(exitCode===0){
  try{if(record.format==='json'){const value=JSON.parse(stdoutCapture.text());fs.writeFileSync(output,JSON.stringify(value,null,2)+'\n');}
   else fs.writeFileSync(output,stdoutBytes);
   record.outputSha256=hash(output);}
  catch(error){record.parseError=error.message;}
 }
 fs.writeFileSync(output+'.provenance.json',JSON.stringify(record,null,2)+'\n');
 console.log(JSON.stringify(record));process.exitCode=exitCode===0&&!record.parseError&&!record.error&&record.executableUnchanged&&!record.build.inputAuditError?0:1;
});
