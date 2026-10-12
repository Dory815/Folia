// Root independent compiling faults; implementation agents must be idle.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),state='src/math/core/math_item_state.cpp',body='src/math/core/math_item.cpp',host='src/math/core/math_item_host.ixx';
const saved=new Map([state,body,host].map(file=>[file,fs.readFileSync(path.join(root,file),'utf8')]));
const faults=[
 ['constructor-location-alias',state,[['else start=std::move(begin);','else start=defaultLocation();']]],
 ['constructor-distinct-defaults',state,[['end=defaultLocation();','end=start;']]],
 ['escaped-strict-null',state,[['return std::holds_alternative<PropertyNull>(display.value);','return !property_host::truthy(display);']]],
 ['metrics-record-rebinding',state,[['metrics=PropertyList{{"em",std::move(em)},{"ex",std::move(ex)},\n                         {"containerWidth",std::move(cwidth)},{"scale",std::move(scale)}};','auto data=PropertyList::record(metrics).value();data.set("em",em);data.set("ex",ex);data.set("containerWidth",cwidth);data.set("scale",scale);']]],
 ['state-removal-restore',state,[['this->removeFromDocument(restore);','this->removeFromDocument(true);']]],
 ['state-removal-before-data',state,[['this->removeFromDocument(restore);','{outputData=PropertyList{};this->removeFromDocument(restore);}']]],
 ['state-current-after-removal',state,[['const Property typesetInput=math_item_host::constant("TYPESET");\n        if(math_item_host::less(target,typesetInput)){\n            const Property currentState=state_;','const Property typesetInput=math_item_host::constant("TYPESET");\n        if(math_item_host::less(target,typesetInput)){\n            const Property currentState=200;']]],
 ['state-input-record-reset',state,[['if(math_item_host::greaterEqual(currentState,compiledCurrent))inputData=PropertyList{};','if(math_item_host::greaterEqual(currentState,compiledCurrent)){}']]],
 ['reset-virtual-dispatch',state,[['this->state(math_item_host::constant("UNPROCESSED"),std::move(restore));','this->AbstractMathItem::state(math_item_host::constant("UNPROCESSED"),std::move(restore));']]],
 ['proto-item-offset-key',state,[['const Property start=PropertyList{{"n",std::move(begin)}};','const Property start=PropertyList{{"i",std::move(begin)}};']]],
 ['state-inherited-membership',host,[['return storage->objectPrototype&&!PropertyList{}.get(key).missing();','return false;']]],
 ['rerender-default-before-callback',body,[['if(start.missing())start=math_item_host::constant("RERENDER");\n    const Property current=this->state();','const Property current=this->state();\n    if(start.missing())start=math_item_host::constant("RERENDER");']]],
 ['rerender-live-actions',body,[['if(start.missing())start=math_item_host::constant("RERENDER");','auto earlyActions=math_item_host::actions(document);\n    if(start.missing())start=math_item_host::constant("RERENDER");'],['if(math_item_host::greaterEqual(current,start))\n        this->state(Property(property_host::number(start)-1),Missing{});\n    auto actions=math_item_host::actions(document);','if(math_item_host::greaterEqual(current,start))\n        this->state(Property(property_host::number(start)-1),Missing{});\n    auto actions=earlyActions;']]],
 ['compile-exclusive-threshold',body,[['if(math_item_host::less(current,threshold)){\n        auto& input','if(!math_item_host::less(threshold,current)){\n        auto& input']]],
 ['compile-live-completion',body,[['const Property completed=math_item_host::constant("COMPILED");','const Property completed=threshold;']]],
 ['compile-root-before-state',body,[['root=input.compile(*this,document);\n        const Property completed=math_item_host::constant("COMPILED");\n        this->state(completed,Missing{});','auto result=input.compile(*this,document);\n        const Property completed=math_item_host::constant("COMPILED");\n        this->state(completed,Missing{});\n        root=result;']]],
 ['typeset-receiver-before-escaped',body,[['const MathOutputJaxReference outputReceiver=math_item_host::output(document);\n        const bool escaped=this->isEscaped();','const bool escaped=this->isEscaped();\n        const MathOutputJaxReference outputReceiver=math_item_host::output(document);']]],
 ['typeset-live-completion',body,[['const Property completed=math_item_host::constant("TYPESET");','const Property completed=threshold;']]],
 ['typeset-output-before-state',body,[['typesetRoot=escaped?output.escaped(*this,document):output.typeset(*this,document);\n        const Property completed=math_item_host::constant("TYPESET");\n        this->state(completed,Missing{});','auto result=escaped?output.escaped(*this,document):output.typeset(*this,document);\n        const Property completed=math_item_host::constant("TYPESET");\n        this->state(completed,Missing{});\n        typesetRoot=result;']]],
 ['remove-virtual-clear',body,[['void AbstractMathItem::removeFromDocument(Property){this->clear();}','void AbstractMathItem::removeFromDocument(Property){this->AbstractMathItem::clear();}']]]
];
function run(label){
 const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_math_item*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);
 const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p45-math-item-'+label+'.log'),log);return{status:r.status,log};
}
const results=[];try{
 const baseline=run('mutation-baseline');assert.equal(baseline.status,0);assert.match(baseline.log,/92 asserts in 92 tests/);
 for(const[id,file,changes]of faults){
  let changed=saved.get(file);for(const[from,to]of changes){assert.equal(changed.split(from).length,2,id+' exact anchor');changed=changed.replace(from,to);}
  fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);
  results.push({id,file,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),saved.get(file));console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail original assertions');
 }
}finally{for(const[file,text]of saved)fs.writeFileSync(path.join(root,file),text);}
assert.equal(run('mutation-restored').status,0);
fs.writeFileSync(path.join(root,'docs/math-port/evidence/p45-math-item-mutations.json'),JSON.stringify({sources:[...saved].map(([file,text])=>({file,sha256:crypto.createHash('sha256').update(text).digest('hex')})),faults:results},null,2)+'\n');
