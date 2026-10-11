// Root original protected-method boundary observations, including actual native stub diagnostics.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const {root,MmlFactory,core}=require('./inheritance-reference.cjs');class Probe extends core.AbstractMmlNode{get kind(){return 'probe';}}
const rows=[];for(const op of ['setTeXclass','updateTeXclass','getPrevClass']){const f=new MmlFactory();f.setNodeClass('probe',Probe);const n=f.create('probe'),p=f.create('probe');n.texClass=0;n.prevClass=11;n.prevLevel=7;p.texClass=2;p.prevClass=5;p.prevLevel=3;p.attributes.set('scriptlevel',3);const result=n[op](op==='setTeXclass'?null:p);rows.push({op,self:result===n,n:[n.texClass,n.prevClass,n.prevLevel],p:[p.texClass,p.prevClass,p.prevLevel]});}
const profile=JSON.stringify(rows,null,2)+'\n',profileFile=path.join(root,'docs/math-port/evidence/p24-base-class-boundary-source.json');
const p=v=>v===null?'Property(PropertyNull{})':'Property('+v+')';
const out=['// Generated only from original CJS protected-method observations.','#include "support/folia_test.hpp"','import folia.math.core.mml_tree.mml_factory;','import folia.math.core.mml_tree.class_binding;','namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;',
'struct Probe:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "probe";}void update(NodeValue p){updateTeXclass(p);}void previous(NodeValue p){getPrevClass(p);}};',
'suite base_class_boundary=[] {'];let assertions=0;
for(const r of rows){out.push('"math_port_class_state_boundary_'+r.op+'"_test=[] {MmlFactory f;f.setNodeClass("probe",classBinding<Probe>());auto*n=dynamic_cast<Probe*>(f.create("probe"));auto*p=dynamic_cast<Probe*>(f.create("probe"));n->setTexClass(0);n->setPrevClass(11);n->setPrevLevel(7);p->setTexClass(2);p->setPrevClass(5);p->setPrevLevel(3);p->attributes()->set("scriptlevel",3);try{');
if(r.op==='setTeXclass'){out.push('expect((n->setTeXclass(NodeValue(PropertyNull{}))==NodeValue(n))=='+r.self+');');assertions++;}else out.push('n->'+(r.op==='updateTeXclass'?'update':'previous')+'(NodeValue(p));');
for(const[id,values]of [['n',r.n],['p',r.p]])for(const[index,member]of ['texClass','prevClass','prevLevel'].entries()){out.push('expect('+id+'->'+member+'()=='+p(values[index])+');');assertions++;}
out.push('}catch(const Error&e){std::cerr<<e.name<<" base-class boundary\\n";expect(false)<<e.name;}};');}
out.push('};}');const output=out.join('\n')+'\n',fixture=path.join(root,'tests/math/reference_mml_base_class_boundary.cpp');
if(process.argv.includes('--check')){assert.equal(fs.readFileSync(profileFile,'utf8').replaceAll('\r\n','\n'),profile);assert.equal(fs.readFileSync(fixture,'utf8').replaceAll('\r\n','\n'),output);}else{assert.ok(!fs.existsSync(profileFile)&&!fs.existsSync(fixture));fs.writeFileSync(profileFile,profile);fs.writeFileSync(fixture,output);}
console.log(JSON.stringify({original_calls:rows.length,assertions,tests:rows.length,sha256:crypto.createHash('sha256').update(profile).digest('hex')}));
