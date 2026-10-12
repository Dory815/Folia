// Root Source observations for the getForms single-space-length boundary.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const {root,M}=require('./mo-pattern-reference.cjs');
const base=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const {MmlFactory}=require(path.join(base,'core/MmlTree/MmlFactory.js'));
const {MmlMrow}=require(path.join(base,'core/MmlTree/MmlNodes/mrow.js'));
let formLength,formPosition,formMo,formEvents;
class Row extends MmlMrow {
 get isEmbellished(){return false;}
 nonSpaceLength(){formEvents.push('length');return formLength;}
 firstNonSpace(){formEvents.push('first');return formPosition==='first'?formMo:null;}
 lastNonSpace(){formEvents.push('last');return formPosition==='last'?formMo:null;}
}
function run(length,position){formLength=length;formPosition=position;const f=new MmlFactory();f.setNodeClass('forms-row',Row);formMo=f.create('mo');formMo.parent=f.create('forms-row');formEvents=[];return formMo.getForms().join(',')+'|'+formEvents.join(',');}
const rows=[];for(const length of [0,1,2])for(const position of ['first','last','neither'])rows.push({length,position,expected:run(length,position)});
const data=JSON.stringify(rows)+'\n';
const out=['// Root generated getForms boundary observations from pinned MathJax.','#include "support/folia_test.hpp"','import folia.math.core.mml_tree.mml_factory;','import folia.math.core.mml_tree.class_binding;','import folia.math.core.mml_tree.mml_nodes.mo;','import folia.math.core.mml_tree.mml_nodes.mrow;',
`namespace {using namespace boost::ut;using namespace folia::math::mml;std::size_t formLength;std::string formPosition,formEvents;MmlNode*formMo;void note(std::string_view s){if(!formEvents.empty())formEvents+=",";formEvents+=s;}struct Row:MmlMrow{using MmlMrow::MmlMrow;bool isEmbellished()const override{return false;}std::size_t nonSpaceLength()override{note("length");return formLength;}MmlNode*firstNonSpace()override{note("first");return formPosition=="first"?formMo:nullptr;}MmlNode*lastNonSpace()override{note("last");return formPosition=="last"?formMo:nullptr;}};struct Case{std::size_t length;std::string_view position,expected;};std::string run(const Case&i){formLength=i.length;formPosition=i.position;MmlFactory f;f.setNodeClass("forms-row",classBinding<Row>());auto*mo=dynamic_cast<MmlMo*>(f.create("mo"));formMo=mo;mo->setParent(f.create("forms-row"));formEvents.clear();const auto forms=mo->getForms();std::string value;for(std::size_t j=0;j<forms.size();++j){if(j)value+=",";value+=std::get<std::string>(forms.get(j).value);}return value+"|"+formEvents;}suite formsBoundary=[] {"math_port_forms_length_boundary_source"_test=[] {const Case cases[]={`];
for(const r of rows)out.push('{'+r.length+','+JSON.stringify(r.position)+','+JSON.stringify(r.expected)+'},');
out.push('};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.position<<i.length;};};}');
const profile=path.join(root,'docs/math-port/evidence/p37-forms-boundary-source.json'),fixture=path.join(root,'tests/math/reference_mml_forms_boundary.cpp'),output=out.join('\n')+'\n';
if(process.argv.includes('--check')){assert.equal(fs.readFileSync(profile,'utf8').replaceAll('\r\n','\n'),data);assert.equal(fs.readFileSync(fixture,'utf8').replaceAll('\r\n','\n'),output);}else{assert.ok(!fs.existsSync(profile)&&!fs.existsSync(fixture));fs.writeFileSync(profile,data);fs.writeFileSync(fixture,output);}
console.log(JSON.stringify({source_calls:rows.length,sha256:crypto.createHash('sha256').update(data).digest('hex')}));
