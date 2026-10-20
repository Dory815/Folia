'use strict';
// Development witness: original AMS methods and real TeX/MML, no substitute parser.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
assert.equal(digest('.reference/MathJax-src/ts/input/tex/ams/AmsMethods.ts'),'e07f24fe8df2dd12cfdaf850efee73c3d36824783001b22e1624f3db877c2ec1');
assert.equal(digest('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/ams/AmsMethods.js'),'1b24b7bfa46c4e8764756ef66972cd788512fbd33bd80eeed18f88a2db72de51');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {AmsMethods}=load('input/tex/ams/AmsMethods.js'),BaseMethods=load('input/tex/base/BaseMethods.js').default;
const {Macro}=load('input/tex/Token.js');
const own=['AmsEqnArray','AlignAt','Multline','XalignAt','FlalignArray','HandleDeclareOp','HandleOperatorName','SideSet','operatorLetter','MultiIntegral','xArrow','HandleShove','CFrac','Genfrac','HandleTag'];
const aliases=['HandleNoTag','HandleRef','Macro','Accent','Tilde','Array','Spacer','NamedOp','EqnArray','Equation'];
assert.deepEqual(Object.keys(AmsMethods).sort(),[...own,...aliases].sort());
for(const name of own)assert.equal(typeof AmsMethods[name],'function');
for(const name of aliases)assert.equal(AmsMethods[name]===BaseMethods[name],true);
let cases=0;
function fixture(options={},groups=false) {
  let error='';const jax=new TeX({packages:groups?['base','begingroup','ams']:['base','ams'],maxMacros:1000,...options,formatError:(jax,e)=>{error=e.id;return jax.formatError(e);}});jax.setMmlFactory(new MmlFactory());
  jax.parseOptions.handlers.retrieve('AMSmath-macros').add('widearrow',new Macro('widearrow',AmsMethods.xArrow,[0x2192,5,10,3]));
  return {jax,compile(source,display=true){++cases;error='';const root=jax.compile(new AbstractMathItem(source,jax,display),reference.document);let text='';root.walkTree(n=>{if(n.isToken)text+=n.getText();});return {root,text,error};}};
}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n);});return result;}
function ok(f,source,text){const result=f.compile(source);assert.equal(result.error,'',source);if(text!==undefined)assert.equal(result.text,text,source);return result.root;}
{
 const f=fixture();
 for(const source of ['\\begin{aligned}[t]a&=b\\\\c&=d\\end{aligned}','\\begin{gathered}[b]a\\\\b\\end{gathered}','\\begin{alignat*}{2}a&=b&c&=d\\end{alignat*}','\\begin{alignedat}[t]{1}a&=b\\end{alignedat}','\\begin{xalignat*}{1}a&=b\\end{xalignat*}','\\begin{xxalignat}{1}a&=b\\end{xxalignat}','\\begin{flalign*}a&=b\\end{flalign*}'])assert.equal(nodes(ok(f,source),'mtable').length,1);
 const mult=nodes(ok(f,'\\begin{multline*}\\shoveleft a+b\\\\\\shoveright c+d\\end{multline*}'),'mtable')[0];assert.equal(mult.attributes.get('width'),'100%');assert.equal(mult.attributes.get('columnspacing'),'100%');
 assert.equal(nodes(ok(f,'\\begin{alignedat}{}a\\end{alignedat}'),'mtable').length,1);
 assert.equal(nodes(ok(f,'\\begin{xalignat*}{}a\\end{xalignat*}'),'mtable').length,1);
 assert.equal(nodes(ok(f,'\\begin{aligned}[]a&=b\\end{aligned}'),'mtable').length,1);
}
{
 const f=fixture();
 let root=ok(f,'\\operatorname{lim}_{x}y');assert.equal(nodes(root,'msub').length,1);
 root=ok(f,'\\operatorname*{lim}_{x}y');assert.equal(nodes(root,'munder').length,1);
 root=ok(f,'\\operatorname{lim}\\limits_{x}y');assert.equal(nodes(root,'msub').length,1);
 root=ok(f,'\\mathbf{\\operatorname{a-b*}}+a-b*');const named=nodes(root,'mi').find(n=>n.getText()==='a-b*');assert.ok(named);assert.equal(named.attributes.get('mathvariant'),'normal');
 root=ok(f,'\\operatorname{a+b}');assert.ok(nodes(root,'TeXAtom').some(n=>n.getProperty('texClass')===1&&n.texClass===1));
 root=ok(f,'\\operatorname{lim}');assert.equal(nodes(root,'mi')[0].texClass,1);
 ok(f,'\\operatorname{lim}\\alpha','lim\u2061α');
}
{
 const f=fixture({},true);
 ok(f,'\\DeclareMathOperator{\\foo}{Foo}');ok(f,'\\begingroup\\DeclareMathOperator{\\foo}{Local}');ok(f,'\\foo','Local');ok(f,'\\endgroup\\foo','Foo');
 ok(f,'\\begingroup\\global\\DeclareMathOperator*{\\foo}{Global}\\endgroup');const root=ok(f,'\\foo_x','Globalx');assert.equal(nodes(root,'munder').length,1);
}
{
 const f=fixture();let root=ok(f,'\\sideset{_a^b}{_c^d}{\\sum}');let multi=nodes(root,'mmultiscripts')[0];assert.ok(multi);assert.equal(multi.getProperty('scriptalign'),'left');assert.equal(multi.childNodes.length,6);
 root=ok(f,'\\sideset{_a}{}{\\sum}');multi=nodes(root,'mmultiscripts')[0];assert.equal(nodes(multi,'none').length,1);
 root=ok(f,'\\sideset{_a+x}{_b+y}{\\sum}');assert.equal(nodes(root,'mphantom').length,1);assert.equal(nodes(root,'mmultiscripts').length,0);
 root=ok(f,'\\sideset{}{_a}{\\sum}');assert.equal(nodes(root,'msub').length,1);
}
{
 const f=fixture();let root=ok(f,'\\idotsint_a^b');assert.equal(nodes(root,'mo').filter(n=>n.getText()==='∫').length,2);
 root=ok(f,'\\idotsint\\limits_a^b');assert.equal(nodes(root,'munderover').length,1);
 root=ok(f,'\\xrightarrow[b]{a}');assert.equal(nodes(root,'munderover').length,1);assert.equal(nodes(root,'mpadded').length,2);
 root=ok(f,'\\xleftarrow[]{a}');assert.equal(nodes(root,'mover').length,1);assert.equal(nodes(root,'mpadded').length,1);
 root=ok(f,'\\widearrow{a}');assert.equal(nodes(root,'mo').find(n=>n.getText()==='→').attributes.get('minsize'),'3em');
}
{
 const f=fixture();for(const align of ['', 'l','r']){const root=ok(f,'\\cfrac['+align+']{a}{b}'),frac=nodes(root,'mfrac')[0];assert.equal(frac.attributes.get('numalign'),align==='l'?'left':align==='r'?'right':'center');}
 for(const [style,level,display]of [['0',0,true],['1',0,false],['2',1,false],['3',2,false],['2tail',1,false]]){const root=ok(f,'\\genfrac{(}{)}{0pt}{'+style+'}{a}{b}'),frac=nodes(root,'mfrac')[0];assert.equal(frac.attributes.get('linethickness'),'0pt');assert.equal(frac.getProperty('withDelims'),true);assert.equal(frac.attributes.get('scriptlevel'),level);assert.equal(frac.attributes.get('displaystyle'),display);}
 ok(f,'\\genfrac{}{}{}{}{a}{b}','ab');
}
{
 const f=fixture({tags:'ams'});let root=ok(f,'\\begin{align}a&=b\\tag{A}\\label{first}\\end{align}');assert.equal(nodes(root,'mlabeledtr').length,1);assert.equal(f.jax.parseOptions.tags.allLabels.first.tag,'A');
 ok(f,'\\eqref{first}','(A)');root=ok(f,'\\begin{align}a&=b\\notag\\end{align}');assert.equal(nodes(root,'mlabeledtr').length,0);
 ok(f,'\\ref{first}','A');
 root=ok(f,'x\\tag*{raw}');assert.ok(nodes(root,'mtext').some(n=>n.getText()==='raw'));
}
const errors=[
 ['\\begin{alignat}{x}a\\end{alignat}','PositiveIntegerArg'],['\\begin{xalignat}{x}a\\end{xalignat}','PositiveIntegerArg'],
 ['\\shoveleft x','CommandOnlyAllowedInEnv'],['\\begin{multline*}x\\shoveright y\\end{multline*}','CommandAtTheBeginingOfLine'],
 ['\\cfrac[q]{a}{b}','IllegalAlign'],['\\cfrac[q]{\\unknownAmsCommand}{b}','UndefinedControlSequence'],['\\genfrac{bad}{}{}{}{a}{b}','MissingOrUnrecognizedDelim'],['\\genfrac{}{}{}{4}{a}{b}','BadMathStyleFor'],['\\genfrac{}{}{}{4}{\\unknownAmsCommand}{b}','UndefinedControlSequence'],
 ['x\\tag{a}\\tag{b}','MultipleCommand'],['\\begin{split}a&=b\\tag{a}\\end{split}','CommandNotAllowedInEnv'],
 ['\\begin{align}a&=b\\label{x}\\label{y}\\end{align}','MultipleCommand'],['\\begin{align}a&=b\\label{x}\\\\c&=d\\label{x}\\end{align}','MultipleLabel']
];
for(const [source,error]of errors){const f=fixture({tags:'ams'});assert.equal(f.compile(source).error,error,source);ok(f,'a+b','a+b');}
console.log(JSON.stringify({methods:own.length,aliases:aliases.length,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
