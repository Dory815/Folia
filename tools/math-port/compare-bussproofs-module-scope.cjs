'use strict';
const fs=require('node:fs'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const nativePath=process.argv[2];
if(!nativePath)throw Error('Native Bussproofs module-scope capture required');
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const sourcePath='build/math-port/bussproofs-module-scope-original.json';
assert.equal(hash(sourcePath),'c8003603a905fbea8412c61831b4764a671b47670f5f6c6e1a20b9e5236a21ac');
assert.equal(hash('.reference/MathJax-src/ts/input/tex/bussproofs/BussproofsUtil.ts'),
 'f6e3f1f95cd8e0961f3ddbf28c4bf1d4f3f0b3c30e488f68ae125341014992bd');
assert.equal(hash('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/bussproofs/BussproofsUtil.js'),
 '8ed15ce9672065e5d4ae2e3f06703bbb456f836287fa1fb6a0aa45ecfb1a305d');
const source=JSON.parse(fs.readFileSync(sourcePath)),native=JSON.parse(fs.readFileSync(nativePath));
assert.deepEqual(native,source,'Complete module-scope observation differs');
assert.equal(JSON.stringify(native),JSON.stringify(source),'Source field order differs');
console.log(JSON.stringify({accepted:true,sourceSha256:hash(sourcePath),nativeSha256:hash(nativePath)}));
