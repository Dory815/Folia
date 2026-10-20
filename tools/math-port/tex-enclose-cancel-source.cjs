'use strict';
// Development witness: original enclosure packages and real TeX/MML classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
for(const [name,folder,ts,cjs]of [
 ['EncloseConfiguration','enclose','318e8e6a80dc40b2fc12441dc597fcc493e54ed241f10105adb36c57e200c2d8','a7a2c9d3a21a2c5cb7df96c4d3cd68704aca6799cbb28fab01cc0826ae74b232'],
 ['CancelConfiguration','cancel','684b21b3eb7eab7ea80e225dc57c5690594cac01cc425e9da41bad3ce26ac5d7','646af6c259f13917cb5b85ae79072648afcdc7e7aa8b3230f5a250f404bd694e']
]){assert.equal(digest('.reference/MathJax-src/ts/input/tex/'+folder+'/'+name+'.ts'),ts);assert.equal(digest('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/'+folder+'/'+name+'.js'),cjs);}
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {NodeFactory}=load('input/tex/NodeFactory.js');
const {ENCLOSE_OPTIONS,EncloseMethods}=load('input/tex/enclose/EncloseConfiguration.js');
const {CancelMethods}=load('input/tex/cancel/CancelConfiguration.js');
assert.deepEqual(Object.keys(ENCLOSE_OPTIONS),['data-arrowhead','color','mathcolor','background','mathbackground','data-padding','data-thickness']);
assert.deepEqual(Object.keys(EncloseMethods),['Enclose']);assert.deepEqual(Object.keys(CancelMethods),['Cancel','CancelTo']);
let cases=0;
function fixture(packages){let error='',message='';const jax=new TeX({packages:['base',...packages],formatError:(jax,e)=>{error=e.id;message=e.message;return jax.formatError(e);}});jax.setMmlFactory(new MmlFactory());return {jax,compile(source){++cases;error='';message='';const root=jax.compile(new AbstractMathItem(source,jax,false),reference.document);return {root,error,message};}};}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n);});return result;}
function ok(f,source){const result=f.compile(source);assert.equal(result.error,'',source);return result.root;}
{
 const f=fixture(['enclose']);let enclosed=nodes(ok(f,'\\enclose{box,,circle}{x}'),'menclose');assert.equal(enclosed.length,1);assert.equal(enclosed[0].attributes.get('notation'),'box  circle');
 enclosed=nodes(ok(f,'\\enclose{box}[color=red,unknown=x,notation=wrong]{x}'),'menclose');assert.equal(enclosed[0].attributes.get('notation'),'box');assert.equal(enclosed[0].attributes.get('color'),'red');assert.equal(enclosed[0].attributes.hasExplicit('unknown'),false);
 assert.equal(f.compile('\\cancel{x}').error,'UndefinedControlSequence');
}
{
 const f=fixture(['enclose']),requests=[];
 f.jax.parseOptions.nodeFactory.set('node',(factory,kind,children,def,text)=>{if(kind==='menclose')requests.push({kind,def:{...def}});return NodeFactory.createNode(factory,kind,children,def,text);});
 ok(f,'\\enclose{box,updiagonalstrike}[data-arrowhead={foo\\,bar},color=red,mathcolor=blue,background={a,b=c},mathbackground=yellow,data-padding={{1em}},data-thickness=false,unknown=ignored,notation=wrong]{x}');
 assert.deepEqual(requests,[{kind:'menclose',def:{'data-arrowhead':'foo\\,bar',color:'red',mathcolor:'blue',background:'a,b=c',mathbackground:'yellow','data-padding':'1em','data-thickness':false,notation:'box updiagonalstrike'}}]);
 const enclosed=nodes(ok(f,'\\enclose{circle}[data-arrowhead=true,data-padding={x\\}y}]{x}'),'menclose')[0];assert.equal(enclosed.attributes.get('data-arrowhead'),true);assert.equal(enclosed.attributes.get('data-padding'),'x\\}y');
}
{
 const f=fixture(['cancel']);
 for(const [command,notation]of [['cancel','updiagonalstrike'],['bcancel','downdiagonalstrike'],['xcancel','updiagonalstrike downdiagonalstrike']]){const enclosed=nodes(ok(f,'\\'+command+'[mathcolor=red,notation=wrong,unknown=x]{x}'),'menclose');assert.equal(enclosed.length,1);assert.equal(enclosed[0].attributes.get('notation'),notation);assert.equal(enclosed[0].attributes.get('mathcolor'),'red');assert.equal(enclosed[0].attributes.hasExplicit('unknown'),false);}
 assert.equal(f.compile('\\enclose{box}{x}').error,'UndefinedControlSequence');
 const tree=ok(f,'\\cancelto[mathcolor=blue]{0}{x}'),sup=nodes(tree,'msup')[0],pad=nodes(tree,'mpadded')[0],enclosed=nodes(tree,'menclose')[0];assert.ok(sup);assert.equal(sup.childNodes[0]===enclosed,true);assert.equal(sup.childNodes[1]===pad,true);assert.equal(enclosed.attributes.get('notation'),'updiagonalstrike updiagonalarrow northeastarrow');
 for(const [key,value]of Object.entries({depth:'-.1em',height:'+.1em',voffset:'.1em'}))assert.equal(pad.attributes.get(key),value);assert.deepEqual(nodes(pad,'mn').map(n=>n.getText()),['0']);assert.deepEqual(nodes(enclosed,'mi').map(n=>n.getText()),['x']);
}
{
 const f=fixture(['cancel']),requests=[];
 f.jax.parseOptions.nodeFactory.set('node',(factory,kind,children,def,text)=>{if(['mpadded','menclose','msup'].includes(kind))requests.push({kind,def:{...def}});return NodeFactory.createNode(factory,kind,children,def,text);});
 ok(f,'\\cancelto[data-padding={{1em}},data-thickness=false]{0}{x}');
 assert.deepEqual(requests,[{kind:'mpadded',def:{depth:'-.1em',height:'+.1em',voffset:'.1em'}},{kind:'menclose',def:{'data-padding':'1em','data-thickness':false,notation:'updiagonalstrike updiagonalarrow northeastarrow'}},{kind:'msup',def:{}}]);
}
{
 const f=fixture(['enclose','cancel']);const root=ok(f,'\\enclose{box}{\\cancel{x}+\\cancelto{0}{y}}');assert.equal(nodes(root,'menclose').length,3);assert.equal(nodes(root,'msup').length,1);
}
for(const [packages,source,id]of [
 [['enclose'],'\\enclose','MissingArgFor'],[['enclose'],'\\enclose{box}','MissingArgFor'],[['enclose'],'\\enclose{box}[color=x}]{\\unknown}','ExtraCloseLooking'],[['enclose'],'\\enclose{box}[color={x]{\\unknown}','MissingCloseBracket'],[['enclose'],'\\enclose{box}[unknown=x]{\\unknown}','UndefinedControlSequence'],
 [['cancel'],'\\cancel','MissingArgFor'],[['cancel'],'\\cancelto','MissingArgFor'],[['cancel'],'\\cancelto{0}','MissingArgFor'],[['cancel'],'\\cancelto[color=x}]{\\unknown}{x}','ExtraCloseLooking'],[['cancel'],'\\cancelto[unknown=x]{\\unknown}{x}','UndefinedControlSequence']
]){const f=fixture(packages);assert.equal(f.compile(source).error,id,source);ok(f,packages[0]==='enclose'?'\\enclose{box}{x}':'\\cancel{x}');}
{
 const f=fixture(['enclose','cancel','newcommand']);ok(f,'\\let\\savedEnclose\\enclose\\let\\savedCancel\\cancel\\let\\savedTo\\cancelto');assert.equal(nodes(ok(f,'\\savedEnclose{circle}{\\savedCancel{x}}'),'menclose').length,2);
 const result=f.compile('\\savedTo{0}');assert.equal(result.error,'MissingArgFor');assert.equal(result.message,'Missing argument for \\savedTo');ok(f,'\\savedTo{0}{x}');
}
console.log(JSON.stringify({methods:3,allowed:7,commands:5,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
