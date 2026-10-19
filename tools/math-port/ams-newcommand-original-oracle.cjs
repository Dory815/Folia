'use strict';
// Source-derived observations from the pinned MathJax 4.1.3 CJS package.
// No native code is loaded by this process.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const packageRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const tsRoot=path.join(packageRoot,'ts/input/tex'),cjsRoot=path.join(packageRoot,'cjs/input/tex');
const files=['ams/AmsItems','ams/AmsMethods','ams/AmsMappings','ams/AmsConfiguration',
  'newcommand/NewcommandItems','newcommand/NewcommandUtil','newcommand/NewcommandMethods',
  'newcommand/NewcommandMappings','newcommand/NewcommandConfiguration'];
const expected={
 'ams/AmsItems.ts':'6fe73b33063f5b8c86a3e07bcf7bce45aaa5256b2d321146ffc39a7435264926',
 'ams/AmsMethods.ts':'afc713c62b1f53e9ebc8136eda3f5e7bcea2685b8c8de139b842d815851d2126',
 'ams/AmsMappings.ts':'01e0a7a2291f7a5d3b1c1799a79c82cf8641415a953ca8b70abc1eff7cbbfbb5',
 'ams/AmsConfiguration.ts':'5dc58a7d4cad9ba6d70a44cec59d9c38514965b7b14dd39421a57c0459ad8e51',
 'newcommand/NewcommandItems.ts':'d4ac79bffc72954a6d6fe3e99ee7c1d9274b55d2d87d3008c09d6be2f2f59b7b',
 'newcommand/NewcommandUtil.ts':'58b42e9601faa07d95333fd1e56d3e611a6505687c10f590422f4f19d73a6e07',
 'newcommand/NewcommandMethods.ts':'8db96297b5921f0420db38a36c5ead0e4f165be46c4e9fc0a918a5ce1ce27937',
 'newcommand/NewcommandMappings.ts':'42f2181845827166f5ebaaba6c7f0dcad54af314abf510e23d3ab37dd0b445b7',
 'newcommand/NewcommandConfiguration.ts':'fc7ac30d0e6a1f2b9cd723b9f2dbc613e0fab3f506309e080425330f919312c7',
 'ams/AmsItems.js':'24f1b6b0dabb84eddc52150bf5f629fb8203a3e9eeaecb5f425c7df8fe4656d5',
 'ams/AmsMethods.js':'1b24b7bfa46c4e8764756ef66972cd788512fbd33bd80eeed18f88a2db72de51',
 'ams/AmsMappings.js':'83337b1aafc4e7431050696abc27a3392c26e5626d70d22595d0b250bc5f2af9',
 'ams/AmsConfiguration.js':'ff8e1aa5936fb48e68da3cc5ff8745f1365d5dbd2416c1a908c6a06842989f79',
 'newcommand/NewcommandItems.js':'8772e5089015efc52fedce9e5e9b3968a89d5732a0f480c0c906e424baa73b01',
 'newcommand/NewcommandUtil.js':'496a1c7154521e6870c5469b47070323766182150740f2e846cd005460b7e4bf',
 'newcommand/NewcommandMethods.js':'7aa2e76b2263b2f12961958c86e2325acba83083334ca54f6949f86a25bd80cb',
 'newcommand/NewcommandMappings.js':'0b52824ee1e59343672612bcd1a0bffd1be922fc4122497a6cbfa7399508838b',
 'newcommand/NewcommandConfiguration.js':'8c4de6d6933fd485916114d588400f2e4b5fb1763e5720db74af9b9b5bc24ab2',
};
for(const stem of files){
 const source=`${stem}.ts`,compiled=`${stem}.js`;
 const sourceBytes=fs.readFileSync(path.join(tsRoot,source));
 const sourceHash=crypto.createHash('sha256').update(sourceBytes).digest('hex');
 assert.equal(sourceHash,expected[source],`${source} locked Source changed`);
 const installed=fs.readFileSync(path.join(tsRoot,source),'utf8').replaceAll('\r\n','\n');
 assert.equal(installed,sourceBytes.toString('utf8').replaceAll('\r\n','\n'),`${source} installed TS differs`);
 const cjsHash=crypto.createHash('sha256').update(fs.readFileSync(path.join(cjsRoot,compiled))).digest('hex');
 assert.equal(cjsHash,expected[compiled],`${compiled} pinned CJS changed`);
}
const load=name=>require(path.join(packageRoot,'cjs',name));
// Loading these actual Source modules registers the real static configuration maps.
const {AmsConfiguration}=load('input/tex/ams/AmsConfiguration.js');
const {NewcommandConfiguration,NewcommandConfig}=load('input/tex/newcommand/NewcommandConfiguration.js');
const {MapHandler}=load('input/tex/MapHandler.js');
const TeX=load('input/tex.js').TeX;
const {MmlFactory}=load('core/MmlTree/MmlFactory.js');
const AbstractMathItem=load('core/MathItem.js').AbstractMathItem;
const rows=[];
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
function observe(name,input){
 let jax,math,result,error;
 try{jax=new TeX({packages:['base','newcommand','ams']});jax.setMmlFactory(new MmlFactory());math=new AbstractMathItem(input,jax,true);result=node(jax.compile(math,{}));}
 catch(e){error={name:e.name||e.constructor.name,id:e.id??null,message:String(e.message)};}
 rows.push({name,input,result:result??null,error:error??null,dynamicMaps:jax?mapState(jax):null,
   packageData:jax?plain(jax.parseOptions.packageData):null});
}
const cases=[
 ['plain-x','x'],['newcommand-simple',String.raw`\newcommand{\foo}{x}\foo`],
 ['newcommand-redefinition',String.raw`\newcommand{\foo}{x}\renewcommand{\foo}{y}\foo`],
 ['newcommand-optional-default',String.raw`\newcommand{\foo}[2][d]{#1+#2}\foo{x}`],
 ['newcommand-optional-explicit',String.raw`\newcommand{\foo}[2][d]{#1+#2}\foo[z]{x}`],
 ['newcommand-invalid-count',String.raw`\newcommand{\foo}[a]{x}`],
 ['def-parameter-template',String.raw`\def\foo#1,#2;{#2+#1}\foo a,b;`],
 ['def-optional-brace',String.raw`\def\foo#1#2{#1+#2}\foo{x}{y}`],
 ['let-control-sequence',String.raw`\let\foo=\alpha\foo`],
 ['new-environment',String.raw`\newenvironment{xx}{a}{b}\begin{xx}x\end{xx}`],
 ['new-environment-optional',String.raw`\newenvironment{xx}[2][d]{#1}{#2}\begin{xx}{x}z\end{xx}`],
 ['ams-frac',String.raw`\dfrac{1}{2}`],['ams-binomial',String.raw`\binom{n}{k}`],
 ['ams-matrix',String.raw`\begin{matrix}a&b\\c&d\end{matrix}`],
 ['ams-cases',String.raw`\begin{cases}x&x>0\\-x&x<0\end{cases}`],
 ['ams-align',String.raw`\begin{align}a&=b\\c&=d\end{align}`],
 ['ams-align-star',String.raw`\begin{align*}a&=b\\c&=d\end{align*}`],
 ['ams-multline',String.raw`\begin{multline}a+b\\c+d\end{multline}`],
 ['ams-flalign',String.raw`\begin{flalign}a&=b\end{flalign}`],
 ['ams-operatorname',String.raw`\operatorname{rank}(A)`],['ams-tag-reference',String.raw`\begin{equation}\tag{A}x\end{equation}`],
 ['ams-sideset',String.raw`\sideset{_a^b}{_c^d}\sum`],['ams-arrow',String.raw`\xrightarrow[a]{b}`],
 ['protected-recursive-limit',String.raw`\newcommand{\foo}{\foo}\foo`],
];
for(const [name,input]of cases)observe(name,input);
const mapNames=['AMSmath-mathchar0mo','AMSmath-operatorLetter','AMSmath-macros','AMSmath-environment','AMSmath-delimiter',
 'AMSsymbols-mathchar0mi','AMSsymbols-mathchar0mo','AMSsymbols-delimiter','AMSsymbols-macros'];
const maps=Object.fromEntries(mapNames.map(name=>{const m=MapHandler.getMap(name);return[name,m?{constructor:m.constructor.name,name:m.name,parser:m.parser?.name||null,pattern:m._regExp?.source||null,flags:m._regExp?.flags||null,entries:plain(m.map)}:null];}));
const metadata={upstream:{version:'4.1.3',commit:'fb987178c2d279a99b0db5ea02e751691703955e'},sources:files.map(stem=>({source:`ts/input/tex/${stem}.ts`,sha256:expected[`${stem}.ts`],cjs:`input/tex/${stem}.js`,cjs_sha256:expected[`${stem}.js`]})),sourceMapRegistrations:maps,configurations:{ams:{name:AmsConfiguration.name,parser:AmsConfiguration.parser,items:Object.keys(AmsConfiguration.items),tags:Object.keys(AmsConfiguration.tags)},newcommand:{name:NewcommandConfiguration.name,parser:NewcommandConfiguration.parser,items:Object.keys(NewcommandConfiguration.items),options:plain(NewcommandConfiguration.options),configName:NewcommandConfig.name,length:NewcommandConfig.length}},cases:rows};
const output=path.join(root,'tools/math-port/ams-newcommand-reference/ams-newcommand-source.json');
fs.writeFileSync(output,JSON.stringify(metadata,null,2)+'\n');
console.log(JSON.stringify({sources:files.length,cases:rows.length,configurations:2,maps:mapNames.length,output:path.relative(root,output)}));
