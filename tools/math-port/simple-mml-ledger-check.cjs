// Root verifies every accepted P16a member against pinned TypeScript AST.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const {root,hashes}=require('./simple-mml-reference.cjs');
const fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'member-ledger.cjs'),...hashes.map(h=>h.source)],{encoding:'utf8'}));
const accepted=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p16a-simple-mml-members.json')));
assert.equal(accepted.files.length,5);let count=0;
fresh.files.forEach((f,i)=>{const a=accepted.files[i];assert.equal(a.source,f.source);assert.equal(a.source_sha256,f.source_sha256);assert.equal(a.members.length,f.members.length);f.members.forEach((m,j)=>{const entry=a.members[j];for(const[k,v]of Object.entries(m))if(k!=='target'&&k!=='review')assert.deepEqual(entry[k],v,k);assert.equal(entry.review,'root_review_passed');assert.ok(entry.target);assert.equal(entry.evidence,'docs/math-port/evidence/p16a-simple-mml-acceptance.md');++count;});});assert.equal(count,22);console.log(JSON.stringify({source_files:5,members:count,status:'root_reviewed_inventory_matches_pinned_source'}));
