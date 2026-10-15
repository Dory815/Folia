'use strict';
// Complete SVG comparison; the native probe separately asserts typed Scene
// geometry and owner identities. This does not claim adaptor-trace parity.
const fs=require('node:fs'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const nativePath=process.argv[2];
if(!nativePath)throw Error('Usage: compare-texttip-svg.cjs <native-log> [report-json]');
const reportPath=process.argv[3]||nativePath+'.comparison.json';
const sourcePath='build/math-port/isolated/scene_layering/texttip-original.json';
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
assert.equal(hash(sourcePath),'47265558f42212b68733c7bf6f4ba0912ae9cf903fe29369650fef1b0093f083');
assert.equal(hash('tools/math-port/drafts/scene-layering-texttip-original.cjs'),
 'c3430d67e3b71699ac8108b5f996a48abfa155556afee048e79d8033e726b253');
const source=JSON.parse(fs.readFileSync(sourcePath));
assert.equal(source.tex,'\\texttip{x}{tip}');
assert.equal(source.provenance.upstream.commit,'fb987178c2d279a99b0db5ea02e751691703955e');
const provenancePath=nativePath+'.provenance.json';
const provenance=JSON.parse(fs.readFileSync(provenancePath));
assert.equal(provenance.format,'text');
assert.equal(hash(provenance.executable),provenance.sha256);
assert.equal(provenance.gate,provenance.build.gate);
for(const pin of Object.values(provenance.build).filter(value=>value&&typeof value==='object'))
 assert.equal(hash(pin.path),pin.sha256);
assert.deepEqual(JSON.parse(fs.readFileSync(provenance.build.audit.path)),[]);
assert.equal(hash(nativePath+'.stdout'),provenance.stdoutSha256);
assert.equal(hash(nativePath+'.stderr'),provenance.stderrSha256);
const stderr=new TextDecoder('utf-8',{fatal:true}).decode(fs.readFileSync(nativePath+'.stderr'));
const matches=[...stderr.matchAll(/^texttip svg: (.*)\r?$/gm)];
const svg=matches.length===1?matches[0][1].replace(/\r$/,''):null;
let firstDifference=null;
if(provenance.exitCode!==0||provenance.signal||provenance.timedOut)
 firstDifference={path:'process',exitCode:provenance.exitCode,signal:provenance.signal,timedOut:!!provenance.timedOut};
else if(svg===null)firstDifference={path:'svg.records',source:1,native:matches.length};
else if(svg!==source.svg){
 let offset=0;while(offset<source.svg.length&&offset<svg.length&&source.svg[offset]===svg[offset])offset++;
 firstDifference={path:'svg',offset,sourceLength:source.svg.length,nativeLength:svg.length,
  source:source.svg.slice(Math.max(0,offset-40),offset+100),native:svg.slice(Math.max(0,offset-40),offset+100)};
}
const report={accepted:firstDifference===null,gate:provenance.gate,source:sourcePath,
 sourceSha256:hash(sourcePath),native:nativePath,provenanceSha256:hash(provenancePath),
 compared:'complete SVG string',adaptorTraceCompared:false,firstDifference};
fs.writeFileSync(reportPath,JSON.stringify(report,null,2)+'\n');
console.log(JSON.stringify(report));process.exitCode=report.accepted?0:1;
