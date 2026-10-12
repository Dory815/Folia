// Root semantic faults reuse build/core and leave frozen gold unchanged.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file='src/math/util/unicode_normalization.cpp',saved=fs.readFileSync(path.join(root,file),'utf8');
const gold=['src/math/util/unicode_normalization_data.ixx','tests/math/reference_unicode_normalization.cpp','docs/math-port/evidence/p54-unicode-normalization-source.json'].map(file=>({file,bytes:fs.readFileSync(path.join(root,file))}));
const faults=[
 ['recursive-decomposition-stops-after-one-level',[['self(self,unicode16::values[it->offset+i]);','decomposed.push_back(unicode16::values[it->offset+i]);']]],
 ['hangul-decomposition-disabled',[['if(point>=SBase&&point<SBase+SCount){','if(false && point>=SBase&&point<SBase+SCount){']]],
 ['empty-hangul-tail-emits-jamo',[['if(trailing)self(self,TBase+trailing);','self(self,TBase+trailing);']]],
 ['combining-classes-descending',[['return combiningClass(left)<combiningClass(right);','return combiningClass(left)>combiningClass(right);']]],
 ['equal-class-order-replaced-by-codepoint-order',[['return combiningClass(left)<combiningClass(right);','auto lc=combiningClass(left),rc=combiningClass(right);return lc<rc||(lc==rc&&left<right);']]],
 ['ordering-crosses-starter',[['while(cursor<decomposed.size()&&combiningClass(decomposed[cursor])!=0)++cursor;','while(cursor<decomposed.size())++cursor;']]],
 ['valid-surrogate-pairs-decoded-separately',[['if(second>=0xDC00&&second<=0xDFFF){','if(false && second>=0xDC00&&second<=0xDFFF){']]],
 ['isolated-surrogate-replaced',[['char32_t point=first;','char32_t point=(first>=0xD800&&first<=0xDFFF)?0xFFFD:first;']]],
 ['embedded-nul-truncates-input',[['for(std::size_t i=0;i<input.size();++i){','for(std::size_t i=0;i<input.size()&&input[i]!=0;++i){']]]
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_nfd*'],{cwd:root,encoding:'utf8',timeout:180000,maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p54-nfd-'+label+'.log'),log);return {status:r.status,log};}
const results=[];
try{
 const baseline=run('mutation-baseline');assert.equal(baseline.status,0);assert.match(baseline.log,/99927 asserts in 103 tests/);
 for(const[id,edits]of faults){let changed=saved;for(const[from,to]of edits){assert.equal(changed.split(from).length,2,id+' exact anchor');changed=changed.replace(from,to);}fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id);const compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status===1&&/FAILED/.test(r.log);results.push({id,compiling,detected,status:r.status});console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail original assertions');}
}finally{fs.writeFileSync(path.join(root,file),saved);}
assert.equal(run('mutation-restored').status,0);
for(const {file,bytes}of gold)assert.ok(bytes.equals(fs.readFileSync(path.join(root,file))),file+' unchanged');
fs.writeFileSync(path.join(root,'docs/math-port/evidence/p54-nfd-mutations.json'),JSON.stringify({source:{file,sha256:crypto.createHash('sha256').update(saved).digest('hex')},baseline:{assertions:99927,tests:103},faults:results,gold_unchanged:true},null,2)+'\n');
