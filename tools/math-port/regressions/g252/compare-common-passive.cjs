'use strict';
// Strict G252 Common Source promotion comparison. No field normalization or tolerances.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const root=path.resolve(__dirname,'../../../..');
const args=process.argv.slice(2);
if(args.length!==3)throw Error('Usage: compare-common-passive.cjs <source-json> <native-json> <summary-json>');
const resolve=p=>path.resolve(root,p);
const read=p=>JSON.parse(fs.readFileSync(p,'utf8').replace(/^\uFEFF/,''));
const hash=p=>crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex');
const lockPath=path.join(__dirname,'common-reference-lock.json');
const lock=read(lockPath),sourcePath=resolve(args[0]),nativePath=resolve(args[1]),summaryPath=resolve(args[2]);
const source=read(sourcePath),native=read(nativePath);
function assertHash(file,want,label){const actual=hash(file);if(actual!==want)throw Error(`${label} SHA-256 mismatch: ${actual}`);return actual;}
function firstDiff(a,b,p='$') {
  if(Object.is(a,b))return null;
  if(typeof a!==typeof b||a===null||b===null)return {path:p,source:a,native:b};
  if(Array.isArray(a)||Array.isArray(b)){
    if(!Array.isArray(a)||!Array.isArray(b)||a.length!==b.length)return {path:p+'.length',source:Array.isArray(a)?a.length:typeof a,native:Array.isArray(b)?b.length:typeof b};
    for(let i=0;i<a.length;i++){const d=firstDiff(a[i],b[i],`${p}[${i}]`);if(d)return d;}return null;
  }
  if(typeof a==='object'){
    const ak=Object.keys(a).sort(),bk=Object.keys(b).sort();
    if(JSON.stringify(ak)!==JSON.stringify(bk))return {path:p+'.$keys',source:ak,native:bk};
    for(const k of ak){const d=firstDiff(a[k],b[k],p+'.'+k);if(d)return d;}return null;
  }
  return {path:p,source:a,native:b};
}
if(lock.schema!==1||lock.records!==75||lock.sourceCommit!=='fb987178c2d279a99b0db5ea02e751691703955e')throw Error('Unsupported or stale Common promotion lock');
assertHash(path.join(__dirname,lock.producer.path),lock.producer.sha256,'producer');
assertHash(path.join(__dirname,lock.host.path),lock.host.sha256,'host');
assertHash(sourcePath,lock.sourceCaptureSha256,'Source capture');
assertHash(resolve(lock.baseObserverPath),lock.baseObserverSha256,'base observer');
assertHash(resolve(lock.host.upstreamCapture),lock.host.upstreamCaptureSha256,'G213 Source capture');
assertHash(resolve(lock.host.listenerCapture),lock.host.listenerCaptureSha256,'G213 listener capture');
assertHash(resolve(lock.sourceRunnerPath),lock.sourceRunnerSha256,'G213 Source runner');
assertHash(resolve(lock.profile.scopePath),lock.profile.scopeSha256,'scope');
assertHash(resolve(lock.profile.file),lock.profile.sha256,'profile');
assertHash(resolve(lock.corpus.path),lock.corpus.sha256,'corpus');
assertHash(resolve(lock.corpus.profileLockPath),lock.corpus.profileLockSha256,'profile source lock');
assertHash(resolve(lock.corpus.inputCapturePath),lock.corpus.inputCaptureSha256,'locked profile input');
assertHash(resolve(lock.dependencyLockPath),lock.dependencyLockSha256,'dependency package lock');
assertHash(resolve(lock.corpus.inputCapturePath),lock.oldSourceCaptureSha256,'old Common Source capture');
if(source.schema!==1||source.version!=='4.1.3'||source.upstream?.version!=='4.1.3'||source.upstream?.commit!==lock.sourceCommit||source.scopeHash!==lock.profile.scopeSha256||source.corpusSha256!==lock.corpus.sha256||source.records?.length!==75)throw Error('Fresh Source capture metadata/profile is not the pinned 75-row profile');
if(source.witness?.observerSha256!==lock.producer.sha256||source.witness?.overlaySha256!==lock.host.sha256||source.witness?.oldCaptureSha256!==lock.oldSourceCaptureSha256||source.witness?.profileCaptureSha256!==lock.host.upstreamCaptureSha256||source.witness?.listenerCaptureSha256!==lock.host.listenerCaptureSha256)throw Error('Fresh Source witness provenance does not match promotion lock');
if(source.configuredPackages?.length!==lock.profile.configuredPackages)throw Error('Source capture package inventory count changed');
if(native.schema!==source.schema||native.version!==source.version||native.scopeHash!==source.scopeHash||native.corpusSha256!==source.corpusSha256)throw Error('Native global Source/profile metadata differs');
const packageDiff=firstDiff(source.configuredPackages,native.configuredPackages,'configuredPackages');
if(packageDiff)throw Error(`Native configured packages mismatch: ${JSON.stringify(packageDiff)}`);
if(!Array.isArray(native.registeredPackages)||!Array.isArray(native.configurationWarnings))throw Error('Native observer omitted actual package registration state');
const registeredDiff=firstDiff(source.configuredPackages,native.registeredPackages,'registeredPackages');
if(registeredDiff)throw Error(`Native actual Context package registrations mismatch: ${JSON.stringify(registeredDiff)}`);
if(native.configurationWarnings.length!==0)throw Error('Native Context reports configuration warnings');
if(!Array.isArray(native.records)||native.records.length!==75)throw Error('Native observation must have 75 records');
const old=read(resolve(lock.corpus.inputCapturePath));
const actions=new Set(lock.actionRows),unchanged=[],matches=[],failures=[];
for(let i=0;i<75;i++){
  const fresh=source.records[i],prior=old.records[i],actual=native.records[i];
  if(fresh.name!==prior.name||fresh.name!==actual.name)failures.push({kind:'record-order',index:i,source:fresh.name,old:prior.name,native:actual.name});
  if(!actions.has(fresh.name)){
    const d=firstDiff(prior,fresh,`oldSource.records[${i}](${fresh.name})`);
    if(d)failures.push({kind:'old-source-preservation',...d});else unchanged.push(fresh.name);
  }
  const d=firstDiff(fresh,actual,`records[${i}](${fresh.name})`);
  if(d)failures.push({kind:'strict-source-native',...d});else matches.push(fresh.name);
}
const eventExpected={'action-toggle':['click'],'action-tooltip':['mouseover','mouseout']};
for(const [name,types] of Object.entries(eventExpected)){
  const row=source.records.find(r=>r.name===name),event=source.passiveHost?.records?.find(r=>r.name===name);
  if(!row||row.error!==null||typeof row.svg!=='string'||!row.wrappers?.length)failures.push({kind:'action-not-complete',name});
  if(!event||firstDiff(types,event.listeners?.map(x=>x.type),`passiveHost.${name}.listeners`))failures.push({kind:'action-listeners',name,expected:types,actual:event?.listeners?.map(x=>x.type)});
}
const summary={schema:1,sourceCommit:lock.sourceCommit,sourcePath:args[0],sourceSha256:hash(sourcePath),nativePath:args[1],nativeSha256:hash(nativePath),lockSha256:hash(lockPath),profileSha256:lock.profile.sha256,corpusSha256:lock.corpus.sha256,oldSourceSha256:lock.oldSourceCaptureSha256,total:75,unchangedOldSourceRows:unchanged.length,unchangedOldSourceNames:unchanged,nativeStrictMatches:matches.length,actualRegisteredPackages:native.registeredPackages.length,configurationWarnings:native.configurationWarnings.length,actionListenerRows:Object.keys(eventExpected),accepted:failures.length===0&&unchanged.length===lock.unchangedOldRowsExpected&&matches.length===75,failures};
fs.mkdirSync(path.dirname(summaryPath),{recursive:true});fs.writeFileSync(summaryPath,JSON.stringify(summary,null,2)+'\n');
console.log(JSON.stringify({accepted:summary.accepted,unchangedOldSourceRows:summary.unchangedOldSourceRows,nativeStrictMatches:summary.nativeStrictMatches,failures:failures.length,summary:args[2]}));
if(!summary.accepted)process.exitCode=1;
