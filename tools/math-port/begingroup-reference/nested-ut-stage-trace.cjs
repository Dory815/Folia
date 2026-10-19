'use strict';
const path=require('node:path');
const root=path.resolve(__dirname,'../../..');
const pkg=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const load=n=>require(path.join(pkg,n));
load('input/tex/base/BaseConfiguration.js');
load('input/tex/newcommand/NewcommandConfiguration.js');
load('input/tex/begingroup/BegingroupConfiguration.js');
const TeX=load('input/tex.js').TeX;
const TexParser=load('input/tex/TexParser.js').default;
const {MmlFactory}=load('core/MmlTree/MmlFactory.js');
const {begingroupStack}=load('input/tex/begingroup/BegingroupStack.js');
const {NewcommandTables}=load('input/tex/newcommand/NewcommandUtil.js');
const {MapHandler}=load('input/tex/MapHandler.js');
function snap(jax){const s=begingroupStack(jax.parseOptions);return {i:s.i,top:s.top,base:s.base,global:Object.fromEntries(Object.entries(s.global).map(([k,m])=>[k,{name:m.name,priority:m.priority,foo:m.lookup('\\bgLocal'),global:m.lookup('\\bgGlobal')}]))};}
function node(n){return n&&{kind:n.kind,text:n.getText?.()??null,children:(n.childNodes||[]).map(node)};}
function run(jax,input,label,out){const po=jax.parseOptions;po.clear();const before=snap(jax);try{const n=new TexParser(input,{},po).mml();out.push({label,input,before,after:snap(jax),tree:node(n),text:n.getText?.()??null,error:null});}catch(e){out.push({label,input,before,after:snap(jax),tree:null,error:{name:e.name,id:e.id??null,message:String(e.message)}});}}
const jax=new TeX({packages:['base','begingroup','newcommand']});jax.setMmlFactory(new MmlFactory());
const rows=[];
run(jax,String.raw`\newcommand{\bgLocal}{z}\begingroup\renewcommand{\bgLocal}{x}\begingroup\renewcommand{\bgLocal}{y}\bgLocal\endgroup\bgLocal\endgroup\bgLocal`,'local-shadow',rows);
run(jax,String.raw`\begingroup\global\def\bgGlobal{x}\endgroup\bgGlobal`,'global-definition',rows);
run(jax,String.raw`\bgGlobal`,'global-after-scope',rows);
console.log(JSON.stringify({rows,final:snap(jax)},null,2));

const repeated=[];
run(jax,String.raw`\begingroup\newcommand{\bgShared}{x}`,'leave-group-open',repeated);
run(jax,String.raw`\bgShared`,'repeated-formula-shared-definition',repeated);
run(jax,String.raw`\endgroup`,'close-group',repeated);
run(jax,String.raw`\bgShared`,'after-group-removed',repeated);
console.log(JSON.stringify({repeated},null,2));
const NodeUtil=load('input/tex/NodeUtil.js').default;
const jaxText=new TeX({packages:['base','begingroup','newcommand']});jaxText.setMmlFactory(new MmlFactory());
const textNode=new TexParser(String.raw`\newcommand{\bgLocal}{z}\begingroup\renewcommand{\bgLocal}{x}\begingroup\renewcommand{\bgLocal}{y}\bgLocal\endgroup\bgLocal\endgroup\bgLocal`,{},jaxText.parseOptions).mml();
console.log(JSON.stringify({nodeKind:textNode.kind,nodeGetTextType:typeof textNode.getText,children:textNode.childNodes?.map(x=>({kind:x.kind,textType:typeof x.getText,text:typeof x.getText==='function'?x.getText():null})),nodeUtilError:(()=>{try{return NodeUtil.getText(textNode)}catch(e){return {name:e.name,message:e.message}}})()}));


