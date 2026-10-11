// Root constant Unicode data, independent of RegExp matching expectations.
// Experimental generated output stays under ignored build/. No VM at runtime.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
assert.equal(process.version,'v24.13.0');assert.equal(process.versions.unicode,'16.0');assert.equal(process.versions.icu,'77.1');
const root=path.resolve(__dirname,'../..'),pairs=[],canonical=[];let expansions=0,asciiGuard=0;
for(let cp=0;cp<=0xffff;++cp){
 const upper=String.fromCharCode(cp).toUpperCase();let target=cp;
 if(upper.length!==1)++expansions;
 else if(cp>=128&&upper.charCodeAt(0)<128)++asciiGuard;
 else target=upper.charCodeAt(0);
 canonical.push(target);if(target!==cp)pairs.push([cp,target]);
}
for(const[,target]of pairs)assert.equal(canonical[target],target,'Canonical targets are fixed points');
const payload=crypto.createHash('sha256').update(JSON.stringify(pairs)).digest('hex');
const output=`// Root generated Node 24.13.0 / Unicode 16.0 / ICU 77.1 uppercase data.
// Full uppercase UTF16-length restriction and non-ASCII-to-ASCII restriction.
// Payload SHA256: ${payload}. Experimental, not an adopted engine.
pub(crate) const MAPPINGS: &[(u32,u32)] = &[
${pairs.map(([a,b])=>`    (${a},${b}),`).join('\n')}
];
pub(crate) fn canonicalize(unit:u32)->u32 {
    match MAPPINGS.binary_search_by_key(&unit, |(from,_)|*from) {
        Ok(i)=>MAPPINGS[i].1,
        Err(_)=>unit,
    }
}
`;
const file=path.join(root,'build/math-port-regexp-casefold/src/legacy_case.rs');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),output);
else{fs.mkdirSync(path.dirname(file),{recursive:true});fs.writeFileSync(file,output);}
console.log(JSON.stringify({mapped_units:pairs.length,full_expansions:expansions,ascii_guard:asciiGuard,payload_sha256:payload,mode:process.argv.includes('--check')?'check':'generate'}));
