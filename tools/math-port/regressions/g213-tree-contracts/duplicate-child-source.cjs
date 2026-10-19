'use strict';
const assert=require('node:assert/strict');
const crypto=require('node:crypto');
const fs=require('node:fs');
const path=require('node:path');
const cjs=path.resolve(process.cwd(),'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const pkg=require(path.resolve(cjs,'../package.json'));
assert.equal(pkg.version,'4.1.3');
const commit='fb987178c2d279a99b0db5ea02e751691703955e';
const hashes={
 'core/Tree/Node.js':'94174004B89F19BF3C98E8EEC5FABDD0BCBF1E13E0B740963FA69554AD8A12A7',
 'core/MmlTree/MmlNode.js':'86C5EB572DCF2652C3C52A60B66C720D231E17F600CC9AEEF3ACE4AEAFC81D45',
 'core/MmlTree/MmlFactory.js':'D0C4E25237D04A9D21D12EE44776E46D30777E7F284DB6F4F3ED1CC625B752D3',
 'core/MmlTree/MmlNodes/mrow.js':'5846D2B0334223B759A4059DFB0AC5E31AEF1F5A96C041DA45B3DE94C0689896',
};
for(const [file,expected] of Object.entries(hashes)) assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(cjs,file))).digest('hex').toUpperCase(),expected,file+' source hash mismatch');
const {MmlFactory}=require(path.join(cjs,'core/MmlTree/MmlFactory.js'));
const factory=new MmlFactory();
const row=factory.create('mrow');
const sourceArray=[{}];
const foreignChild=sourceArray[0];
row.childNodes=sourceArray;
const returned=row.appendChild(foreignChild);
assert.strictEqual(returned,foreignChild);
assert.strictEqual(row.childNodes,sourceArray);
assert.equal(row.childNodes.length,2);
assert.strictEqual(row.childNodes[0],foreignChild);
assert.strictEqual(row.childNodes[1],foreignChild);
assert.equal(row.childNodes.indexOf(foreignChild),0);
assert.strictEqual(foreignChild.parent,row);
const copySource=factory.create('mrow');
const marker=Symbol('copy-key');
copySource.properties[marker]='symbol-value';
const copied=copySource.copy();
assert.equal(copied.properties[marker],'symbol-value');
console.log(JSON.stringify({version:pkg.version,pinnedCommit:commit,sourceHashes:hashes,
 duplicateAppend:{sameRawArray:row.childNodes===sourceArray,length:row.childNodes.length,firstOccurrenceIndex:row.childNodes.indexOf(foreignChild),parentIsRow:foreignChild.parent===row},
 symbolPropertyCopy:{ownSymbolValue:copied.properties[marker],symbols:Object.getOwnPropertySymbols(copied.properties).length}
},null,2));
