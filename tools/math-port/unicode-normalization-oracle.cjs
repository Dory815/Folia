// Root original String.normalize('NFD') observations, Unicode 16.0 host profile.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),source=fs.readFileSync(path.join(root,'.reference/unicode16/NormalizationTest.txt'));
const hash=crypto.createHash('sha256').update(source).digest('hex');
assert.equal(hash,'d811971453e7075e1ad56fb1b301eece5aa80757b81f6156e74a1bfb3ae5ceb1');
assert.equal(process.versions.unicode,'16.0');
const from=x=>String.fromCodePoint(...x.trim().split(' ').filter(Boolean).map(x=>parseInt(x,16)));
const hex=s=>Array.from({length:s.length},(_,i)=>s.charCodeAt(i).toString(16).padStart(4,'0')).join('');
const literal=s=>'std::u16string_view(u"'+Array.from({length:s.length},(_,i)=>'\\x'+s.charCodeAt(i).toString(16).padStart(4,'0')).join('')+'",'+s.length+')';
const rows=[];
for(const line of source.toString().split(/\r?\n/)){
 const content=line.split('#')[0].trim();if(!content||content.startsWith('@'))continue;
 const fields=content.split(';').slice(0,5),inputs=fields.map(from),outputs=inputs.map(x=>x.normalize('NFD'));
 assert.equal(fields.length,5);for(let i=0;i<5;++i)assert.equal(outputs[i],inputs[i<3?2:4]);
 rows.push({inputs,outputs});
}
const extras=['','abc','é','Å','Å','ά','𝔸','ﬁ','\u212b\u0315\u0300','a\u0315\u0300','\u0315\u0300','a\u0301\u0300','a\u034f\u0315\u0300','a\u200d\u0315\u0300','\ud800','\udc00','\ud800é\udc00','a\u0315\ud800\u0300','😀é𝔸','\ud800\ud800\udc00\udc00','\u0000é\u0000'];
let state=0x514e4644;const random=()=>{state^=state<<13;state^=state>>>17;state^=state<<5;return state>>>0;};
const pool=[...'aAéÅάᾂÅ한각𝔸😀'].map(x=>x).concat(['\u0315','\u0300','\u0323','\u034f','\u0301','\ud800','\udc00','\u200d','\ufe0f']);
for(let i=0;i<64;++i){let input='';for(let j=0;j<40;++j)input+=pool[random()%pool.length];extras.push(input);}
const extraRows=extras.map(input=>({input,output:input.normalize('NFD')}));
const planeDigests=[];for(let plane=0;plane<=16;++plane){let digest=0xcbf29ce484222325n;const update=x=>{digest=BigInt.asUintN(64,(digest^BigInt(x))*0x100000001b3n);};for(let cp=plane*65536;cp<(plane+1)*65536;++cp){const output=String.fromCodePoint(cp).normalize('NFD');update(cp);update(output.length);for(let i=0;i<output.length;++i)update(output.charCodeAt(i));}planeDigests.push(digest.toString(16));}
let fixture=String.raw`// Generated only from original Node NFD; official corpus and full-plane digests.
#include "support/folia_test.hpp"
import folia.math.util.unicode_normalization;
namespace {using namespace boost::ut;using folia::math::Unicode16Normalization;
struct Row {std::array<std::u16string_view,5> inputs,outputs;};
const Row corpus[]={
`;
fixture+=rows.map(row=>'{ {'+row.inputs.map(literal).join(',')+'}, {'+row.outputs.map(literal).join(',')+'} }').join(',\n')+'\n};\n';
fixture+='suite unicode_normalization=[] {\n';
fixture+='"math_port_nfd_official_corpus"_test=[] {Unicode16Normalization host;for(std::size_t i=0;i<std::size(corpus);++i)for(std::size_t j=0;j<5;++j){try{expect(host.nfd(corpus[i].inputs[j])==corpus[i].outputs[j])<<"corpus row "<<i<<" column "<<j;}catch(const std::exception&error){expect(false)<<"corpus row "<<i<<" column "<<j<<" threw "<<error.what();}}};\n';
for(const[i,row]of extraRows.entries())fixture+='"math_port_nfd_sequence_'+i+'"_test=[] {Unicode16Normalization host;try{expect(host.nfd('+literal(row.input)+')=='+literal(row.output)+');}catch(const std::exception&error){expect(false)<<error.what();}};\n';
for(const[plane,digest]of planeDigests.entries())fixture+='"math_port_nfd_plane_'+plane+'"_test=[] {Unicode16Normalization host;std::uint64_t digest=0xcbf29ce484222325ULL;auto update=[&](std::uint32_t value){digest=(digest^value)*0x100000001b3ULL;};try{for(std::uint32_t cp='+plane*65536+';cp<'+(plane+1)*65536+';++cp){std::u16string input;if(cp<0x10000)input.push_back(static_cast<char16_t>(cp));else{auto offset=cp-0x10000;input.push_back(static_cast<char16_t>(0xd800+(offset>>10)));input.push_back(static_cast<char16_t>(0xdc00+(offset&1023)));}auto output=host.nfd(input);update(cp);update(static_cast<std::uint32_t>(output.size()));for(auto unit:output)update(unit);}expect(eq(digest,std::uint64_t(0x'+digest+'ULL)));}catch(const std::exception&error){expect(false)<<error.what();}};\n';
fixture+='};}\n';
const manifest=JSON.stringify({stage:'original NFD host observations; not French math-style algorithm acceptance',node:process.version,icu:process.versions.icu,unicode:process.versions.unicode,corpus_sha256:hash,corpus_rows:rows.length,corpus_comparisons:rows.length*5,extra_rows:extraRows.map(x=>({input:hex(x.input),output:hex(x.output)})),plane_digests:planeDigests,plane_codepoints:17*65536},null,2)+'\n';
for(const[file,data]of [['tests/math/reference_unicode_normalization.cpp',fixture],['docs/math-port/evidence/p54-unicode-normalization-source.json',manifest]]){const target=path.join(root,file);if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),data);else fs.writeFileSync(target,data);}
console.log(JSON.stringify({corpus_rows:rows.length,corpus_comparisons:rows.length*5,sequence_cases:extraRows.length,planes:planeDigests.length,all_codepoints:17*65536}));
