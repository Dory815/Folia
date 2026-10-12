// Root semantic faults reuse build/core; no duplicated build tree or gold edits.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..');
const map='src/math/input/tex/map_handler.cpp',node='src/math/input/tex/node_factory.cpp',tags='src/math/input/tex/tags.cpp';
const saved=new Map([map,node,tags].map(file=>[file,fs.readFileSync(path.join(root,file),'utf8')]));
const gold=['tests/math/reference_parser_records.cpp','docs/math-port/evidence/p55-parser-record-source.json'].map(file=>({file,bytes:fs.readFileSync(path.join(root,file))}));
const faults=[
 {id:'handler-argument-gets-swapped',file:map,edits:[['const options::Value maps = context.get(handlers, key);\n        const options::Value fallback = context.get(fallbacks, key);','const options::Value fallback = context.get(fallbacks, key);\n        const options::Value maps = context.get(handlers, key);',2]]},
 {id:'slice-holes-filled-with-undefined',file:map,edits:[['if (context.has(receiver, slot)) {','if (true || context.has(receiver, slot)) {']]},
 {id:'reverse-upper-has-before-lower-get',file:map,edits:[['const bool lowerExists = context.has(receiver, lowerKey);\n        const std::optional<options::Value> lowerValue = lowerExists\n            ? std::optional<options::Value>(context.get(receiver, lowerKey))\n            : std::nullopt;\n        const bool upperExists = context.has(receiver, upperKey);','const bool lowerExists = context.has(receiver, lowerKey);\n        const bool upperExists = context.has(receiver, upperKey);\n        const std::optional<options::Value> lowerValue = lowerExists\n            ? std::optional<options::Value>(context.get(receiver, lowerKey))\n            : std::nullopt;']]},
 {id:'remove-captures-array-length',file:map,edits:[['const options::ObjectRef captured = *array;','const options::ObjectRef captured = *array;\n    const auto fixedLength = context.arrayLength(captured);'],['if (index >= context.arrayLength(captured)) return std::nullopt;','if (index >= fixedLength) return std::nullopt;']]},
 {id:'missing-map-still-registers-fallback',file:map,edits:[['warn(std::move(message));\n            return;','warn(std::move(message));\n            if (truthy(fallback)) fallback_.add(fallback, priority);\n            return;']]},
 {id:'creator-prototypes-not-enumerated',file:node,edits:[['current = options::Value(*prototype);','break;']]},
 {id:'creator-deleted-slot-still-installed',file:node,edits:[['slot == slots.end() || !context.has(maps, slot->key)','slot == slots.end()']]},
 {id:'creator-own-deletion-hides-inherited-value',file:node,edits:[['!context.has(maps, slot->key)','!context.hasOwn(maps, slot->key)']]},
 {id:'creator-nonenumerable-shadow-forgotten',file:node,edits:[['if (!name || !seen.insert(*name).second) continue;','if (!name) continue;'],['if (property && property->enumerable) {','if (property && property->enumerable && seen.insert(*name).second) {']]},
 {id:'tag-info-no-tag-parameter-discarded',file:tags,edits:[['noTag(noTagValue)','noTag(false)']]},
 {id:'tag-info-format-identity-discarded',file:tags,edits:[['tagFormat(std::move(tagFormatValue))','tagFormat(u"" )']]}
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_parser_records*'],{cwd:root,encoding:'utf8',timeout:180000,maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p55-parser-record-'+label+'.log'),log);return {status:r.status,log};}
const results=[];
try{
 const baseline=run('mutation-baseline');assert.equal(baseline.status,0);assert.match(baseline.log,/27 asserts in 27 tests/);
 for(const fault of faults){let changed=saved.get(fault.file);for(const[from,to,count=1]of fault.edits){assert.equal(changed.split(from).length,count+1,fault.id+' exact anchor');changed=changed.replace(from,to);}fs.writeFileSync(path.join(root,fault.file),changed);const r=run('fault-'+fault.id);const compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status===1&&/FAILED/.test(r.log);results.push({id:fault.id,file:fault.file,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,fault.file),saved.get(fault.file));console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,fault.id+' must compile and fail original assertions');}
}finally{for(const[file,source]of saved)fs.writeFileSync(path.join(root,file),source);}
assert.equal(run('mutation-restored').status,0);
for(const {file,bytes}of gold)assert.ok(bytes.equals(fs.readFileSync(path.join(root,file))),file+' unchanged');
fs.writeFileSync(path.join(root,'docs/math-port/evidence/p55-parser-record-mutations.json'),JSON.stringify({sources:[...saved].map(([file,source])=>({file,sha256:crypto.createHash('sha256').update(source).digest('hex')})),baseline:{assertions:27,tests:27},faults:results,gold_unchanged:true},null,2)+'\n');
