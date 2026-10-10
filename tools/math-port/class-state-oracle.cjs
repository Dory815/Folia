// Root prerequisite fixtures from pinned executable MmlNode, before host repair.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),npm=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
assert.equal(process.version,'v24.13.0');
const source=fs.readFileSync(path.join(root,'.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts'));
assert.equal(crypto.createHash('sha256').update(source).digest('hex'),'cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656');
assert.equal(fs.readFileSync(path.join(npm,'ts/core/MmlTree/MmlNode.ts'),'utf8').replaceAll('\r\n','\n'),source.toString().replaceAll('\r\n','\n'));
const runtime=path.join(npm,'cjs/core/MmlTree/MmlNode.js');
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(runtime)).digest('hex'),'86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45');
const factoryRuntime=path.join(npm,'cjs/core/Tree/NodeFactory.js');
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(factoryRuntime)).digest('hex'),'148c9a3a2e469fc464036b88deb7067cd779989a43356812d31365a84ec75c54');
const factorySource=fs.readFileSync(path.join(root,'.reference/MathJax-src/ts/core/Tree/NodeFactory.ts'));
assert.equal(crypto.createHash('sha256').update(factorySource).digest('hex'),'b02c5ac2cabb61755785d35def6b57f8246322c626f548df600ca81af27b4599');
assert.equal(fs.readFileSync(path.join(npm,'ts/core/Tree/NodeFactory.ts'),'utf8').replaceAll('\r\n','\n'),factorySource.toString().replaceAll('\r\n','\n'));
const {AbstractMmlNode:N,TextNode}=require(runtime),{AbstractNodeFactory:F}=require(factoryRuntime);
class Raw extends N{
  get kind(){return 'raw';}
  get texClass(){if(this.enabled)this.trace+='c';return this.current;}
  set texClass(v){if(this.enabled)this.trace+='C';this.current=v;}
  get prevClass(){if(this.enabled)this.trace+='p';return this.sequence?.length?this.sequence.shift():this.previous;}
  set prevClass(v){if(this.enabled)this.trace+='P';this.previous=v;}
  get prevLevel(){if(this.enabled)this.trace+='l';return this.level;}
  set prevLevel(v){if(this.enabled)this.trace+='L';this.level=v;}
  get attributes(){if(this.enabled)this.trace+='a';return this.attrs;}
  set attributes(v){if(this.enabled)this.trace+='A';this.attrs=v;}
  update(n){this.updateTeXclass(n);}
}
class MathNode extends N{get kind(){return 'math';}}
const factory=()=>new F({raw:Raw,math:MathNode,text:TextNode});
function fresh(){const f=factory(),n=f.create('raw');n.current=null;n.previous=null;n.level=0;n.attributes.set('scriptlevel',0);n.enabled=true;n.trace='';return {f,n};}
const values=[undefined,null,0,-0,1,2,3,7,99,-1,NaN,Infinity,false,true,'','0','1','-1','01','1.0','length','toString','constructor','__proto__',[],[0],[1],{},Object.create(null)];
const p=v=>v===undefined?'Missing{}':v===null?'PropertyNull{}':typeof v==='boolean'?String(v):typeof v==='string'?JSON.stringify(v):typeof v==='number'?Number.isNaN(v)?'std::numeric_limits<double>::quiet_NaN()':v===Infinity?'std::numeric_limits<double>::infinity()':Object.is(v,-0)?'-0.0':String(v)+'.0':Array.isArray(v)?'PropertyArray{'+v.map(p).join(',')+'}':Object.getPrototypeOf(v)===null?'nullObject()':'PropertyList{}';
const out=['// Generated root CJS raw-class matrix; native results never generate expectations.','#include "support/folia_test.hpp"','import folia.math.core.mml_tree.mml_node;','import folia.math.core.mml_tree.mml_node_host;','import folia.math.core.mml_tree.class_binding;','import folia.math.core.tree.node_factory;',
'namespace m=folia::math::mml;namespace h=m::host;namespace tr=folia::math::tree;using namespace boost::ut;using folia::math::Property;using folia::math::PropertyList;using folia::math::PropertyArray;using folia::math::Missing;using folia::math::PropertyNull;namespace {',
'PropertyList nullObject(){PropertyList p;p.set("__proto__",PropertyNull{});return p;}',
'bool same(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return (std::isnan(*x)&&std::isnan(*y))||std::bit_cast<std::uint64_t>(*x)==std::bit_cast<std::uint64_t>(*y);return a==b;}',
'struct Raw:m::AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;Property current=PropertyNull{},previous=PropertyNull{},level=0;std::vector<Property> sequence;mutable std::string trace;bool enabled=false;std::string kind()const override{return "raw";}Property texClass()override{if(enabled)trace+=\'c\';return current;}void setTexClass(Property p)override{if(enabled)trace+=\'C\';current=std::move(p);}Property prevClass()override{if(enabled)trace+=\'p\';if(sequence.empty())return previous;auto p=sequence.front();sequence.erase(sequence.begin());return p;}void setPrevClass(Property p)override{if(enabled)trace+=\'P\';previous=std::move(p);}Property prevLevel()const override{if(enabled)trace+=\'l\';return level;}void setPrevLevel(Property p)override{if(enabled)trace+=\'L\';level=std::move(p);}std::shared_ptr<folia::math::Attributes> attributes()const override{if(enabled)trace+=\'a\';return AbstractMmlNode::attributes();}void setAttributes(std::shared_ptr<folia::math::Attributes> a)override{if(enabled)trace+=\'A\';AbstractMmlNode::setAttributes(std::move(a));}void update(m::MmlNode*n){updateTeXclass(n);}};',
'struct MathNode:m::AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "math";}};',
'using F=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode> classes(){return {{"raw",m::classBinding<Raw>()},{"math",m::classBinding<MathNode>()},{"text",m::classBinding<m::TextNode>()}};}',
'Raw* fresh(F&f){auto*n=dynamic_cast<Raw*>(f.create("raw"));n->attributes()->set("scriptlevel",0);n->enabled=true;n->trace.clear();return n;}',
'std::vector<Property> inputs(){return {'+values.map(v=>'Property('+p(v)+')').join(',')+'};}',
'Property outcome(unsigned id){switch(id){case 0:return "";case 1:return "thinmathspace";case 2:return "mediummathspace";case 3:return "thickmathspace";default:return Missing{};}}',
'}','suite math_port_class_state_tests=[] {'];
let assertions=0,cases=0;const emit=x=>out.push(x),eq=(v,e)=>{assertions++;emit('expect(bool(('+e+')=='+JSON.stringify(v)+'));');},begin=n=>{cases++;emit('"math_port_class_state_'+n+'"_test=[] {');},end=()=>emit('};');
begin('base_storage_identity_and_initial_null');
{const f=factory(),n=f.create('math');assert.equal(n.texClass,null);assert.equal(n.prevClass,null);emit('F f(classes());auto*n=f.create("math");expect(n->texClass()==Property(PropertyNull{}));expect(n->prevClass()==Property(PropertyNull{}));');assertions+=2;
for(const v of values){n.texClass=v;n.prevClass=v;assert.ok(Object.is(n.texClass,v)&&Object.is(n.prevClass,v));}
emit('auto values=inputs();for(const auto&v:values){n->setTexClass(v);n->setPrevClass(v);expect(same(n->texClass(),v));expect(same(n->prevClass(),v));}');assertions+=2*values.length;end();}
begin('raw_roundtrip_nullish_selection');
emit('F f(classes());auto*n=f.create("raw");auto*prev=f.create("raw");auto values=inputs();');
emit('const std::array<bool,'+values.length+'> selected={'+values.map(v=>{const {f,n}=fresh(),prev=f.create('raw');n.current=v;return n.setTeXclass(prev)===n;}).join(',')+'};');
emit('for(std::size_t i=0;i<values.size();++i){n->setTexClass(values[i]);n->setPrevClass(values[i]);');for(const v of values){const {n}=fresh();n.texClass=v;n.prevClass=v;assert.ok(Object.is(n.texClass,v)&&Object.is(n.prevClass,v));}assertions+=3*values.length;emit('expect(same(n->texClass(),values[i]));expect(same(n->prevClass(),values[i]));expect(bool((n->setTeXclass(prev)==n)==selected[i]));}');end();
begin('source_spacing_state_matrix_and_read_order');
emit('struct Case{unsigned prev,current,script,outcome;std::string_view trace;};const Case cases[]={');
const names=['','thinmathspace','mediummathspace','thickmathspace'];let rows=0;
for(let i=0;i<values.length;i++)for(let j=0;j<values.length;j++)for(let script=0;script<2;script++){
  const {n}=fresh();n.previous=values[i];n.current=values[j];n.level=script?1:0;let result,id;try{result=n.texSpacing();id=result===undefined?4:names.indexOf(result);assert.ok(id>=0);}catch(e){assert.equal(e.name,'TypeError');id=5;}
  emit('{'+[i,j,script,id].join(',')+','+JSON.stringify(n.trace)+'},');rows++;
}
emit('};F f(classes());auto values=inputs();for(const auto& c:cases){auto*n=fresh(f);n->previous=values[c.prev];n->current=values[c.current];n->level=static_cast<double>(c.script);bool caught=false;Property result;try{result=n->texSpacing();}catch(const m::Error&e){caught=true;expect(e.name==std::string("TypeError"));}expect(caught==(c.outcome==5))<<c.prev<<":"<<c.current<<":"<<c.script<<" exception";if(!caught)expect(result==outcome(c.outcome))<<c.prev<<":"<<c.current<<":"<<c.script<<" outcome";expect(n->trace==c.trace)<<c.prev<<":"<<c.current<<":"<<c.script<<" trace="<<n->trace<<" expected="<<c.trace;}');assertions+=3*rows;end();
begin('prevclass_reread_and_current_capture');
for(const seq of [[0,3],[null,3],[undefined,3],[0,undefined],[0,-1]]){const {n}=fresh();n.sequence=[...seq];n.current=2;let value,id;try{value=n.texSpacing();id=value===undefined?4:names.indexOf(value);}catch(e){assert.equal(e.name,'TypeError');id=5;}emit('{F f(classes());auto*n=fresh(f);n->sequence={'+seq.map(v=>'Property('+p(v)+')').join(',')+'};n->current=2;bool caught=false;Property result;try{result=n->texSpacing();}catch(const m::Error&){caught=true;}');eq(id===5,'caught');if(id!==5){assertions++;emit('expect(result==outcome('+id+'));');}eq(n.trace,'n->trace');end();}end();
begin('raw_transfer_and_empty_readonly');
let sourceTrace,targetTrace;for(const v of values){const {f,n:a}=fresh(),b=f.create('raw');a.current=v;a.previous=v;a.level=v;b.current=0;b.previous=0;b.level=0;b.enabled=true;b.trace='';b.update(a);assert.ok(Object.is(b.current,v)&&Object.is(b.previous,v)&&Object.is(b.level,v));assert.equal(a.previous,null);assert.equal(a.level,null);sourceTrace=a.trace;targetTrace=b.trace;}
emit('F f(classes());auto*a=fresh(f);auto*b=fresh(f);auto values=inputs();for(const auto& v:values){a->enabled=false;b->enabled=false;a->current=v;a->previous=v;a->level=v;b->current=0;b->previous=0;b->level=0;a->trace.clear();b->trace.clear();a->enabled=true;b->enabled=true;b->update(a);expect(same(b->current,v));expect(same(b->previous,v));expect(same(b->level,v));expect(a->previous==Property(PropertyNull{}));expect(a->level==Property(PropertyNull{}));expect(a->trace=='+JSON.stringify(sourceTrace)+');expect(b->trace=='+JSON.stringify(targetTrace)+');}');assertions+=7*values.length;
emit('auto*t=f.create("text");for(const auto& v:values){bool tc=false,pc=false;try{t->setTexClass(v);}catch(const m::Error&e){tc=e.name=="TypeError";}try{t->setPrevClass(v);}catch(const m::Error&e){pc=e.name=="TypeError";}expect(tc);expect(pc);expect(t->texClass()==Property(-1));expect(t->prevClass()==Property(-1));}');assertions+=4*values.length;end();
begin('raw_previous_and_current_scriptlevel_coercion');
emit('struct Case{unsigned previousLevel,currentLevel,currentClass,outcome;std::string_view trace;};const Case cases[]={');
let levelRows=0;
for(let i=0;i<values.length;i++)for(let j=0;j<values.length;j++)for(const current of [1,2,99]){
  const {n}=fresh();n.previous=0;n.current=current;n.level=values[i];n.attributes.set('scriptlevel',values[j]);n.trace='';let result,id;
  try{result=n.texSpacing();id=result===undefined?4:names.indexOf(result);assert.ok(id>=0);}catch(e){assert.equal(e.name,'TypeError');id=5;}
  emit('{'+[i,j,current,id].join(',')+','+JSON.stringify(n.trace)+'},');levelRows++;
}
emit('};F f(classes());auto values=inputs();for(const auto&c:cases){auto*n=fresh(f);n->previous=0;n->current=static_cast<double>(c.currentClass);n->level=values[c.previousLevel];n->attributes()->set("scriptlevel",values[c.currentLevel]);n->trace.clear();bool caught=false;Property result;try{result=n->texSpacing();}catch(const m::Error&e){caught=true;expect(e.name==std::string("TypeError"));}expect(caught==(c.outcome==5))<<c.previousLevel<<":"<<c.currentLevel<<":"<<c.currentClass<<" exception";if(!caught)expect(result==outcome(c.outcome))<<c.previousLevel<<":"<<c.currentLevel<<":"<<c.currentClass<<" outcome";expect(n->trace==c.trace)<<"trace="<<n->trace<<" expected="<<c.trace;}');assertions+=3*levelRows;end();
begin('null_node_guards');
for(const v of values){const {n}=fresh();n.current=v;n.previous=v;n.level=v;n.update(null);assert.equal(n.trace,'');assert.ok(Object.is(n.current,v)&&Object.is(n.previous,v)&&Object.is(n.level,v));assert.equal(n.setTeXclass(null)===n,v!==undefined&&v!==null);assert.equal(n.trace,'c');}
emit('const std::array<bool,'+values.length+'> selectsSelf={'+values.map(v=>{const {n}=fresh();n.current=v;return n.setTeXclass(null)===n;}).join(',')+'};F f(classes());auto values=inputs();for(std::size_t i=0;i<values.size();++i){const auto&v=values[i];auto*n=fresh(f);n->current=v;n->previous=v;n->level=v;n->update(nullptr);expect(n->trace.empty());expect(same(n->current,v));expect(same(n->previous,v));expect(same(n->level,v));const auto selected=n->setTeXclass(nullptr);expect(selected==(selectsSelf[i]?n:nullptr));expect(n->trace=="c");}');assertions+=6*values.length;end();
begin('bound_number_intrinsics');
const boxes=[8,Number.prototype,Number,Number.prototype.toString,Number.prototype.valueOf];
const boxed=['Property(8)','Property(folia::math::BuiltinProperty::numberPrototype)','Property(folia::math::BuiltinProperty::numberConstructor)','Property(folia::math::BuiltinProperty::numberToString)','Property(folia::math::BuiltinProperty::numberValueOf)'];
const intrinsic=new Map([[Number.prototype,'numberPrototype'],[Number,'numberConstructor'],[Number.prototype.toString,'numberToString'],[Number.prototype.valueOf,'numberValueOf'],[Object.prototype,'prototype']]);
for(let i=0;i<boxes.length;i++){
 const keys=i<2?['__proto__','constructor','toString','valueOf','0','length']:i===2?['prototype','name','length']:['name','length'];
 for(const key of keys){const value=boxes[i][key],expected=intrinsic.has(value)?'folia::math::BuiltinProperty::'+intrinsic.get(value):p(value);assertions++;emit('expect(h::get('+boxed[i]+','+JSON.stringify(key)+')==Property('+expected+'));');}
}
eq(String(Number.prototype),'h::debugText(h::string(Property(folia::math::BuiltinProperty::numberPrototype)))');
eq(Number(Number.prototype),'h::number(Property(folia::math::BuiltinProperty::numberPrototype))');
eq(Boolean(Number.prototype),'h::truthy(Property(folia::math::BuiltinProperty::numberPrototype))');end();
emit('};');const output=out.join('\n')+'\n',target=path.join(root,'tests/math/reference_class_state.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);console.log(JSON.stringify({values:values.length,matrix_rows:rows,level_rows:levelRows,assertion_slots:assertions,cases,mode:process.argv.includes('--check')?'check':'generate'}));
