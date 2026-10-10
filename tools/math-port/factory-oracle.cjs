// Root-owned expectations from actual pinned JS, never native implementations.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),npmRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const sources={
 'Factory':'e61d2f5868287e2f382503cf690cd241698bf212957c1795d0cd35ce3bef049e',
 'NodeFactory':'b02c5ac2cabb61755785d35def6b57f8246322c626f548df600ca81af27b4599',
 'WrapperFactory':'8c59ed6a5e646e0736730070ba87161d7060f1f4a083e92afcc8012345537cea'
};
for(const [name,hash]of Object.entries(sources)){
 const source=fs.readFileSync(path.join(root,'.reference/MathJax-src/ts/core/Tree',name+'.ts'));
 assert.equal(crypto.createHash('sha256').update(source).digest('hex'),hash);
 assert.equal(fs.readFileSync(path.join(npmRoot,'ts/core/Tree',name+'.ts'),'utf8').replaceAll('\r\n','\n'),source.toString('utf8').replaceAll('\r\n','\n'));
}
const {AbstractFactory:F}=require(path.join(npmRoot,'cjs/core/Tree/Factory.js'));
const {AbstractNodeFactory:NF}=require(path.join(npmRoot,'cjs/core/Tree/NodeFactory.js'));
const {AbstractWrapperFactory:WF}=require(path.join(npmRoot,'cjs/core/Tree/WrapperFactory.js'));
class B{constructor(factory,...args){this.factory=factory;this.args=args;}get kind(){return 'reported-b';}}
class D extends B{get kind(){return 'reported-d';}}
class U{constructor(factory,...args){this.factory=factory;this.args=args;}get kind(){return 'reported-b';}}
const out=[
'// Frozen by root from pinned MathJax factory operations. Do not hand-edit.',
'#include "support/folia_test.hpp"',
'import folia.math.core.tree.node_factory;', 'import folia.math.core.tree.wrapper_factory;',
'namespace tr=folia::math::tree;using namespace boost::ut;',
'namespace {',
'struct Probe:tr::FactoryNode {tr::FactoryServices<Probe>* factory=nullptr;std::vector<std::any> args;std::string report="reported-b";std::string kind() const override{return report;}};',
'struct BaseProbe:Probe{};struct DerivedProbe:BaseProbe{DerivedProbe(){report="reported-d";}};struct UnrelatedProbe:Probe{};',
'static_assert(!std::is_copy_constructible_v<tr::AbstractFactory<Probe>> && !std::is_move_constructible_v<tr::AbstractFactory<Probe>> && !std::is_copy_assignable_v<tr::AbstractFactory<Probe>> && !std::is_move_assignable_v<tr::AbstractFactory<Probe>>);',
'template<class T>tr::FactoryClassRef<Probe> klass(){auto c=std::make_shared<tr::FactoryClass<Probe>>();c->nativeType=typeid(T);c->construct=[](tr::FactoryServices<Probe>& factory,tr::FactoryArgs args){auto p=std::make_unique<T>();p->factory=&factory;p->args.assign(args.begin(),args.end());return factory.adopt(std::move(p));};c->isInstance=[](const Probe* p){return dynamic_cast<const T*>(p)!=nullptr;};return c;}',
'struct DefaultGuard {tr::FactoryClassMap<Probe> old=tr::AbstractFactory<Probe>::defaultNodes;~DefaultGuard(){tr::AbstractFactory<Probe>::defaultNodes=std::move(old);}};',
'struct Exposed:tr::AbstractFactory<Probe>{using tr::AbstractFactory<Probe>::AbstractFactory;void registerOnly(std::string key,tr::FactoryClassRef<Probe> c){for(auto& e:nodeMap_)if(e.first==key){e.second=std::move(c);return;}nodeMap_.emplace_back(std::move(key),std::move(c));}};',
'tr::FactoryCreatorRef<Probe> own(tr::FactoryCreatorView<Probe> view,std::string_view key){for(auto& e:view->entries)if(e.first==key)return e.second;return {};}',
'void writeOwn(tr::FactoryCreatorView<Probe> view,std::string key,tr::FactoryCreatorRef<Probe> c){for(auto& e:view->entries)if(e.first==key){e.second=std::move(c);return;}view->entries.emplace_back(std::move(key),std::move(c));}',
'bool same_properties(const std::any& x,const folia::math::PropertyList& p){return std::any_cast<folia::math::PropertyList>(x).getOwn("v")==p.getOwn("v") && folia::math::Property(std::any_cast<folia::math::PropertyList>(x))==folia::math::Property(p);}',
'}', 'suite math_port_factory_reference=[] {'
];
let assertions=0,cases=0;
const s=x=>JSON.stringify(x), strings=xs=>'std::vector<std::string>{'+xs.map(s).join(',')+'}';
function begin(name){cases++;out.push(`"math_port_factory_${name}"_test=[] {`,'auto b=klass<BaseProbe>(),d=klass<DerivedProbe>(),u=klass<UnrelatedProbe>();');}
function expect(v,expr){out.push(`expect(bool((${expr})==${s(v)}));`);assertions++;}
function throws(fn,expr){let e;try{fn();}catch(error){e=error;}assert.ok(e,'Missing source exception');out.push(`{bool caught=false;try{${expr};}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name==${s(e.name)}));}expect(caught);}`);assertions+=2;}
function kinds(f,expr){out.push(`expect(bool(${expr}==${strings(f.getKinds())}));`);assertions++;}
function end(){out.push('};');}
{
 begin('defaults');F.defaultNodes={first:B,unknown:U};out.push('DefaultGuard guard;tr::AbstractFactory<Probe>::defaultNodes={{"first",b},{"unknown",u}};tr::AbstractFactory<Probe> inherited;');
 const f=new F();kinds(f,'inherited.getKinds()');expect(f.create('absent').kind,'inherited.create("absent")->kind()');
 const empty=new F({});out.push('tr::AbstractFactory<Probe> empty(tr::FactoryClassMap<Probe>{});');kinds(empty,'empty.getKinds()');throws(()=>empty.create('first'),'(void)empty.create("first")');
 class ChildF extends F{};ChildF.defaultNodes={own:D};const child=new ChildF();out.push('tr::FactoryClassMap<Probe> derived_defaults={{"own",d}};tr::AbstractFactory<Probe> child(std::nullopt,&derived_defaults);');kinds(child,'child.getKinds()');expect(child.create('own').kind,'child.create("own")->kind()');
 F.defaultNodes={after:D};const later=new F();out.push('tr::AbstractFactory<Probe>::defaultNodes={{"after",d}};tr::AbstractFactory<Probe> later;');kinds(later,'later.getKinds()');kinds(f,'inherited.getKinds()');F.defaultNodes={};end();
}
{
 begin('map_order_and_class_identity');const f=new F({'10':B,'2':D,alpha:U,'01':B,'4294967295':D});out.push('tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{{"10",b},{"2",d},{"alpha",u},{"01",b},{"4294967295",d}});');kinds(f,'f.getKinds()');
 f.setNodeClass('alpha',B);out.push('f.setNodeClass("alpha",b);');kinds(f,'f.getKinds()');f.deleteNodeClass('2');f.setNodeClass('2',D);out.push('f.deleteNodeClass("2");f.setNodeClass("2",d);');kinds(f,'f.getKinds()');
 f.setNodeClass('alias',B);out.push('f.setNodeClass("alias",b);');expect(f.getNodeClass('alias')===f.getNodeClass('alpha'),'f.getNodeClass("alias")==f.getNodeClass("alpha")');
 const original=f.create('alpha'),derived=f.create('2'),unrelated=new U(f);out.push('auto* original=f.create("alpha");auto* derived=f.create("2");auto unrelated=std::make_unique<UnrelatedProbe>();');
 for(const [node,cpp]of [[original,'original'],[derived,'derived'],[unrelated,'unrelated.get()']])expect(f.nodeIsKind(node,'alpha'),`f.nodeIsKind(${cpp},"alpha")`);
 f.setNodeClass('alpha',D);out.push('f.setNodeClass("alpha",d);');expect(f.nodeIsKind(original,'alpha'),'f.nodeIsKind(original,"alpha")');expect(f.nodeIsKind(derived,'alpha'),'f.nodeIsKind(derived,"alpha")');expect(f.nodeIsKind(original,'alias'),'f.nodeIsKind(original,"alias")');expect(f.create('alpha').kind,'f.create("alpha")->kind()');
 expect(f.nodeIsKind(null,'alias'),'f.nodeIsKind(nullptr,"alias")');throws(()=>f.nodeIsKind(original,'missing'),'(void)f.nodeIsKind(original,"missing")');expect(f.getNodeClass('missing')===undefined,'!f.getNodeClass("missing")');
 f.setNodeClass('invalid',undefined);out.push('f.setNodeClass("invalid",{});');kinds(f,'f.getKinds()');throws(()=>f.create('invalid'),'(void)f.create("invalid")');throws(()=>f.nodeIsKind(original,'invalid'),'(void)f.nodeIsKind(original,"invalid")');end();
}
{
 begin('capture_and_arguments');const f=new F({chosen:B,unknown:U});out.push('tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{{"chosen",b},{"unknown",u}});');const r=f.create('chosen',3,'arg',null);out.push('std::array<std::any,3> args{3.0,std::string("arg"),nullptr};auto* r=f.create("chosen",args);');expect(r.factory===f,'r->factory==&f');expect(r.args.length,'r->args.size()');expect(r.args[0],'std::any_cast<double>(r->args[0])');expect(r.args[1],'std::any_cast<std::string>(r->args[1])');expect(r.args[2]===null,'std::any_cast<std::nullptr_t>(r->args[2])==nullptr');
 f.defaultKind='chosen';out.push('f.defaultKind="chosen";');expect(f.create('absent',4).kind,'f.create("absent",std::array<std::any,1>{4.0})->kind()');f.deleteNodeClass('chosen');out.push('f.deleteNodeClass("chosen");');throws(()=>f.create('absent'),'(void)f.create("absent")');
 const metadata={v:7};B.defaults=metadata;out.push('auto metadata=std::make_shared<int>(7);b->staticData=metadata;');expect(B.defaults===metadata,'std::any_cast<std::shared_ptr<int>>(b->staticData)==metadata');
 const h=new F({chosen:B}),handle={v:7};const hr=h.create('chosen',handle,handle);out.push('tr::AbstractFactory<Probe> h(tr::FactoryClassMap<Probe>{{"chosen",b}});folia::math::PropertyList handle{{"v",7}};auto* hr=h.create("chosen",std::array<std::any,2>{handle,handle});');expect(hr.args[0]===handle,'same_properties(hr->args[0],handle)');expect(hr.args[0]===hr.args[1],'folia::math::Property(std::any_cast<folia::math::PropertyList>(hr->args[0]))==folia::math::Property(std::any_cast<folia::math::PropertyList>(hr->args[1]))');end();
}
{
 begin('prototype_creator_retained');const f=new F({unknown:U});out.push('tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{{"unknown",u}});');throws(()=>f.create('__proto__'),'(void)f.create("__proto__")');f.setNodeClass('__proto__',B);out.push('f.setNodeClass("__proto__",b);');kinds(f,'f.getKinds()');expect(f.create('__proto__').kind,'f.create("__proto__")->kind()');f.deleteNodeClass('__proto__');out.push('f.deleteNodeClass("__proto__");');kinds(f,'f.getKinds()');expect(f.create('__proto__').kind,'f.create("__proto__")->kind()');expect(f.getNodeClass('__proto__')===undefined,'!f.getNodeClass("__proto__")');throws(()=>f.nodeIsKind(f.create('__proto__'),'__proto__'),'(void)f.nodeIsKind(f.create("__proto__"),"__proto__")');end();
}
{
 begin('node_factory_direct_and_defaults');const f=new NF({chosen:B,unknown:U});out.push('tr::AbstractNodeFactory<Probe> f(tr::FactoryClassMap<Probe>{{"chosen",b},{"unknown",u}});');throws(()=>f.create('absent'),'(void)f.create("absent")');
 const one=f.create('chosen'),two=f.create('chosen');out.push('auto* one=f.create("chosen");auto* two=f.create("chosen");');expect(one.args.length,'one->args.size()');expect(one.args[0]!==two.args[0],'folia::math::Property(std::any_cast<folia::math::PropertyList>(one->args[0]))!=folia::math::Property(std::any_cast<folia::math::PropertyList>(two->args[0]))');expect(one.args[1]!==two.args[1],'std::any_cast<tr::FactoryChildren<Probe>>(one->args[1])!=std::any_cast<tr::FactoryChildren<Probe>>(two->args[1])');
 const props={v:7},children=[one,null];const r=f.create('chosen',props,children);out.push('folia::math::PropertyList props{{"v",7}};auto children=std::make_shared<std::vector<Probe*>>(std::initializer_list<Probe*>{one,nullptr});auto* r=f.create("chosen",props,children);');expect(r.args[0]===props,'same_properties(r->args[0],props)');expect(r.args[1]===children,'std::any_cast<tr::FactoryChildren<Probe>>(r->args[1])==children');
 const raw=f.create('chosen',undefined,null,'discarded');out.push('tr::FactoryServices<Probe>& service=f;std::array<std::any,3> raw_args{std::any{},nullptr,std::string("discarded")};auto* raw=service.create("chosen",raw_args);');expect(raw.args.length,'raw->args.size()');expect(raw.args[1]===null,'std::any_cast<std::nullptr_t>(raw->args[1])==nullptr');
 const all=f.create('chosen',null,null);out.push('std::array<std::any,2> null_args{nullptr,nullptr};auto* all=service.create("chosen",null_args);');expect(all.args[0]===null,'std::any_cast<std::nullptr_t>(all->args[0])==nullptr');end();
}
{
 begin('wrapper_factory_forwarding');const f=new WF({chosen:B,unknown:U});out.push('tr::AbstractWrapperFactory<Probe,Probe> f(tr::FactoryClassMap<Probe>{{"chosen",b},{"unknown",u}});');let reads=0;const node={get kind(){reads++;return 'chosen';}};const r=f.wrap(node,7,'extra');out.push('struct View:Probe {mutable int reads=0;std::string kind()const override{++reads;return "chosen";}} node;std::array<std::any,2> args{7.0,std::string("extra")};auto* r=f.wrap(&node,args);');expect(reads,'node.reads');expect(r.factory===f,'r->factory==&f');expect(r.args.length,'r->args.size()');expect(r.args[0]===node,'std::any_cast<Probe*>(r->args[0])==&node');expect(r.args[1],'std::any_cast<double>(r->args[1])');expect(r.args[2],'std::any_cast<std::string>(r->args[2])');
 const missing={kind:'absent'};out.push('Probe missing;missing.report="absent";');expect(f.wrap(missing).kind,'f.wrap(&missing)->kind()');throws(()=>f.wrap(null),'(void)f.wrap(nullptr)');end();
}
{
 begin('independent_live_creator_object');const f=new F({chosen:B,unknown:U});const view=f.node,saved=view.chosen;out.push('Exposed f(tr::FactoryClassMap<Probe>{{"chosen",b},{"unknown",u}});auto view=f.creatorView();auto saved=own(view,"chosen");');
 expect(view===f.node,'view==f.creatorView()');f.nodeMap.set('chosen',D);out.push('f.registerOnly("chosen",d);');expect(f.getNodeClass('chosen')===D,'f.getNodeClass("chosen")==d');expect(f.create('chosen').kind,'f.create("chosen")->kind()');expect(f.nodeIsKind(f.create('chosen'),'chosen'),'f.nodeIsKind(f.create("chosen"),"chosen")');
 f.setNodeClass('chosen',D);out.push('f.setNodeClass("chosen",d);');expect(view===f.node,'view==f.creatorView()');expect(saved().kind,'saved->call({})->kind()');expect(view.chosen().kind,'own(view,"chosen")->call({})->kind()');
 view.alias=saved;out.push('writeOwn(view,"alias",saved);');expect(f.create('alias').kind,'f.create("alias")->kind()');expect(f.getNodeClass('alias')===undefined,'!f.getNodeClass("alias")');throws(()=>f.nodeIsKind(f.create('alias'),'alias'),'(void)f.nodeIsKind(f.create("alias"),"alias")');
 view.chosen=undefined;out.push('writeOwn(view,"chosen",{});');expect(f.create('chosen').kind,'f.create("chosen")->kind()');expect(f.getNodeClass('chosen')===D,'f.getNodeClass("chosen")==d');f.deleteNodeClass('chosen');out.push('f.deleteNodeClass("chosen");');expect(saved().kind,'saved->call({})->kind()');expect(Object.hasOwn(view,'chosen'),'bool(own(view,"chosen"))');end();
}
{
 begin('own_proto_and_falsy_direct');const f=new F({base:B,derived:D,unknown:U});out.push('tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{{"base",b},{"derived",d},{"unknown",u}});auto view=f.creatorView();');
 f.setNodeClass('__proto__',B);out.push('f.setNodeClass("__proto__",b);');const retained=Object.getPrototypeOf(f.node);out.push('auto retained=view->prototypeCreator;');Object.defineProperty(f.node,'__proto__',{value:f.node.derived,writable:true,configurable:true});out.push('writeOwn(view,"__proto__",own(view,"derived"));');expect(f.create('__proto__').kind,'f.create("__proto__")->kind()');
 f.setNodeClass('__proto__',D);out.push('f.setNodeClass("__proto__",d);');expect(Object.getPrototypeOf(f.node)===retained,'view->prototypeCreator==retained');f.deleteNodeClass('__proto__');out.push('f.deleteNodeClass("__proto__");');expect(f.create('__proto__').kind,'f.create("__proto__")->kind()');
 f.node.toString=undefined;out.push('writeOwn(view,"toString",{});');expect(f.create('toString').kind,'f.create("toString")->kind()');
 const direct=new NF({base:B,unknown:U});direct.node.base=undefined;out.push('tr::AbstractNodeFactory<Probe> direct(tr::FactoryClassMap<Probe>{{"base",b},{"unknown",u}});writeOwn(direct.creatorView(),"base",{});');throws(()=>direct.create('base'),'(void)direct.create("base")');direct.setNodeClass('__proto__',B);direct.deleteNodeClass('__proto__');out.push('direct.setNodeClass("__proto__",b);direct.deleteNodeClass("__proto__");');expect(direct.create('__proto__').kind,'direct.create("__proto__")->kind()');
 for(const key of ['call','apply','bind','arguments','caller'])throws(()=>f.create(key),`(void)f.create(${s(key)})`);
 for(const key of ['length','name'])expect(f.create(key).kind,`f.create(${s(key)})->kind()`);
 f.node.call=undefined;out.push('writeOwn(view,"call",{});');expect(f.create('call').kind,'f.create("call")->kind()');
 f.node.bad={};out.push('writeOwn(view,"bad",std::make_shared<tr::FactoryCreator<Probe>>());');throws(()=>f.create('bad'),'(void)f.create("bad")');end();
}
{
 begin('predicate_is_not_construction');class Throwing extends B{constructor(...args){super(...args);throw new TypeError('body');}}const f=new F({chosen:Throwing,unknown:U});const r=Object.create(Throwing.prototype);out.push('auto throwing=klass<DerivedProbe>();throwing->nativeType=typeid(void);throwing->construct=[](tr::FactoryServices<Probe>&,tr::FactoryArgs)->Probe*{throw tr::FactoryError("TypeError",u"body");};tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{{"chosen",throwing},{"unknown",u}});DerivedProbe r;');expect(f.nodeIsKind(r,'chosen'),'f.nodeIsKind(&r,"chosen")');throws(()=>f.create('chosen'),'(void)f.create("chosen")');
 const duplicate={a:B,'10':B,'2':B};duplicate.a=D;out.push('tr::AbstractFactory<Probe> duplicate(tr::FactoryClassMap<Probe>{{"a",b},{"10",b},{"2",b},{"a",d}});');const other=new F(duplicate);kinds(other,'duplicate.getKinds()');expect(other.create('a').kind,'duplicate.create("a")->kind()');end();
}
// Native ownership adaptation: independent of the JS behavioral expectations.
out.push('"math_port_factory_native_session_lifetime"_test=[] {',
'int live=0;struct Live:Probe {int* count;explicit Live(int* c):count(c){++*count;}~Live(){--*count;}};',
'{tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{});auto partial=std::make_shared<tr::FactoryClass<Probe>>();partial->construct=[&live](tr::FactoryServices<Probe>& service,tr::FactoryArgs)->Probe*{service.adopt(std::make_unique<Live>(&live));throw tr::FactoryError("TypeError",u"initialization failed");};f.setNodeClass("partial",partial);bool caught=false;try{(void)f.create("partial");}catch(const tr::FactoryError&){caught=true;}expect(caught);expect(bool(live==1));(void)f.adopt(std::make_unique<Live>(&live));expect(bool(live==2));}',
'expect(bool(live==0));','};');
out.push('};');const text=out.join('\n')+'\n',target=path.join(root,'tests/math/reference_factory.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),text,'Frozen factory fixture differs');else fs.writeFileSync(target,text);
console.log(JSON.stringify({sources,js_cases:cases,js_assertions:assertions,native_lifetime_tests:1,native_lifetime_assertions:4,mode:process.argv.includes('--check')?'check':'generate'}));
