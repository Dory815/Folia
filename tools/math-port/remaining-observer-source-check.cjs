// Root frozen original calculations and data, independent of Native adapters.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),hash=s=>crypto.createHash('sha256').update(s).digest('hex');
const frozen=[
 ['mo-order','4d3687c6c1dd120ed3743b710b4570064f40880eaaec9cade38b285d55b452cb',50,538],
 ['arithmetic','6c06f2317b75b689ce1dbc49cc0ba236220b518715934e46a52f322d15e9a342',7,13501],
 ['arithmetic-order','627cf1539387ae4adab2e2bbf894b33a9202c69f58f3f151ae50853819888f56',22,56],
 ['table','b8047a5f0cc76c80ffbca22e5a86c2b9fec18f6488b9d3bdf59055d82bfd0875',135,4700,'0abb4b14efc0785fa8d0d945229eab088a49956925a817f4fba3c945ef0ebefc','4713bb24fa19f07cf98c366b82a3409df3172ffa4d32ae3348ac7ea6c6196994',2],
 ['atom-semantics','38cf61ed7a418da5f4f722752a3a363e310a31d7f302b198f3aa7acf2ef9f8ff',74,6888,'9414b6ad0f2e01980cae9c75d31caedf8a0740a74151ad9910a53a4ec4716552','ac9060e1bd5d4f60c5696a26c0b2d61a844b0bcc3f24359dffbd0cd953ded388',2],
 ['multiscript','955bdcad6e78208adfe87b6f0de157a8c33f3125da3eb079df2086a67673a4ca',226,1304,'19ca975aba5dd2b301e5be6626e3d5100c60c88f9658e6d8aa9fd4c7e12b5bc1','e8d245697830ade8fa4020c7bca6c9b9ff1da4b53a844638d8aa3c89b1e2bb0a',1],
];
for(const[name,prefix,cases,assertions,values,rows,rowCount]of frozen){
 const oracle=path.join(__dirname,name+'-oracle.cjs'),source=fs.readFileSync(oracle,'utf8').replaceAll('\r\n','\n'),end=source.indexOf('const out=[');assert.ok(end>0);assert.equal(hash(source.slice(0,end)),prefix,'Original Source prefix '+name);
 const fixture=fs.readFileSync(path.join(root,'tests/math/reference_'+name.replaceAll('-','_')+'.cpp'),'utf8').replaceAll('\r\n','\n');
 if(values){const v=fixture.match(/namespace\s*\{\s*const std::vector<Property> sourceValues=\[\][\s\S]*?return values;\}\(\);\s*\}/);assert.ok(v);assert.equal(hash(v[0]),values,'Original values '+name);const r=[...fixture.matchAll(/struct (?:Row|Rule)\{[\s\S]*?\};for\(/g)];assert.equal(r.length,rowCount);assert.equal(hash(r.map(x=>x[0]).join('\n')),rows,'Original raw rows '+name);}
 if(name==='arithmetic'){const r=fixture.match(/const std::vector<Property> attrPool=[\s\S]*?\};for\(const auto&r:rows\)/);assert.ok(r);assert.equal(hash(r[0]),'9750b3f83cd59462d518b20f8eb30ed906940631010dd2c3d59f7d30becce87d','Original arithmetic pools and 1440 rows');}
 const replay=cp.spawnSync(process.execPath,[oracle,'--check'],{encoding:'utf8',maxBuffer:1024*1024});assert.equal(replay.status,0,'Original replay '+name+' '+replay.stderr);const profile=JSON.parse(replay.stdout.trim());assert.equal(profile.cases,cases);assert.equal(profile.assertions,assertions);console.log(name+' '+replay.stdout.trim());
}
console.log('Six original prefixes, complete replay profiles, value tables and raw matrices unchanged.');
