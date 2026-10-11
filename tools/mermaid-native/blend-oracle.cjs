// Root Source rendering oracle, independent of native geometry/backend modules.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/mermaid-blend-reference');fs.mkdirSync(work,{recursive:true});
const modes=['normal','multiply','screen','overlay','darken','lighten','color-dodge','color-burn','hard-light','soft-light','difference','exclusion','hue','saturation','color','luminosity'];
const configs=[[[51,179,89,255],[204,38,166,255],1],[[220,54,102,255],[16,200,240,255],1],[[22,120,231,180],[231,78,16,143],.65],[[255,255,255,255],[128,128,128,255],1],[[0,0,0,255],[255,160,48,255],1],[[115,31,241,100],[14,233,88,240],.4],[[37,37,37,240],[172,172,172,77],.75],[[50,90,210,0],[200,70,120,170],1],[[50,90,210,170],[200,70,120,0],.5],[[50,90,210,50],[255,255,255,255],0],[[250,245,13,255],[1,10,250,255],.35],[[179,59,128,255],[17,233,94,255],.875]];
const input=modes.flatMap(mode=>configs.map(([background,foreground,opacity])=>({mode,background,foreground,opacity})));
const env={...process.env,CARGO_TARGET_DIR:path.join(root,'build/mermaid-blend-reference-target')},manifest=path.join(__dirname,'blend-reference/Cargo.toml');
const metadata=JSON.parse(cp.execFileSync('cargo',['metadata','--format-version','1','--locked','--offline','--manifest-path',manifest],{env,encoding:'utf8'}));
for(const[name,version,file,expected]of[
 ['usvg','0.47.0','src/tree/mod.rs','cb11738534ce989b8583827d55e3e01096831ad40075f7c8351365da7ece35b5'],
 ['tiny-skia','0.12.0','src/pipeline/highp.rs','81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416']]){
 const pkg=metadata.packages.find(p=>p.name===name&&p.version===version);assert.ok(pkg,'Pinned Source '+name);
 assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(path.dirname(pkg.manifest_path),file))).digest('hex'),expected,'Unchanged Source '+name+'/'+file);
}
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest],{env,encoding:'utf8'});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Source oracle must compile');
const exe=path.join(env.CARGO_TARGET_DIR,'debug/folia-blend-source-reference'+(process.platform==='win32'?'.exe':''));const run=cp.spawnSync(exe,[],{input:JSON.stringify(input),encoding:'utf8',timeout:30000});assert.equal(run.status,0,'Source oracle must finish');
const values=JSON.parse(run.stdout);assert.equal(values.length,input.length);assert.deepEqual(values.map(x=>x.input),input);for(const value of values)assert.equal(value.samples.length,4);
const target=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/direct2d-blends.json');
// One case per line keeps independent expected-value changes reviewable.
const text='{"schema":1,"stage":"source reference; no native output used","usvg":"0.47.0","resvg":"0.47.0","cases":[\n'+values.map(value=>JSON.stringify(value)).join(',\n')+'\n]}\n';
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),text);else fs.writeFileSync(target,text);
console.log(JSON.stringify({cases:values.length,dpi:4,pixel_samples:values.length*4*2,fixture_sha256:crypto.createHash('sha256').update(text).digest('hex'),mode:process.argv.includes('--check')?'check':'generate'}));
