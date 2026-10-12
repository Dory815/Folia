// Root independent compiling faults against actual-original NodeFactory gold.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file='src/math/input/tex/node_factory.cpp',target=path.join(root,file),saved=fs.readFileSync(target,'utf8');
const faults=[
 ['truthy-fallback','if (!mml::host::truthy(creator))','if (creator.missing())'],
 ['registration-dispatch','if (kind == "node")','if (!kind.empty())'],
 ['fresh-configuration','const mml::NodeValue node = node_factory_host::invoke(creator, identity(), rest);','const auto before=configuration;\n    const mml::NodeValue node = node_factory_host::invoke(creator, identity(), rest);\n    configuration=before;'],
 ['setter-return','return node_factory_host::setText(mml::NodeValue(rawNode), node_factory_host::text(text));','node_factory_host::setText(mml::NodeValue(rawNode), node_factory_host::text(text));\n    return mml::NodeValue(rawNode);'],
 ['token-text-default','if (text.missing()) text = std::string{};','(void)text;'],
 ['text-append','if (text.isNode()) node.requireNode("appendChild")->appendChild(text);','(void)text;'],
 ['children-before-properties','node.requireNode("setChildren")->setChildren(std::move(children));','NodeUtil::instance().setProperties(node, definition);\n    node.requireNode("setChildren")->setChildren(std::move(children));'],
 ['raw-error-message','const Property errorDefinition = PropertyList{{"data-mjx-error", message}};','const Property errorDefinition = PropertyList{};'],
 ['zero-constructor-args','mml::MmlNode* const rawNode = factory.requireMmlFactory().create(node_factory_host::kind(kind), {});','const std::array<std::any,1> unwanted{std::any(PropertyList{})};\n    mml::MmlNode* const rawNode = factory.requireMmlFactory().create(node_factory_host::kind(kind), unwanted);']
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_node_factory*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p41-'+label+'.log'),log);return {status:r.status,log};}
const results=[];try{assert.equal(run('baseline').status,0);for(const[id,from,to]of faults){assert.equal(saved.split(from).length,2,id+' must match once');fs.writeFileSync(target,saved.replace(from,to));const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);results.push({id,compiling,detected,status:r.status});fs.writeFileSync(target,saved);console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail assertions');}}finally{fs.writeFileSync(target,saved);}
assert.equal(run('restored').status,0);fs.writeFileSync(path.join(root,'docs/math-port/evidence/p41-node-factory-mutations.json'),JSON.stringify({source_sha256:crypto.createHash('sha256').update(saved).digest('hex'),faults:results},null,2)+'\n');
