// Root inventory of this explicitly bounded projection, never full file acceptance.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),source=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0/src/shaders/radial_gradient.rs');
const bytes=fs.readFileSync(source),hash=b=>crypto.createHash('sha256').update(b).digest('hex');assert.equal(hash(bytes),'3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c');
const text=bytes.toString('utf8');assert.deepEqual(text.match(/\/\*[\s\S]*?\*\//g),['/*unused*/']);
const masked=text.replace(/\/\/[^\r\n]*/g,m=>' '.repeat(m.length)).replace(/\/\*[\s\S]*?\*\//g,m=>m.replace(/[^\r\n]/g,' ')),all=[];
for(const match of masked.matchAll(/\bfn\s+(\w+)\s*\(/g)){
 const start=match.index,open=masked.indexOf('{',start);let depth=1,end=open+1;assert.ok(open>=0);
 for(;depth&&end<masked.length;end++){if(masked[end]==='{')++depth;else if(masked[end]==='}')--depth;}assert.equal(depth,0);
 all.push({name:match[1],line:text.slice(0,start).split('\n').length,end_line:text.slice(0,end).split('\n').length,member_sha256:hash(text.slice(start,end))});
}
assert.deepEqual(all.map(m=>m.name),['set','is_focal_on_circle','is_well_behaved','is_natively_focal','new','new_radial_unchecked','push_stages','create','map_to_unit_x','ts_from_poly_to_poly','from_poly2']);
const selected=all.filter(m=>['is_focal_on_circle','is_well_behaved','is_natively_focal','push_stages'].includes(m.name));assert.equal(selected.length,4);
const target=m=>'third_party/mermaid-rs-renderer/src/scene/prepare/radial_stages.rs:'+(m.name==='push_stages'?'prepare_radial_stages':m.name);
const ledger={schema:1,status:'locally_accepted_radial_stage_projection_only',source:'tiny-skia-0.12.0/src/shaders/radial_gradient.rs',source_sha256:hash(bytes),members:selected.map(m=>({...m,target:target(m),review:'root_review_passed'})),outside_projection:['Gradient::push_stages seed/transform/stops/colors/premultiply wrapper','runtime context mask values','stage sampling kernels','final whole-pipeline precision selection','SVG/Direct2D paint and pixels'],adaptation:'Only radial p0/p1, ordered pre/post-color stages and radial-only lowp eligibility. No complete Source file or backend acceptance.'};
const file=path.join(root,'docs/mermaid-native/evidence/m3f-radial-stages-members.json');
if(process.argv.includes('--freeze-reviewed'))fs.writeFileSync(file,JSON.stringify(ledger,null,2)+'\n');
assert.deepEqual(JSON.parse(fs.readFileSync(file)),ledger);for(const m of ledger.members)assert.ok(fs.existsSync(path.join(root,m.target.split(':')[0])));
console.log(JSON.stringify({mapped_methods:4,status:'pinned_source_stage_projection_matches'}));
