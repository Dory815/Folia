// Root complete original FindTeX observations; no replacement scanner.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),base=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src'),lock=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p46-parser-options-source.json'))),hashes=[];
for(const source of ['ts/input/tex/FindTeX.ts','ts/core/FindMath.ts','ts/core/MathItem.ts','ts/util/string.ts','ts/util/Options.ts']){const entry=lock.hashes.find(x=>x.source===source);assert.ok(entry,source);const bytes=fs.readFileSync(path.join(root,'.reference/MathJax-src',source));assert.equal(crypto.createHash('sha256').update(bytes).digest('hex'),entry.source_sha256);assert.equal(fs.readFileSync(path.join(base,source),'utf8').replaceAll('\r\n','\n'),bytes.toString().replaceAll('\r\n','\n'));assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(base,source.replace(/^ts\//,'cjs/').replace(/\.ts$/,'.js')))).digest('hex'),entry.cjs_sha256);hashes.push(entry);}
const {FindTeX}=require(path.join(base,'cjs/input/tex/FindTeX.js')),rows=[];
function hex(s){return Array.from({length:s.length},(_,i)=>s.charCodeAt(i).toString(16).padStart(4,'0')).join('');}
function text(v){return v===undefined?'U':v===null?'N':typeof v==='object'?'O':String(v);}
function item(v){return[hex(v.open),hex(v.math),hex(v.close),v.n,v.start.n,v.end.n,text(v.display)].join(',');}
function serialize(items){return items.map(item).join('/');}
function fields(j){return[text(j.options),j.start?'R':'U',text(j.end),text(j.hasPatterns),text(j.env),text(j.sub)].join('/');}
function fixture(){const e={trace:[],hooks:{phase:false,scan:false,capture:false,mutation:0,input:null,saved:null},j:null};class Probe extends FindTeX{
getPatterns(){if(e.hooks.phase)e.trace.push('before:'+fields(this));super.getPatterns();if(e.hooks.phase)e.trace.push('after:'+fields(this));}
findMathInString(math,n,s){if(e.hooks.scan)e.trace.push('scan:'+n+':'+text(s));if(n===0&&e.hooks.mutation)e.hooks.input[e.hooks.mutation===1?1:e.hooks.input.length]='$z$';super.findMathInString(math,n,s);}
findEnd(s,n,start,end){const result=super.findEnd(s,n,start,end);if(e.hooks.capture&&result)e.hooks.saved=result;return result;}}
e.make=o=>(e.j=new Probe(o));return e;}
function add(label,cpp,run){const e=fixture();let result='THREW',error='ok';try{result=run(e);}catch(x){error=x.name;}rows.push({label,cpp,expected:error+'|'+result+'|'+e.trace.join('~')});}
function literal(s){return 'std::u16string({'+Array.from({length:s.length},(_,i)=>'char16_t(0x'+s.charCodeAt(i).toString(16)+')').join(',')+'})';}
function value(v){if(v===undefined)return 'opts::Undefined{}';if(v===null)return 'opts::Null{}';if(typeof v==='boolean'||typeof v==='number')return String(v);if(typeof v==='string')return literal(v);if(Array.isArray(v))return 'e.array({'+v.map(value).join(',')+'})';return 'e.object({'+Object.entries(v).map(([k,x])=>'{'+literal(k)+','+value(x)+'}').join(',')+'})';}
function scan(label,options,strings){add(label,'auto j=e.make('+value(options)+');return serialize(j->findMath(opts::Value('+value(strings)+')));',e=>serialize(e.make(options).findMath(strings)));}
const dollar={inlineMath:[['$','$']]},disabled={inlineMath:[],displayMath:[],processEscapes:false,processEnvironments:false,processRefs:false};
for(const[label,input]of [
 ['default-empty',[]],['default-plain',['abc']],['default-dollar-not-inline',['$x$']],['default-inline',['a \\(x+1\\) b']],['default-display-dollar',['a $$x$$ b']],['default-display-bracket',['\\[x\\]']],['default-mixed',['\\(x\\) $$y$$ \\[z\\]']],
 ['default-multiple-strings',['a \\(x\\)','$$y$$','plain']],['default-unmatched',['\\(x']],['default-empty-math',['\\(\\)']],['default-control-sequence-skips-close',['\\(a\\foo b\\)']],
 ['default-braces-hide-close',['\\(a{b\\)c}d\\)']],['default-unbalanced-braces',['\\(a{b\\)']],['default-extra-close-brace',['\\(a}b\\)']],['default-newlines',['\\(a\nb\\)']],
 ['default-environment',['a \\begin{align}x&y\\end{align} b']],['default-environment-whitespace',['\\begin\u00a0{align}x\\end\ufeff{align}']],['default-empty-environment-name',['\\begin{}x\\end{}']],
 ['default-punctuation-environment',['\\begin{a-b:c}x\\end{a-b:c}']],['default-unmatched-environment',['\\begin{align}x\\end{other}']],['default-ref-and-eqref',['a \\ref{r} \\eqref{s}']],
 ['default-ref-whitespace',['\\ref\u00a0{r}']],['default-escaped-dollar',['a \\$ b']],['default-escaped-backslash',['a \\\\ b']],['default-substitutions-mixed',['\\$ \\ref{r} \\(x\\)']],
 ['default-astral-offsets',['😀 \\(😀+x\\) 🧮']],['default-surrogate-halves',['\ud800 \\(\udc00\\)']],['default-slash-astral-control',['\\(\\😀{x}\\)']]
])scan(label,{},input);
for(const[label,input]of [['inline-dollar',['a $x$ b']],['inline-dollar-empty',['$$']],['display-dollar-wins-prefix',['$$x$$ $y$']],['multiple-dollar',['$x$ $y$']],['escaped-dollar-with-inline',['\\$x $y$']],['dollar-in-braces',['$a{b$c}d$']],['dollar-escaped-close',['$a\\$b$']],['incomplete-then-valid',['$a \\(x\\)']]])scan(label,dollar,input);
scan('adjacent-default-math',{},['\\(x\\)\\(y\\)']);
scan('adjacent-inline-dollar',dollar,['$x$$y$']);
scan('disabled-no-patterns',disabled,['$x$ \\(y\\) \\ref{r}']);
scan('escapes-disabled',{...dollar,processEscapes:false},['\\$x$']);
scan('environments-disabled',{processEnvironments:false},['\\begin{align}x\\end{align}']);
scan('references-disabled',{processRefs:false},['\\ref{r} \\eqref{s}']);
scan('only-references',{...disabled,processRefs:true},['\\ref{r} \\eqref{s}']);
scan('only-escapes',{...disabled,processEscapes:true},['\\$ \\\\']);
scan('only-environments',{...disabled,processEnvironments:true},['\\begin{x}a\\end{x}']);
scan('literal-quoted-punctuation',{inlineMath:[[':(','-]']],displayMath:[]},['a :(x-] b']);
scan('literal-all-regex-punctuation',{inlineMath:[['^$(){}.+*?-|[]:\\','END.*']],displayMath:[]},['^$(){}.+*?-|[]:\\xEND.*']);
scan('longer-opener-wins',{inlineMath:[['@','@'],['@@','@@']],displayMath:[]},['@@x@@ @y@']);
scan('same-length-opener-lexical-sort',{inlineMath:[['b@','@b'],['a@','@a']],displayMath:[]},['b@x@b a@y@a']);
scan('duplicate-opener-last-end-wins',{inlineMath:[['@','!'],['@','?']],displayMath:[]},['@x!y?']);
scan('display-definition-overwrites-inline',{inlineMath:[['@','!']],displayMath:[['@','?']]},['@x?']);
scan('ordinary-end-prototype-key',{inlineMath:[['__proto__','END']],displayMath:[]},['__proto__xEND']);
scan('ordinary-end-constructor-key',{inlineMath:[['constructor','END']],displayMath:[]},['constructorxEND']);
scan('ordinary-end-tostring-key',{inlineMath:[['toString','END']],displayMath:[]},['toStringxEND']);
scan('astral-delimiter-and-utf16-offsets',{inlineMath:[['😀','🧮']],displayMath:[]},['a😀x🧮b']);
scan('empty-close-delimiter',{inlineMath:[['@','']],displayMath:[]},['@x']);
scan('null-inline-array-is-type-error',{inlineMath:null},[]);
scan('false-inline-array-is-type-error',{inlineMath:false},[]);
scan('missing-close-is-type-error',{inlineMath:[['@']]},[]);
scan('nonstring-close-is-type-error',{inlineMath:[['@',4]]},[]);
add('constructor-derived-fields-and-virtual-phase','e.hooks.phase=true;auto j=e.make(e.object({}));return j->fields();',e=>{e.hooks.phase=true;return fields(e.make({}));});
add('fresh-results-and-reset-start-cursor','auto j=e.make('+value(dollar)+');auto input='+value(['$x$'])+';auto a=j->findMath(opts::Value(input));auto b=j->findMath(opts::Value(input));return yes(a!=b)+":"+serialize(a)+":"+serialize(b);',e=>{const j=e.make(dollar),a=j.findMath(['$x$']),b=j.findMath(['$x$']);return[a!==b,serialize(a),serialize(b)].join(':');});
add('environment-mutates-same-proto-record','e.hooks.capture=true;auto j=e.make(e.object({}));auto result=j->findMath(opts::Value('+value(['\\begin{x}a\\end{x}'])+'));return yes(e.hooks.saved==std::get<folia::math::Property>((*result)[0]))+":"+serialize(result)+":"+serializeItem(e.hooks.saved);',e=>{e.hooks.capture=true;const result=e.make({}).findMath(['\\begin{x}a\\end{x}']);return[e.hooks.saved===result[0],serialize(result),item(e.hooks.saved)].join(':');});
add('input-slots-read-live-after-first-scan','e.hooks.scan=true;e.hooks.mutation=1;auto j=e.make('+value(dollar)+');e.hooks.input='+value(['$x$','$y$'])+';return serialize(j->findMath(opts::Value(e.hooks.input)));',e=>{e.hooks.scan=true;e.hooks.mutation=1;e.hooks.input=['$x$','$y$'];return serialize(e.make(dollar).findMath(e.hooks.input));});
add('input-length-captured-before-scan','e.hooks.scan=true;e.hooks.mutation=2;auto j=e.make('+value(dollar)+');e.hooks.input='+value(['$x$'])+';return serialize(j->findMath(opts::Value(e.hooks.input)));',e=>{e.hooks.scan=true;e.hooks.mutation=2;e.hooks.input=['$x$'];return serialize(e.make(dollar).findMath(e.hooks.input));});
add('input-getters-read-once-in-order','auto j=e.make('+value(dollar)+');auto input=e.context.array(2);e.getter(input,u"0",[&]{e.trace.push_back("get0");return opts::Value(u"$x$");});e.getter(input,u"1",[&]{e.trace.push_back("get1");return opts::Value(u"$y$");});return serialize(j->findMath(opts::Value(input)));',e=>{const j=e.make(dollar),input=[];Object.defineProperty(input,0,{get(){e.trace.push('get0');return '$x$';}});Object.defineProperty(input,1,{get(){e.trace.push('get1');return '$y$';}});return serialize(j.findMath(input));});
add('input-hole-exec-coercion-is-deferred','auto j=e.make(e.object({}));auto input=e.context.array(1);return serialize(j->findMath(opts::Value(input)));',e=>{const a=new Array(1);return serialize(e.make({}).findMath(a));});
scan('primitive-input-slot-with-no-start',{},[null,undefined,7,false]);
add('matched-nonstring-slot-substring-is-type-error','auto j=e.make('+value({inlineMath:[['n','l']],displayMath:[]})+');return serialize(j->findMath(opts::Value(e.array({opts::Null{}}))));',e=>serialize(e.make({inlineMath:[['n','l']],displayMath:[]}).findMath([null])));
add('disabled-finder-does-not-inspect-null-input','auto j=e.make('+value(disabled)+');return serialize(j->findMath(opts::Value(opts::Null{})));',e=>serialize(e.make(disabled).findMath(null)));
add('enabled-finder-null-input-is-type-error','auto j=e.make(e.object({}));return serialize(j->findMath(opts::Value(opts::Null{})));',e=>serialize(e.make({}).findMath(null)));
add('getpatterns-replaces-end-and-regex-identities','auto j=e.make(e.object({}));auto oldEnd=j->endValue(),oldStart=j->startValue();j->refresh();return yes(oldEnd!=j->endValue())+":"+yes(oldStart!=j->startValue());',e=>{const j=e.make({}),end=j.end,start=j.start;j.getPatterns();return[end!==j.end,start!==j.start].join(':');});
add('getpatterns-real-looked-up-forEach','auto j=e.make(e.object({}));auto options=j->currentOptions();auto inlines=e.context.get(options,opts::Key(u"inlineMath"));auto method=e.context.function(u"override",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("forEach");return opts::Undefined{};});e.context.set(inlines,opts::Key(u"forEach"),method);j->refresh();return j->fields();',e=>{const j=e.make({});j.options.inlineMath.forEach=()=>e.trace.push('forEach');j.getPatterns();return fields(j);});
add('delimiter-getters-consumed-in-source-order','auto j=e.make('+value({...disabled,inlineMath:[['@','!']]})+');auto tuple=e.context.get(e.context.get(j->currentOptions(),opts::Key(u"inlineMath")),opts::Key(u"0"));auto object=std::get<opts::ObjectRef>(tuple.data);e.getter(object,u"0",[&]{e.trace.push_back("open");return opts::Value(u"@");});e.getter(object,u"1",[&]{e.trace.push_back("close");return opts::Value(u"!");});j->refresh();return serialize(j->findMath(opts::Value('+value(['@x!'])+')));',e=>{const j=e.make({...disabled,inlineMath:[['@','!']]}),t=j.options.inlineMath[0];Object.defineProperty(t,0,{get(){e.trace.push('open');return '@';}});Object.defineProperty(t,1,{get(){e.trace.push('close');return '!';}});j.getPatterns();return serialize(j.findMath(['@x!']));});
for(const shrink of [false,true])add(shrink?'delimiter-iterator-rechecks-length-after-open-getter':'delimiter-iterator-skips-inherited-close-beyond-length',
'auto j=e.make('+value({...disabled,inlineMath:[['@','!']]})+');auto tuple=e.context.get(e.context.get(j->currentOptions(),opts::Key(u"inlineMath")),opts::Key(u"0"));auto object=std::get<opts::ObjectRef>(tuple.data);auto prototype=e.context.objectWithPrototype(e.context.intrinsics().arrayPrototype);e.getter(prototype,u"1",[&]{e.trace.push_back("inherited-close");return opts::Value(u"!");});e.context.setPrototype(object,prototype);'+(shrink?'':'e.context.set(object,opts::Key(u"length"),1);')+'e.getter(object,u"0",[&]{e.trace.push_back("open");'+(shrink?'e.context.set(object,opts::Key(u"length"),1);':'')+'return opts::Value(u"@");});j->refresh();return "unexpected";',
e=>{const j=e.make({...disabled,inlineMath:[['@','!']]}),tuple=j.options.inlineMath[0],prototype=Object.create(Array.prototype);Object.defineProperty(prototype,1,{get(){e.trace.push('inherited-close');return '!';}});Object.setPrototypeOf(tuple,prototype);if(!shrink)tuple.length=1;Object.defineProperty(tuple,0,{get(){e.trace.push('open');if(shrink)tuple.length=1;return '@';}});j.getPatterns();return 'unexpected';});
add('actual-constructor-metadata-and-defaults',String.raw`auto klass=e.module.constructorIdentity();auto defaults=e.context.get(klass,opts::Key(u"OPTIONS"));std::string out=text(e.context.get(klass,opts::Key(u"name")))+":"+text(e.context.get(klass,opts::Key(u"length")))+":"+yes(e.context.hasOwn(klass,opts::Key(u"OPTIONS")));for(auto name:{u"inlineMath",u"displayMath"}){auto array=e.context.get(defaults,opts::Key(name));auto object=std::get<opts::ObjectRef>(array.data);for(std::uint32_t i=0;i<e.context.arrayLength(object);++i){auto key=std::to_string(i);auto tuple=e.context.get(array,opts::Key(std::u16string(key.begin(),key.end())));out+=":"+text(e.context.get(tuple,opts::Key(u"0")))+","+text(e.context.get(tuple,opts::Key(u"1")));}}for(auto name:{u"processEscapes",u"processEnvironments",u"processRefs"})out+=":"+text(e.context.get(defaults,opts::Key(name)));return out;`,()=>{let out=[FindTeX.name,FindTeX.length,Object.hasOwn(FindTeX,'OPTIONS')].join(':');for(const name of ['inlineMath','displayMath'])for(const pair of FindTeX.OPTIONS[name])out+=':'+pair.join(',');for(const name of ['processEscapes','processEnvironments','processRefs'])out+=':'+FindTeX.OPTIONS[name];return out;});
let native=String.raw`// Generated only from pinned original FindTeX, with real superclass and protoItem.
#include "support/folia_test.hpp"
import folia.math.input.tex.find_tex_host;
import folia.math.util.property_host;
import folia.math.util.js_number;
namespace {using namespace boost::ut;namespace opts=folia::math::options;using namespace folia::math;
std::string yes(bool v){return v?"true":"false";}
std::string hex(std::u16string_view s){constexpr char digits[]="0123456789abcdef";std::string out;for(auto c:s)for(int shift=12;shift>=0;shift-=4)out+=digits[(c>>shift)&15];return out;}
std::string text(opts::Value v){if(std::holds_alternative<opts::Undefined>(v.data))return "U";if(std::holds_alternative<opts::Null>(v.data))return "N";if(auto*b=std::get_if<bool>(&v.data))return yes(*b);if(auto*n=std::get_if<double>(&v.data))return js_number::toString(*n);if(auto*s=std::get_if<std::u16string>(&v.data))return property_host::debugText(*s);return "O";}
std::string serializeItem(Property p){auto r=PropertyList::record(p);if(!r)return "null";auto number=[](Property v){return property_host::debugText(property_host::string(v));};return hex(property_host::string(r->get("open")))+","+hex(property_host::string(r->get("math")))+","+hex(property_host::string(r->get("close")))+","+number(r->get("n"))+","+number(property_host::get(r->get("start"),"n"))+","+number(property_host::get(r->get("end"),"n"))+","+(std::holds_alternative<PropertyNull>(r->get("display").value)?"N":number(r->get("display")));}
std::string serialize(core::ProtoMathItems items){std::string out;for(auto&i:*items){if(!out.empty())out+='/';out+=serializeItem(std::get<Property>(i));}return out;}
struct Hooks{std::vector<std::string>*trace;bool phase=false,scan=false,capture=false;int mutation=0;opts::ObjectRef input;Property saved=PropertyNull{};};
struct Probe:tex::FindTeX{Probe(opts::Context&c,opts::ObjectRef i,regexp_host::RegExpBindings&r):FindTeX(c,i,r){}
Hooks&hooks(){return *std::any_cast<Hooks*>(optionsContext().payload(optionsContext().intrinsics().objectPrototype));}
std::string fields(){return text(options_)+"/"+(std::holds_alternative<opts::Undefined>(start_.data)?"U":"R")+"/"+text(end_)+"/"+text(hasPatterns_)+"/"+text(env_)+"/"+text(sub_);}
opts::Value currentOptions(){return options_;}opts::Value endValue(){return end_;}opts::Value startValue(){return start_;}void refresh(){getPatterns();}
void getPatterns()override{auto&h=hooks();if(h.phase)h.trace->push_back("before:"+fields());FindTeX::getPatterns();if(h.phase)h.trace->push_back("after:"+fields());}
void findMathInString(core::ProtoMathItems math,double n,opts::Value s)override{auto&h=hooks();if(h.scan)h.trace->push_back("scan:"+js_number::toString(n)+":"+text(s));if(n==0&&h.mutation){auto index=h.mutation==1?1:optionsContext().arrayLength(h.input);auto key=std::to_string(index);optionsContext().set(h.input,opts::Key(std::u16string(key.begin(),key.end())),u"$z$");}FindTeX::findMathInString(math,n,s);}
Property findEnd(opts::Value s,double n,const regexp_host::Match&start,EndItem end)override{auto result=FindTeX::findEnd(s,n,start,end);if(hooks().capture&&PropertyList::record(result))hooks().saved=result;return result;}
};
struct Fixture{std::vector<std::string>trace;Hooks hooks{&trace};opts::Context context;core::FindMathModule base{context};regexp_host::RegExpBindings regexp{context};tex::FindTeXModule module{context,base,regexp};Fixture(){context.payload(context.intrinsics().objectPrototype)=&hooks;}
opts::ObjectRef object(std::initializer_list<std::pair<std::u16string,opts::Value>>values){auto o=context.object();for(auto&[k,v]:values)context.set(o,opts::Key(k),v);return o;}
opts::ObjectRef array(std::initializer_list<opts::Value>values){auto a=context.array();std::uint32_t i=0;for(auto&v:values){auto key=std::to_string(i++);context.set(a,opts::Key(std::u16string(key.begin(),key.end())),v);}return a;}
std::shared_ptr<Probe>make(opts::Value supplied){return module.make<Probe>(std::move(supplied));}
void getter(opts::ObjectRef object,std::u16string key,std::function<opts::Value()>body){auto function=context.function(u"getter",[body](auto&,auto,auto){return body();});context.define(object,opts::Key(std::move(key)),opts::Descriptor::access(function,{}));}
};std::string join(const std::vector<std::string>&values){std::string out;for(auto&v:values){if(!out.empty())out+='~';out+=v;}return out;}
suite find_tex=[] {
`;
for(const[i,row]of rows.entries())native+='"math_port_find_tex_source_'+i+'"_test=[] {Fixture e;std::string error="ok",result="THREW";try{auto run=[&]()->std::string{'+row.cpp+'};result=run();}catch(const opts::Exception&x){error=x.name;}expect(eq(error+"|"+result+"|"+join(e.trace),std::string('+JSON.stringify(row.expected)+')))<<'+JSON.stringify(row.label)+';};\n';native+='};}\n';
for(const[file,data]of [['docs/math-port/evidence/p52-find-tex-source.json',JSON.stringify({stage:'complete actual FindTeX algorithms; not TeXParser',node:process.version,hashes,rows},null,2)+'\n'],['tests/math/reference_find_tex.cpp',native]]){const target=path.join(root,file);if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),data);else fs.writeFileSync(target,data);}
console.log(JSON.stringify({source_cases:rows.length}));
