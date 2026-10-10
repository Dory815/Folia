// Root-owned expected data, sampled only from pinned upstream/Node.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const source='ts/util/lengths.ts',hash='213f65e9a8fa6d30bda4df4a6c0cc3a1e8d50567267539d85a1f19c1c20cea73';
const bytes=fs.readFileSync(path.join(root,'.reference/MathJax-src',source));
assert.equal(crypto.createHash('sha256').update(bytes).digest('hex'),hash);
const npmRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
assert.equal(require(path.join(npmRoot,'package.json')).version,'4.1.3');
assert.equal(fs.readFileSync(path.join(npmRoot,source),'utf8').replaceAll('\r\n','\n'),bytes.toString('utf8').replaceAll('\r\n','\n'));
const lengths=require(path.join(npmRoot,'cjs/util/lengths.js'));
const quote=s=>'std::string('+JSON.stringify(s).replace(/[\u007f-\uffff]/g,c=>'\\u'+c.charCodeAt(0).toString(16).padStart(4,'0'))+')';
const bits=v=>{const b=Buffer.alloc(8);b.writeDoubleLE(v);return b.readBigUInt64LE().toString(16).padStart(16,'0');};
const number=v=>`from_bits(0x${bits(v)}ULL)`;
const result=v=> {
  if(typeof v==='number') return `Property(${number(v)})`;
  const builtins={constructor:Object,__defineGetter__:Object.prototype.__defineGetter__,__defineSetter__:Object.prototype.__defineSetter__,hasOwnProperty:Object.prototype.hasOwnProperty,__lookupGetter__:Object.prototype.__lookupGetter__,__lookupSetter__:Object.prototype.__lookupSetter__,isPrototypeOf:Object.prototype.isPrototypeOf,propertyIsEnumerable:Object.prototype.propertyIsEnumerable,toString:Object.prototype.toString,valueOf:Object.prototype.valueOf,toLocaleString:Object.prototype.toLocaleString,prototype:Object.prototype};
  const names={__defineGetter__:'defineGetter',__defineSetter__:'defineSetter',__lookupGetter__:'lookupGetter',__lookupSetter__:'lookupSetter'};
  const name=Object.keys(builtins).find(k=>builtins[k]===v); assert.ok(name,'Unmapped upstream return');
  return `Property(BuiltinProperty::${Object.hasOwn(names,name)?names[name]:name})`;
};
const lines=['// Root-generated from pinned MathJax lengths.ts and Node Number. Do not hand-edit.',
'#include "support/folia_test.hpp"','import folia.math.core.tree.property;','import folia.math.util.js_number;','import folia.math.util.lengths;',
'using namespace folia::math;','using namespace boost::ut;','namespace {',
'double from_bits(std::uint64_t n) { return std::bit_cast<double>(n); }',
'bool number_equal(double a,double b) { if (std::isnan(b)) return std::isnan(a); if (std::isinf(b)) return a==b; if (b==0) return a==0 && std::signbit(a)==std::signbit(b); return std::isfinite(a) && std::abs(a-b)<=1e-10+1e-10*std::abs(b); }',
'bool property_equal(const Property& a,const Property& b) { if (auto n=std::get_if<double>(&b.value)) { auto m=std::get_if<double>(&a.value); return m && number_equal(*m,*n); } return a==b; }','}',
'suite math_port_numeric_reference_tests=[] {'];
let checks=0;
const cases=[0,-0,NaN,Infinity,-Infinity,Number.MIN_VALUE,-Number.MIN_VALUE,Number.MAX_VALUE,-Number.MAX_VALUE,
1,1.5,-1.5,2.5,-2.5,0.5,-0.5,0.125,-0.125,2.55,1.005,2.675,0.0005,0.001,-0.001,0.1,-0.1,0.0000001,0.000001,1e20,1e21,-1e21,1000000000000000128];
const buffer=Buffer.alloc(8);
for(const x of [0.001,0.1,1e-6,1e21,1e20,Number.MIN_VALUE]) {buffer.writeDoubleLE(x);const n=buffer.readBigUInt64LE();for(const d of [-1n,1n]){buffer.writeBigUInt64LE(n+d);cases.push(buffer.readDoubleLE());}}
let seed=0x46f01a67bca13dn;
for(let i=0;i<2048;i++) {seed=BigInt.asUintN(64,seed*6364136223846793005n+1442695040888963407n);buffer.writeBigUInt64LE(seed);cases.push(buffer.readDoubleLE());}
lines.push('"math_port_number_format_reference"_test=[] {',
'struct Row { std::uint64_t bits; unsigned digits; const char* shortest; const char* fixed; };',
'const Row rows[]={');
for(let i=0;i<cases.length;i++) {
  const x=cases[i],precisions=i<44?[0,1,2,3,17,50,100]:[i%101,1,3];
  for(const digits of new Set(precisions)) {lines.push(`{0x${bits(x)}ULL,${digits},${JSON.stringify(String(x))},${JSON.stringify(x.toFixed(digits))}},`);checks+=2;}
}
lines.push('};','for (const auto& row:rows) { const double x=from_bits(row.bits); expect(bool(js_number::toString(x)==row.shortest)) << row.shortest; expect(bool(js_number::toFixed(x,row.digits)==row.fixed)) << row.fixed; }',
'bool caught=false; try { (void)js_number::toFixed(from_bits(0x7ff8000000000000ULL),101); } catch(const std::out_of_range&) { caught=true; } expect(caught);','};');checks++;
lines.splice(lines.length-1,0,'expect(bool(js_number::toFixed(-0.5)=="-1"));');checks++;
lines.push('"math_port_lengths_reference"_test=[] {');
lines.push(`expect(property_equal(lengths::length2em("1em"),${result(lengths.length2em('1em'))}));`);
lines.push(`expect(property_equal(lengths::length2em(1.0),${result(lengths.length2em(1))}));`);
lines.push(`expect(bool(lengths::px(1.0)==${quote(lengths.px(1))}));`);checks+=3;
for(const table of ['UNITS','RELUNITS','MATHSPACE']) {const entries=Object.entries(lengths[table]);lines.push(`expect(bool(lengths::${table}.size()==${entries.length}));`);checks++;entries.forEach(([key,val],i)=>{lines.push(`expect(bool(lengths::${table}[${i}].first==${JSON.stringify(key)})); expect(number_equal(lengths::${table}[${i}].second,${number(val)}));`);checks+=2;});}
const strings=['','0','-0','1','-1','+1','.5','1.','+.5','-.0','.','+','-','1e3','1E-3','0x10','2 em','em','%', ' pt', '  junk','1.5unknown','PX','EM','Infinity','infinity','NaN','1px2', '1pt trailing','-.5%'];
for(const key of Object.keys(lengths.MATHSPACE)) strings.push(key);
for(const key of Object.getOwnPropertyNames(Object.prototype)) strings.push(key);
for(const unit of ['pt','em','ex','mu','px','pc','in','mm','cm','%']) for(const n of ['','0','-0','2.5','-.5'])strings.push(n+unit);
for(const n of [9,10,11,12,13,32,160,5760,8192,8193,8194,8195,8196,8197,8198,8199,8200,8201,8202,8232,8233,8239,8287,12288,65279,6158,8203]) strings.push(String.fromCodePoint(n)+'2em');
strings.push('9'.repeat(400)+'px','-0.'+'0'.repeat(400)+'1px');
for(const text of strings) for(const args of [[0,1,16],[2,1.3,19],[-2,0,0],[NaN,1,16]]) {
  lines.push(`expect(property_equal(lengths::length2em(${quote(text)},${args.map(number).join(',')}),${result(lengths.length2em(text,...args))})) << ${quote(text)};`);checks++;
}
for(const x of cases.slice(0,44)) for(const args of [[2,1,16],[0,0,0]]) {lines.push(`expect(property_equal(lengths::length2em(${number(x)},${args.map(number).join(',')}),${result(lengths.length2em(x,...args))}));`);checks++;}
for(const x of cases.slice(0,256)) {
  lines.push(`expect(bool(lengths::percent(${number(x)})==${quote(lengths.percent(x))})); expect(bool(lengths::em(${number(x)})==${quote(lengths.em(x))}));`);checks+=2;
  for(const args of [[-1000000,16],[0,16],[1.5,1.3],[NaN,0]]) {lines.push(`expect(bool(lengths::px(${number(x)},${args.map(number).join(',')})==${quote(lengths.px(x,...args))}));`);checks++;}
}
lines.push('};','};');
const output=lines.join('\n')+'\n',target=path.join(root,'tests/math/reference_numeric.cpp');
if(process.argv.includes('--check')) assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output,'Frozen numeric fixtures changed');
else fs.writeFileSync(target,output);
console.log(JSON.stringify({source,hash,number_samples:cases.length,assertions:checks,node:process.version,mode:process.argv.includes('--check')?'check':'generate'}));
