// Root oracle: actual String.match truthiness; native output never expectations.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,M,names,specs,u}=require('./mo-pattern-reference.cjs');
const out=['// Generated actual-CJS Mo regex observations. Do not hand-edit.',
'#include "support/folia_test.hpp"','import folia.math.core.mml_tree.mo_patterns;',
'namespace m=folia::math::mml;using namespace boost::ut;namespace {',
'std::u16string units(std::initializer_list<unsigned>v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}',
'std::string expand(std::initializer_list<std::pair<std::size_t,char>>runs){std::string s;for(auto[n,c]:runs)s.append(n,c);return s;}',
'template<class Fn>std::string bits(Fn f){constexpr char hex[]="0123456789abcdef";std::string s;for(unsigned start=0;start<65536;start+=4){unsigned b=0;for(unsigned j=0;j<4;++j)if(f(static_cast<char16_t>(start+j)))b|=1u<<j;s.push_back(hex[b]);}return s;}',
'} suite math_port_mo_pattern_tests=[] {'];
let cases=0,assertions=0,sourceQueries=0;
const emit=x=>out.push(x),begin=name=>{cases++;emit('"math_port_mo_pattern_'+name+'"_test=[] {');},end=()=>emit('};');
function expect(expr){assertions++;emit('expect(bool('+expr+'));');}
function runs(s){const r=[];for(const c of s){const last=r.at(-1);if(last&&last[1]===c)last[0]++;else r.push([1,c]);}return 'expand({'+r.map(([n,c])=>'{'+n+",'"+c+"'}").join(',')+'})';}
function observed(id,text){sourceQueries++;return !!text.match(M[names[id]]);}
function mask(fn){let s='';for(let i=0;i<65536;i+=4){let n=0;for(let j=0;j<4;j++)if(fn(String.fromCharCode(i+j)))n|=1<<j;s+=n.toString(16);}return s;}
for(const [i,s]of specs.entries()){
 begin(s.name+'_metadata');emit('const auto&spec=m::moPatternSpec(m::MoPattern::'+s.name+');');
 expect('spec.source=='+u(M[s.name].source));expect('spec.flags==std::string_view("")');
 expect('spec.minimum=='+s.minimum);expect(s.maximum===undefined?'!spec.maximum':'spec.maximum==std::optional<std::size_t>('+s.maximum+')');
 emit('std::vector<std::pair<unsigned,unsigned>> ranges;for(const auto&r:spec.units)ranges.emplace_back(r.first,r.last);');
 expect('(ranges==std::vector<std::pair<unsigned,unsigned>>{'+s.ranges.map(([a,b])=>'{'+a+','+b+'}').join(',')+'})');end();
 begin(s.name+'_complete_utf16_domain');emit('constexpr auto kind=m::MoPattern::'+s.name+';');
 const first=s.ranges[0][0],a=String.fromCharCode(first);
 const variants=[['std::u16string(1,c)',c=>c],['std::u16string(2,c)',c=>c+c],['std::u16string(3,c)',c=>c+c+c],
 ['units({'+first+'})+std::u16string(1,c)',c=>a+c],['std::u16string(1,c)+units({'+first+'})',c=>c+a],
 ['units({'+first+'})+std::u16string(1,c)+units({'+first+'})',c=>a+c+a]];
 for(const [native,make]of variants){const wanted=mask(c=>observed(i,make(c)));expect('bits([&](char16_t c){return m::matchMoPattern(kind,'+native+');})=='+runs(wanted));}
 end();
 begin(s.name+'_mixed_products_and_adversarial_text');
 const members=[];for(let x=0;x<65536;x++)if(observed(i,String.fromCharCode(x).repeat(s.minimum)))members.push(x);
 let product='';for(const x of members)for(const y of members)for(const tail of ['',String.fromCharCode(first),'\n','\r\n','\u2028','\u2029','\0'])product+=observed(i,String.fromCharCode(x,y)+tail)?'1':'0';
 emit('constexpr auto kind=m::MoPattern::'+s.name+';const std::vector<unsigned> members={'+members.join(',')+'};std::string product;for(auto x:members)for(auto y:members)for(const auto&tail:std::vector<std::u16string>{u"",'+u(a)+',u"\\n",u"\\r\\n",'+u('\u2028')+','+u('\u2029')+',units({0})})product+=m::matchMoPattern(kind,units({x,y})+tail)?\'1\':\'0\';');
 expect('product=='+runs(product));
 const samples=['',a,a+a,a+a+a,a.repeat(512),a.repeat(4096),a+'\n',a+'\r',a+'\r\n',a+'\u2028',a+'\u2029','\n'+a,'\r'+a,'\0'+a,a+'\0',a+'x'+a,'abc','aA','Aa','aa','zZ','Az9','alpha','ALPHA','áá','ＡＡ','KK','İİ','ßß','²²','\u2032\u2033','\ud800','\udfff','\ud800\udc00','😀','\ud800'+a,a+'\udc00','^','→','→←','^\u0301','"\u2018','\u00aa\u208e',' '+a+a,a+a+' ','\t'+a+a,a+a+'\t','\n'+a+a,a+a+'\n','\r\n'+a+a,a+a+'\r\n'];
 emit('struct Row{std::u16string text;bool wanted;};const Row samples[]={');
 for(const text of samples)emit('{'+u(text)+','+observed(i,text)+'},');
 emit('};auto first=m::makeMoPattern(kind);auto second=m::makeMoPattern(kind);');expect('bool(first)');expect('bool(second)');
 emit('for(const auto&r:samples){expect(m::matchMoPattern(kind,r.text)==r.wanted);expect(first(r.text)==r.wanted);expect(second(r.text)==r.wanted);}');assertions+=3*samples.length;
 emit('first={};for(const auto&r:samples)expect(second(r.text)==r.wanted);');assertions+=samples.length;end();
}
begin('bound_errors_and_independent_callback_lifetime');
emit('for(const auto kind:{static_cast<m::MoPattern>(-1),static_cast<m::MoPattern>(5),static_cast<m::MoPattern>(99)}){bool match=false,create=false;try{(void)m::matchMoPattern(kind,u"");}catch(const m::Error&e){match=e.name=="TypeError";}try{(void)m::makeMoPattern(kind);}catch(const m::Error&e){create=e.name=="TypeError";}expect(match);expect(create);}');assertions+=6;
emit('const auto build=[](m::MoPattern kind){return m::makeMoPattern(kind);};std::array<m::Pattern,5> callbacks;');
for(const [i,name]of names.entries())emit('callbacks['+i+']=build(m::MoPattern::'+name+');');
for(const text of ["*","'",'ab','^','→','',"'''",'a1'])for(let i=0;i<names.length;i++)expect('callbacks['+i+']('+u(text)+')=='+observed(i,text));end();
emit('};');const output=out.join('\n')+'\n',target=path.join(root,'tests/math/reference_mo_patterns.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
console.log(JSON.stringify({patterns:names.length,cases,assertions,actual_source_queries:sourceQueries,unit_domain_masks:30,mode:process.argv.includes('--check')?'check':'generate'}));
