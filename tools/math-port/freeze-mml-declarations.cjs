// Root one-time sole API freeze. No implementation or compatibility overload.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),base=path.join(root,'src/math/core/mml_tree'),changed=[];
if(process.argv.includes('--check')){
 const files=['mml_node.ixx','operator_class.ixx',...['maction','mathchoice','menclose','mfenced','mfrac','mi','mo','mpadded','mroot','mrow','mspace','msqrt','mstyle','mtable','mtd','mtr','semantics','tex_atom'].map(x=>'mml_nodes/'+x+'.ixx')];
 for(const f of files){const text=fs.readFileSync(path.join(base,f),'utf8');assert.doesNotMatch(text,/MmlNode\*\s+(Parent|core|coreMO|setTeXclass|adjustTeXclass)\s*\(|bool\s+notParent\s*\(|(inheritAttributesFrom|updateTeXclass|getPrevClass)\(MmlNode\*\)/,f);assert.ok(text.includes('NodeValue')||text.includes('Property notParent'),f);}
 assert.ok(fs.readFileSync(path.join(base,'mml_node.ixx'),'utf8').includes('using NodeValue = tree::ChildValue<MmlNode>;'));
 console.log(JSON.stringify({sole_declaration_files:files.length,scope:'binding check only; no semantic acceptance'}));return;
}
assert.ok(!fs.readFileSync(path.join(base,'mml_node.ixx'),'utf8').includes('using NodeValue ='),'Do not overwrite an already frozen interface');
function walk(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const file=path.join(dir,e.name);if(e.isDirectory())walk(file);else if(e.name.endsWith('.ixx')){
 const before=fs.readFileSync(file,'utf8');let text=before;
 text=text.replace(/MmlNode\*(\s+)(Parent|core|coreMO)\s*\(/g,'NodeValue$1$2(')
  .replace(/MmlNode\*(\s+)(setTeXclass|adjustTeXclass)\s*\(MmlNode\*\)/g,'NodeValue$1$2(NodeValue)')
  .replace(/bool(\s+)notParent\s*\(/g,'Property$1notParent(')
  .replace(/(inheritAttributesFrom|updateTeXclass|getPrevClass)\(MmlNode\*\)/g,'$1(NodeValue)')
  .replace('MmlNode* appendChild(MmlNode*)','NodeValue appendChild(NodeValue)')
  .replace('MmlNode* replaceChild(MmlNode*,MmlNode*)','NodeValue replaceChild(NodeValue,NodeValue)')
  .replace('MmlNode* adjustOperatorTeXclass(MmlNode& receiver, MmlNode* previous)','NodeValue adjustOperatorTeXclass(MmlNode& receiver, NodeValue previous)')
  .replace('coreText(MmlNode*)','coreText(NodeValue)');
 if(e.name==='mml_node.ixx')text=text.replace('using Children = tree::FactoryChildren<MmlNode>;','using Children = tree::FactoryChildren<MmlNode>;\nusing NodeValue = tree::ChildValue<MmlNode>;');
 if(text!==before){changed.push({file,text});}
 }}}walk(base);assert.equal(changed.length,20,'Exact Root declaration family');for(const{file,text}of changed)fs.writeFileSync(file,text);console.log(JSON.stringify({declaration_files:changed.length,changed:changed.map(x=>path.relative(root,x.file))}));
