// Root compiling semantic faults against frozen actual-original fixtures.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),node='src/math/input/tex/node_util.cpp',error='src/math/input/tex/tex_error.cpp';
const files=[node,error],saved=new Map(files.map(f=>[f,fs.readFileSync(path.join(root,f),'utf8')]));
const faults=[
 ['class-assignment',node,'node.requireNodeForWrite("texClass")->setTexClass(value);','(void)value;'],
 ['movable-kind',node,'node.requireNode("isKind")->isKind("mo") ||','node.requireNode("isKind")->isKind("mi") ||'],
 ['entries-snapshot',node,'newNode.requireNode("setProperty")->setProperty(name, value);','newNode.requireNode("setProperty")->setProperty(name, host::get(properties,name));'],
 ['captured-form-table',node,'const Property formTable = propertyAt(operatorTable, host::debugText(host::string(form)));\n    const std::u16string text = nodeText(node, "mo");','const std::u16string text = nodeText(node, "mo");\n    const Property formTable = propertyAt(mml::MmlMo::operatorTable(), host::debugText(host::string(form)));'],
 ['false-null-attribute',node,'if (!value.missing()) return value;','if (host::truthy(value)) return value;'],
 ['all-forms',node,'index < forms.size()','index < std::min(std::size_t(1),forms.size())'],
 ['whole-braced-comparison',error,'const auto remainder=part.substr(1);','const auto remainder=part.substr(1,1);'],
 ['nullish-join',error,'if(value.missing()||std::holds_alternative<PropertyNull>(value.value))return {};','if(value.missing())return {};']
];
function run(label,filter){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat',filter],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p40-'+label+'.log'),log);return {status:r.status,log};}
const results=[];try{
 for(const file of files){const baseline=run('baseline-'+path.basename(file),file===node?'*math_port_node_util*':'*math_port_tex_error*');assert.equal(baseline.status,0,'baseline must pass');}
 for(const [id,file,from,to]of faults){const original=saved.get(file);assert.equal(original.split(from).length,2,id+' must match once');fs.writeFileSync(path.join(root,file),original.replace(from,to));const r=run('fault-'+id,file===node?'*math_port_node_util*':'*math_port_tex_error*');const compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);results.push({id,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),original);console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and be detected');}
}finally{for(const [file,original]of saved)fs.writeFileSync(path.join(root,file),original);}
for(const file of files){const final=run('restored-'+path.basename(file),file===node?'*math_port_node_util*':'*math_port_tex_error*');assert.equal(final.status,0,'restored baseline must pass');}
const report={source_sha256:Object.fromEntries([...saved].map(([f,v])=>[f,crypto.createHash('sha256').update(v).digest('hex')])),faults:results};fs.writeFileSync(path.join(root,'docs/math-port/evidence/p40-parser-utilities-mutations.json'),JSON.stringify(report,null,2)+'\n');
