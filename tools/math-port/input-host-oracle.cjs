// Root-owned native host expectations. Never imports native implementation.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
assert.equal(process.version,'v24.13.0','Pinned reference runtime changed');
const root=path.resolve(__dirname,'../..');
const out=['// Root-generated Node 24.13.0 input host expectations. Do not edit.',
'#include "support/folia_test.hpp"','import folia.math.util.js_numeric_input;',
'import folia.math.util.utf16;','namespace ni=folia::math::js_input;',
'namespace u16=folia::math::utf16;','using namespace boost::ut;',
'namespace { std::u16string units(std::initializer_list<unsigned> v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}',
'bool exact(double n,std::uint64_t bits){auto e=std::bit_cast<double>(bits);return std::isnan(e)?std::isnan(n):std::bit_cast<std::uint64_t>(n)==bits;}',
'std::string bytes(std::initializer_list<unsigned> v){std::string s;for(auto x:v)s.push_back(static_cast<char>(x));return s;} }',
'suite math_port_input_host_tests=[] {'];
let assertions=0;const counts={};
const units=s=>'units({'+Array.from({length:s.length},(_,i)=>s.charCodeAt(i)).join(',')+'})';
const bytes=a=>'bytes({'+Array.from(a).join(',')+'})';
const bits=n=>{const b=Buffer.alloc(8);b.writeDoubleLE(n);return '0x'+b.readBigUInt64LE().toString(16).padStart(16,'0')+'ULL';};
let state=0x79429317;function random(){state=(Math.imul(state,1664525)+1013904223)>>>0;return state;}
const ws=[9,10,11,12,13,32,160,5760,8192,8193,8194,8195,8196,8197,8198,8199,8200,8201,8202,8232,8233,8239,8287,12288,65279];
const numbers=['',' ','\0','-0','+0','-0e999999','-0e-999999','Infinity','+Infinity','-Infinity','inf','NaN','infinity','1.','.1','+.5','-.5','1e+2','1e','1e-','1_2','1n','1px','0x','0x1','0XfF','-0x1','+0x1','0o10','0b11','0b12','0o8','0x1g','0x1.2','0x1p2','0o','0b','1e309','-1e309','1e-9999','-1e-9999','2.2250738585072014e-308','2.2250738585072012e-308','4.9406564584124654e-324','2.4703282292062327e-324','2.4703282292062328e-324','1.7976931348623157e308','1.7976931348623159e308','9007199254740993','9007199254740995','1.00000000000000011102230246251565404236316680908203125','1.00000000000000011102230246251565404236316680908203126','9'.repeat(500),'-0.'+'0'.repeat(500)+'1'];
for(const c of ws)numbers.push(String.fromCharCode(c)+'-2.5'+String.fromCharCode(c));
for(const c of [0,133,6158,8203,55296,56320,65535])numbers.push(String.fromCharCode(c)+'1','1'+String.fromCharCode(c));
for(const exponent of [51,52,53,54,63,64,1022,1023,1024,1100])for(const delta of [-3n,-2n,-1n,0n,1n,2n,3n]){
 const n=(1n<<BigInt(exponent))+delta;
 for(const [prefix,radix]of [['0x',16],['0o',8],['0b',2]])numbers.push(prefix+n.toString(radix));
}
numbers.push('0x'+'0'.repeat(1500)+'20000000000001','00000000000000001','-0000.000e9999','000.e-9999');
numbers.push('0.'+'0'.repeat(600)+'1e600','1'+'0'.repeat(600)+'e-600',
 '-0.'+'0'.repeat(600)+'1e1000','-1'+'0'.repeat(600)+'e-1000',
 '1e'+'9'.repeat(500),'-1e-'+'9'.repeat(500));
for(let i=0;i<400;i++){let digits='';for(let j=0,n=1+random()%100;j<n;j++)digits+=random()%10;const cut=random()%(digits.length+1);numbers.push((random()%2?'-':'+')+digits.slice(0,cut)+'.'+digits.slice(cut)+'e'+(Number(random()%1401)-700));}
for(const [prefix,radix]of [['0x',16],['0o',8],['0b',2]])for(let i=0;i<100;i++){let digits='';for(let j=0,n=1+random()%1200;j<n;j++)digits+='0123456789abcdef'[random()%radix];numbers.push(prefix+digits);}
for(let start=0;start<numbers.length;start+=24){out.push(`"math_port_input_host_number_${start}"_test=[] {`);
 for(const s of numbers.slice(start,start+24)){out.push(`expect(exact(ni::number(${units(s)}),${bits(Number(s))}));`);assertions++;}
 out.push('};');
}counts.number=numbers.length;
const ints=['',' ','-0','-0junk','+12px','-12.9','0x10','-0x10','0Xff','0b11','0o11','08','010','123e4','zZ','Infinity','0x','0xg','1_2','9007199254740993','9007199254740995','-9'.repeat(2),'9'.repeat(500)];
ints.push('0'.repeat(1500)+'9007199254740993!', '-0'.replace('0','0'.repeat(1500))+'!', '0x'+'0'.repeat(1500)+'20000000000001!');
for(const exponent of [52,53,54,63,64,1023,1024])for(const delta of [-1n,0n,1n,2n,3n]){
 const n=(1n<<BigInt(exponent))+delta;
 ints.push(n.toString(10),'0x'+n.toString(16),n.toString(8));
}
for(const c of ws)ints.push(String.fromCharCode(c)+'-012junk');
for(const c of [0,133,6158,8203,55296])ints.push(String.fromCharCode(c)+'12');
const integerLines=[];
const radixName={0:'Automatic',8:'Octal',10:'Decimal',16:'Hex'};
for(const s of ints)for(const radix of [0,8,10,16]){integerLines.push(`expect(exact(ni::parseInt(${units(s)},ni::IntegerRadix::${radixName[radix]}),${bits(parseInt(s,radix))}));`);assertions++;}
for(let i=0;i<300;i++){const radix=[8,10,16][random()%3];let digits='';for(let j=0,n=1+random()%1000;j<n;j++)digits+='0123456789abcdef'[random()%radix];const s=(random()%2?'-':'')+digits+'!';integerLines.push(`expect(exact(ni::parseInt(${units(s)},ni::IntegerRadix::${radixName[radix]}),${bits(parseInt(s,radix))}));`);assertions++;}
for(let start=0;start<integerLines.length;start+=24){out.push(`"math_port_input_host_parseInt_${start}"_test=[] {`,...integerLines.slice(start,start+24),'};');}
out.push('"math_port_input_host_parseInt_native_radix_bound"_test=[] {for(int r:{-1,2,36,37}){bool rejected=false;try{(void)ni::parseInt(u"12",static_cast<ni::IntegerRadix>(r));}catch(const std::invalid_argument&){rejected=true;}expect(rejected);}};');assertions+=4;
counts.parseInt=ints.length*4+300;
const strings=['','a','a\0b','\uD800','\uDC00','\uD800\uDC00','\uD800x\uDC00','\uDC00\uD800','\uD800\uD800\uDC00','\uDBFF\uDFFF','e\u0301','a😀中'];
for(let i=0;i<100;i++){let s='';for(let j=0,n=random()%20;j<n;j++)s+=String.fromCharCode(random()%65536);strings.push(s);}
out.push('"math_port_input_host_utf16_iteration_encoding"_test=[] {');
for(const s of strings){out.push(`{auto s=${units(s)};`);for(let i=0;i<=s.length+1;i++){const v=s.codePointAt(i);out.push(v===undefined?`expect(!u16::codePointAt(s,${i}).has_value());`:`expect(bool(u16::codePointAt(s,${i})==std::optional<char32_t>{${v}}));`);assertions++;}out.push(`expect(bool(u16::codePoints(s)==std::vector<char32_t>{${Array.from(s,c=>c.codePointAt(0)).join(',')}}));`,`expect(bool(u16::encode(s)==${bytes(new TextEncoder().encode(s))}));`,'}');assertions+=2;}
out.push('};');counts.strings=strings.length;
const valid=['','abc','a\0b','\u007f\u0080\u07ff\u0800\ud7ff\ue000\uffff','a😀中','\u{10000}\u{10ffff}','e\u0301','\ufeffx'];
for(let i=0;i<60;i++){let s='';for(let j=0,n=random()%20;j<n;j++){let cp=random()%0x110000;if(cp>=0xd800&&cp<=0xdfff)cp=0xfffd;s+=String.fromCodePoint(cp);}valid.push(s);}
out.push('"math_port_input_host_utf8_decode_offsets"_test=[] {');
for(const s of valid){const b=new TextEncoder().encode(s);assert.equal(new TextDecoder('utf-8',{fatal:true,ignoreBOM:true}).decode(b),s);const offsets=[];let at=0;for(const c of s){for(let j=0;j<c.length;j++)offsets.push(at);at+=new TextEncoder().encode(c).length;}offsets.push(at);out.push(`{auto v=u16::decode(${bytes(b)});expect(bool(v.text==${units(s)}));expect(bool(v.byteOffsets==std::vector<std::size_t>{${offsets.join(',')}}));}`);assertions+=2;}
out.push('};');counts.decode=valid.length;
const invalid=[[0x80],[0xbf],[0xc0,0x80],[0xc1,0xbf],[0xc2],[0xc2,0x20],[0xe0,0x80,0x80],[0xe0,0x9f,0xbf],[0xed,0xa0,0x80],[0xed,0xbf,0xbf],[0xe1,0x80],[0xe1,0x80,0x20],[0xf0,0x80,0x80,0x80],[0xf0,0x8f,0xbf,0xbf],[0xf4,0x90,0x80,0x80],[0xf5,0x80,0x80,0x80],[0xff],[0xfe],[0xf1,0x80,0x80],[0xf1,0x80,0x80,0x20]];
out.push('"math_port_input_host_utf8_invalid"_test=[] {');
for(const tail of invalid)for(const prefix of [[],[0x61],[0xf0,0x9f,0x98,0x80]]){const b=Uint8Array.from([...prefix,...tail]);assert.throws(()=>new TextDecoder('utf-8',{fatal:true}).decode(b));out.push(`{bool rejected=false;try{(void)u16::decode(${bytes(b)});}catch(const u16::DecodeError& e){rejected=true;expect(bool(e.byteOffset==${prefix.length}));}expect(rejected);}`);assertions+=2;}
out.push('};','};');counts.invalid=invalid.length*3;
const target=path.join(root,'tests/math/reference_input_host.cpp'),output=out.join('\n')+'\n';
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output,'Frozen input host fixtures changed');else fs.writeFileSync(target,output);
console.log(JSON.stringify({node:process.version,assertions,counts,mode:process.argv.includes('--check')?'check':'generate'}));
