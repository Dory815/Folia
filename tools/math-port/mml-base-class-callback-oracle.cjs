// Root actual original base methods under virtual callbacks, aliasing and tagged nullish inputs.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');const{root,MmlFactory,core}=require('./inheritance-reference.cjs');
const modes=['','throw-n-P','throw-n-L','throw-n-C','throw-p-L','throw-p-P','mutate-n-P','attrs-null','attrs-rebind','skip-hook','null-hook','get-current-change'];
const inputs=[];for(const op of ['set','update','previous'])for(const value of ['N','U','self','p'])for(const mode of modes)for(const current of ['0','N','U'])inputs.push({op,value,mode,current});
const scalar=v=>v===undefined?'U':v===null?'N':String(v),raw=v=>v==='N'?null:v==='U'?undefined:Number(v);
function run(i){let active=false,trace=[],n,p;function note(s){if(active)trace.push(s);}function action(who,key){if(!active)return;if(i.mode==='throw-'+who+'-'+key)throw Object.assign(new Error('witness'),{name:'WitnessError'});if(i.mode==='mutate-n-P'&&who==='n'&&key==='P')p.pl=9;}
 class Probe extends core.AbstractMmlNode{
  get kind(){return 'probe';}get texClass(){note(this.id+'.c');const v=this.cur;if(active&&i.mode==='get-current-change'&&this.id==='n')this.cur=null;return v;}set texClass(v){note(this.id+'.C='+scalar(v));this.cur=v;action(this.id,'C');}
  get prevClass(){note(this.id+'.p');return this.pc;}set prevClass(v){note(this.id+'.P='+scalar(v));this.pc=v;action(this.id,'P');}
  get prevLevel(){note(this.id+'.l');return this.pl;}set prevLevel(v){note(this.id+'.L='+scalar(v));this.pl=v;action(this.id,'L');}
  get attributes(){note(this.id+'.a');const a=this.a;if(active&&i.mode==='attrs-rebind'&&this.id==='p')this.a=null;return a;}set attributes(v){this.a=v;}
  getPrevClass(v){note(this.id+'.hook');if(active&&i.mode==='skip-hook')return;if(active&&i.mode==='null-hook'){this.cur=null;return;}return core.AbstractMmlNode.prototype.getPrevClass.call(this,v);}
 }
 const f=new MmlFactory();f.setNodeClass('probe',Probe);n=f.create('probe');p=f.create('probe');n.id='n';p.id='p';n.cur=raw(i.current);n.pc=11;n.pl=7;p.cur=2;p.pc=5;p.pl=3;p.a.set('scriptlevel',3);if(i.mode==='attrs-null')p.a=null;
 const arg=i.value==='N'?null:i.value==='U'?undefined:i.value==='self'?n:p;let result='U',error='ok';active=true;
 try{if(i.op==='set'){const v=n.setTeXclass(arg);result=v===n?'self':v===p?'p':v===undefined?'U':v===null?'N':'UNBOUND';}else n[i.op==='update'?'updateTeXclass':'getPrevClass'](arg);}catch(e){error=e.name+'|'+e.message;result='THREW';}active=false;
 return error+'|'+result+'|'+[n.cur,n.pc,n.pl,p.cur,p.pc,p.pl].map(scalar).join(',')+'|'+Number(n.a!==null)+','+Number(p.a!==null)+'|'+trace.join(',');
}
const rows=inputs.map(input=>({input,expected:run(input)})),data=JSON.stringify(rows,null,2)+'\n',profile=path.join(root,'docs/math-port/evidence/p24-base-class-callback-source.json');
const out=['// Generated only from original CJS base methods with controlled virtual callbacks.','#include "support/folia_test.hpp"','import folia.math.core.mml_tree.mml_factory;','import folia.math.core.mml_tree.class_binding;','import folia.math.core.mml_tree.mml_node_host;',
`namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;namespace h=folia::math::mml::host;
std::string scalar(const Property&v){if(v.missing())return "U";if(std::holds_alternative<PropertyNull>(v.value))return "N";return h::debugText(h::string(v));}
Property raw(std::string_view v){if(v=="N")return PropertyNull{};if(v=="U")return Missing{};return 0;}
bool active=false;std::string mode;std::vector<std::string>trace;struct Probe;Probe*other=nullptr;void note(std::string s){if(active)trace.push_back(std::move(s));}void action(Probe*,std::string_view);
struct Probe:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string id;Property cur=Missing{},pc=Missing{},pl=Missing{};mutable std::shared_ptr<Attributes>a;
std::string kind()const override{return "probe";}Property texClass()override{note(id+".c");const auto v=cur;if(active&&mode=="get-current-change"&&id=="n")cur=PropertyNull{};return v;}void setTexClass(Property v)override{note(id+".C="+scalar(v));cur=v;action(this,"C");}
Property prevClass()override{note(id+".p");return pc;}void setPrevClass(Property v)override{note(id+".P="+scalar(v));pc=v;action(this,"P");}Property prevLevel()const override{note(id+".l");return pl;}void setPrevLevel(Property v)override{note(id+".L="+scalar(v));pl=v;action(this,"L");}
std::shared_ptr<Attributes>attributes()const override{note(id+".a");const auto result=a;if(active&&mode=="attrs-rebind"&&id=="p")a.reset();return result;}void setAttributes(std::shared_ptr<Attributes>v)override{a=std::move(v);}
void update(NodeValue v){updateTeXclass(v);}void previous(NodeValue v){getPrevClass(v);}protected:void getPrevClass(NodeValue v)override{note(id+".hook");if(active&&mode=="skip-hook")return;if(active&&mode=="null-hook"){cur=PropertyNull{};return;}AbstractMmlNode::getPrevClass(v);}
};
void action(Probe*p,std::string_view key){if(!active)return;if(mode=="throw-"+p->id+"-"+std::string(key))throw Error("WitnessError",u"witness");if(mode=="mutate-n-P"&&p->id=="n"&&key=="P")other->pl=9;}
struct Case{std::string op,value,mode,current,expected;};
std::string run(const Case&c){active=false;mode=c.mode;trace.clear();MmlFactory f;f.setNodeClass("probe",classBinding<Probe>());auto*n=dynamic_cast<Probe*>(f.create("probe"));auto*p=dynamic_cast<Probe*>(f.create("probe"));other=p;n->id="n";p->id="p";n->cur=raw(c.current);n->pc=11;n->pl=7;p->cur=2;p->pc=5;p->pl=3;p->a->set("scriptlevel",3);if(mode=="attrs-null")p->a.reset();
NodeValue arg=c.value=="N"?NodeValue(PropertyNull{}):c.value=="U"?NodeValue(Missing{}):NodeValue(c.value=="self"?n:p);std::string result="U",error="ok";active=true;try{if(c.op=="set"){auto v=n->setTeXclass(arg);result=v==NodeValue(n)?"self":v==NodeValue(p)?"p":v.kind()==tree::ChildKind::Undefined?"U":v.kind()==tree::ChildKind::Null?"N":"UNBOUND";}else if(c.op=="update")n->update(arg);else n->previous(arg);}catch(const Error&e){error=e.name+"|"+h::debugText(e.message);result="THREW";}active=false;
std::string output=error+"|"+result+"|";for(const Property&v:{n->cur,n->pc,n->pl,p->cur,p->pc,p->pl}){if(output.back()!='|')output+=',';output+=scalar(v);}output+="|"+std::to_string(bool(n->a))+","+std::to_string(bool(p->a))+"|";for(std::size_t i=0;i<trace.size();++i){if(i)output+=',';output+=trace[i];}return output;}
suite base_class_callbacks=[] {"math_port_class_state_callbacks_aliases_and_partial_effects"_test=[] {const Case cases[]={`];
for(const r of rows)out.push('{'+[r.input.op,r.input.value,r.input.mode,r.input.current,r.expected].map(JSON.stringify).join(',')+'},');out.push('};for(const auto&c:cases)expect(eq(run(c),c.expected))<<c.op<<"/"<<c.value<<"/"<<c.mode<<"/"<<c.current;};};}');
const output=out.join('\n')+'\n',fixture=path.join(root,'tests/math/reference_mml_base_class_callbacks.cpp');if(process.argv.includes('--check')){assert.equal(fs.readFileSync(profile,'utf8').replaceAll('\r\n','\n'),data);assert.equal(fs.readFileSync(fixture,'utf8').replaceAll('\r\n','\n'),output);}else{assert.ok(!fs.existsSync(profile)&&!fs.existsSync(fixture));fs.writeFileSync(profile,data);fs.writeFileSync(fixture,output);}
console.log(JSON.stringify({source_calls:rows.length,original_errors:rows.filter(r=>!r.expected.startsWith('ok|')).length,sha256:crypto.createHash('sha256').update(data).digest('hex')}));
