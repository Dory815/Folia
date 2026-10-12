// Original builtin Map observations for the parser family's actual string-key owners.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..');
const sources=['ts/input/tex/Configuration.ts','ts/input/tex/Tags.ts','ts/input/tex/ParseOptions.ts'];
const hashes=sources.map(source=>({source,sha256:crypto.createHash('sha256').update(fs.readFileSync(path.join(root,'.reference/MathJax-src',source))).digest('hex')}));
const rows=[];
function add(label,cpp,run){rows.push({label,cpp:cpp.replaceAll('options::','opts::'),expected:run()});}
function txt(x){return x===undefined?'U':x===null?'N':typeof x==='object'?'O':String(x);}
function keys(m){return [...m.keys()].join(',');}
add('empty-get-and-has','SourceStringMap<options::Value> m;return text(m.get(u"a"))+":"+yes(m.has(u"a"))+":"+keys(m);',()=>{const m=new Map();return txt(m.get('a'))+':'+m.has('a')+':'+keys(m);});
add('replacement-preserves-order','SourceStringMap<options::Value> m;m.set(u"b",1);m.set(u"a",2);m.set(u"b",3);return keys(m)+":"+text(m.get(u"b"))+":"+std::to_string(m.size());',()=>{const m=new Map([['b',1],['a',2]]);m.set('b',3);return keys(m)+':'+m.get('b')+':'+m.size;});
add('prototype-names-are-map-keys','SourceStringMap<options::Value> m;m.set(u"__proto__",1);m.set(u"constructor",2);m.set(u"toString",3);return keys(m)+":"+text(m.get(u"__proto__"));',()=>{const m=new Map([['__proto__',1],['constructor',2],['toString',3]]);return keys(m)+':'+m.get('__proto__');});
add('undefined-value-is-present','SourceStringMap<options::Value> m;m.set(u"a",options::Undefined{});return yes(m.has(u"a"))+":"+text(m.get(u"a"))+":"+yes(m.get(u"a").has_value())+":"+yes(m.get(u"b").has_value());',()=>{const m=new Map([['a',undefined]]);return m.has('a')+':'+txt(m.get('a'))+':'+m.has('a')+':'+m.has('b');});
add('null-false-zero-empty-values','SourceStringMap<options::Value> m;m.set(u"n",options::Null{});m.set(u"b",false);m.set(u"z",0);m.set(u"s",u"");return text(m.get(u"n"))+":"+text(m.get(u"b"))+":"+text(m.get(u"z"))+":"+text(m.get(u"s"));',()=>{const m=new Map([['n',null],['b',false],['z',0],['s','']]);return ['n','b','z','s'].map(k=>txt(m.get(k))).join(':');});
add('map-handle-alias','SourceStringMap<options::Value> m;auto alias=m;alias.set(u"x",1);m.set(u"x",2);return keys(m)+":"+text(alias.get(u"x"));',()=>{const m=new Map(),alias=m;alias.set('x',1);m.set('x',2);return keys(m)+':'+alias.get('x');});
add('fresh-map-independent','SourceStringMap<options::Value> a,b;a.set(u"x",1);b.set(u"x",2);return text(a.get(u"x"))+":"+text(b.get(u"x"));',()=>{const a=new Map([['x',1]]),b=new Map([['x',2]]);return a.get('x')+':'+b.get('x');});
add('object-values-retain-identity','options::Context c;auto value=c.object();SourceStringMap<options::Value> m;m.set(u"o",value);return yes(m.get(u"o")==std::optional<options::Value>(value));',()=>{const o={},m=new Map([['o',o]]);return String(m.get('o')===o);});
add('keys-iterator-sees-appends','SourceStringMap<options::Value> m;m.set(u"a",1);auto iterator=m.keys();auto first=key(iterator.next());m.set(u"b",2);auto second=key(iterator.next());return first+":"+second+":"+key(iterator.next());',()=>{const m=new Map([['a',1]]),i=m.keys(),a=i.next().value;m.set('b',2);return a+':'+i.next().value+':'+txt(i.next().value);});
add('keys-completion-is-sticky','SourceStringMap<options::Value> m;m.set(u"a",1);auto iterator=m.keys();auto first=key(iterator.next());auto end=key(iterator.next());m.set(u"b",2);return first+":"+end+":"+key(iterator.next());',()=>{const m=new Map([['a',1]]),i=m.keys(),a=i.next().value,end=i.next().value;m.set('b',2);return a+':'+txt(end)+':'+txt(i.next().value);});
add('keys-iterator-retains-owner','auto iterator=[] {SourceStringMap<options::Value> m;m.set(u"a",1);return m.keys();}();auto first=key(iterator.next());auto second=key(iterator.next());return first+":"+second;',()=>{const i=new Map([['a',1]]).keys();return i.next().value+':'+txt(i.next().value);});
add('two-key-iterators-independent','SourceStringMap<options::Value> m;m.set(u"a",1);m.set(u"b",2);auto a=m.keys(),b=m.keys();auto aa=key(a.next());auto ab=key(a.next());auto ba=key(b.next());return aa+":"+ab+":"+ba;',()=>{const m=new Map([['a',1],['b',2]]),a=m.keys(),b=m.keys();return a.next().value+':'+a.next().value+':'+b.next().value;});
let fixture=String.raw`// Generated from original builtin Map observations; not parser semantics.
#include "support/folia_test.hpp"
import folia.math.util.string_map;
import folia.math.util.options;
import folia.math.util.property_host;
import folia.math.util.js_number;
namespace {using namespace boost::ut;using namespace folia::math;namespace opts=folia::math::options;
std::string yes(bool x){return x?"true":"false";}
std::string text(std::optional<opts::Value> value){if(!value||std::holds_alternative<opts::Undefined>(value->data))return "U";if(std::holds_alternative<opts::Null>(value->data))return "N";if(auto*b=std::get_if<bool>(&value->data))return yes(*b);if(auto*n=std::get_if<double>(&value->data))return js_number::toString(*n);if(auto*s=std::get_if<std::u16string>(&value->data))return property_host::debugText(*s);return "O";}
std::string key(std::optional<std::u16string> value){return value?property_host::debugText(*value):"U";}
std::string keys(const SourceStringMap<opts::Value>&m){auto i=m.keys();std::string out;while(auto k=i.next()){if(!out.empty())out+=',';out+=property_host::debugText(*k);}return out;}
suite string_map=[] {
`;
for(const[i,row]of rows.entries())fixture+='"math_port_string_map_source_'+i+'"_test=[] {auto run=[]()->std::string{'+row.cpp+'};expect(eq(run(),std::string('+JSON.stringify(row.expected)+')))<<'+JSON.stringify(row.label)+';};\n';
fixture+='};}\n';
for(const[file,data]of [['tests/math/reference_string_map.cpp',fixture],['docs/math-port/evidence/p53-string-map-source.json',JSON.stringify({stage:'native string-keyed Map host; not parser algorithm acceptance',node:process.version,hashes,rows},null,2)+'\n']]){const target=path.join(root,file);if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),data);else fs.writeFileSync(target,data);}
console.log(JSON.stringify({source_cases:rows.length}));
