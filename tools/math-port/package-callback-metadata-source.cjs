// Pinned MathJax 4.1.3 TeX configuration callback witness.
// Run from repository root: node tools/math-port/package-callback-metadata-source.cjs
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const {execFileSync} = require('node:child_process');
const packageRoot = path.resolve('tools/mathjax-bundle/node_modules/@mathjax/src');
const sourceRoot = path.resolve('.reference/MathJax-src');
const sourceCommit = 'fb987178c2d279a99b0db5ea02e751691703955e';
const scopeSha256 = '9705d389538b2fc8bf782feea4372ace639b3727304f2971937bbcfb0042a0d1';
assert.equal(execFileSync('git', ['-C', sourceRoot, 'rev-parse', 'HEAD'], {encoding:'utf8'}).trim(), sourceCommit);
const pkg = require(path.join(packageRoot, 'package.json'));
assert.equal(pkg.version, '4.1.3');
const texRoot = path.join(packageRoot, 'cjs/input/tex');
const configFiles = [];
(function walk(dir) { for (const entry of fs.readdirSync(dir,{withFileTypes:true})) { const file=path.join(dir,entry.name); if(entry.isDirectory()) walk(file); else if(entry.name.endsWith('Configuration.js')) configFiles.push(file); } })(texRoot);
for (const file of configFiles) require(file);
const {ParserConfiguration, ConfigurationHandler} = require(path.join(texRoot,'Configuration.js'));
const packageNames = fs.readdirSync('src/math/input/tex',{withFileTypes:true}).filter(e=>e.isDirectory()).map(e=>e.name).concat(['textmacros']).sort();
const callbackRecords=[];
for (const packageName of packageNames) {
  const config=ConfigurationHandler.get(packageName);
  if (!config) continue;
  const parser=new ParserConfiguration([packageName]);
  for (const [slot,list] of [['initMethod',parser.initMethod],['configMethod',parser.configMethod]]) {
    for (const {item,priority} of list.items) {
      const sourceCallback=slot==='initMethod'?config.init:config.config;
      callbackRecords.push({package:packageName,slot,priority,name:item.name,length:item.length,sameConfigurationCallback:item===sourceCallback});
      assert.equal(item,sourceCallback,`${packageName}.${slot} must retain Source callback identity`);
    }
  }
}
const recordsByPackage = new Map();
for (const record of callbackRecords) { const key=record.package; if(!recordsByPackage.has(key)) recordsByPackage.set(key,[]); recordsByPackage.get(key).push([record.slot,record.name,record.length]); }
const expected = {
  ams:[['configMethod','NewcommandConfig',2]], base:[['configMethod','',2]], bboldx:[['configMethod','',2]],
  begingroup:[['configMethod','',2]], color:[['configMethod','config',2]], colortbl:[['configMethod','config',2]],
  configmacros:[['initMethod','configmacrosInit',1],['configMethod','configmacrosConfig',2]],
  extpfeil:[['configMethod','NewcommandConfig',2]], mathtools:[['configMethod','configMathtools',2]],
  newcommand:[['configMethod','NewcommandConfig',2]], setoptions:[['configMethod','setoptionsConfig',2]],
  tagformat:[['configMethod','tagformatConfig',2]], texhtml:[['configMethod','',0]],
  textcomp:[['configMethod','',2]], textmacros:[['configMethod','',2]],
};
assert.equal(callbackRecords.length,16,'current native package callback count changed');
for(const [name, records] of Object.entries(expected)) assert.deepEqual(recordsByPackage.get(name),records,`${name} callback metadata changed`);
const relevantFiles = [
 'cjs/input/tex/Configuration.js','cjs/util/FunctionList.js','cjs/util/PrioritizedList.js',
 ...Object.keys(expected).map(name=>{const dirs={ams:'ams/AmsConfiguration.js',base:'base/BaseConfiguration.js',bboldx:'bboldx/BboldxConfiguration.js',begingroup:'begingroup/BegingroupConfiguration.js',color:'color/ColorConfiguration.js',colortbl:'colortbl/ColortblConfiguration.js',configmacros:'configmacros/ConfigMacrosConfiguration.js',extpfeil:'extpfeil/ExtpfeilConfiguration.js',mathtools:'mathtools/MathtoolsConfiguration.js',newcommand:'newcommand/NewcommandConfiguration.js',setoptions:'setoptions/SetOptionsConfiguration.js',tagformat:'tagformat/TagFormatConfiguration.js',texhtml:'texhtml/TexHtmlConfiguration.js',textcomp:'textcomp/TextcompConfiguration.js',textmacros:'textmacros/TextMacrosConfiguration.js'}; return 'cjs/input/tex/'+dirs[name];})
];
const hashes=Object.fromEntries(relevantFiles.map(relative=>[relative,crypto.createHash('sha256').update(fs.readFileSync(path.join(packageRoot,relative))).digest('hex')]));
const sourceLock=JSON.parse(fs.readFileSync('tools/math-port/common-layout-reference/profile-source-lock.json','utf8'));
assert.equal(sourceLock.scopeHash,scopeSha256,'the locked profile scope changed');
const lockedHashes=new Map(sourceLock.dependencies.map(({file,sha256})=>[file,sha256]));
for(const [relative,actual] of Object.entries(hashes)) {
  const expected=lockedHashes.get('src/'+relative);
  assert.ok(expected,`${relative} is not in the locked profile`);
  assert.equal(actual,expected,`${relative} changed`);
}
process.stdout.write(JSON.stringify({version:pkg.version,sourceCommit,scopeSha256,hashes,callbackRecords},null,2)+'\n');
