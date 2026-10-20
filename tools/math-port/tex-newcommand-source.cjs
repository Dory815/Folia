'use strict';
// Development oracle: original Newcommand package, actual TeX and document classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
const locks={
  NewcommandConfiguration:['ba8a493e2f2154c868600ccecbe8a28ad9337267c8a3d1460cf130098ad01b4d','8c4de6d6933fd485916114d588400f2e4b5fb1763e5720db74af9b9b5bc24ab2'],
  NewcommandItems:['500dc1df6fe295938744b0e399bcf0e16a140f91f4da0b22e5afe9b1d76c55ff','8772e5089015efc52fedce9e5e9b3968a89d5732a0f480c0c906e424baa73b01'],
  NewcommandMappings:['553aa9eefdb70ee77ce97c645bdfc978e59619334f091ac5fddc09db2652c382','0b52824ee1e59343672612bcd1a0bffd1be922fc4122497a6cbfa7399508838b'],
  NewcommandMethods:['2d3a5422123d9afb36e9277f506d5a24a7a1d3295163a34371c8fb7400b41cfa','7aa2e76b2263b2f12961958c86e2325acba83083334ca54f6949f86a25bd80cb'],
  NewcommandUtil:['9dd994444bd3c0b49c83ab070dfb82f022a69364109e775b535d7281d9b33002','496a1c7154521e6870c5469b47070323766182150740f2e846cd005460b7e4bf']
};
for(const [name,[ts,cjs]] of Object.entries(locks)) {
  assert.equal(digest('.reference/MathJax-src/ts/input/tex/newcommand/'+name+'.ts'),ts);
  assert.equal(digest('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/newcommand/'+name+'.js'),cjs);
}
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const Methods=load('input/tex/newcommand/NewcommandMethods.js').default;
const {NewcommandConfiguration,NewcommandConfig}=load('input/tex/newcommand/NewcommandConfiguration.js');
const {MapHandler}=load('input/tex/MapHandler.js');
const BaseMethods=load('input/tex/base/BaseMethods.js').default;
assert.deepEqual(Object.keys(Methods).sort(),['NewCommand','NewEnvironment','MacroDef','Let','MacroWithTemplate','BeginEnv','Macro'].sort());
let cases=0;
function fixture(options={}) {
  let errorId='';const jax=new TeX({packages:['base','newcommand'],...options,formatError:(jax,error)=>{errorId=error.id;return jax.formatError(error);}});jax.setMmlFactory(new MmlFactory());
  return {jax,compile(source){++cases;errorId='';const root=jax.compile(new AbstractMathItem(source,jax,false),reference.document);let text='';root.walkTree(node=>{if(node.isToken)text+=node.getText();});return {root,text,error:jax.parseOptions.error,errorId};}};
}
const sourceCases=[
  ['\\newcommand{\\ten}[10]{#1+#9}\\ten abcdefghij+z','a+i+z',false],
  ['\\newcommand{\\zero}[0][d]{#1}\\zero[q]','q',false],
  ['\\newcommand{\\zero}[0]{#1}\\zero','Illegal macro parameter reference',true],
  ['\\newenvironment{z}[0][d]{#1}{x}\\begin{z}[q]\\end{z}','qx',false],
  ['\\let\\sin\\notDefined\\sin','Undefined control sequence \\sin',true],
  ['\\let\\langle\\notDefined\\langle','',false],
  ['\\let\\foo(\\left\\foo x\\right)','(x)',false],
  ['\\let\\foo\\langle\\foo','⟨',false],
  ['\\newcommand{\\a}{x}\\let\\b\\a\\renewcommand{\\a}{y}\\b\\a','xy',false],
  ['\\let\\a\\alpha\\let\\b\\a\\let\\a\\beta\\b\\a','αβ',false],
  ['\\def\\pair(#1,#2){#2+#1}\\pair(a,b)','b+a',false],
  ['\\newcommand{\\pair}[2][d]{#1+#2}\\pair{x}+\\pair[q]{r}','d+x+q+r',false],
  ['\\newcommand{\\pair}[2][d]{#1+#2}\\renewcommand\\pair[1]{#1#1}\\pair{x}','xx',false],
  ['\\def\\take#1abc{#1}\\take{xy}abc','xy',false],
  ['\\def\\foo#1\\stop{#1}\\foo a\\stop','a',false],
  ['\\def\\plain#1#2 {#2#1}\\plain ab','ba',false],
  ['\\def\\pair(#1,#2){#2+#1}\\let\\saved\\pair\\def\\pair{x}\\saved(a,b)','b+a',false],
  ['\\let\\fraction\\frac\\fraction12','12',false],
  ['\\let\\a\\alpha\\let\\a\\a\\a','α',false],
  ['\\let\\define\\def\\define\\c{x}\\c','x',false],
  ['\\let\\bgroup={\\bgroup a}','a',false],
  ['\\let\\car^ a\\car b','ab',false],
  ['\\newenvironment{e}[2][d]{#1+#2+}{+z}\\begin{e}{x}y\\end{e}','d+x+y+z',false],
  ['\\newenvironment{e}[2][d]{#1+#2+}{+z}\\begin{e}[q]{r}s\\end{e}','q+r+s+z',false],
  ['\\newenvironment{n}{a}{b}\\begin{n}x\\begin{n}y\\end{n}z\\end{n}','axaybzb',false],
  ['\\newenvironment{n}{a}{b}\\renewenvironment{n}{c}{d}\\begin{n}x\\end{n}','cxd',false],
  ['\\newenvironment{m}{\\begin{array}{cc}}{\\end{array}}\\begin{m}a&b\\end{m}','ab',false]
];
{
  const f=fixture(),commands=MapHandler.getMap('new-Command'),delimiters=MapHandler.getMap('new-Delimiter'),environments=MapHandler.getMap('new-Environment');
  assert.equal(NewcommandConfiguration.name,'newcommand');assert.deepEqual(NewcommandConfiguration.handler.macro,['Newcommand-macros']);assert.equal(NewcommandConfiguration.options.maxMacros,1000);assert.deepEqual(NewcommandConfiguration.options.protectedMacros,['begingroupSandbox']);
  NewcommandConfig(f.jax.configuration,f.jax);assert.equal(MapHandler.getMap('new-Command')===commands,true);assert.equal(MapHandler.getMap('new-Delimiter')===delimiters,true);assert.equal(MapHandler.getMap('new-Environment')===environments,true);
  assert.equal(f.jax.parseOptions.itemFactory.create('beginEnv').kind,'beginEnv');assert.equal(Methods.Macro===BaseMethods.Macro,true);
  f.compile('\\def\\pair(#1,#2){#2+#1}');const args=f.jax.parseOptions.handlers.retrieve('new-Command').lookup('pair').args;assert.deepEqual(args,['#2+#1','2','(',',',')']);
}
for(const [source,text,error] of sourceCases) {const actual=fixture().compile(source);assert.equal(actual.text,text);assert.equal(actual.error,error);}
const errors=[
  ['\\def x{a}','MissingCS'],['\\newcommand{a-b}{x}','IllegalControlSequenceName'],['\\newcommand{\\a}[x]{x}','IllegalParamNumber'],
  ['\\def\\a#0{x}','CantUseHash2'],['\\def\\a#2{x}','SequentialParam'],['\\def\\a','MissingReplacementString'],
  ['\\newcommand{\\begingroupSandbox}{x}','ProtectedMacro'],['\\newenvironment{e}{a}{b}\\begin{e}','EnvMissingEnd'],
  ['\\newenvironment{e}{a}{b}\\begin{e}\\end{equation}','EnvBadEnd'],['\\newenvironment{e}[1]{#1}{#1}\\begin{e}{x}','IllegalMacroParam'],
  ['\\def\\pair(#1,#2){#2+#1}\\pair[a,b)','MismatchUseDef'],['\\def\\foo#1\\stop{#1}\\foo a\\stopmore','RunawayArgument'],['\\let\\foo\\notDefined\\foo','UndefinedControlSequence']
];
for(const [source,id] of errors){const result=fixture().compile(source);assert.equal(result.errorId,id);assert.equal(result.error,true);}
{
  const f=fixture();assert.equal(f.compile('\\newcommand{\\persistent}{x}').text,'');assert.equal(f.compile('\\persistent').text,'x');f.jax.reset(11);assert.equal(f.compile('\\persistent').text,'x');
  assert.equal(fixture().compile('\\persistent').errorId,'UndefinedControlSequence');assert.equal(f.compile('\\persistent').text,'x');
}
{
  const f=fixture({packages:['base','begingroup','newcommand']});
  assert.equal(f.compile('\\def\\a{x}\\begingroup\\def\\a{y}\\a\\endgroup\\a').text,'yx');
  assert.equal(f.compile('\\begingroup\\def\\a{y}\\global\\def\\a{z}\\a\\endgroup\\a').text,'zz');
  assert.equal(f.compile('\\begingroup\\def\\a{q}').text,'');assert.equal(f.compile('\\a').text,'q');assert.equal(f.compile('\\endgroup\\a').text,'z');
}
{
  const f=fixture({maxMacros:12});assert.equal(f.compile('\\def\\loop{\\loop}\\loop').errorId,'MaxMacroSub1');assert.equal(f.compile('\\def\\loop{x}\\loop').text,'x');
  assert.equal(fixture({maxMacros:2}).compile('\\newenvironment{e}{}{}\\begin{e}\\begin{e}\\end{e}\\end{e}').errorId,'MaxMacroSub2');
}
console.log(JSON.stringify({methods:7,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
