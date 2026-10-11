// Root-generated accessor; algorithmic methods are a separate constrained packet.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,HtmlNode,MmlFactory,source,sha256}=require('./html-node-reference.cjs');
const ts=require(path.join(root,'build/math-port-tooling/node_modules/typescript'));assert.equal(ts.version,'5.9.3');
const ast=ts.createSourceFile(source,fs.readFileSync(path.join(root,'.reference/MathJax-src',source),'utf8'),ts.ScriptTarget.Latest,true);
const cls=ast.statements.filter(ts.isClassDeclaration);assert.equal(cls.length,1);
const getter=cls[0].members.filter(ts.isGetAccessorDeclaration);assert.equal(getter.length,1);assert.equal(getter[0].name.text,'kind');
const f=new MmlFactory();f.setNodeClass('html',HtmlNode);const kind=f.create('html').kind;assert.equal(kind,'html');
const output=`// Generated pinned ${source}; Apache-2.0.\n// Source SHA256 ${sha256}\nmodule folia.math.core.mml_tree.mml_nodes.html_node;\nimport std;\nnamespace folia::math::mml {std::string HtmlNode::kind()const{return ${JSON.stringify(kind)};}}\n`;
const target=path.join(root,'src/math/core/mml_tree/mml_nodes/html_node_bindings.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
console.log(JSON.stringify({accessors:1,mode:process.argv.includes('--check')?'check':'generate'}));
