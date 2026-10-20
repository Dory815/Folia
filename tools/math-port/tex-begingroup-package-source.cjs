'use strict';
// Development oracle: original Begingroup package and actual TeX/MML classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
const locks={
  BegingroupStack:['39150fade01fc10e021541380770a782f5735d4ddad0fb093ec4bd635a66f30d','f4b5408d18a22da1289d9be4ff310b751de0e4703ddd0dbea4d60d6a07294725'],
  BegingroupMethods:['e3abda9266a0100cfef4abd346180096ca6768b0b402770575d6490798719202','c13631818b600ab46e28f399731cc8695a0d26472b34271a15106c2fe918e993'],
  BegingroupConfiguration:['5109804cf89666cf43b215ef62867fd6c5c57ba112ffa147c8bf33cd7dc601c0','4e25653273ff39a916deb4aa8879292fa88f75a49d37af1189d88d02f7cf0789']
};
for(const [name,[ts,cjs]] of Object.entries(locks)) {
  assert.equal(digest('.reference/MathJax-src/ts/input/tex/begingroup/'+name+'.ts'),ts);
  assert.equal(digest('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/begingroup/'+name+'.js'),cjs);
}
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {BegingroupMethods}=load('input/tex/begingroup/BegingroupMethods.js');
const {BegingroupConfiguration}=load('input/tex/begingroup/BegingroupConfiguration.js');
const {begingroupStack}=load('input/tex/begingroup/BegingroupStack.js');
const {MapHandler}=load('input/tex/MapHandler.js'),{Macro}=load('input/tex/Token.js');
const BaseMethods=load('input/tex/base/BaseMethods.js').default;
assert.deepEqual(Object.keys(BegingroupMethods).sort(),['begingroup','endgroup','reset','sandbox','global','macro'].sort());
assert.equal(BegingroupMethods.macro===BaseMethods.Macro,true);
assert.equal(BegingroupConfiguration.name,'begingroup');
assert.deepEqual(Object.keys(BegingroupConfiguration.handler),['character','delimiter','macro','environment']);
assert.deepEqual(BegingroupConfiguration.handler.macro,['begingroup']);
for(const kind of ['character','delimiter','environment']) assert.deepEqual(BegingroupConfiguration.handler[kind],[]);
assert.deepEqual(BegingroupConfiguration.options.begingroup.allowGlobal,['let','def','newcommand','DeclareMathOperator','Newextarrow']);
assert.equal(BegingroupConfiguration.preprocessors.length,1);
assert.equal(BegingroupConfiguration.postprocessors.length,1);
let cases=0;
function fixture(options={}) {
  let error='',message='',observedSandbox=false;
  const jax=new TeX({packages:['base','begingroup','newcommand'],...options,formatError:(jax,e)=>{error=e.id;message=e.message;return jax.formatError(e);}});
  jax.setMmlFactory(new MmlFactory());
  const commands=MapHandler.getMap('begingroup');
  commands.add('observeSandbox',new Macro('observeSandbox',parser=>{observedSandbox=!!parser.stack.global.isSandbox;}));
  commands.add('interruptedFormula',new Macro('interruptedFormula',()=>{throw Error('Interrupted formula before finish');}));
  return {jax,stack:begingroupStack(jax.parseOptions),get observedSandbox(){return observedSandbox;},compile(source){
    ++cases;error='';message='';const root=jax.compile(new AbstractMathItem(source,jax,false),reference.document);
    let text='';root.walkTree(node=>{if(node.isToken)text+=node.getText();});return {text,error,message};
  }};
}
function text(f,source,expected){const result=f.compile(source);assert.equal(result.error,'',source);assert.equal(result.text,expected,source);}
function state(f,index,top=index,base=-100){assert.equal(f.stack.i,index);assert.equal(f.stack.top,top);assert.equal(f.stack.base,base);}
{
  const f=fixture();state(f,-100);
  const commands=MapHandler.getMap('begingroup');
  for(const name of ['begingroup','endgroup','global','gdef','begingroupReset','begingroupSandbox']) assert.equal(commands.contains(name),true);
  assert.equal(commands.lookup('gdef').func===BaseMethods.Macro,true);assert.deepEqual(commands.lookup('gdef').args,['\\global\\def']);
  text(f,'\\def\\foo{g}\\let\\delim(\\newenvironment{e}{a}{b}','');
  text(f,'\\begingroup\\def\\foo{l}\\let\\delim[\\renewenvironment{e}{c}{d}\\foo\\delim\\begin{e}x\\end{e}','l[cxd');state(f,-101);
  text(f,'\\begingroup\\def\\foo{q}\\foo','q');state(f,-102);
  text(f,'\\endgroup\\foo','l');
  text(f,'\\endgroup\\foo\\delim\\begin{e}x\\end{e}','g(axb');state(f,-100);
}
{
  const f=fixture();
  text(f,'\\def\\a{x}\\begingroup\\def\\a{y}\\begingroup\\gdef\\a{z}\\a\\endgroup\\a\\endgroup\\a','zzz');
  text(f,'\\begingroup\\global \\newcommand{\\b}{q}\\global\\let\\d(\\endgroup\\b\\d','q(');
  text(f,'\\begingroup\\def\\a{l}\\global\\def\\a{r}\\endgroup\\a','r');
}
{
  const f=fixture();
  let result=f.compile('\\global\\frac12');assert.equal(result.error,'IllegalGlobal');assert.equal(result.message,'Invalid use of \\global');
  assert.equal(f.compile('\\global x').error,'IllegalGlobal');
  result=f.compile('\\let\\glob\\global\\glob\\frac12');assert.equal(result.error,'IllegalGlobal');assert.equal(result.message,'Invalid use of \\glob');
  assert.equal(f.compile('\\global\\newenvironment{e}{a}{b}').error,'IllegalGlobal');
  const none=fixture({begingroup:{allowGlobal:[]}});assert.equal(none.compile('\\global\\def\\a{x}').error,'IllegalGlobal');
  const environments=fixture({begingroup:{allowGlobal:['newenvironment']}});
  text(environments,'\\begingroup\\global\\newenvironment{e}{a}{b}\\endgroup\\begin{e}x\\end{e}','axb');
}
{
  const f=fixture();
  text(f,'\\def\\a{g}\\let\\d(\\newenvironment{e}{a}{b}','');
  text(f,'\\begingroup\\def\\a{l}\\let\\d[\\renewenvironment{e}{c}{d}\\begingroup\\def\\a{q}','');
  text(f,'\\begingroupReset\\a\\d\\begin{e}x\\end{e}','g(axb');state(f,-100);
  assert.equal(f.compile('\\endgroup').error,'MissingBegingroup');text(f,'\\a','g');
}
{
  const f=fixture();
  text(f,'\\def\\a{g}\\let\\d(\\newenvironment{e}{a}{b}','');
  text(f,'\\begingroupSandbox\\observeSandbox\\global\\def\\a{s}\\global\\let\\d[\\renewenvironment{e}{c}{d}\\a\\d\\begin{e}x\\end{e}','s[cxd');
  assert.equal(f.observedSandbox,true);state(f,-101,-101,-101);
  text(f,'\\observeSandbox\\a','s');assert.equal(f.observedSandbox,false);
  text(f,'\\begingroupReset\\a','s');
  text(f,'\\begingroupSandbox\\observeSandbox\\a\\d\\begin{e}x\\end{e}','g(axb');assert.equal(f.observedSandbox,true);
  assert.equal(f.compile('\\endgroup').error,'MissingBegingroup');
}
{
  const f=fixture();text(f,'\\def\\a{g}','');
  assert.equal(f.compile('\\begingroup\\def\\a{l}\\unknownGroupCommand').error,'UndefinedControlSequence');state(f,-101);
  text(f,'\\a\\endgroup\\a','lg');
}
{
  const f=fixture();text(f,'\\def\\a{g}\\begingroup\\def\\a{o}','');
  assert.throws(()=>f.compile('\\begingroup\\def\\a{i}\\interruptedFormula'),/Interrupted formula before finish/);state(f,-102,-101);
  text(f,'\\a','o');state(f,-101);
  text(f,'\\endgroup\\a','g');
}
{
  const f=fixture();
  text(f,'\\def\\a{g}\\let\\start\\begingroup\\let\\finish\\endgroup\\start\\def\\a{l}\\a\\finish\\a','lg');
  assert.equal(f.compile('\\newcommand{\\begingroupSandbox}{x}').error,'ProtectedMacro');
  f.jax.reset(7);text(f,'\\a','g');
}
console.log(JSON.stringify({methods:6,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
