// Root observations of actual ECMAScript RegExp and pinned FindTeX generators.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),base=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src'),lock=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p46-parser-options-source.json'))),hashes=[];
for(const source of ['ts/input/tex/FindTeX.ts','ts/util/string.ts']){const entry=lock.hashes.find(x=>x.source===source);assert.ok(entry,source);const bytes=fs.readFileSync(path.join(root,'.reference/MathJax-src',source));assert.equal(crypto.createHash('sha256').update(bytes).digest('hex'),entry.source_sha256);assert.equal(fs.readFileSync(path.join(base,source),'utf8').replaceAll('\r\n','\n'),bytes.toString().replaceAll('\r\n','\n'));assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(base,source.replace(/^ts\//,'cjs/').replace(/\.ts$/,'.js')))).digest('hex'),entry.cjs_sha256);hashes.push(entry);}
const {FindTeX}=require(path.join(base,'cjs/input/tex/FindTeX.js')),{quotePattern}=require(path.join(base,'cjs/util/string.js'));
const finder=new FindTeX({}),rows=[];
function hex(s){return Array.from({length:s.length},(_,i)=>s.charCodeAt(i).toString(16).padStart(4,'0')).join('');}
function observe(r,input,count){const out=[];for(let i=0;i<count;i++){const m=r.exec(input);out.push((m?'index='+m.index+',caps='+Array.from(m,x=>x===undefined?'U':hex(x)).join('/')+',input='+hex(m.input):'null')+';last='+String(r.lastIndex));}return out.join('|');}
function add(label,pattern,input,flags='g',lastIndex=0,count=3){let expected;try{const r=new RegExp(pattern,flags);r.lastIndex=lastIndex;expected=observe(r,input,count);}catch(x){expected=x.name;}rows.push({label,pattern,input,flags,lastIndex:Number.isFinite(lastIndex)?lastIndex:String(lastIndex),count,expected});}
add('dot-splits-surrogate-pair','.','😀');
add('dot-starting-at-low-surrogate','.','😀','g',1,2);
add('negated-class-splits-surrogates','[^}]','😀');
add('literal-astral-pair','😀','a😀b');
add('optional-captures-are-undefined','(a)?(b*)','b');
add('unmatched-alternative-captures','(a)|(b)','b a');
add('absolute-index-with-nonzero-cursor','x','abx x','g',3);
add('preceding-context-is-not-sliced','\\bx','ax x','g',1);
add('global-failure-resets-cursor','x','ab','g',1);
add('cursor-after-end','x','x','g',2);
add('negative-cursor-is-zero','x','x','g',-4);
add('fractional-cursor-is-floored','x','xx','g',1.9);
add('nan-cursor-is-zero','x','x','g',NaN);
add('infinite-cursor-fails','x','x','g',Infinity);
add('large-cursor-fails','x','x','g',9007199254740992);
add('plain-scan-ignores-and-retains-cursor','x','ax','','17');
// Above caller profile is numeric; keep the Source stimulus numeric too.
rows.at(-1).lastIndex=17;{const r=new RegExp('x');r.lastIndex=17;rows.at(-1).expected=observe(r,'ax',3);}
add('plain-negative-cursor-retained','x','ax','',-3);
add('empty-global-match-does-not-advance','','abc','g',1);
add('lookahead-zero-width-does-not-advance','(?=x)','ax','g',1);
add('sticky-failure-resets-cursor','x','ax','y',0);
add('sticky-starts-at-cursor','x','ax','y',1);
add('global-and-sticky','x','xx','gy',1);
add('dot-excludes-all-four-line-terminators','.','\n\r\u2028\u2029a');
add('space-includes-ecmascript-whitespace','\\s+','\u0085\u00a0\u1680\u2000\u2028\u2029\u202f\u205f\u3000\ufeff');
add('identity-escape-colon','\\:',':');
add('identity-escape-hyphen','\\-','-');
const punctuation='^$(){}.+*?-|[]:\\';add('all-quoted-delimiter-punctuation',quotePattern(punctuation),'a'+punctuation+'b');
add('actual-start-environment-and-refs',finder.start.source,'\\begin\u00a0{align}x\\ref\ufeff{a}\\$\\\\');
add('actual-end-pattern-skips-low-surrogate',finder.endPattern('\\)').source,'\\😀{x}\\)');
add('actual-end-pattern-skips-control-sequences',finder.endPattern('$$').source,'\\a{b}$$');
add('actual-environment-end-capture',finder.endPattern(null,'\\\\end\\s*(\\{a\\-b\\})').source,'x\\end\u00a0{a-b}');
add('invalid-group-is-syntax-error','(','x');
add('duplicate-flags-are-syntax-error','x','x','gg');
add('unknown-flag-is-syntax-error','x','x','z');
function literal(s){return 'std::u16string({'+Array.from({length:s.length},(_,i)=>'char16_t(0x'+s.charCodeAt(i).toString(16)+')').join(',')+'})';}
function number(n){return typeof n==='number'?String(n):n==='NaN'?'std::numeric_limits<double>::quiet_NaN()':'std::numeric_limits<double>::infinity()';}
let native=String.raw`// Generated from original RegExp exec, using pinned FindTeX-generated patterns.
#include "support/folia_test.hpp"
import folia.math.util.regexp_host;
import folia.math.util.js_number;
namespace {using namespace boost::ut;namespace host=folia::math::regexp_host;
std::string hex(std::u16string_view s){constexpr char digits[]="0123456789abcdef";std::string out;for(auto c:s){for(int shift=12;shift>=0;shift-=4)out.push_back(digits[(c>>shift)&15]);}return out;}
std::string observe(host::RegExp&r,const std::u16string&input,int count){std::string out;for(int i=0;i<count;i++){if(i)out+='|';auto m=r.exec(input);if(m){out+="index="+std::to_string(m->index)+",caps=";for(std::size_t j=0;j<m->captures.size();j++){if(j)out+='/';out+=m->captures[j]?hex(*m->captures[j]):"U";}out+=",input="+hex(m->input);}else out+="null";out+=";last="+folia::math::js_number::toString(r.lastIndex);}return out;}
suite regexp=[] {
`;
for(const[i,row]of rows.entries())native+='"math_port_scanner_regexp_source_'+i+'"_test=[] {std::string actual;try{host::RegExp r('+literal(row.pattern)+','+literal(row.flags)+');r.lastIndex='+number(row.lastIndex)+';actual=observe(r,'+literal(row.input)+','+row.count+');}catch(const folia::math::options::Exception&error){actual=error.name;}expect(eq(actual,std::string('+JSON.stringify(row.expected)+')))<<'+JSON.stringify(row.label)+';};\n';native+='};}\n';
const manifest={stage:'native numeric-cursor non-Unicode scanner RegExp host; not complete ECMAScript regex',node:process.version,srell_sha256:crypto.createHash('sha256').update(fs.readFileSync(path.join(root,'third_party/srell/srell.hpp'))).digest('hex'),hashes,rows};
for(const[file,data]of [['docs/math-port/evidence/p50-regexp-host-source.json',JSON.stringify(manifest,null,2)+'\n'],['tests/math/reference_scanner_regexp.cpp',native]]){const target=path.join(root,file);if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),data);else fs.writeFileSync(target,data);}
console.log(JSON.stringify({source_cases:rows.length}));
