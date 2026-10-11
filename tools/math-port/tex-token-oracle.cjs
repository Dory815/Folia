// Root executes every actual Token/Macro member; expected values never use C++.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,u,Token,Macro,ConfigurationType,HandlerType,TexConstant}=require('./tex-token-reference.cjs');
const {valueEmitter}=require('./oracle-values.cjs'),values=valueEmitter(u),{p}=values;
let checks=0;const cases=[];
const check=(expr,v)=>{++checks;return 'expect('+expr+' == '+(v?'true':'false')+');';};
const eq=(expr,v)=>{++checks;return 'expect(deep('+expr+','+p(v)+'));';};
const raw=[undefined,null,false,true,0,-0,NaN,Infinity,-Infinity,'','\\mathbb','ℝ𝔸中文','\ud800x\udfff','a\u0000b',[],[null,false,1,'x'],{}, {key:'v',nested:[1,2]}];
for(let i=0;i<raw.length;++i){const a=raw[i],t=new Token(a,raw[(i+1)%raw.length],raw[(i+2)%raw.length]);
 cases.push(`"math_port_tex_token_raw_${i}"_test=[] { t::Token n(${p(a)},${p(raw[(i+1)%raw.length])},${p(raw[(i+2)%raw.length])}); ${eq('n.token()',t.token)} ${eq('n.character()',t.char)} ${eq('n.attributes()',t.attributes)} };`);
 const m=new Macro(a,raw[(i+1)%raw.length],a);
 cases.push(`"math_port_tex_macro_raw_${i}"_test=[] { t::Macro n(${p(a)},t::ParseMethod{${p(raw[(i+1)%raw.length])}},${p(a)}); ${eq('n.token()',m.token)} ${eq('std::get<Property>(n.func())',m.func)} ${eq('n.args()',m.args)} };`);
}
const attrs={nested:[1]},token={name:'input'},chr=['translation'],n=new Token(token,chr,attrs);
const flags=[n.token===token,n.char===chr,n.attributes===attrs,n.attributes===n.attributes];attrs.changed=true;chr.push('later');token.name='mutated';
cases.push(`"math_port_tex_token_identity"_test=[] {PropertyList tok{{"name","input"}},a{{"nested",PropertyArray{1}}};PropertyArray ch{"translation"};t::Token n(tok,ch,a);${check('n.token()==Property(tok)',flags[0])}${check('n.character()==Property(ch)',flags[1])}${check('n.attributes()==Property(a)',flags[2])}${check('n.attributes()==n.attributes()',flags[3])}a.setOwn("changed",true);ch.push("later");tok.setOwn("name","mutated");${eq('n.token()',n.token)}${eq('n.character()',n.char)}${eq('n.attributes()',n.attributes)} };`);
const args=[false,1,'x',null],fn=()=>{},m=new Macro(token,fn,args),other=new Macro('x',()=>{},args);
const identities=[m.func===fn,m.func!==other.func,m.args===args,m.args===m.args];args.push('mutated');
cases.push(`"math_port_tex_macro_identity"_test=[] {PropertyList tok{{"name","mutated"}};PropertyArray a{false,1,"x",PropertyNull{}};auto fn=std::make_shared<t::ParseCallable>();auto other=std::make_shared<t::ParseCallable>();t::Macro n(tok,t::ParseMethod{fn},a),m("x",t::ParseMethod{other},a);${check('std::get<t::ParseCallableRef>(n.func())==fn',identities[0])}${check('std::get<t::ParseCallableRef>(n.func())!=std::get<t::ParseCallableRef>(m.func())',identities[1])}${check('n.args()==Property(a)',identities[2])}${check('n.args()==n.args()',identities[3])}a.push("mutated");${eq('n.args()',m.args)} };`);
const d1=new Macro('x',fn),d2=new Macro('x',fn,undefined);const defaultFlags=[d1.args===d1.args,d1.args!==d2.args];d1.args.push('one');
cases.push(`"math_port_tex_macro_default_fresh"_test=[] {auto fn=std::make_shared<t::ParseCallable>();t::Macro a("x",t::ParseMethod{fn}),b("x",t::ParseMethod{fn},Missing{});${check('a.args()==a.args()',defaultFlags[0])}${check('a.args()!=b.args()',defaultFlags[1])}auto ar=PropertyArray::array(a.args()).value();ar.push("one");${eq('a.args()',d1.args)}${eq('b.args()',d2.args)} };`);
cases.push(`"math_port_tex_token_omitted_attributes"_test=[] {t::Token n("x","y");${eq('n.attributes()',new Token('x','y').attributes)} };`);
for(const [name,table] of Object.entries({ConfigurationType,HandlerType,TexConstant})){
cases.push(`"math_port_tex_table_${name}"_test=[] {${eq('Property(t::'+name+')',table)} };`);
}
// Native callable seam witnesses use real Token/Macro storage and all typed alternatives.
cases.push(`"math_port_tex_callable_protocol"_test=[] {
struct Parser:t::ParserContext {Property cs="";Property currentCS()const override{return cs;}void setCurrentCS(Property v)override{cs=std::move(v);}} parser;
struct Stack:t::StackItemContext{};auto stack=std::make_shared<Stack>();auto symbol=std::make_shared<const t::ParseSymbol>(t::ParseSymbol{u"fallback"});auto token=std::make_shared<t::Token>("x","y");
std::array<std::any,4> rest{Property(false),PropertyArray{1,"x"},stack,token};std::size_t calls=0;
auto fn=t::makeParseCallable([&](const folia::math::FunctionReceiver&,t::ParserContext& ctx,const t::ParseAtom& atom,t::ParseRest r)->t::ParseResult {expect(&ctx==&parser);expect(std::get<t::TokenRef>(atom)==token);expect(r.size()==4u);expect(std::any_cast<Property>(r[0])==Property(false));expect(std::any_cast<std::shared_ptr<Stack>>(r[2])==stack);expect(std::any_cast<t::TokenRef>(r[3])==token);ctx.setCurrentCS("\\\\x");switch(calls++){case 0:return Property(Missing{});case 1:return Property(false);case 2:return t::StackItemRef(stack);default:return t::ParseSymbolRef(symbol);}});
t::Macro n("x",t::ParseMethod{fn},PropertyArray{false});auto invoke=[&]{return t::invokeParseMethod(n.func(),folia::math::FunctionReceiver{Missing{}},parser,t::ParseAtom{token},rest);};
expect(std::get<Property>(invoke()).missing());expect(std::get<Property>(invoke())==Property(false));expect(std::get<t::StackItemRef>(invoke())==stack);expect(std::get<t::ParseSymbolRef>(invoke())==symbol);expect(parser.currentCS()==Property("\\\\x"));
t::ParseInput input{parser,"x"};expect(&input.parser==&parser);expect(input.token==Property("x"));t::ParseAtom raw{Property("raw")},item{t::StackItemRef(stack)};expect(std::get<Property>(raw)==Property("raw"));expect(std::get<t::StackItemRef>(item)==stack);
};`);
const header=`// Generated by tex-token-oracle.cjs from actual pinned MathJax 4.1.3.
#include "support/folia_test.hpp"
import folia.math.input.tex.macro;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.tex_constants;
import folia.math.core.mml_tree.mml_node_host;
namespace {using namespace boost::ut;namespace t=folia::math::tex;namespace h=folia::math::mml::host;using folia::math::Property;using folia::math::PropertyList;using folia::math::PropertyArray;using folia::math::Missing;using folia::math::PropertyNull;
std::u16string units(std::initializer_list<char16_t>v){return {v.begin(),v.end()};}
Property clone(const Property&v){if(auto a=PropertyArray::array(v)){PropertyArray r;for(std::size_t i=0;i<a->size();++i)r.push(clone(a->get(i)));return r;}if(auto a=PropertyList::object(v)){PropertyList r;for(const auto&k:a->keys())r.setOwn(k,clone(a->getOwn(k)));return r;}return v;}
bool deep(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return(std::isnan(*x)&&std::isnan(*y))||(*x==*y&&std::signbit(*x)==std::signbit(*y));auto x=PropertyList::record(a),y=PropertyList::record(b);if(x&&y){if(PropertyArray::array(a).has_value()!=PropertyArray::array(b).has_value()||x->keys()!=y->keys())return false;for(const auto&k:x->keys())if(!deep(x->getOwn(k),y->getOwn(k)))return false;return true;}return a==b;}
}
`;
const text=header+values.declarations().join('\n')+'\nnamespace {suite tex_tokens=[] {\n'+cases.join('\n')+'\n};}\n',target=path.join(root,'tests/math/reference_tex_token.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8'),text);else fs.writeFileSync(target,text);
console.log(JSON.stringify({cases:cases.length,source_assertions:checks,source_value_records:values.size(),native_protocol_witnesses:1,mode:process.argv.includes('--check')?'check':'generate'}));
