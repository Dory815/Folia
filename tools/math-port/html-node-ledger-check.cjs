// Root acceptance must retain all seven pinned HtmlNode members.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const {root,source}=require('./html-node-reference.cjs');
const fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'member-ledger.cjs'),source],{encoding:'utf8'}));
const accepted=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p15d-html-members.json'),'utf8'));
assert.equal(fresh.files.length,1);assert.equal(accepted.files.length,1);
const f=fresh.files[0],a=accepted.files[0];assert.equal(a.source,f.source);assert.equal(a.source_sha256,f.source_sha256);assert.equal(a.members.length,f.members.length);assert.equal(a.members.length,7);
f.members.forEach((m,i)=>{const entry=a.members[i];for(const [key,v]of Object.entries(m))if(key!=='target'&&key!=='review')assert.deepEqual(entry[key],v,key);assert.equal(entry.review,'root_review_passed');assert.ok(entry.target);assert.equal(entry.evidence,'docs/math-port/evidence/p15d-html-node-acceptance.md');});
console.log(JSON.stringify({members:a.members.length,source_files:1,status:'root_reviewed_inventory_matches_pinned_source'}));
