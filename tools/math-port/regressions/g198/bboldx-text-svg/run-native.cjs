'use strict';
// Feed default cases to Renderer and explicit TeX options to NativeRenderSession. Does not write captures.
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../../../../..'),cases=JSON.parse(fs.readFileSync(path.join(__dirname,'cases.json'),'utf8')).cases;
const exe=process.argv[2];assert.ok(exe,'usage: node run-native.cjs <FoliaMathBboldxTextSvgProbe.exe>');
const b64=s=>Buffer.from(s,'utf8').toString('base64');const groups=['continuous','fresh-session-per-case'];
const input=groups.map(group=>'@group\t'+b64(group)+'\n'+cases.map((c,i)=>[i,b64(c.name),b64(c.tex),c.options?'bboldx-light':'default','0',16,1280].join('\t')).join('\n')+'\n').join('');
const run=cp.spawnSync(path.resolve(exe),[],{cwd:root,input,encoding:'utf8',maxBuffer:64*1024*1024,timeout:120000,windowsHide:true});assert.equal(run.status,0,run.stderr||run.error?.message||`probe exited ${run.status}`);const result=JSON.parse(run.stdout);assert.deepEqual(result.groups,groups);assert.equal(result.records.length,cases.length*2);process.stdout.write(JSON.stringify(result));
