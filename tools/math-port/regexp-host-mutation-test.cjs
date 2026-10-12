// Root host faults against actual RegExp / FindTeX-generated observations.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file='src/math/util/regexp_host.cpp',saved=fs.readFileSync(path.join(root,file),'utf8');
const faults=[
 ['surrogate-codepoints-replace-codeunits',[['srell::re_detail::utf_traits_core<char16_t>','srell::re_detail::utf16_traits<char16_t>']]],
 ['identity-escape-adaptation-skipped',[['!alphanumeric&&syntax.find(escaped)==syntax.npos&&escaped<0x80','false']]],
 ['cursor-ignored',[['stateful?cursorLength(lastIndex):0','0']]],
 ['failure-keeps-global-cursor',[['if(stateful)lastIndex=0;','if(false)lastIndex=0;',2]]],
 ['all-captures-treated-as-present',[['if(capture.matched)','if(true)']]],
 ['zero-width-matches-incorrectly-advance',[['result.index+matches.length()','result.index+std::max<std::size_t>(1,matches.length())']]],
 ['nonzero-offsets-reported-relative',[['static_cast<std::size_t>(matches.position())','static_cast<std::size_t>(matches.position())-static_cast<std::size_t>(cursor)']]]
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_scanner_regexp*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p50-regexp-host-'+label+'.log'),log);return{status:r.status,log};}
const results=[];try{const baseline=run('mutation-baseline');assert.equal(baseline.status,0);assert.match(baseline.log,/34 asserts in 34 tests/);assert.match(baseline.log,/4 asserts in 1 tests/);for(const[id,edits]of faults){let changed=saved;for(const[from,to,count=1]of edits){assert.equal(changed.split(from).length,count+1,id+' exact anchor');changed=changed.replaceAll(from,to);}fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);results.push({id,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),saved);console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail original assertions');}}finally{fs.writeFileSync(path.join(root,file),saved);}assert.equal(run('mutation-restored').status,0);fs.writeFileSync(path.join(root,'docs/math-port/evidence/p50-regexp-host-mutations.json'),JSON.stringify({sources:[{file,sha256:crypto.createHash('sha256').update(saved).digest('hex')}],faults:results},null,2)+'\n');
