'use strict';
// Supplemental branch observations for actual pinned MathJax CJS. The original
// 24-row AMS/Newcommand corpus remains untouched and independently locked.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const packageRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const pinned=JSON.parse(fs.readFileSync(path.join(root,'tools/math-port/ams-newcommand-reference/ams-newcommand-source.json'),'utf8'));
assert.equal(pinned.upstream?.commit,'fb987178c2d279a99b0db5ea02e751691703955e','unexpected original-corpus provenance');
for(const entry of pinned.sources){
  const tsFile=path.join(packageRoot,entry.source);
  const tsHash=crypto.createHash('sha256').update(fs.readFileSync(tsFile)).digest('hex');
  assert.equal(tsHash,entry.sha256,`${entry.source} lock differs from original corpus`);
  const file=path.join(packageRoot,'cjs',entry.cjs);
  const hash=crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
  assert.equal(hash,entry.cjs_sha256,`${entry.cjs} lock differs from original corpus`);
}
const load=name=>require(path.join(packageRoot,'cjs',name));
const {AmsConfiguration}=load('input/tex/ams/AmsConfiguration.js');
const {NewcommandConfiguration}=load('input/tex/newcommand/NewcommandConfiguration.js');
const {MapHandler}=load('input/tex/MapHandler.js');
const TeX=load('input/tex.js').TeX;
const {MmlFactory}=load('core/MmlTree/MmlFactory.js');
const AbstractMathItem=load('core/MathItem.js').AbstractMathItem;
function plain(value,seen=new WeakSet()){
 if(value===undefined)return {$:'undefined'};if(value===null)return null;
 if(typeof value==='number'){if(Number.isNaN(value))return {$:'NaN'};if(Object.is(value,-0))return {$:'-0'};if(!Number.isFinite(value))return {$:value<0?'-Infinity':'Infinity'};return value;}
 if(typeof value==='function')return {$:'function',name:value.name||'',length:value.length};
 if(typeof value!=='object')return value;if(seen.has(value))return {$:'cycle'};seen.add(value);
 if(Array.isArray(value)){const items=[];for(let i=0;i<value.length;i++)items.push(Object.hasOwn(value,i)?plain(value[i],seen):{$:'hole'});return {$:'array',items};}
 if(value instanceof Map)return {$:'map',entries:[...value].map(([k,v])=>[plain(k,seen),plain(v,seen)])};
 const out={};for(const k of Reflect.ownKeys(value)){const name=typeof k==='symbol'?`symbol:${k.description||''}`:k;out[name]=plain(value[k],seen);}return out;
}
function node(n){if(!n)return null;const result={kind:n.kind,attrs:plain(n.attributes?.getAllAttributes?.()??{}),properties:plain(n.getAllProperties?.()??{}),children:(n.childNodes||[]).map(node)};if(typeof n.getText==='function')result.text=n.getText();return result;}
function mapState(jax){const po=jax.parseOptions;const names=['new-Delimiter','new-Command','new-Environment'];return Object.fromEntries(names.map(name=>{const m=MapHandler.getMap(name);return[name,m?{sameRegistryMap:po.handlers.retrieve(name)===m,entries:plain(m.map)}:null];}));}
const cases=[
 ['newcommand-standard-name-redefinition',String.raw`\newcommand{\frac}{x}`],
 ['newcommand-missing-definition',String.raw`\newcommand{\zz}[2]`],
 ['newcommand-illegal-control-sequence-name',String.raw`\newcommand{bad!}{x}`],
 ['newcommand-empty-optional-default',String.raw`\newcommand{\zz}[2][]{#1/#2}\zz{q}`],
 ['newcommand-explicit-empty-optional',String.raw`\newcommand{\zz}[2][d]{#1/#2}\zz[]{q}`],
 ['def-delimited-argument-match',String.raw`\def\zz#1,#2;{#2/#1}\zz a,b;`],
 ['def-delimited-argument-mismatch',String.raw`\def\zz#1,#2;{#1}\zz a;c`],
 ['def-runaway-delimited-argument',String.raw`\def\zz#1,{#1}\zz abc`],
 ['def-missing-control-sequence',String.raw`\def foo{x}`],
 ['let-undefined-control-sequence',String.raw`\let\zz=\notDefined\zz`],
 ['let-existing-newcommand',String.raw`\newcommand{\orig}{x}\let\copy=\orig\copy`],
 ['let-existing-character-token',String.raw`\let\zz=\alpha\zz`],
 ['let-special-character',String.raw`\let\zz=^\zz x`],
 ['let-special-delimiter',String.raw`\let\paren=(\paren`],
 ['new-environment-omitted-optional-default',String.raw`\newenvironment{xx}[1][d]{(#1)}{!}\begin{xx}q\end{xx}`],
 ['new-environment-explicit-empty-optional',String.raw`\newenvironment{xx}[1][d]{(#1)}{!}\begin{xx}[]q\end{xx}`],
 ['new-environment-mismatched-end',String.raw`\newenvironment{xx}{a}{b}\begin{xx}x\end{yy}`],
 ['declare-math-operator-unstarred',String.raw`\DeclareMathOperator{\rank}{rank}\rank(A)`],
 ['declare-math-operator-starred',String.raw`\DeclareMathOperator*{\limfoo}{lim}\limfoo_n`],
 ['ams-arrow-empty-labels',String.raw`\xrightarrow[]{}+\xleftarrow{}`],
 ['ams-shove-outside-multline',String.raw`\shoveleft{x}`],
 ['ams-shove-after-row-content',String.raw`\begin{multline}a\shoveleft b\end{multline}`],
 ['ams-cfrac-illegal-alignment',String.raw`\cfrac[x]{a}{b}`],
 ['ams-cfrac-right-alignment',String.raw`\cfrac[r]{a}{b}`],
 ['ams-tag-duplicate',String.raw`\begin{equation}\tag{a}\tag{b}x\end{equation}`],
 ['ams-tag-starred',String.raw`\begin{equation}\tag*{A}x\end{equation}`],
 ['ams-tag-untaggable-environment',String.raw`\begin{split}a&=b\tag{x}\end{split}`],
 ['ams-alignat-invalid-column-count',String.raw`\begin{alignat}{x}a&=b\end{alignat}`],
 ['ams-genfrac-invalid-style',String.raw`\genfrac{(}{)}{0pt}{4}{a}{b}`],
 ['ams-xalignat-row',String.raw`\begin{xalignat}{1}a&=b\end{xalignat}`],
 ['ams-alignedat-two-pairs',String.raw`\begin{alignedat}{2}a&=b&c&=d\end{alignedat}`],
 ['ams-shoveright-first-row',String.raw`\begin{multline}\shoveright a+b\\c+d\end{multline}`],
 ['ams-arrow-bottom-label',String.raw`\xleftarrow[a]{b}`],
 ['ams-genfrac-text-style',String.raw`\genfrac{(}{)}{0pt}{1}{a}{b}`],
 ['ams-operatorname-limits',String.raw`\operatorname{lim}\limits_{n}`],
 ['ams-multint-with-limits',String.raw`\iint\limits_a^b`],
];
const rows=[];
function observe(name,input){
 let jax,result,error;
 try{jax=new TeX({packages:['base','newcommand','ams']});jax.setMmlFactory(new MmlFactory());const math=new AbstractMathItem(input,jax,true);result=node(jax.compile(math,{}));}
 catch(e){error={name:e.name||e.constructor.name,id:e.id??null,message:String(e.message)};}
 rows.push({name,input,result:result??null,error:error??null,dynamicMaps:jax?mapState(jax):null,packageData:jax?plain(jax.parseOptions.packageData):null});
}
for(const [name,input] of cases)observe(name,input);
const output=path.join(root,'tools/math-port/ams-newcommand-reference/ams-newcommand-boundary-source.json');
fs.writeFileSync(output,JSON.stringify({upstream:pinned.upstream,sources:pinned.sources,configurations:{ams:AmsConfiguration.name,newcommand:NewcommandConfiguration.name},cases:rows},null,2)+'\n');
console.log(JSON.stringify({sourceCases:rows.length,lockedOriginalCases:pinned.cases.length,output:path.relative(root,output)}));
