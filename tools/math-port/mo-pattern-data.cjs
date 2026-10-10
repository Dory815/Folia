// Root-generated immutable data, independently tested against executable CJS.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,specs}=require('./mo-pattern-reference.cjs');
const out=['// Generated source pattern data from pristine MathJax 4.1.3 mo.ts.',
'// Apache-2.0; see src/math/LICENSE-MathJax.txt. Root owns this file.',
'module folia.math.core.mml_tree.mo_patterns;','import std;','namespace folia::math::mml {namespace {'];
for(const [i,s]of specs.entries()){
 const units=[];for(let j=0;j<s.source.length;j++)units.push(s.source.charCodeAt(j));
 out.push('constexpr std::array<char16_t,'+units.length+'> source'+i+'={'+units.join(',')+'};');
 out.push('constexpr std::array<MoCodeUnitRange,'+s.ranges.length+'> ranges'+i+'={{'+s.ranges.map(([a,b])=>'{'+a+','+b+'}').join(',')+'}};');
}
out.push('const std::array<MoPatternSpec,5> specs={{');
for(const [i,s]of specs.entries())out.push('{std::u16string_view(source'+i+'.data(),source'+i+'.size()),"",ranges'+i+','+s.minimum+','+(s.maximum===undefined?'std::nullopt':s.maximum)+'},');
out.push('}};','}','const MoPatternSpec& moPatternSpec(MoPattern kind){const auto index=static_cast<std::size_t>(kind);if(index>=specs.size())host::typeError("Invalid native MmlMo pattern");return specs[index];}','}');
const output=out.join('\n')+'\n',target=path.join(root,'src/math/core/mml_tree/mo_pattern_data.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
console.log(JSON.stringify({patterns:specs.map(({name,minimum,maximum,ranges})=>({name,minimum,maximum,ranges:ranges.length})),mode:process.argv.includes('--check')?'check':'generate'}));
