// Root durable guards: original computations and value records, with native emitters independent.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..'),hash=s=>crypto.createHash('sha256').update(s).digest('hex');
const frozen=[
 ['simple-mml','d4cf210f1ee7cac3e0cd2c2d713813d76d827307a36b18a6a2429fe27a458c29','08b5ba33439a0fd6c0639dedac19e31614b226105a4adbdacbdf281834064020'],
 ['mfenced','25808d4ecfa761aafbf61321ddc0c6e77a619ee9191328ec68e2790e89d47091','226b5161981fba0c567ede972e923dd8f3e9bf21b100072ae18d793a6c1025c4'],
 ['maction','90cc9fdb82726ce4ea8140165007b244649588f5ce6ebdcc39416f1384cb0c88','05ce7985818966da1b6c593cf5be069f5b417c707bd9bf21a372ed54a812e88f'],
];
for(const[name,prefixPin,tablePin]of frozen){const oracle=path.join(__dirname,name+'-oracle.cjs'),source=fs.readFileSync(oracle,'utf8').replaceAll('\r\n','\n'),marker=name==='maction'?'const header=`':'const header=[',end=source.indexOf(marker);assert.ok(end>0);assert.equal(hash(source.slice(0,end)),prefixPin,'Frozen original Source prefix '+name);const fixture=fs.readFileSync(path.join(root,'tests/math/reference_'+name.replace('-','_')+'.cpp'),'utf8').replaceAll('\r\n','\n'),start=fixture.indexOf('namespace {const std::vector<Property> sourceValues='),finish=fixture.indexOf('return values;}();}',start)+'return values;}();}'.length;assert.ok(start>=0&&finish>start);assert.equal(hash(fixture.slice(start,finish)),tablePin,'Frozen original value table '+name);const replay=cp.spawnSync(process.execPath,[oracle,'--check'],{encoding:'utf8',maxBuffer:1024*1024});assert.equal(replay.status,0,'Actual original replay '+name+' '+replay.stderr);console.log(replay.stdout.trim());}
console.log('All three unchanged original prefixes/value tables and concrete replay profiles pass.');
