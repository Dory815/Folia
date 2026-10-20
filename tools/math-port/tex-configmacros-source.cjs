'use strict';
// Development witness: pinned ConfigMacrosConfiguration with actual TeX/MML.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
assert.equal(digest('.reference/MathJax-src/ts/input/tex/configmacros/ConfigMacrosConfiguration.ts'),'1578e2cec78cd03e83dff7439c0b2eedbb4564cef2002dbd2dba528c25197c7a');
assert.equal(digest('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/configmacros/ConfigMacrosConfiguration.js'),'050cfb81fdc2914459e6c4128ab2ba8a3162207f72db921351cc1fb3320914a0');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {ConfigMacrosConfiguration}=load('input/tex/configmacros/ConfigMacrosConfiguration.js');
const Methods=load('input/tex/newcommand/NewcommandMethods.js').default;
const BaseMethods=load('input/tex/base/BaseMethods.js').default;
const {MacroMap,CommandMap,EnvironmentMap}=load('input/tex/TokenMap.js');
const {BeginEnvItem}=load('input/tex/newcommand/NewcommandItems.js');
assert.equal(ConfigMacrosConfiguration.name,'configmacros');
assert.equal(typeof ConfigMacrosConfiguration.init,'function');
assert.equal(typeof ConfigMacrosConfiguration.config,'function');
assert.equal(ConfigMacrosConfiguration.items.beginEnv===BeginEnvItem,true);
for(const name of ['active','macros','environments'])assert.equal(Object.keys(ConfigMacrosConfiguration.options[name]).length,0);
let cases=0;
function fixture(options={},newcommand=false) {
  let error='';
  const jax=new TeX({packages:['base','configmacros',...(newcommand?['newcommand']:[])],...options,formatError:(jax,e)=>{error=e.id;return jax.formatError(e);}});
  jax.setMmlFactory(new MmlFactory());
  return {jax,map(name){return jax.parseOptions.handlers.retrieve(name);},compile(source){++cases;error='';const root=jax.compile(new AbstractMathItem(source,jax,false),reference.document);let text='';root.walkTree(n=>{if(n.isToken)text+=n.getText();});return {root,text,error};}};
}
function text(f,source,expected){const result=f.compile(source);assert.equal(result.error,'',source);assert.equal(result.text,expected,source);}
const pair=['#2+#1',2,['(',',',')']];
{
  const f=fixture();
  assert.equal(f.map('configmacros-active-map') instanceof MacroMap,true);
  assert.equal(f.map('configmacros-map') instanceof CommandMap,true);
  assert.equal(f.map('configmacros-env-map') instanceof EnvironmentMap,true);
  assert.equal(f.jax.parseOptions.handlers.retrieve('new-Command'),null);
  assert.equal(f.jax.parseOptions.itemFactory.create('beginEnv') instanceof BeginEnvItem,true);
  const priorities=[...f.jax.parseOptions.handlers.get('macro')._configuration].filter(({item})=>item.name==='configmacros-map').map(({priority})=>priority);
  assert.deepEqual(priorities,[3]);
}
{
  const f=fixture({macros:{plain:'x',swap:['#2+#1',2],pair:['#1+#2',2,'d'],empty:['#1+x',1,''],numericZero:['x',0,'d'],stringZero:['#1','0','d']}});
  text(f,'\\plain+\\swap ab+\\pair{x}+\\pair[q]{r}+\\empty','x+b+a+d+x+q+r++x');
  text(f,'\\numericZero[q]','x[q]');text(f,'\\stringZero[q]','q');
  assert.equal(f.map('configmacros-map').lookup('plain').func===BaseMethods.Macro,true);
}
{
  const f=fixture({macros:{pair,sin:'s'},active:{'@':pair,a:'q','2':'r'}});
  text(f,'\\pair(x,y)+@(x,y)+a+2+\\sin','y+x+y+x+q+r+s');
  const macro=f.map('configmacros-map').lookup('pair');assert.equal(macro.func===Methods.MacroWithTemplate,true);assert.deepEqual(macro.args,['#2+#1',2,'(',',',')']);
}
{
  const f=fixture({environments:{e:['#1+#2+','+z',2,'d'],n:['a','b'],m:['\\begin{array}{cc}','\\end{array}'],zero:['#1','x','0','d']}});
  text(f,'\\begin{e}{x}y\\end{e}+\\begin{e}[q]{r}s\\end{e}','d+x+y+z+q+r+s+z');
  text(f,'\\begin{n}x\\begin{n}y\\end{n}z\\end{n}','axaybzb');
  text(f,'\\begin{m}a&b\\end{m}','ab');text(f,'\\begin{zero}[q]\\end{zero}','qx');
  assert.equal(f.map('configmacros-env-map').lookup('e').func===Methods.BeginEnv,true);
}
{
  const options={macros:{kept:'x'}},f=fixture(options);
  options.macros.kept='y';f.jax.parseOptions.options.macros.kept='z';
  text(f,'\\kept','x');f.jax.reset(11);text(f,'\\kept+\\kept','x+x');
  assert.equal(fixture().compile('\\kept').error,'UndefinedControlSequence');
}
{
  const f=fixture({macros:{kept:'x',pair},environments:{e:['a','b']}},true);
  text(f,'\\let\\saved\\kept\\renewcommand{\\kept}{y}\\saved\\kept','xy');
  text(f,'\\let\\savedPair\\pair\\def\\pair{x}\\savedPair(a,b)','b+a');
  text(f,'\\renewenvironment{e}{c}{d}\\begin{e}x\\end{e}','cxd');
  assert.equal(f.map('configmacros-map').lookup('kept').args[0],'x');
}
{
  const f=fixture({macros:{pair,loop:'\\loop',bad:['#1',1]},environments:{e:['a','b'],recursive:['\\begin{recursive}','']},maxMacros:8});
  assert.equal(f.compile('\\pair[a,b)').error,'MismatchUseDef');text(f,'\\pair(a,b)','b+a');
  for(const [source,error]of [['\\bad','MissingArgFor'],['\\begin{e}','EnvMissingEnd'],['\\begin{e}\\end{array}','EnvBadEnd'],['\\loop','MaxMacroSub1'],['\\begin{recursive}','MaxMacroSub2']])assert.equal(f.compile(source).error,error,source);
  text(f,'\\begin{e}x\\end{e}','axb');
}
console.log(JSON.stringify({configurations:1,maps:3,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
