'use strict';
// Pinned MathJax 4.1.3 own-key witness for TexParser/TextParser construction.
// This observes the real constructors; it does not recreate their field writes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../../..');
const bundle=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const cjs=path.join(bundle,'cjs');
const packageJson=JSON.parse(fs.readFileSync(path.join(bundle,'package.json'),'utf8'));
assert.equal(packageJson.version,'4.1.3','pinned MathJax version');
const expectedHashes={
  'input/tex/TexParser.js':'106d7ff9298dce899d046339eab1996358f3efca3a7ce66f5ca915747dc9c6a8',
  'input/tex/textmacros/TextParser.js':'ec2fa0145f5e90ef231b023a38ed7d9c1064a8cc58a46bb683d46439bd8fb400'
};
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
for(const [file,expected] of Object.entries(expectedHashes))assert.equal(hash(path.join(cjs,file)),expected,`${file} pinned CJS hash`);
const evidence=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/textmacros-package-original-source.json'),'utf8'));
assert.equal(hash(path.join(root,'docs/math-port/scope.json')),evidence.scope_sha256,'Source scope hash');
for(const [file,expected] of Object.entries(expectedHashes)){
  const lock=evidence.modules.find(item=>item.cjs===file);assert.ok(lock,`${file} evidence record`);
  assert.equal(lock.cjs_sha256,expected,`${file} evidence SHA`);
}
const {TeX}=require(path.join(cjs,'input/tex.js'));
const {MmlFactory}=require(path.join(cjs,'core/MmlTree/MmlFactory.js'));
require(path.join(cjs,'input/tex/textmacros/TextMacrosConfiguration.js'));
const {TextParser}=require(path.join(cjs,'input/tex/textmacros/TextParser.js'));
const jax=new TeX({packages:['base','textmacros']});jax.setMmlFactory(new MmlFactory());
const parser=new TextParser('',{},jax.parseOptions);
const ownKeys=Reflect.ownKeys(parser);
const expectedOwnKeys=['_string','configuration','macroCount','i','currentCS','saveI','stack','text','nodes','envStack','level'];
assert.deepEqual(ownKeys,expectedOwnKeys,'actual TexParser/TextParser own-key initialization order');
console.log(JSON.stringify({version:packageJson.version,cjsHashes:expectedHashes,ownKeys}));
