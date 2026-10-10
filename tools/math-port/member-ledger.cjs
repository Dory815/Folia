// Root-owned source inventory, not a port or acceptance generator.
// Uses MathJax's pinned TypeScript development version to avoid regex omissions.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname,'../..');
const reference = path.join(root,'.reference/MathJax-src');
const ts = require(path.join(root,'build/math-port-tooling/node_modules/typescript'));
assert.equal(ts.version,'5.9.3');
assert.equal(require(path.join(reference,'package.json')).devDependencies.typescript,'^5.9.3');
const summary = process.argv.includes('--summary');
const inputs = process.argv.slice(2).filter(a=>a!=='--summary');
assert.ok(inputs.length,'Usage: node tools/math-port/member-ledger.cjs [--summary] ts/path/File.ts ...');
const hash = s=>crypto.createHash('sha256').update(s).digest('hex');
const results=[];
for (const input of inputs) {
  const absolute = path.resolve(reference,input);
  assert.ok(absolute.startsWith(reference+path.sep) && input.startsWith('ts/') && input.endsWith('.ts'),'Expected a reference TypeScript source');
  const bytes=fs.readFileSync(absolute);
  const source=ts.createSourceFile(input,bytes.toString('utf8'),ts.ScriptTarget.Latest,true,ts.ScriptKind.TS);
  assert.equal(source.parseDiagnostics.length,0,`Parse error in ${input}`);
  const members=[];
  const line=n=>source.getLineAndCharacterOfPosition(n).line+1;
  function context(node) {
    if (node.name) return node.name.getText(source);
    for (let p=node.parent;p;p=p.parent)
      if ((ts.isFunctionDeclaration(p)||ts.isVariableDeclaration(p)) && p.name) return p.name.getText(source)+'/<class>';
    return '<anonymous>';
  }
  function record(node,owner,type) {
    const text=node.getText(source);
    members.push({owner,kind:type,name:ts.isConstructorDeclaration(node)?'constructor':node.name.getText(source),
      static:!!node.modifiers?.some(m=>m.kind===ts.SyntaxKind.StaticKeyword),
      line:line(node.getStart(source)),end_line:line(node.end),member_sha256:hash(text),
      target:null,review:'unreviewed'});
  }
  function visit(node) {
    if (ts.isClassDeclaration(node)||ts.isClassExpression(node)) {
      const owner=context(node);
      for (const member of node.members) {
        if (ts.isConstructorDeclaration(member) && member.body) record(member,owner,'constructor');
        else if (ts.isMethodDeclaration(member) && member.body) record(member,owner,'method');
        else if (ts.isGetAccessorDeclaration(member) && member.body) record(member,owner,'get');
        else if (ts.isSetAccessorDeclaration(member) && member.body) record(member,owner,'set');
        else if (ts.isPropertyDeclaration(member)) record(member,owner,member.initializer?'initialized_field':'declared_field');
      }
    } else if (ts.isFunctionDeclaration(node) && node.body && node.name) record(node,'<module>','function');
    else if (ts.isVariableDeclaration(node) && node.initializer &&
      ts.isVariableDeclarationList(node.parent) &&
      ts.isVariableStatement(node.parent.parent) && ts.isSourceFile(node.parent.parent.parent))
      record(node,'<module>','initialized_global');
    ts.forEachChild(node,visit);
  }
  visit(source);
  results.push({source:input,source_sha256:hash(bytes),members});
}
if (summary) for (const result of results)
  console.log(`${result.source}: ${result.members.length} inventoried members (${result.members.filter(m=>['method','get','set','constructor','function'].includes(m.kind)).length} methods/accessors/constructors/functions)`);
else console.log(JSON.stringify({schema:1,typescript:ts.version,status:'unreviewed_member_inventory',files:results},null,2));
