// Root reproducible data-only successor; no production engine adoption.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),source=path.join(root,'.reference/regress'),prior=path.join(root,'build/math-port-regexp-unicode-escape-source'),data=path.join(root,'build/math-port-unicode16');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex'),read=f=>fs.readFileSync(f),lf=f=>read(f).toString().replaceAll('\r\n','\n');
cp.execFileSync(process.execPath,[path.join(__dirname,'regexp-unicode-escape-prepare.cjs')]);
for(const[name,pin]of Object.entries({'UCD.zip':'c86dd81f2b14a43b0cc064aa5f89aa7241386801e35c59c7984e579832634eb2','emoji-sequences.txt':'3fe3c77e72e8f26df302dc7d99b106c5d08fd808ef7246fb5d4502d659fe659c','emoji-zwj-sequences.txt':'9423ec235474356f970a696506737e2d5d65453a67f45df66b8bbe920c3fab83'}))assert.equal(hash(read(path.join(data,name))),pin,name+' primary data identity');
assert.match(lf(path.join(data,'ucd/ReadMe.txt')),/16\.0\.0/);
// Verify extracted generator inputs against the pinned archive, not just its filename.
const zipCheck=cp.spawnSync('powershell.exe',['-NoProfile','-Command',`Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive=[System.IO.Compression.ZipFile]::OpenRead($env:FOLIA_UCD_VERIFY_ZIP)
$sha=[System.Security.Cryptography.SHA256]::Create()
$count=0
try { foreach($entry in $archive.Entries) { if($entry.Name.Length -eq 0) { continue }
 $file=Join-Path $env:FOLIA_UCD_VERIFY_ROOT $entry.FullName
 $stream=$entry.Open()
 try { $expected=[BitConverter]::ToString($sha.ComputeHash($stream)) } finally { $stream.Dispose() }
 $actual=[BitConverter]::ToString($sha.ComputeHash([IO.File]::ReadAllBytes($file)))
 if($actual -ne $expected) { throw "Extracted UCD input differs: $($entry.FullName)" };$count++
} } finally { $archive.Dispose();$sha.Dispose() }
if($count -lt 50) { throw 'Incomplete UCD archive' };Write-Output $count`],{env:{...process.env,FOLIA_UCD_VERIFY_ZIP:path.join(data,'UCD.zip'),FOLIA_UCD_VERIFY_ROOT:path.join(data,'ucd')},encoding:'utf8',timeout:60000});assert.equal(zipCheck.status,0,zipCheck.stderr||'All UCD entries equal pinned archive');
for(const name of ['emoji-sequences.txt','emoji-zwj-sequences.txt'])assert.ok(read(path.join(data,name)).equals(read(path.join(data,'ucd',name))));
assert.equal(hash(read(path.join(source,'gen-unicode/src/main.rs'))),'f420ac195dc831b3aadf9ba65cbcac5047d6974b8f07c1cdd617ceb7fff20bd0');
const names=cp.execFileSync('git',['-C',source,'ls-files','-z'],{encoding:'utf8'}).split('\0').filter(Boolean),all=[...names,'src/legacy_case.rs','Cargo.lock'];
const protectedBytes=new Map();for(const dir of [source,prior])for(const name of all){const file=path.join(dir,name);if(fs.existsSync(file))protectedBytes.set(file,read(file));}
function adaptedMain(){let main=lf(path.join(source,'gen-unicode/src/main.rs'));
 main=main.replace('"/tmp/ucd-17.0.0"',JSON.stringify(path.join(data,'ucd').split(path.sep).join('/')));
 main=main.replace('"../tests/unicode_property_escapes.rs"',JSON.stringify(path.join(data,'generated_unicode_property_escapes.rs').split(path.sep).join('/')));
 main=main.replaceAll(/std::process::Command::new\("cargo"\)\s*\.arg\("fmt"\)\s*\.arg\("--"\)/g,'std::process::Command::new("rustfmt")');
 assert.equal((main.match(/Command::new\("rustfmt"\)/g)||[]).length,2);assert.ok(!main.includes('Command::new("cargo")'));return main;
}
function prepare(dest){const fresh=!fs.existsSync(dest);if(fresh){for(const name of all){const file=path.resolve(dest,name);assert.ok(file.startsWith(dest+path.sep));fs.mkdirSync(path.dirname(file),{recursive:true});fs.copyFileSync(path.join(prior,name),file);}fs.writeFileSync(path.join(dest,'gen-unicode/src/main.rs'),adaptedMain());}
 assert.equal(lf(path.join(dest,'gen-unicode/src/main.rs')),adaptedMain(),'Only path/output/formatter adaptation');
 for(const name of all.filter(n=>!['src/unicodetables.rs','gen-unicode/src/main.rs'].includes(n)))assert.ok(read(path.join(dest,name)).equals(read(path.join(prior,name))),'Unchanged '+name);
 if(fresh){const p=cp.spawnSync('cargo',['run','--locked','--offline','--manifest-path','Cargo.toml','-p','gen-unicode'],{cwd:path.join(dest,'gen-unicode'),env:{...process.env,CARGO_TARGET_DIR:path.join(root,'build/math-port-regexp-unicode16-generator-target')},encoding:'utf8',timeout:180000});fs.writeFileSync(path.join(data,path.basename(dest)+'-generation.log'),(p.stdout||'')+(p.stderr||''));assert.equal(p.status,0,'Unchanged generator must finish');}
 assert.equal(hash(read(path.join(dest,'src/unicodetables.rs'))),'475cb03589f0deb859b9a2ccfb34e3f33026a1e89500524024ce575a135e5daf');
 assert.equal(hash(read(path.join(data,'generated_unicode_property_escapes.rs'))),'2931a8d6826a522e39b14a1454ae625d819d99fc8b855fbeeccb2ca5e7573fa3');
}
prepare(path.join(root,'build/math-port-regexp-unicode16-source'));if(process.argv.includes('--fresh-check'))prepare(path.join(root,'build/math-port-regexp-unicode16-fresh-check'));
for(const[file,bytes]of protectedBytes)assert.ok(bytes.equals(read(file)),'Protected original/predecessor '+file);
const manifest={stage:'verified Unicode 16 data-only candidate successor; no engine adoption',original_generator_sha256:hash(read(path.join(source,'gen-unicode/src/main.rs'))),adapted_generator_sha256:hash(read(path.join(root,'build/math-port-regexp-unicode16-source/gen-unicode/src/main.rs'))),generator_modules:Object.fromEntries(names.filter(n=>n.startsWith('gen-unicode/src/')&&n!=='gen-unicode/src/main.rs').map(n=>[n,hash(read(path.join(source,n)))])),source_unicode17_table_sha256:hash(read(path.join(source,'src/unicodetables.rs'))),generated_unicode16_table_sha256:hash(read(path.join(root,'build/math-port-regexp-unicode16-source/src/unicodetables.rs'))),generated_ignored_tests_sha256:hash(read(path.join(data,'generated_unicode_property_escapes.rs'))),primary_data:Object.fromEntries(['UCD.zip','emoji-sequences.txt','emoji-zwj-sequences.txt'].map(n=>[n,hash(read(path.join(data,n)))])),ucd_archive_entries_verified:Number(zipCheck.stdout.trim())};
const evidence=path.join(root,'docs/math-port/evidence/p21c-unicode16-generation.json');if(process.argv.includes('--check'))assert.deepEqual(JSON.parse(read(evidence)),manifest);else fs.writeFileSync(evidence,JSON.stringify(manifest,null,2)+'\n');
console.log('Unicode 16 data-only successor verified; original and predecessor unchanged.');
