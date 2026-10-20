'use strict';
// Development reference: all seven original FilterUtil passes on genuine MML trees.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
assert.equal(hash('.reference/MathJax-src/ts/input/tex/FilterUtil.ts'),'dc821e2483fcd0820159a13d984b2bf299952e8c9f40418d72ff4c183d93053a');
assert.equal(hash('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/FilterUtil.js'),'2e626da51d95be455ddde976660abb8ebab035a8712db0f0820870b95e9b1bd8');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const FilterUtil=load('input/tex/FilterUtil.js').default;
const NodeUtil=load('input/tex/NodeUtil.js').default;
const {Configuration,ParserConfiguration}=load('input/tex/Configuration.js');
const ParseOptions=load('input/tex/ParseOptions.js').default;
const {MmlFactory}=load('core/MmlTree/MmlFactory.js');
const {AbstractMathItem}=load('core/MathItem.js');
const methods=['cleanSubSup','setInherited','checkScriptlevel','moveLimits','cleanStretchy','cleanAttributes','combineRelations'];
assert.deepEqual(Object.keys(FilterUtil).sort(),[...methods].sort());
const called=new Set();let cases=0;
function run(name,f) {called.add(name);FilterUtil[name](f.arg);}
function fixture() {
  const config=new ParserConfiguration([]);config.append(Configuration.local({}));
  const options=new ParseOptions(config,[{mathStyle:'TeX'}]);options.nodeFactory.setMmlFactory(new MmlFactory());
  const math=new AbstractMathItem('',null),arg={math,data:options};
  const node=(kind,children=[])=>options.nodeFactory.create('node',kind,children);
  const token=(kind='mi',text='x')=>options.nodeFactory.create('token',kind,{},text);
  const root=children=>options.root=node('math',children);
  const scripts=(kind,base,low,up)=>{const n=node(kind);NodeUtil.setChild(n,0,base);NodeUtil.setChild(n,1,low);NodeUtil.setChild(n,2,up);return n;};
  const relation=text=>{const n=token('mo',text);n.texClass=3;return n;};
  ++cases;return {options,math,arg,node,token,root,scripts,relation};
}
const attr=(node,name)=>node.attributes.getExplicit(name);
const descendants=(root,kind)=>{const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n);});return result;};
for(const [source,result,lower] of [['msubsup','msub',true],['msubsup','msup',false],['munderover','munder',true],['munderover','mover',false]]) {
  const f=fixture(),base=f.token(),script=f.token('mi','s'),old=f.scripts(source,base,lower?script:null,lower?null:script);
  old.attributes.set('data-latex','x_s');old.attributes.setInherited('mathcolor','red');old.setProperty('subsupOK',false);old.texClass=3;
  const atom=f.node('TeXAtom',[old]);f.root([atom]);const parent=old.parent;run('cleanSubSup',f);const replacement=parent.childNodes[0];
  assert.equal(replacement.kind,result);assert.equal(replacement.childNodes.length,2);assert.equal(replacement.childNodes[0]===base,true);assert.equal(replacement.childNodes[1]===script,true);
  assert.equal(old.parent,null);assert.equal(replacement.parent===parent,true);assert.equal(replacement.texClass,null);assert.equal(replacement.getProperty('subsupOK'),false);
  assert.equal(attr(replacement,'data-latex'),'x_s');assert.equal(replacement.attributes.get('mathcolor'),'red');assert.equal(f.options.getList(source).length,0);assert.equal(f.options.getList(result).length,1);
}
{
  const f=fixture(),base=f.token(),sub=f.token('mi','i'),sup=f.token('mi','j'),complete=f.scripts('msubsup',base,sub,sup);
  const incomplete=f.scripts('msubsup',f.token(),f.token(),null),root=f.root([complete,incomplete]);
  f.options.error=true;run('cleanSubSup',f);assert.equal(descendants(root,'msubsup').length,2);
  f.options.error=false;run('cleanSubSup',f);assert.equal(descendants(root,'msubsup').length,1);assert.equal(descendants(root,'msub').length,1);
}
for(const display of [false,true]) {
  const f=fixture();f.math.display=display;const base=f.token(),sub=f.token('mi','i'),sup=f.token('mi','j'),scripts=f.scripts('msubsup',base,sub,sup),root=f.root([scripts]);
  run('setInherited',f);assert.equal(root.attributes.get('displaystyle'),display);assert.equal(base.attributes.get('displaystyle'),display);assert.equal(sub.attributes.get('displaystyle'),false);assert.equal(sub.attributes.get('scriptlevel'),1);
}
{
  const f=fixture(),identifier=f.token(),redundant=f.node('mstyle',[identifier]);
  redundant.attributes.set('displaystyle',false);redundant.attributes.set('scriptlevel',0);redundant.attributes.set('data-latex','\\textstyle x');
  const kept=f.node('mstyle',[f.token()]);kept.attributes.set('mathvariant','bold');const multiple=f.node('mstyle',[f.token(),f.token()]);f.math.display=false;const root=f.root([redundant,kept,multiple]);
  run('setInherited',f);const parent=redundant.parent;run('checkScriptlevel',f);assert.equal(redundant.parent,null);assert.equal(identifier.parent===parent,true);assert.equal(attr(identifier,'data-latex'),'\\textstyle x');assert.notEqual(kept.parent,null);assert.notEqual(multiple.parent,null);assert.equal(f.options.getList('mstyle').length,2);assert.equal(descendants(root,'mstyle').length,2);
}
for(const [source,result] of [['munderover','msubsup'],['munder','msub'],['mover','msup']])for(const display of [false,true])for(const explicit of [false,true]) {
  const f=fixture(),base=f.token('mi','lim');base.setProperty('movablelimits',true);if(explicit)base.attributes.set('movablelimits',false);
  const scripts=source==='munderover'?f.scripts(source,base,f.token(),f.token()):f.node(source,[base,f.token()]);
  scripts.attributes.set('data-latex','limits');scripts.setProperty('movesupsub',true);f.math.display=display;const root=f.root([scripts]);run('setInherited',f);const parent=scripts.parent;
  run('moveLimits',f);const moved=!display&&!explicit;assert.equal(parent.childNodes[0].kind===result,moved);assert.equal(scripts.parent===null,moved);
  if(moved){const replacement=parent.childNodes[0];assert.equal(replacement.getProperty('movesupsub'),true);assert.equal(attr(replacement,'data-latex'),'limits');assert.equal(descendants(root,source).length,0);}
}
{
  const f=fixture(),stretchy=f.relation('→'),ordinary=f.relation('='),falseMarker=f.relation('↔');
  stretchy.setProperty('fixStretchy',true);ordinary.setProperty('fixStretchy',true);falseMarker.setProperty('fixStretchy',false);
  for(const n of [stretchy,ordinary,falseMarker])f.options.addNode('fixStretchy',n);f.root([stretchy,ordinary,falseMarker]);run('cleanStretchy',f);
  assert.equal(stretchy.attributes.get('stretchy'),false);assert.equal(stretchy.getProperty('fixStretchy'),undefined);assert.equal(ordinary.getProperty('fixStretchy'),undefined);assert.equal(falseMarker.getProperty('fixStretchy'),false);assert.equal(f.options.getList('fixStretchy').length,3);
}
{
  const f=fixture(),identifier=f.token();identifier.attributes.set('mathvariant','normal');identifier.attributes.setInherited('mathvariant','normal');identifier.attributes.set('mathcolor','red');identifier.attributes.setInherited('mathcolor','red');
  identifier.attributes.set('data-latex-item','x');identifier.attributes.set('data-latex','x');identifier.setProperty('keep-attrs','mathcolor data-latex-item');f.root([identifier]);run('cleanAttributes',f);
  assert.equal(attr(identifier,'mathvariant'),undefined);assert.equal(attr(identifier,'mathcolor'),'red');assert.equal(attr(identifier,'data-latex-item'),undefined);assert.equal(attr(identifier,'data-latex'),'x');
}
{
  const f=fixture(),first=f.relation('<'),second=f.relation('='),third=f.relation('>');
  first.attributes.set('lspace','1em');first.attributes.set('stretchy',false);first.attributes.set('data-latex','<');second.attributes.set('rspace','2em');second.attributes.set('data-latex','=');second.setProperty('mathaccent',false);second.setProperty('breakable',true);third.attributes.set('mathvariant','bold');
  const row=f.node('mrow',[first,second,third]);f.root([row]);run('combineRelations',f);
  assert.equal(row.childNodes.length,2);assert.equal(row.childNodes[0]===first,true);assert.equal(first.getText(),'<=');assert.equal(second.parent,null);assert.equal(second.getProperty('relationsCombined'),true);
  assert.equal(attr(first,'data-latex'),'<=');assert.equal(attr(first,'lspace'),'1em');assert.equal(attr(first,'rspace'),'2em');assert.equal(attr(third,'lspace'),'0pt');assert.equal(first.getProperty('mathaccent'),false);assert.equal(first.getProperty('breakable'),true);assert.equal(first.getProperty('texClass'),3);assert.equal(f.options.getList('mo').length,2);assert.equal(typeof first.attributes.getInherited('form'),'string');
}
{
  const f=fixture(),first=f.relation('='),second=f.relation('=');first.setProperty('variantForm',false);second.setProperty('variantForm',true);first.attributes.set('rspace','3pt');second.attributes.set('lspace','4pt');
  const row=f.node('mrow',[first,second]),inside=f.relation('='),sup=f.node('msup',[inside,f.token()]);f.root([row,sup]);run('combineRelations',f);
  assert.equal(row.childNodes.length,2);assert.equal(attr(first,'rspace'),'3pt');assert.equal(attr(second,'lspace'),'4pt');assert.equal(inside.attributes.getInheritedNames().includes('form'),false);
}
assert.deepEqual(methods.filter(name=>!called.has(name)),[]);
console.log(JSON.stringify({methods:called.size,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
