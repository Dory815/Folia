'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),cp=require('node:child_process');
const {captureOutput}=require('./process-output.cjs');
const gate=Number(process.argv[2]);
if(!Number.isSafeInteger(gate)||gate<1)throw Error('A completed build number is required');
const state=JSON.parse(fs.readFileSync('build/math-port/shared-build-state.json'));
if(state.gate!==gate||state.status!=='terminal'||state.exitCode!==0)
 throw Error('The selected shared build must have completed successfully');
const hash=p=>crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex');
const casesPath='tools/math-port/math-api-cases.json';
const cases=JSON.parse(fs.readFileSync(casesPath)).cases;
const base64=s=>Buffer.from(s,'utf8').toString('base64');
const input=cases.map(item=>[base64(item.name),base64(item.tex),item.display?1:0,
 item.em??16,item.width??1280].join(' ')).join('\n')+'\n';
const directory=`build/math-port/gate${gate}-runners`;fs.mkdirSync(directory,{recursive:true});
const source='build/core/FoliaMathApiDump.exe';
const executable=path.resolve(`${directory}/FoliaMathApiDump-observation.exe`);
fs.copyFileSync(source,executable);
if(hash(source)!==hash(executable))throw Error('Executable copy differs');
const destination=`build/math-port/math-api-native-g${gate}.json`;
const record={gate,executable,sha256:hash(executable),corpusSha256:hash(casesPath),
 stdinSha256:crypto.createHash('sha256').update(input).digest('hex'),startedAt:new Date().toISOString()};
const child=cp.spawn(executable,[],{windowsHide:true});record.pid=child.pid;
const stdoutCapture=captureOutput(child.stdout),stderrCapture=captureOutput(child.stderr);
child.on('error',error=>{record.error=error.message;});
child.stdin.on('error',error=>{record.stdinError=error.message;});
child.stdin.end(input);
child.on('close',(exitCode,signal)=>{
 Object.assign(record,{endedAt:new Date().toISOString(),exitCode,signal});
 fs.writeFileSync(destination+'.stdout',stdoutCapture.bytes());fs.writeFileSync(destination+'.stderr',stderrCapture.bytes());
 record.stdoutSha256=hash(destination+'.stdout');record.stderrSha256=hash(destination+'.stderr');
 if(exitCode===0){
  try{const value=JSON.parse(stdoutCapture.text());fs.writeFileSync(destination,JSON.stringify(value,null,2)+'\n');record.outputSha256=hash(destination);}
  catch(error){record.parseError=error.message;}
 }
 fs.writeFileSync(destination+'.provenance.json',JSON.stringify(record,null,2)+'\n');
 console.log(JSON.stringify(record));process.exitCode=exitCode===0&&!record.parseError&&!record.error?0:1;
});
