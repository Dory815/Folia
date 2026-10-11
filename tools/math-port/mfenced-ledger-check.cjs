// Root verifies the accepted ten-member ledger against pinned TypeScript.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const {root,source}=require('./mfenced-reference.cjs');
const fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'member-ledger.cjs'),source],{encoding:'utf8'}));
const accepted=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p16c-mfenced-members.json')));
assert.equal(accepted.files.length,1);const actual=fresh.files[0],record=accepted.files[0];
assert.equal(record.source,actual.source);assert.equal(record.source_sha256,actual.source_sha256);assert.equal(record.members.length,10);assert.equal(actual.members.length,10);
for(let i=0;i<actual.members.length;++i){const entry=record.members[i];for(const[k,v]of Object.entries(actual.members[i]))if(k!=='target'&&k!=='review')assert.deepEqual(entry[k],v,k);assert.equal(entry.review,'root_review_passed');assert.ok(entry.target);assert.equal(entry.evidence,'docs/math-port/evidence/p16c-mfenced-acceptance.md');}
console.log(JSON.stringify({source_files:1,members:10,status:'root_reviewed_inventory_matches_pinned_source'}));
