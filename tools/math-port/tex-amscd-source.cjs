'use strict';
// Original AMScd methods/configuration/maps, exercised through real TeX/MML.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for (const [file,sha] of [
 ['.reference/MathJax-src/ts/input/tex/amscd/AmsCdMethods.ts','b008533b599a6ee27cc8f69bd4568ee2d0f9e1467dce00dcb697647c43033c4e'],
 ['.reference/MathJax-src/ts/input/tex/amscd/AmsCdMappings.ts','bc5ed241ee4aa367c5a15c3a859eaa336daf617c743ca3152517b07f388d44a5'],
 ['.reference/MathJax-src/ts/input/tex/amscd/AmsCdConfiguration.ts','6bae294956700d7c4343d24bac9de47ef21870f85c6388ef18dd471175469f8d'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/amscd/AmsCdMethods.js','dabf40b6e66ad1f29572aadc3fd1a90b8aa4b6cc5c011b416560667ab632f032'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/amscd/AmsCdMappings.js','5e422724181f8473629c0ceccb5b3f0a242a96f65ee06637066d6016922aa25f'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/amscd/AmsCdConfiguration.js','51de0ea229419ffd74daf8ddabf45469cdd9f258a312d06b6a9f83ebe476f854']
]) assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),sha,file);
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {NodeFactory}=load('input/tex/NodeFactory.js'),methods=load('input/tex/amscd/AmsCdMethods.js').default;
const configuration=load('input/tex/amscd/AmsCdConfiguration.js').AmsCdConfiguration;
assert.deepEqual(Object.keys(methods),['CD','arrow','cell','minCDarrowwidth','minCDarrowheight']);
assert.deepEqual(configuration.handler,{character:['amscd_special'],delimiter:[],macro:['amscd_macros'],environment:['amscd_environment']});
assert.deepEqual(configuration.options.amscd,{colspace:'5pt',rowspace:'5pt',harrowsize:'2.75em',varrowsize:'1.75em',hideHorizontalLabels:false});
let cases=0;
function fixture(options={}) {
 let error='';
 const jax=new TeX({packages:['base','amscd'],...options,formatError(j,e){error=e.id;return j.formatError(e);}});
 jax.setMmlFactory(new MmlFactory());
 return {jax,compile(source){++cases;error='';const tree=jax.compile(new AbstractMathItem(source,jax,true),reference.document);return {tree,error};}};
}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n);});return result;}
function text(root){let result='';root.walkTree(n=>{if(n.isToken)result+=n.getText();});return result;}
function ok(f,s){const r=f.compile(s);assert.equal(r.error,'',s);return r.tree;}
function rows(tree){return nodes(tree,'mtable')[0].childNodes.map(n=>n.childNodes.length).join('/')+'/';}
function struts(tree){return nodes(tree,'mpadded').filter(n=>n.attributes.get('height')==='8.5pt'&&n.attributes.get('depth')==='2pt').length;}
const f=fixture();
{
 const handlers=f.jax.parseOptions.handlers;
 assert.equal(handlers.retrieve('amscd_environment').lookup('CD').func,methods.CD);
 for(const name of ['minCDarrowwidth','minCDarrowheight'])
  assert.equal(handlers.retrieve('amscd_macros').lookup(name).func,methods[name]);
 assert.equal(handlers.retrieve('amscd_special').lookup('@').func,methods.arrow);
 const table=nodes(ok(f,'\\begin{CD}A@>>>B\\end{CD}'),'mtable')[0];
 assert.equal(table.attributes.get('columnspacing'),'5pt');assert.equal(table.attributes.get('rowspacing'),'5pt');
 assert.equal(table.attributes.get('displaystyle'),true);
}
for (const [s,shape,count] of [
 ['\\begin{CD}A @>f>> B \\\\ @VgVV @AAhA \\\\ C @<<k< D\\end{CD}','3/4/3/',2],
 ['\\begin{CD}@>>>@<<<\\\\@VVV@AAA\\\\@.\\end{CD}','5/4/3/',2],
 ['\\begin{CD}@| @= @.\\end{CD}','7/',1],['\\begin{CD}@>>>\\end{CD}','3/',1],
 ['\\begin{CD}@VaVbV @AaAbA\\end{CD}','5/',1]
]) {const tree=ok(f,s);assert.equal(rows(tree),shape);assert.equal(struts(tree),count);}
{
 const arrows=nodes(ok(f,'\\begin{CD}@| @= @.\\end{CD}'),'mo');
 assert.deepEqual(arrows.map(n=>[n.getText(),n.attributes.get('minsize')]),[['∥','1.75em'],['=','2.75em']]);
}
for(const direction of ['>','<'])for(const [a,b]of [['',''],['a',''],['','b'],['a','b']]) {
 const tree=ok(f,'\\begin{CD}A@'+direction+a+direction+b+direction+'B\\end{CD}');
 const scripts=nodes(tree,b?'munderover':'mover');assert.equal(scripts.length,1);
 const child=scripts[0].childNodes;assert.equal(child.length,b?3:2);
 assert.equal(child[0].getText(),direction==='>'?'→':'←');assert.equal(child[0].attributes.get('minsize'),'2.75em');
 const above=child[b?2:1];assert.equal(above.kind,'mpadded');
 assert.deepEqual(['width','lspace','voffset'].map(k=>above.attributes.get(k)),['+.67em','.33em','.1em']);
 assert.equal(text(above),a);const kern=nodes(above,'mspace');assert.equal(kern.length,a?0:1);
 if(kern.length)assert.equal(kern[0].attributes.get('width'),'2.75em');
 if(b){assert.deepEqual(['width','lspace'].map(k=>child[1].attributes.get(k)),['+.67em','.33em']);assert.equal(child[1].attributes.getExplicit('voffset'),undefined);assert.equal(text(child[1]),b);}
}
for(const direction of ['V','A'])for(const [a,b]of [['',''],['a',''],['','b'],['a','b']]) {
 const tree=ok(f,'\\begin{CD}@'+direction+a+direction+b+direction+'\\end{CD}');
 const arrows=nodes(tree,'mo');assert.equal(arrows.length,1);
 const arrow=arrows[0];assert.equal(text(tree),a+(direction==='V'?'↓':'↑')+b);
 assert.deepEqual(['minsize','lspace','rspace','symmetric'].map(k=>arrow.attributes.get(k)),['1.75em',0,0,true]);
 const pads=nodes(tree,'mpadded');
 assert.equal(pads.filter(n=>n.attributes.get('voffset')==='+.125em'&&n.attributes.get('height')==='+.125em'&&n.attributes.get('depth')==='-.125em').length,Number(!!a)+Number(!!b));
 assert.equal(pads.filter(n=>n.attributes.get('width')===0&&n.attributes.get('lspace')==='-1width').length,Number(!!a));
}
{
 const sized=fixture({amscd:{colspace:'2pt',rowspace:'3pt',harrowsize:'6em',varrowsize:'5em'}});
 for(const [source,w,h]of [
  ['\\minCDarrowwidth{4em}\\minCDarrowheight{3em}\\begin{CD}\\minCDarrowwidth{8em}\\minCDarrowheight{7em}@>>>\\\\@VVV\\end{CD}','4em','3em'],
  ['\\begin{CD}@>>>\\\\@VVV\\end{CD}','6em','5em']
 ]) {const tree=ok(sized,source);assert.deepEqual(nodes(tree,'mo').map(n=>n.attributes.get('minsize')),[w,h]);
  const table=nodes(tree,'mtable')[0];assert.equal(table.attributes.get('columnspacing'),'2pt');assert.equal(table.attributes.get('rowspacing'),'3pt');}
 const tree=ok(sized,'\\minCDarrowwidth{4em}\\begin{CD}{\\minCDarrowwidth{8em}\\begin{CD}@>>>\\end{CD}}@>>>\\end{CD}');
 assert.deepEqual(nodes(tree,'mo').map(n=>n.attributes.get('minsize')),['8em','4em']);
}
{
 const hidden=fixture({amscd:{hideHorizontalLabels:true}}),requests=[];
 hidden.jax.parseOptions.nodeFactory.set('token',(factory,kind,def,t)=>{
  if(kind==='mo')requests.push([t,def.minsize,def.stretchy,def.symmetric??null,def.lspace??null,def.rspace??null]);
  return NodeFactory.createToken(factory,kind,def,t);
 });
 const tree=ok(hidden,'\\begin{CD}A@>a>b>B\\\\@VaVbV\\end{CD}');
 assert.deepEqual(requests,[['→','2.75em',true,null,null,null],['↓','1.75em',true,true,0,0]]);
 assert.equal(nodes(tree,'mpadded').filter(n=>n.attributes.get('height')==='.67em'&&n.attributes.get('depth')===0).length,1);
}
assert.equal(text(ok(f,'@ \\,x @😀 @')),'@x@😀@');
{
 const tree=ok(f,'\\begin{array}{ccc}a@>u>>b\\end{array}');assert.equal(rows(tree),'3/');
 assert.equal(nodes(tree,'mo')[0].attributes.get('minsize'),undefined);
}
for(const [s,error]of [
 ['@>>>','Misplaced'],['\\begin{CD}@>f\\end{CD}','TokenNotFoundForCommand'],
 ['\\minCDarrowwidth{no}','MissingDimOrUnits'],['\\minCDarrowheight{no}','MissingDimOrUnits'],
 ['\\begin{array}{ccc}a@>>>b\\end{array}','MissingDimOrUnits']
]) {assert.equal(f.compile(s).error,error,s);assert.equal(rows(ok(f,'\\begin{CD}@>>>\\end{CD}')),'3/');}
console.log(JSON.stringify({methods:5,maps:3,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
