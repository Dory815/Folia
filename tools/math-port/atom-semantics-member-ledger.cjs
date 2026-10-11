// Root inventory includes the effective prototype assignment, not just the class stub.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),cp=require('node:child_process'),assert=require('node:assert/strict');
const {root,hashes}=require('./atom-semantics-reference.cjs');
const ledger=JSON.parse(cp.execFileSync(process.execPath,[path.join(root,'tools/math-port/member-ledger.cjs'),...hashes.map(h=>h.source)],{encoding:'utf8'}));
const ts=require(path.join(root,'build/math-port-tooling/node_modules/typescript'));assert.equal(ts.version,'5.9.3');
const item=hashes.find(h=>h.name==='TeXAtom'),ast=ts.createSourceFile(item.source,fs.readFileSync(path.join(root,'.reference/MathJax-src',item.source),'utf8'),ts.ScriptTarget.Latest,true);
const assignments=ast.statements.filter(s=>ts.isExpressionStatement(s)&&ts.isBinaryExpression(s.expression)&&s.expression.operatorToken.kind===ts.SyntaxKind.EqualsToken);
assert.equal(assignments.length,1);const node=assignments[0],expression=node.expression;assert.equal(expression.left.getText(ast),'TeXAtom.prototype.adjustTeXclass');assert.equal(expression.right.getText(ast),'MmlMo.prototype.adjustTeXclass');
ledger.files[0].module_bindings=[{kind:'prototype_assignment',name:expression.left.getText(ast),line:ast.getLineAndCharacterOfPosition(node.getStart(ast)).line+1,member_sha256:crypto.createHash('sha256').update(node.getText(ast)).digest('hex'),target:null,review:'unreviewed'}];
ledger.status='unreviewed_member_inventory_with_module_binding';console.log(JSON.stringify(ledger,null,2));
