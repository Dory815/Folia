// Root direct pinned-CJS oracle for the complete MathChoice semantic class.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,u,MmlFactory,core,MathChoice}=require('./inheritance-reference.cjs');
const {valueEmitter}=require('./oracle-values.cjs'),v=valueEmitter(u),{p}=v;
let active=false,events=[],calls=[],raw,mode='',choice,other,late;
const note=x=>{if(active)events.push(x);},id=n=>n?n.getProperty('id'):null;
class Receive extends core.AbstractMmlNode {
get kind(){return 'receive';}
setInheritedAttributes(a,d,l,p){note('inherit');calls.push({id:id(this),a,d,l,p,same:a===raw});if(mode==='inherit-throw')throw Object.assign(new Error('witness'),{name:'WitnessError'});}
}
class Row extends Receive {get kind(){return 'mrow';}}
class Parent extends core.AbstractMmlNode {
get kind(){return 'parent';}
replaceChild(n,o){note('replace:'+id(this));if(mode==='replace-before')throw Object.assign(new Error('witness'),{name:'WitnessError'});const result=super.replaceChild(n,o);if(mode==='replace-after')throw Object.assign(new Error('witness'),{name:'WitnessError'});if(mode==='replace-rebind')this.childNodes=[late];return mode==='replace-return-null'?null:result;}
}
function setup(shape,scenario){active=false;events=[];calls=[];mode=scenario;
const f=new MmlFactory();f.setNodeClass('MathChoice',MathChoice);f.setNodeClass('receive',Receive);f.setNodeClass('parent',Parent);f.setNodeClass('mrow',Row);
const rs=[0,1,2,3].map(i=>{const n=f.create('receive');n.setProperty('id',i);return n;});
choice=f.create('MathChoice',{},rs);choice.setProperty('id',99);const parent=f.create('parent');parent.setProperty('id',10);other=f.create('parent');other.setProperty('id',11);late=f.create('receive');late.setProperty('id',8);parent.appendChild(choice);
if(shape==='empty')choice.childNodes=[];if(shape==='null-array')choice.childNodes=null;if(shape==='null-slot')choice.childNodes=[rs[0],null,rs[2],rs[3]];if(shape==='short')choice.childNodes=[rs[0]];
if(scenario==='detached')choice.parent=null;
const create=f.create;f.create=function(kind,...args){if(active&&kind==='mrow'){note('factory:'+args.length);if(mode==='factory-throw')throw Object.assign(new Error('witness'),{name:'WitnessError'});if(mode==='factory-null')return null;if(mode==='factory-reparent')choice.parent=other;if(mode==='factory-rebind')choice.childNodes=[late];const n=create.call(this,kind,...args);n.setProperty('id',4);return n;}return create.call(this,kind,...args);};
return {f,n:choice,parent,other,rs};}
function observe(s,args,count){let error='';active=true;try{s.n.setInheritedAttributes(...args.slice(0,count));}catch(e){error=e.name;}active=false;return {error,kind:s.n.kind,arity:s.n.arity,notParent:s.n.notParent,parent:id(s.n.parent),children:s.n.childNodes?.map(id)??null,first:s.parent.childNodes.map(id),second:s.other.childNodes.map(id),rs:s.rs.map(n=>id(n.parent)),calls,events};}
const cases=[];function add(name,attrs,display,level,prime,count=4,shape='full',scenario=''){
const s=setup(shape,scenario);raw=attrs;const wanted=observe(s,[raw,display,level,prime],count);
cases.push('"math_port_mathchoice_'+name+'"_test=[] {reset();Setup s('+JSON.stringify(shape)+','+JSON.stringify(scenario)+');raw='+p(attrs)+';std::array<Property,4>args{raw,'+p(display)+','+p(level)+','+p(prime)+'};expect(deep(observe(s,args,'+count+'),'+p(wanted)+'));};');}
const levels=[undefined,null,false,true,-Infinity,-3,-0,0,0.5,1,1.5,2,3,Infinity,NaN,'','2','x',[],[1],[1,2],{}, {valueOf:0,toString:null}];
const displays=[undefined,null,false,true,0,-0,NaN,'',[],{},'raw'];
let index=0;for(const d of displays)for(const l of levels)add('selection_'+index++,{mathcolor:['math','blue']},d,l,'raw-prime');
for(const a of [undefined,null,false,3,'xy',[],{}, {mathsize:['math','2em']}])for(const count of [0,1,2,3,4])add('arity_'+index++,a,true,1,true,count);
for(const shape of ['full','empty','null-array','null-slot','short'])for(const scenario of ['', 'detached','factory-throw','factory-null','factory-reparent','factory-rebind','inherit-throw','replace-before','replace-after','replace-rebind','replace-return-null'])for(const level of [0,NaN])add('state_'+index++,null,false,level,MissingValue(),4,shape,scenario);
function MissingValue(){return undefined;}
function tree(n){return {kind:n.kind,text:n.kind==='text'?n.getText():null,inherited:n.attributes?.getAllInherited()??null,properties:n.getAllProperties(),children:n.childNodes.map(tree)};}
for(const display of ['inline','block'])for(const level of [0,1,2]){
active=false;const f=new MmlFactory();f.setNodeClass('MathChoice',MathChoice);
const token=text=>f.create('mi',{},[f.create('text').setText(text)]);
const inner=f.create('MathChoice',{},['D','T','S','SS'].map(x=>f.create('mrow',{},[token(x)])));
const branches=['display','text','script','scriptscript'].map(x=>f.create('mrow',{},[token(x),f.create('MathChoice',{},['α','β','γ','δ'].map(y=>f.create('mrow',{},[token(y)])))]));
branches[0].appendChild(inner);const math=f.create('math',{display},[f.create('MathChoice',{},branches)]);math.setInheritedAttributes({mathcolor:['math','blue']},false,level,false);const wanted=tree(math);
cases.push('"math_port_mathchoice_real_'+display+'_'+level+'"_test=[] {expect(deep(realTree('+JSON.stringify(display)+','+level+'),'+p(wanted)+'));};');
}
const defaults=Object.assign({},MathChoice.defaults);assert.notEqual(MathChoice.defaults,core.AbstractMmlBaseNode.defaults);const old=core.AbstractMmlBaseNode.defaults['choice-root-probe'];core.AbstractMmlBaseNode.defaults['choice-root-probe']='later';assert.equal(MathChoice.defaults['choice-root-probe'],undefined);if(old===undefined)delete core.AbstractMmlBaseNode.defaults['choice-root-probe'];else core.AbstractMmlBaseNode.defaults['choice-root-probe']=old;
cases.push('"math_port_mathchoice_defaults"_test=[] {expect(deep(m::MathChoice::defaults(),'+p(defaults)+'));expect(Property(m::MathChoice::defaults())!=Property(m::AbstractMmlBaseNode::defaults()));const auto old=m::AbstractMmlBaseNode::defaults().getOwn("choice-root-probe");m::AbstractMmlBaseNode::defaults().setOwn("choice-root-probe","later");expect(m::MathChoice::defaults().getOwn("choice-root-probe").missing());if(old.missing())m::AbstractMmlBaseNode::defaults().erase("choice-root-probe");else m::AbstractMmlBaseNode::defaults().setOwn("choice-root-probe",old);};');
const header=`// Generated from actual pinned MathJax by mathchoice-oracle.cjs.
#include "support/folia_test.hpp"
import folia.math.core.tree.node_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mathchoice;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.leaf_host;
namespace {using namespace boost::ut;namespace m=folia::math::mml;namespace tr=folia::math::tree;namespace h=m::host;using folia::math::Property;using folia::math::PropertyList;using folia::math::PropertyArray;using folia::math::Missing;using folia::math::PropertyNull;
std::u16string units(std::initializer_list<char16_t>v){return {v.begin(),v.end()};}
Property clone(const Property&v){if(auto a=PropertyArray::array(v)){PropertyArray r;for(std::size_t i=0;i<a->size();++i)r.push(clone(a->get(i)));return r;}if(auto a=PropertyList::object(v)){PropertyList r;for(const auto&k:a->keys())r.setOwn(k,clone(a->getOwn(k)));return r;}return v;}
bool deep(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return(std::isnan(*x)&&std::isnan(*y))||(*x==*y&&std::signbit(*x)==std::signbit(*y));auto x=PropertyList::record(a),y=PropertyList::record(b);if(x&&y){if(PropertyArray::array(a).has_value()!=PropertyArray::array(b).has_value()||x->keys()!=y->keys())return false;for(const auto&k:x->keys())if(!deep(x->getOwn(k),y->getOwn(k)))return false;return true;}return a==b;}
bool active=false;PropertyArray events,calls;Property raw;std::string mode;m::MmlNode*choice=nullptr,*other=nullptr,*late=nullptr;void note(std::string s){if(active)events.push(s);}void reset(){active=false;events=PropertyArray{};calls=PropertyArray{};raw=Missing{};mode.clear();choice=other=late=nullptr;}Property id(m::MmlNode*n){return n?n->getProperty("id"):Property(PropertyNull{});}void fail(){throw m::Error("WitnessError",u"witness");}
 m::Children children(std::initializer_list<m::MmlNode*>v={}){auto result=std::make_shared<tr::ChildArray<m::MmlNode>>();for(auto*node:v)result->push_back(m::NodeValue(node));return result;}
struct Receive:m::AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "receive";}void inheritAttributes(m::AttributeList a,Property d,Property l,Property p)override{note("inherit");calls.push(PropertyList{{"id",id(this)},{"a",a},{"d",d},{"l",l},{"p",p},{"same",a==raw}});if(mode=="inherit-throw")fail();}};
struct Row:Receive{using Receive::Receive;std::string kind()const override{return "mrow";}};
struct Parent:m::AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "parent";}m::NodeValue replaceChild(m::NodeValue n,m::NodeValue o)override{note("replace:"+h::debugText(h::string(id(this))));if(mode=="replace-before")fail();auto result=AbstractMmlNode::replaceChild(n,o);if(mode=="replace-after")fail();if(mode=="replace-rebind")childNodes=children({late});return mode=="replace-return-null"?m::NodeValue(PropertyNull{}):result;}};
using NF=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode>bindings(){return{{"math",m::classBinding<m::MmlMath>()},{"MathChoice",m::classBinding<m::MathChoice>()},{"receive",m::classBinding<Receive>()},{"parent",m::classBinding<Parent>()},{"mrow",m::classBinding<Row>()}};}
struct Factory:NF{using NF::NF;m::MmlNode*create(std::string_view k,tr::FactoryArgs a={})override{if(active&&k=="mrow"){note("factory:"+std::to_string(a.size()));if(mode=="factory-throw")fail();if(mode=="factory-null")return nullptr;if(mode=="factory-reparent")choice->setParent(m::NodeValue(other));if(mode=="factory-rebind")choice->childNodes=children({late});auto*n=NF::create(k,a);n->setProperty("id",4);return n;}return NF::create(k,a);}};
struct Setup{Factory f{bindings()};m::MmlNode*n,*parent,*other;std::array<m::MmlNode*,4>rs;Setup(std::string shape,std::string scenario){mode=scenario;for(std::size_t i=0;i<4;++i){rs[i]=f.create("receive");rs[i]->setProperty("id",double(i));}n=f.create("MathChoice",std::array<std::any,2>{PropertyList{},children({rs[0],rs[1],rs[2],rs[3]})});choice=n;n->setProperty("id",99);parent=f.create("parent");parent->setProperty("id",10);other=f.create("parent");other->setProperty("id",11);::other=other;late=f.create("receive");late->setProperty("id",8);parent->appendChild(n);if(shape=="empty")n->childNodes=children();if(shape=="null-array")n->childNodes=nullptr;if(shape=="null-slot")n->childNodes=children({rs[0],nullptr,rs[2],rs[3]});if(shape=="short")n->childNodes=children({rs[0]});if(scenario=="detached")n->setParent(m::NodeValue(PropertyNull{}));}};
Property ids(m::Children c){if(!c)return PropertyNull{};PropertyArray r;r.resize(c->size());for(std::size_t i=0;i<c->size();++i)if(c->has(i))r.set(i,id(c->get(i).nodeOrNull()));return r;}
Property observe(Setup&s,const std::array<Property,4>&a,int count){std::string error;active=true;try{switch(count){case 0:s.n->setInheritedAttributes();break;case 1:s.n->setInheritedAttributes(a[0]);break;case 2:s.n->setInheritedAttributes(a[0],a[1]);break;case 3:s.n->setInheritedAttributes(a[0],a[1],a[2]);break;default:s.n->setInheritedAttributes(a[0],a[1],a[2],a[3]);}}catch(const m::Error&e){error=e.name;}active=false;PropertyArray parents;for(auto*n:s.rs)parents.push(id(n->getParent().nodeOrNull()));return PropertyList{{"error",error},{"kind",s.n->kind()},{"arity",s.n->arity()},{"notParent",s.n->notParent()},{"parent",id(s.n->getParent().nodeOrNull())},{"children",ids(s.n->childNodes)},{"first",ids(s.parent->childNodes)},{"second",ids(s.other->childNodes)},{"rs",parents},{"calls",calls},{"events",events}};}
Property tree(m::MmlNode*n){Property text=PropertyNull{};if(auto*t=dynamic_cast<m::TextNode*>(n))text=h::debugText(t->getText());auto attrs=n->attributes();PropertyArray nodes;auto children=n->childNodes;nodes.resize(children->size());for(std::size_t i=0;i<children->size();++i)if(children->has(i))nodes.set(i,tree(children->get(i).requireNode()));return PropertyList{{"kind",n->kind()},{"text",text},{"inherited",attrs?Property(attrs->getAllInherited()):Property(PropertyNull{})},{"properties",n->getAllProperties()},{"children",nodes}};}
Property realTree(std::string display,double level){active=false;NF f(tr::FactoryClassMap<m::MmlNode>{{"math",m::classBinding<m::MmlMath>()},{"mrow",m::classBinding<m::MmlMrow>()},{"inferredMrow",m::classBinding<m::MmlInferredMrow>()},{"mi",m::classBinding<m::MmlMi>()},{"text",m::classBinding<m::TextNode>()},{"MathChoice",m::classBinding<m::MathChoice>()}});auto create=[&](std::string_view kind,m::Children c,PropertyList a=PropertyList{}){return f.create(kind,std::array<std::any,2>{a,c});};auto token=[&](std::string text){auto*t=dynamic_cast<m::TextNode*>(f.create("text"));t->setText(h::text(text));return create("mi",children({t}));};auto choiceOf=[&](std::initializer_list<std::string>labels){auto c=children();for(const auto&label:labels)c->push_back(create("mrow",children({token(label)})));return create("MathChoice",c);};auto*inner=choiceOf({"D","T","S","SS"});auto branches=children();for(const auto&label:{"display","text","script","scriptscript"})branches->push_back(create("mrow",children({token(label),choiceOf({"α","β","γ","δ"})})));branches->get(0).requireNode()->appendChild(inner);auto*math=create("math",children({create("MathChoice",branches)}),PropertyList{{"display",display}});math->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},false,level,false);return clone(tree(math));}
}
`;
const output=header+v.declarations().join('\n')+'\nnamespace {suite choice_tests=[] {\n'+cases.join('\n')+'\n};}\n',file=path.join(root,'tests/math/reference_mathchoice.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(file,output);console.log(JSON.stringify({cases:cases.length,source_values:v.size(),mode:process.argv.includes('--check')?'check':'generate'}));
