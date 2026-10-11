// Root: preserve original numeric operations; bind original default std/simd profile.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),source=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0');
const dest=path.join(root,'third_party/mermaid-rs-renderer/src/scene/prepare/sampling/wide');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const manifest=path.join(root,'docs/mermaid-native/evidence/m3i2-numeric-provider-pins.json');
const entries=fs.readdirSync(path.join(source,'src/wide')).sort().filter(n=>n.endsWith('.rs'));
assert.equal(entries.length,9);
const pins=entries.map(name=>({name,source_sha256:hash(fs.readFileSync(path.join(source,'src/wide',name)))}));
const license=fs.readFileSync(path.join(source,'LICENSE'));
if(process.argv.includes('--check'))assert.deepEqual(JSON.parse(fs.readFileSync(manifest,'utf8')),{profile:'original std=true, simd=true, no-std-float=false; target_feature branches unchanged',license_sha256:hash(license),files:pins});
else{assert.ok(!fs.existsSync(manifest));fs.writeFileSync(manifest,JSON.stringify({profile:'original std=true, simd=true, no-std-float=false; target_feature branches unchanged',license_sha256:hash(license),files:pins},null,2)+'\n');}
for(const {name} of pins){
 const original=fs.readFileSync(path.join(source,'src/wide',name),'utf8');
 // cfg constants only. Original operation bodies and target_feature choices stay intact.
 const adapted=original.replaceAll('feature = "simd"','all()').replaceAll('feature = "std"','all()').replaceAll('feature = "no-std-float"','any()');
 const file=path.join(dest,name);
 if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),adapted.replaceAll('\r\n','\n'));
 else{assert.ok(!fs.existsSync(file));fs.mkdirSync(dest,{recursive:true});fs.writeFileSync(file,adapted);}
}
const licenseFile=path.join(dest,'LICENSE');
if(process.argv.includes('--check'))assert.ok(fs.readFileSync(licenseFile).equals(license));else fs.writeFileSync(licenseFile,license);
console.log('Nine original numeric Source files and BSD license verified; only three compile-profile cfg constants bound.');
