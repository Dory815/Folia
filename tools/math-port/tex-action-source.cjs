'use strict';
// Original ActionConfiguration and actual TeX/MML classes, development only.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for(const [file,hash]of [
 ['.reference/MathJax-src/ts/input/tex/action/ActionConfiguration.ts','0e647387a28c524bd2d4b7a57758b78bfc95555297874ecf3e46da8a1d163b1d'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/action/ActionConfiguration.js','cfe9cf324fcda305c163911f99ab30c884d03a1dacaaa6c3da1422da60b2f6fb']
])assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),hash,file);
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {NodeFactory}=load('input/tex/NodeFactory.js');
const {ActionMethods,ActionConfiguration}=load('input/tex/action/ActionConfiguration.js');
const BaseMethods=load('input/tex/base/BaseMethods.js').default;
assert.deepEqual(Object.keys(ActionMethods),['Toggle','Mathtip','Macro']);
assert.equal(ActionMethods.Macro===BaseMethods.Macro,true);
assert.deepEqual(ActionConfiguration.handler.macro,['action-macros']);
let cases=0;
function fixture(aliases=false,selected=true){
 let error='',message='';
 const jax=new TeX({packages:['base',...(selected?['action']:[]),...(aliases?['newcommand']:[])],formatError:(j,e)=>{error=e.id;message=e.message;return j.formatError(e)}});
 jax.setMmlFactory(new MmlFactory());
 return {jax,compile(source){++cases;error='';message='';const tree=jax.compile(new AbstractMathItem(source,jax,false),reference.document);return {tree,error,message}}};
}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n)});return result;}
function text(root){const result=[];root.walkTree(n=>{if(n.isToken)result.push(n.getText())});return result.join('');}
function ok(f,source){const result=f.compile(source);assert.equal(result.error,'',source);return result.tree;}
{
 const f=fixture(),map=f.jax.parseOptions.handlers.retrieve('action-macros');
 for(const [command,method]of [['toggle','Toggle'],['mathtip','Mathtip'],['texttip','Macro']])assert.equal(map.lookup(command).func===ActionMethods[method],true);
 assert.deepEqual(map.lookup('texttip').args,['\\mathtip{#1}{\\text{#2}}',2]);
 for(const [source,children]of [
  ['\\toggle{a}{b}{c}\\endtoggle',['a','b','c']],
  ['\\toggle a b\\endtoggle',['a','b']],
  ['\\toggle{x}{\\endtoggle}',['x']],
  ['\\toggle{}{}{x}\\endtoggle',['','','x']],
  ['\\toggle\\endtoggle',['']],['\\toggle{\\endtoggle}',['']]
 ]){const a=nodes(ok(f,source),'maction');assert.equal(a.length,1);assert.equal(a[0].attributes.get('actiontype'),'toggle');assert.deepEqual(a[0].childNodes.map(text),children);}
 assert.equal(fixture(false,false).compile('\\mathtip{x}{tip}').error,'UndefinedControlSequence');
}
{
 const f=fixture();
 for(const [source,children]of [
  ['\\mathtip{x+y}{z^2}',['x+y','z2']],['\\mathtip{}{}',['','']],
  ['\\texttip{x}{plain text}',['x','plain text']]
 ]){const actions=nodes(ok(f,source),'maction');assert.equal(actions.length,1);assert.equal(actions[0].attributes.get('actiontype'),'tooltip');assert.deepEqual(actions[0].childNodes.map(text),children);}
 const root=ok(f,'\\toggle{\\mathtip{x}{y}}{\\texttip{z}{tip}}\\endtoggle');
 assert.deepEqual(nodes(root,'maction').map(n=>n.attributes.get('actiontype')),['toggle','tooltip','tooltip']);
 assert.deepEqual(nodes(root,'mtext').map(n=>n.getText()),['tip']);
 const nested=ok(f,'\\mathtip{\\toggle{x}{y}\\endtoggle}{\\mathtip{a}{b}}');
 assert.deepEqual(nodes(nested,'maction').map(n=>n.attributes.get('actiontype')),['tooltip','toggle','tooltip']);
}
{
 const f=fixture(),root=ok(f,'\\mathbf{\\toggle{\\mathit{x}}{y}\\endtoggle z}');
 assert.deepEqual(nodes(root,'mi').map(n=>[n.getText(),n.attributes.get('mathvariant')]),[['x','-tex-mathit'],['y','bold'],['z','bold']]);
 const tip=ok(f,'\\mathbf{\\mathtip{x}{y}}');
 assert.deepEqual(nodes(tip,'mi').map(n=>[n.getText(),n.attributes.get('mathvariant')]),[['x','bold'],['y','bold']]);
}
{
 const f=fixture(),requests=[];
 f.jax.parseOptions.nodeFactory.set('node',(factory,kind,children,def,txt)=>{if(kind==='maction')requests.push({children:[...children],def:{...def}});return NodeFactory.createNode(factory,kind,children,def,txt)});
 const root=ok(f,'\\toggle{x}{y}\\endtoggle+\\mathtip{a}{b}');
 const actions=nodes(root,'maction');assert.equal(requests.length,2);
 assert.deepEqual(requests.map(r=>r.def),[{actiontype:'toggle'},{actiontype:'tooltip'}]);
 assert.deepEqual(requests.map(r=>r.children.map(text)),[['x','y'],['a','b']]);
 for(let i=0;i<2;++i)for(let j=0;j<2;++j)assert.equal(requests[i].children[j]===actions[i].childNodes[j],true);
 const empty=nodes(ok(f,'\\toggle\\endtoggle'),'maction');assert.equal(requests.length,3);assert.equal(requests[2].children.length,0);assert.equal(empty[0].childNodes.length,1);
}
const errors=[
 ['\\toggle','MissingArgFor'],['\\toggle{x}','MissingArgFor'],
 ['\\toggle{\\unknown}\\endtoggle','UndefinedControlSequence'],
 ['\\toggle{\\endtoggle extra}\\endtoggle','UndefinedControlSequence'],
 ['\\toggle{{x}\\endtoggle','MissingCloseBrace'],
 ['\\mathtip','MissingArgFor'],['\\mathtip{x}','MissingArgFor'],
 ['\\mathtip{\\unknown}{tip}','UndefinedControlSequence'],
 ['\\mathtip{x}{\\unknown}','UndefinedControlSequence'],['\\texttip{x}','MissingArgFor'],
 ['\\endtoggle','UndefinedControlSequence']
];
for(const [source,id]of errors){const f=fixture();assert.equal(f.compile(source).error,id,source);assert.equal(nodes(ok(f,'\\mathtip{x}{y}'),'maction').length,1);}
{
 const f=fixture(true);ok(f,'\\let\\savedToggle\\toggle\\let\\savedTip\\mathtip\\let\\savedText\\texttip');
 const root=ok(f,'\\savedToggle{\\savedTip{x}{y}}{\\savedText{z}{tip}}\\endtoggle');
 assert.deepEqual(nodes(root,'maction').map(n=>n.attributes.get('actiontype')),['toggle','tooltip','tooltip']);
 const result=f.compile('\\savedToggle{x}');assert.equal(result.error,'MissingArgFor');assert.equal(result.message,'Missing argument for \\savedToggle');
 ok(f,'\\savedTip{x}{y}');
}
console.log(JSON.stringify({methods:2,baseAliases:1,commands:3,cases,errorCases:errors.length,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
