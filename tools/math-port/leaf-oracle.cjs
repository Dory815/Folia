// Root reference generator: actual locked concrete classes, never native output.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),npm=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const lock=require(path.join(root,'docs/math-port/evidence/p9-source-lock.json'));
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),modules={};
for(const f of lock.files){const src=fs.readFileSync(path.join(root,'.reference/MathJax-src',f.source));assert.equal(sha(src),f.sha256);assert.equal(fs.readFileSync(path.join(npm,f.source),'utf8').replaceAll('\r\n','\n'),src.toString().replaceAll('\r\n','\n'));
 const resolved=require.resolve('@mathjax/src/js/'+f.source.slice(3,-3)+'.js',{paths:[path.join(root,'tools/mathjax-bundle')]});assert.equal(fs.realpathSync(resolved),fs.realpathSync(path.join(npm,f.runtime)));assert.equal(sha(fs.readFileSync(resolved)),f.runtime_sha256);modules[path.basename(f.source,'.ts')]=require(resolved);}
const N=require(path.join(npm,'cjs/core/MmlTree/MmlNode.js')),NF=require(path.join(npm,'cjs/core/Tree/NodeFactory.js')).AbstractNodeFactory;
const entries=[];for(const f of lock.files)for(const [name,C]of Object.entries(modules[path.basename(f.source,'.ts')]))entries.push([C.prototype.kind,C,name]);
class Probe extends N.AbstractMmlNode{get kind(){return 'probe';}get isSpacelike(){return !!this.getProperty('space');}get isEmbellished(){return !!this.getProperty('embellished');}setTeXclass(p){if(this.onClass)this.onClass(p);return super.setTeXclass(p);}setInheritedAttributes(a,d,l,p){if(this.onInherit)this.onInherit(a,d,l,p);return super.setInheritedAttributes(a,d,l,p);}}
const classes=()=>Object.fromEntries([...entries.map(([kind,C])=>[kind,C]),['text',N.TextNode],['XML',N.XMLNode],['probe',Probe]]);
const out=['// Generated from actual pinned MathJax classes by leaf-oracle.cjs. Do not hand-edit.','#include "support/folia_test.hpp"','import folia.math.core.mml_tree.class_binding;','import folia.math.core.tree.node_factory;',...lock.files.map(f=>'import folia.math.core.mml_tree.mml_nodes.'+path.basename(f.source,'.ts')+';'),'import folia.math.core.mml_tree.leaf_host;','namespace m=folia::math::mml;namespace tr=folia::math::tree;namespace h=m::host;using folia::math::Property;using folia::math::PropertyList;using folia::math::PropertyNull;using folia::math::Missing;using namespace boost::ut;namespace {',
 'std::u16string units(std::initializer_list<unsigned> v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}',
 'bool prop(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return std::isnan(*y)?std::isnan(*x):std::bit_cast<std::uint64_t>(*x)==std::bit_cast<std::uint64_t>(*y);return a==b;}',
 'm::Children children(std::initializer_list<m::MmlNode*> v={}){auto a=std::make_shared<tr::ChildArray<m::MmlNode>>();for(auto*p:v)a->push_back(m::NodeValue(p));return a;}',
 'm::NodeValue valueAt(m::MmlNode*n,std::size_t i){return n&&n->childNodes&&i<n->childNodes->size()?n->childNodes->get(i):m::NodeValue(Missing{});}',
 'm::MmlNode* at(m::MmlNode*n,std::size_t i){return valueAt(n,i).nodeOrNull();}',
 'bool sameNode(m::NodeValue value,m::MmlNode*node){return value.isNode()&&value.nodeOrNull()==node;}',
 'std::size_t count(m::MmlNode*n){return n&&n->childNodes?n->childNodes->size():0;}',
 "struct Probe:m::AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::function<void(m::NodeValue)> onClass;std::function<void(m::AttributeList,Property,Property,Property)> onInherit;std::string kind()const override{return \"probe\";}bool isSpacelike()const override{return h::truthy(getProperty(\"space\"));}bool isEmbellished()const override{return h::truthy(getProperty(\"embellished\"));}m::NodeValue setTeXclass(m::NodeValue p)override{if(onClass)onClass(p);return AbstractMmlNode::setTeXclass(p);}void inheritAttributes(m::AttributeList a,Property d,Property l,Property p)override{if(onInherit)onInherit(a,d,l,p);AbstractMmlNode::inheritAttributes(a,d,l,p);}};",
 'using F=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode> classes(){return {'+entries.map(([k,C,name])=>'{"'+k+'",m::classBinding<m::'+name+'>()}').concat(['{"text",m::classBinding<m::TextNode>()}','{"XML",m::classBinding<m::XMLNode>()}','{"probe",m::classBinding<Probe>()}']).join(',')+'};}',
 'm::MmlNode* create(F&f,std::string_view kind,PropertyList a={},m::Children c=children()){return f.create(kind,std::array<std::any,2>{a,c});}',
 'm::TextNode* text(F&f,std::u16string value){auto*n=dynamic_cast<m::TextNode*>(f.create("text"));n->setText(std::move(value));return n;}',
 'template<class Make>std::string bits(Make make){constexpr char hex[]="0123456789abcdef";std::string out;for(unsigned start=0;start<65536;start+=4){unsigned b=0;for(unsigned j=0;j<4;++j)if(make(static_cast<char16_t>(start+j)))b|=1u<<j;out.push_back(hex[b]);}return out;}',
 '}','suite math_port_leaf_tests=[] {'];
let assertions=0,cases=0;const emit=s=>out.push(s),q=JSON.stringify,u=s=>'units({'+Array.from({length:s.length},(_,i)=>s.charCodeAt(i)).join(',')+'})';
const num=n=>Number.isNaN(n)?'std::numeric_limits<double>::quiet_NaN()':n===Infinity?'std::numeric_limits<double>::infinity()':Object.is(n,-0)?'-0.0':String(n)+(Number.isInteger(n)?'.0':'');
function p(v){if(v===undefined)return 'Property(Missing{})';if(v===null)return 'Property(PropertyNull{})';if(typeof v==='boolean')return 'Property('+v+')';if(typeof v==='number')return 'Property('+num(v)+')';if(typeof v==='string')return 'Property(h::debugText('+u(v)+'))';throw Error('Unbound expected scalar');}
function literal(s){const parts=[];for(let i=0;i<s.length;i+=2048)parts.push(q(s.slice(i,i+2048)));return parts.length?parts.join('\n'):'""';}
function eq(v,e){assertions++;emit('expect(bool(('+e+')=='+(typeof v==='string'?literal(v):typeof v==='number'?num(v):String(v))+'));');}
function pp(v,e){assertions++;emit('expect(prop('+e+','+p(v)+'));');}
function list(v,e){assertions++;emit('expect(bool('+e+'==std::vector<std::string>{'+v.map(q).join(',')+'}));');}
function throws(fn,e){let error;try{fn();}catch(x){error=x;}assert.ok(error,'Reference must throw');assertions+=2;emit('{bool caught=false;try{'+e+';}catch(const tr::FactoryError&error){caught=true;expect(bool(error.name=='+q(error.name)+'));}expect(caught);}');}
function begin(name){cases++;emit(q('math_port_leaf_'+name)+'_test=[] {F f(classes());');return new NF(classes());}function end(){emit('};');}
function text(f,s){return f.create('text').setText(s);}function input(f,k,s,a={}){return f.create(k,a,[text(f,s)]);}
{
 const f=begin('defaults_traits_and_constructor_fields');
 for(const [k,C,name]of entries){const n=f.create(k);emit('{auto*n=f.create('+q(k)+');');eq(n.kind,'n->kind()');eq(n.isToken,'n->isToken()');eq(n.isInferred,'n->isInferred()');eq(!!n.notParent,'h::truthy(n->notParent())');eq(n.arity,'n->arity()');eq(n.linebreakContainer,'n->linebreakContainer()');eq(n.linebreakAlign,'n->linebreakAlign()');eq(n.isEmpty,'n->isEmpty()');eq(n.isSpacelike,'n->isSpacelike()');pp(n.texClass,'n->texClass()');list(Object.keys(C.defaults),'m::'+name+'::defaults().keys()');for(const key of Object.keys(C.defaults))pp(C.defaults[key],'m::'+name+'::defaults().getOwn('+q(key)+')');eq(n.childNodes.length,'count(n)');if(n.arity<0)eq(n.childNodes[0].kind,'at(n,0)->kind()');emit('}');}end();
}
{
 begin('utf16_pattern_full_unit_domain');
 function bitString(fn){let out='';for(let i=0;i<65536;i+=4){let b=0;for(let j=0;j<4;j++)if(fn(String.fromCharCode(i+j)))b|=1<<j;out+=b.toString(16);}return out;}
 const patterns=[['single',c=>modules.mi.MmlMi.singleCharacter.test(c),'m::MmlMi::singleCharacter()(std::u16string(1,c))'],['combining',c=>modules.mi.MmlMi.singleCharacter.test('a'+c),'m::MmlMi::singleCharacter()(std::u16string(u"a")+c)'],['high_prefix',c=>modules.mi.MmlMi.singleCharacter.test('\ud800'+c),'m::MmlMi::singleCharacter()(units({55296})+c)'],['operator',c=>modules.mi.MmlMi.operatorName.test(c),'m::MmlMi::operatorName()(std::u16string(1,c))'],['space',c=>/^\s*$/.test(c),'h::onlySpaces(std::u16string(1,c))']];
 for(const [name,fn,cpp]of patterns)eq(bitString(fn),'bits([](char16_t c){return '+cpp+';})');end();
}
{
 const f=begin('row_core_null_persistence_and_queries'),row=f.create('mrow'),a=f.create('probe'),b=f.create('probe'),space=f.create('mspace');a.setProperty('embellished',true);b.setProperty('embellished',true);row.setChildren([space,a]);row.childNodes.unshift(null);
 emit('auto*row=dynamic_cast<m::MmlMrow*>(f.create("mrow"));auto*a=f.create("probe");auto*b=f.create("probe");auto*space=f.create("mspace");a->setProperty("embellished",true);b->setProperty("embellished",true);row->setChildren(children({space,a}));std::array<m::NodeValue,1> added{m::NodeValue(PropertyNull{})};row->childNodes->splice(0,0,added);');
 // Source setChildren null throws after push; bind the intended host state directly.
 eq(row.isEmbellished,'row->isEmbellished()');eq(row.core()===a,'sameNode(row->core(),a)');eq(row.coreMO()===a,'sameNode(row->coreMO(),a)');eq(row.nonSpaceLength(),'row->nonSpaceLength()');eq(row.firstNonSpace()===a,'row->firstNonSpace()==a');eq(row.lastNonSpace()===a,'row->lastNonSpace()==a');throws(()=>row.isSpacelike,'(void)row->isSpacelike()');row.childNodes.push(b);emit('row->childNodes->push_back(m::NodeValue(b));');eq(row.isEmbellished,'row->isEmbellished()');eq(row.core()===row,'sameNode(row->core(),row)');row.childNodes=[space];emit('row->childNodes=children({space});');eq(row.isEmbellished,'row->isEmbellished()');eq(row.nonSpaceLength(),'row->nonSpaceLength()');eq(row.firstNonSpace()===null,'row->firstNonSpace()==nullptr');eq(row.isSpacelike,'row->isSpacelike()');end();
}
{
 const f=begin('row_delimiter_nullish_and_class_transfer');for(const value of [undefined,null,false,'',0,'(']){const r=f.create('mrow'),a=f.create('probe'),prev=f.create('probe');a.texClass=2;prev.texClass=3;r.setChildren([a]);r.setProperty('open',value);const result=r.setTeXclass(prev);
 emit('{auto*r=f.create("mrow");auto*a=f.create("probe");auto*prev=f.create("probe");a->setTexClass(2);prev->setTexClass(3);r->setChildren(children({a}));r->setProperty("open",'+p(value)+');auto result=r->setTeXclass(m::NodeValue(prev));');eq(result===r,'sameNode(result,r)');pp(r.texClass,'r->texClass()');pp(r.prevClass,'r->prevClass()');pp(a.prevClass,'a->prevClass()');emit('}');}end();
}
{
 const f=begin('inferred_join_capture_cycle_and_utf16'),r=f.create('inferredMrow'),x=text(f,'a\ud800');r.childNodes=[x,null,r];emit('auto*r=f.create("inferredMrow");auto*x=text(f,units({97,55296}));r->childNodes=children({x,nullptr,r});');assertions++;emit('expect(bool(h::text(r->toString())=='+u(r.toString())+'));');end();
}
{
 const f=begin('mi_italic_and_auto_operator');for(const word of ['x','sin','abc2','2x','a-b','a\n','😀','a\u0300','a\u1abf','\ud800\ud800']){const n=input(f,'mi',word);n.setInheritedAttributes();const result=n.setTeXclass(null);
 emit('{auto*n=create(f,"mi",{},children({text(f,'+u(word)+')}));n->setInheritedAttributes();auto result=n->setTeXclass(m::NodeValue(PropertyNull{}));');pp(n.attributes.get('mathvariant'),'n->attributes()->get("mathvariant")');pp(n.attributes.getInherited('mathvariant'),'n->attributes()->getInherited("mathvariant")');pp(n.getProperty('autoOP'),'n->getProperty("autoOP")');pp(n.texClass,'n->texClass()');eq(result===n,'sameNode(result,n)');emit('}');}
 const n=input(f,'mi','x',{mathvariant:'bold'});n.setInheritedAttributes();emit('auto*n=create(f,"mi",PropertyList{{"mathvariant","bold"}},children({text(f,u"x")}));n->setInheritedAttributes();');pp(n.attributes.get('mathvariant'),'n->attributes()->get("mathvariant")');pp(n.attributes.getInherited('mathvariant'),'n->attributes()->getInherited("mathvariant")');end();
}
{
 const f=begin('space_explicit_presence_width_and_newlines');for(const value of ['', '-1em',' \ufeff-0', '-0',0,-0,-1,undefined,null,false,'\u0085-1','\u2028-1']){const n=f.create('mspace',{width:value});emit('{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",'+p(value)+'}}));');eq(n.canBreak,'n->canBreak()');eq(n.isSpacelike,'n->isSpacelike()');eq(n.hasNewline,'n->hasNewline()');emit('}');}for(const attr of ['height','depth','style','mathbackground','background','linebreak'])for(const value of [null,false,0,'']){const n=f.create('mspace',{[attr]:value});emit('{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{'+q(attr)+','+p(value)+'}}));');eq(n.canBreak,'n->canBreak()');eq(n.isSpacelike,'n->isSpacelike()');emit('}');}for(const value of ['newline','indentingnewline','goodbreak',true,null]){const n=f.create('mspace',{linebreak:value});emit('{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",'+p(value)+'}}));');eq(n.hasNewline,'n->hasNewline()');emit('}');}end();
}
{
 const f=begin('mtext_spaces_and_presence');for(const word of ['', ' ','\ufeff\u2028','\u0085','x','😀']){const n=input(f,'mtext',word);emit('{auto*n=create(f,"mtext",{},children({text(f,'+u(word)+')}));');eq(n.isSpacelike,'n->isSpacelike()');emit('}');}for(const attr of modules.mtext.MmlMtext.NONSPACELIKE){const n=input(f,'mtext',' ',{[attr]:false});emit('{auto*n=create(f,"mtext",PropertyList{{'+q(attr)+',false}},children({text(f,u" ")}));');eq(n.isSpacelike,'n->isSpacelike()');emit('}');}end();
}
{
 const f=begin('mstyle_raw_relative_levels_and_prime');for(const incoming of [2,'2',null,undefined])for(const value of ['+2',' -0x2','3x','0x10',false,null]){const spy=f.create('probe'),n=f.create('mstyle',{scriptlevel:value},[spy]);let observed;spy.onInherit=(a,d,l,p)=>observed=[d,l,p];n.setInheritedAttributes({},true,incoming,true);
 emit('{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",'+p(value)+"}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes(PropertyList{},true,"+p(incoming)+',true);');pp(observed[0],'observed[0]');pp(observed[1],'observed[1]');pp(observed[2],'observed[2]');eq(!!n.notParent,'n->notParent()');emit('}');}end();
}
{
 const f=begin('mstyle_display_and_cramped');for(const value of [true,false,'true',1,null])for(const cramped of [undefined,false,'yes']){const spy=f.create('probe'),a={displaystyle:value};if(cramped!==undefined)a['data-cramped']=cramped;const n=f.create('mstyle',a,[spy]);let observed;spy.onInherit=(a,d,l,p)=>observed=[d,l,p];n.setInheritedAttributes({},false,4,true);
 emit('{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",'+p(value)+'}};'+(cramped===undefined?'':'a.set("data-cramped",'+p(cramped)+');')+"auto*n=create(f,\"mstyle\",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes(PropertyList{},false,4,true);");pp(observed[0],'observed[0]');pp(observed[1],'observed[1]');pp(observed[2],'observed[2]');emit('}');}end();
}
{
 const f=begin('math_display_mode_and_explicit_precedence');for(const displaystyle of [undefined,false,true,'yes',0])for(const display of ['inline','block']){const spy=f.create('probe'),a={display};if(displaystyle!==undefined)a.displaystyle=displaystyle;const n=f.create('math',a,[spy]);let observed;spy.onInherit=(a,d,l,p)=>observed=[d,l,p];n.setInheritedAttributes({},false,2,'prime');
 emit('{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",'+p(display)+'}};'+(displaystyle===undefined?'':'a.set("displaystyle",'+p(displaystyle)+');')+"auto*n=create(f,\"math\",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes(PropertyList{},false,2,\"prime\");");pp(observed[0],'observed[0]');pp(observed[1],'observed[1]');pp(observed[2],'observed[2]');pp(n.attributes.getInherited('displaystyle'),'n->attributes()->getInherited("displaystyle")');emit('}');}end();
}
{
 const f=begin('math_constructor_defaults_survive_registry_rebind'),C=modules.math.MmlMath,n=f.create('math'),spy=f.create('probe');n.childNodes[0].appendChild(spy);const old=C.defaults.scriptlevel;C.defaults.scriptlevel=7;class Other extends C{static defaults={...C.defaults,scriptlevel:11};}f.setNodeClass('math',Other);let level;spy.onInherit=(a,d,l)=>level=l;n.setInheritedAttributes();
 emit('auto*n=f.create("math");auto*spy=dynamic_cast<Probe*>(f.create("probe"));at(n,0)->appendChild(spy);auto old=m::MmlMath::defaults().get("scriptlevel");m::MmlMath::defaults().set("scriptlevel",7);struct Other:m::MmlMath{using MmlMath::MmlMath;static PropertyList& defaults(){static PropertyList d=[] {PropertyList p;p.assign(m::MmlMath::defaults());p.set("scriptlevel",11);return p;}();return d;}};f.setNodeClass("math",m::classBinding<Other>());Property level;spy->onInherit=[&](m::AttributeList,Property,Property l,Property){level=l;};n->setInheritedAttributes();');pp(level,'level');C.defaults.scriptlevel=old;emit('m::MmlMath::defaults().set("scriptlevel",old);');end();
}
{
 const f=begin('math_nested_null_options_failure_after_allocation'),parent=f.create('probe'),n=f.create('math');parent.appendChild(n);let error;class Recorded extends modules.merror.MmlMerror{constructor(...args){super(...args);error=this;}}f.setNodeClass('merror',Recorded);
 emit('auto*parent=f.create("probe");auto*n=f.create("math");parent->appendChild(m::NodeValue(n));m::MmlNode*error=nullptr;auto c=m::classBinding<m::MmlMerror>();auto original=c->construct;c->construct=[&](m::Factory&f,tr::FactoryArgs a){error=original(f,a);return error;};f.setNodeClass("merror",c);');throws(()=>n.verifyTree(), 'n->verifyTree()');eq(parent.childNodes[0]===n,'sameNode(at(parent,0),n)');eq(error!==undefined,'error!=nullptr');emit('if(error){');pp(error.attributes.getExplicit('data-mjx-message'),'error->attributes()->getExplicit("data-mjx-message")');eq(error.childNodes.length,'count(error)');eq(error.childNodes[0].childNodes.length,'count(at(error,0))');emit('}');end();
}
{
 const f=begin('source_field_phase_after_virtual_super_hooks'),trace=[];let sets=0;
 class Trace extends modules.mi.MmlMi{setChildren(c){trace.push(this.texClass);return super.setChildren(c);}set texClass(v){sets++;super.texClass=v;}get texClass(){return super.texClass;}}
 f.setNodeClass('mi',Trace);const n=f.create('mi');
 emit('struct Trace:m::MmlMi{using MmlMi::MmlMi;static std::vector<Property>& trace(){static std::vector<Property> v;return v;}static int& sets(){static int v=0;return v;}void setChildren(m::Children c)override{trace().push_back(texClass());MmlMi::setChildren(c);}void setTexClass(Property v)override{++sets();MmlMi::setTexClass(v);}};f.setNodeClass("mi",m::classBinding<Trace>());auto*n=f.create("mi");');eq(trace.length,'Trace::trace().size()');trace.forEach((v,i)=>pp(v,'Trace::trace()['+i+']'));eq(sets,'Trace::sets()');pp(n.texClass,'n->texClass()');end();
}
{
 const f=begin('row_last_search_reads_replaced_array');
 class SpaceProbe extends Probe{get isSpacelike(){if(this.onSpace)this.onSpace();return super.isSpacelike;}}
 f.setNodeClass('probe',SpaceProbe);const r=f.create('mrow'),a=f.create('probe'),b=f.create('probe'),replacement=f.create('probe');a.setProperty('space',true);b.setProperty('space',true);r.childNodes=[a,b];b.onSpace=()=>{r.childNodes=[replacement];};
 emit('struct SpaceProbe:Probe{using Probe::Probe;std::function<void()> onSpace;bool isSpacelike()const override{if(onSpace)onSpace();return Probe::isSpacelike();}};f.setNodeClass("probe",m::classBinding<SpaceProbe>());auto*r=dynamic_cast<m::MmlMrow*>(f.create("mrow"));auto*a=f.create("probe");auto*b=dynamic_cast<SpaceProbe*>(f.create("probe"));auto*replacement=f.create("probe");a->setProperty("space",true);b->setProperty("space",true);r->childNodes=children({a,b});b->onSpace=[&]{r->childNodes=children({replacement});};');eq(r.lastNonSpace()===replacement,'r->lastNonSpace()==replacement');end();
}
{
 const f=begin('row_core_uses_current_array_after_predicate');
 class Flicker extends Probe{get isEmbellished(){if(this.callback)this.callback();return super.isEmbellished;}}
 f.setNodeClass('probe',Flicker);const r=f.create('mrow'),a=f.create('probe'),replacement=f.create('probe');a.setProperty('embellished',true);r.childNodes=[a];a.callback=()=>{r.childNodes=[replacement];};
 emit('struct Flicker:Probe{using Probe::Probe;std::function<void()> callback;bool isEmbellished()const override{if(callback)callback();return Probe::isEmbellished();}};f.setNodeClass("probe",m::classBinding<Flicker>());auto*r=f.create("mrow");auto*a=dynamic_cast<Flicker*>(f.create("probe"));auto*replacement=f.create("probe");a->setProperty("embellished",true);r->childNodes=children({a});a->callback=[&]{r->childNodes=children({replacement});};');eq(r.core()===replacement,'sameNode(r->core(),replacement)');r.childNodes=[a];a.callback=()=>{r.childNodes=[null];};emit('r->childNodes=children({a});a->callback=[&]{r->childNodes=children({nullptr});};');throws(()=>r.coreMO(),'(void)r->coreMO()');end();
}
{
 const f=begin('row_live_class_iteration_and_current_first_child'),r=f.create('mrow'),a=f.create('probe'),late=f.create('probe'),replacement=f.create('probe');a.texClass=2;late.texClass=3;replacement.texClass=1;const original=[a];r.childNodes=original;a.onClass=()=>{original.push(late);r.childNodes=[replacement];};const result=r.setTeXclass(null);
 emit('auto*r=f.create("mrow");auto*a=dynamic_cast<Probe*>(f.create("probe"));auto*late=f.create("probe");auto*replacement=f.create("probe");a->setTexClass(2);late->setTexClass(3);replacement->setTexClass(1);auto original=children({a});r->childNodes=original;a->onClass=[&](m::NodeValue){original->push_back(m::NodeValue(late));r->childNodes=children({replacement});};auto result=r->setTeXclass(m::NodeValue(PropertyNull{}));');eq(result===late,'sameNode(result,late)');pp(r.texClass,'r->texClass()');eq(r.childNodes[0]===replacement,'at(r,0)==replacement');end();
}
{
 const f=begin('mi_tuple_and_property_presence');for(const key of ['autoOP','texClass'])for(const value of [null,false,0,'']){const n=input(f,'mi','sin');n.setProperty(key,value);n.setTeXclass(null);emit('{auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setProperty('+q(key)+','+p(value)+');n->setTeXclass(m::NodeValue(PropertyNull{}));');pp(n.texClass,'n->texClass()');pp(n.getProperty('autoOP'),'n->getProperty("autoOP")');emit('}');}
 const n=input(f,'mi','x');n.setInheritedAttributes({mathvariant:['mstyle','']});emit('auto*n=create(f,"mi",{},children({text(f,u"x")}));n->setInheritedAttributes(PropertyList{{"mathvariant",m::attributeTuple("mstyle","")}});');pp(n.attributes.get('mathvariant'),'n->attributes()->get("mathvariant")');end();
}
{
 const f=begin('math_mode_display_is_inherited_not_explicit');for(const explicit of [false,true]){const a={mode:'display'};if(explicit)a.display='inline';const spy=f.create('probe'),n=f.create('math',a,[spy]);let level;spy.onInherit=(a,d)=>level=d;n.setInheritedAttributes();emit('{auto*spy=dynamic_cast<Probe*>(f.create("probe"));PropertyList a{{"mode","display"}};'+(explicit?'a.set("display","inline");':'')+'auto*n=create(f,"math",a,children({spy}));Property observed;spy->onInherit=[&](m::AttributeList,Property d,Property,Property){observed=d;};n->setInheritedAttributes();');pp(level,'observed');pp(n.attributes.getInherited('display'),'n->attributes()->getInherited("display")');pp(n.attributes.get('display'),'n->attributes()->get("display")');emit('}');}end();
}
{
 const f=begin('mi_operator_assignment_uses_virtual_setter');let sets=0;
 class Observe extends modules.mi.MmlMi{get texClass(){return super.texClass;}set texClass(v){sets++;super.texClass=v;}}
 f.setNodeClass('mi',Observe);const n=input(f,'mi','sin');n.setTeXclass(null);
 emit('struct Observe:m::MmlMi{using MmlMi::MmlMi;static int& sets(){static int n=0;return n;}void setTexClass(Property v)override{++sets();MmlMi::setTexClass(v);}};f.setNodeClass("mi",m::classBinding<Observe>());auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setTeXclass(m::NodeValue(PropertyNull{}));');eq(sets,'Observe::sets()');pp(n.texClass,'n->texClass()');end();
}
{
 const f=begin('math_each_attributes_access_is_virtual');
 const A=require(path.join(npm,'cjs/core/MmlTree/Attributes.js')).Attributes;
 class Sequence extends modules.math.MmlMath{get attributes(){return this.sequence?this.sequence[this.cursor++]:this.saved;}set attributes(v){this.saved=v;}run(){this.setChildInheritedAttributes({},false,0,false);}}
 f.setNodeClass('math',Sequence);const n=f.create('math');n.sequence=Array.from({length:7},()=>new A(modules.math.MmlMath.defaults,modules.math.MmlMath.defaults));n.sequence[4].set('display','block');n.cursor=0;n.run();
 emit('struct Sequence:m::MmlMath{using MmlMath::MmlMath;std::vector<std::shared_ptr<folia::math::Attributes>> sequence;mutable std::size_t cursor=0;std::shared_ptr<folia::math::Attributes> attributes()const override{if(sequence.empty())return MmlMath::attributes();if(cursor>=sequence.size())throw tr::FactoryError("TypeError",u"sequence exhausted");return sequence[cursor++];}void run(){setChildInheritedAttributes({},false,0,false);}};f.setNodeClass("math",m::classBinding<Sequence>());auto*n=dynamic_cast<Sequence*>(f.create("math"));for(int i=0;i<7;++i)n->sequence.push_back(std::make_shared<folia::math::Attributes>(m::MmlMath::defaults(),m::MmlMath::defaults()));n->sequence[4]->set("display","block");n->cursor=0;n->run();');eq(n.cursor,'n->cursor');pp(n.sequence[5].getInherited('displaystyle'),'n->sequence[5]->getInherited("displaystyle")');end();
}
{
 const f=begin('mstyle_defaults_precede_first_write_and_partial_throw');
 class Partial extends modules.mstyle.MmlMstyle{get attributes(){if(this.armed&&++this.calls===2)throw new TypeError('second read');return this.saved;}set attributes(v){this.saved=v;}}
 f.setNodeClass('mstyle',Partial);const n=f.create('mstyle'),saved=n.attributes;n.armed=true;n.calls=0;
 emit('struct Partial:m::MmlMstyle{using MmlMstyle::MmlMstyle;bool armed=false;mutable int calls=0;std::shared_ptr<folia::math::Attributes> attributes()const override{if(armed&&++calls==2)throw tr::FactoryError("TypeError",u"second read");return MmlMstyle::attributes();}};f.setNodeClass("mstyle",m::classBinding<Partial>());auto*n=dynamic_cast<Partial*>(f.create("mstyle"));auto saved=n->attributes();n->armed=true;n->calls=0;');throws(()=>n.setInheritedAttributes({},undefined,undefined,undefined),"n->setInheritedAttributes(PropertyList{},Missing{},Missing{},Missing{})");pp(saved.getInherited('displaystyle'),'saved->getInherited("displaystyle")');eq(n.calls,'n->calls');end();
}
{
 const f=begin('inferred_join_snapshots_array_and_length');
 class Trigger extends N.TextNode{toString(){if(this.callback)this.callback();return super.toString();}}
 f.setNodeClass('text',Trigger);const r=f.create('inferredMrow'),x=text(f,'a'),y=text(f,'b'),late=text(f,'c');const original=[x,y];r.childNodes=original;x.callback=()=>{original.push(late);r.childNodes=[];};
 emit('struct Trigger:m::TextNode{using TextNode::TextNode;std::function<void()> callback;std::string toString()const override{if(callback)callback();return TextNode::toString();}};f.setNodeClass("text",m::classBinding<Trigger>());auto*r=f.create("inferredMrow");auto*x=dynamic_cast<Trigger*>(text(f,u"a"));auto*y=text(f,u"b");auto*late=text(f,u"c");auto original=children({x,y});r->childNodes=original;x->callback=[&]{original->push_back(late);r->childNodes=children();};');eq(r.toString(),'r->toString()');eq(r.childNodes.length,'count(r)');end();
}
{
 const f=begin('mstyle_dispatches_only_literal_first_child'),n=f.create('mstyle'),a=f.create('probe'),b=f.create('probe');n.childNodes=[a,b];let calls=[0,0];a.onInherit=()=>calls[0]++;b.onInherit=()=>calls[1]++;n.setInheritedAttributes();
 emit('auto*n=f.create("mstyle");auto*a=dynamic_cast<Probe*>(f.create("probe"));auto*b=dynamic_cast<Probe*>(f.create("probe"));n->childNodes=children({a,b});std::array<int,2> calls{};a->onInherit=[&](m::AttributeList,Property,Property,Property){++calls[0];};b->onInherit=[&](m::AttributeList,Property,Property,Property){++calls[1];};n->setInheritedAttributes();');calls.forEach((v,i)=>eq(v,'calls['+i+']'));end();
}
{
 const f=begin('newline_is_suppressed_by_break_constraints');for(const a of [{linebreak:'newline',height:0},{linebreak:'indentingnewline',width:'-1em'}]){const n=f.create('mspace',a);emit('{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{'+Object.entries(a).map(([k,v])=>'{'+q(k)+','+p(v)+'}').join(',')+'}));');eq(n.hasNewline,'n->hasNewline()');emit('}');}end();
}
{
 const f=begin('static_bindings_copy_alias_and_live_callbacks'),R=modules.mrow.MmlMrow,I=modules.mrow.MmlInferredMrow,Mi=modules.mi.MmlMi;
 eq(R.defaults===I.defaults,'Property(m::MmlMrow::defaults())==Property(m::MmlInferredMrow::defaults())');R.defaults['p9-own']=1;emit('m::MmlMrow::defaults().set("p9-own",1);');pp(I.defaults['p9-own'],'m::MmlInferredMrow::defaults().get("p9-own")');pp(Mi.defaults['p9-own'],'m::MmlMi::defaults().get("p9-own")');delete R.defaults['p9-own'];emit('m::MmlMrow::defaults().erase("p9-own");');
 const old=Mi.singleCharacter;Mi.singleCharacter=/^xyz$/;const n=input(f,'mi','xyz');n.setInheritedAttributes();emit('auto old=m::MmlMi::singleCharacter();m::MmlMi::singleCharacter()=[](std::u16string_view s){return s==u"xyz";};auto*n=create(f,"mi",{},children({text(f,u"xyz")}));n->setInheritedAttributes();');pp(n.attributes.get('mathvariant'),'n->attributes()->get("mathvariant")');Mi.singleCharacter=old;emit('m::MmlMi::singleCharacter()=old;');
 modules.mspace.MmlMspace.NONSPACELIKE.push('width');const space=f.create('mspace',{width:'1em'});emit('m::MmlMspace::NONSPACELIKE().push_back("width");auto*space=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width","1em"}}));');eq(space.canBreak,'space->canBreak()');modules.mspace.MmlMspace.NONSPACELIKE.pop();emit('m::MmlMspace::NONSPACELIKE().pop_back();');end();
}
// Native resource capacity is frozen literally; these do not probe VM stack size.
cases++;assertions+=6;emit('"math_port_leaf_native_recursive_budget_recovery"_test=[] {F f(classes());auto*root=f.create("mrow");auto*tail=root;for(int i=1;i<512;++i){auto*next=f.create("mrow");tail->childNodes=children({next});tail=next;}expect(root->isSpacelike());tail->childNodes=children({f.create("mrow")});bool caught=false;try{(void)root->isSpacelike();}catch(const tr::FactoryError&e){caught=e.name=="RangeError";}expect(caught);expect(f.create("mrow")->isSpacelike());root=f.create("inferredMrow");tail=root;for(int i=1;i<512;++i){auto*next=f.create("inferredMrow");tail->childNodes=children({next});tail=next;}expect(bool(root->toString().size()==1024));tail->childNodes=children({f.create("inferredMrow")});caught=false;try{(void)root->toString();}catch(const tr::FactoryError&e){caught=e.name=="RangeError";}expect(caught);expect(bool(f.create("inferredMrow")->toString()=="[]"));};');
out.push('};');const target=path.join(root,'tests/math/reference_leaf.cpp'),output=out.join('\n')+'\n';
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output,'Frozen leaf fixture changed');else fs.writeFileSync(target,output);
console.log(JSON.stringify({sources:lock.files.length,assertions,cases,mode:process.argv.includes('--check')?'check':'generate'}));

