'use strict';
// Development reference: real pinned BaseMethods, TexParser, BaseItems and factories.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
assert.equal(hash('.reference/MathJax-src/ts/input/tex/base/BaseMethods.ts'),'7336240c8f9c5037f9fdcd5a84ea079312854f1ef3f406aeb5a8b6a8ff765cda');
assert.equal(hash('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/base/BaseMethods.js'),'b8d5c74e01f85ae391ec8d16ffcd216a4ca4267bd8ffa091c5bfe1462ad7b491');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const BaseMethods=load('input/tex/base/BaseMethods.js').default;
const {splitAlignArray}=load('input/tex/base/BaseMethods.js');
const ParseMethods=load('input/tex/ParseMethods.js').default;
const BaseItems=load('input/tex/base/BaseItems.js');
const {Configuration,ParserConfiguration}=load('input/tex/Configuration.js');
const {RegExpMap,CharacterMap,MacroMap,CommandMap,EnvironmentMap}=load('input/tex/TokenMap.js');
const {Token,Macro}=load('input/tex/Token.js');
const ParseOptions=load('input/tex/ParseOptions.js').default;
const TexParser=load('input/tex/TexParser.js').default;
const {NoTags}=load('input/tex/Tags.js');
const {MmlFactory}=load('core/MmlTree/MmlFactory.js');
const {AbstractMathItem}=load('core/MathItem.js');
const members=('Open Close Bar Tilde Space Superscript Subscript Prime Comment Hash MathFont SetFont SetStyle SetSize Spacer DiscretionaryTimes AllowBreak Break Linebreak LeftRight NamedFn NamedOp Limits Over Frac Sqrt Root MoveRoot Accent UnderOver Overset Underset Overunderset TeXAtom VBox Hsize ParBox BreakAlign MmlToken Strut Phantom Smash Lap RaiseLower MoveLeftRight Hskip Nonscript Rule rule MakeBig BuildRel HBox FBox FrameBox MakeBox Not Dots Matrix Entry Cr CrLaTeX HLine HFill NewColumnType BeginEnd Array AlignedArray IndentAlign Equation EqnArray HandleNoTag HandleLabel HandleRef Macro MathChoice').split(' ');
assert.equal(members.length,75);assert.deepEqual(Object.keys(BaseMethods).sort(),[...members].sort());
const called=new Set();
const methods=Object.fromEntries(members.map(name=>[name,function(...args){called.add(name);return BaseMethods[name](...args);} ]));
let serial=0;
function fixture() {
  const id='base-methods-reference-'+(++serial),name=s=>id+'-'+s;
  new RegExpMap(name('letters'),ParseMethods.variable,/[a-zA-Z]/);
  new RegExpMap(name('numbers'),ParseMethods.digit,/[0-9]/);
  new RegExpMap(name('control'),ParseMethods.controlSequence,/\\/);
  const chars={'{':'Open','}':'Close','|':'Bar','~':'Tilde',' ':'Space','\n':'Space','^':'Superscript','_':'Subscript',"'":'Prime','%':'Comment','#':'Hash','&':'Entry'};
  new MacroMap(name('characters'),Object.fromEntries(Object.entries(chars).map(([c,m])=>[c,methods[m]])));
  const commandData={
    mathrm:['MathFont','normal'],rm:['SetFont','normal'],bf:['SetFont','bold'],
    displaystyle:['SetStyle','D',true,0],scriptstyle:['SetStyle','S',false,1],small:['SetSize',.85],',':['Spacer',1/6],timesbreak:['DiscretionaryTimes'],
    allowbreak:['AllowBreak'],break:['Break'],goodbreak:['Linebreak','goodbreak'],left:['LeftRight'],right:['LeftRight'],middle:['LeftRight'],
    sin:['NamedFn'],lim:['NamedOp'],limits:['Limits',true],nolimits:['Limits',false],over:['Over'],above:['Over'],atopwithdelims:['Over'],
    frac:['Frac'],sqrt:['Sqrt'],root:['Root'],leftroot:['MoveRoot','leftRoot'],uproot:['MoveRoot','upRoot'],hat:['Accent','005E'],vec:['Accent','2192',false],
    overline:['UnderOver','2015'],underbrace:['UnderOver','23DF',true],overset:['Overset'],underset:['Underset'],overunderset:['Overunderset'],
    mathop:['TeXAtom',1],mathrel:['TeXAtom',3],vbox:['VBox','bottom'],hsize:['Hsize'],parbox:['ParBox'],breakAlign:['BreakAlign'],mmlToken:['MmlToken'],strut:['Strut'],
    phantom:['Phantom'],vphantom:['Phantom',1,0],hphantom:['Phantom',0,1],smash:['Smash'],llap:['Lap'],rlap:['Lap'],raise:['RaiseLower'],lower:['RaiseLower'],moveleft:['MoveLeftRight'],moveright:['MoveLeftRight'],
    hskip:['Hskip'],kern:['Hskip',true],nonscript:['Nonscript'],Rule:['Rule'],Space:['Rule','blank'],rule:['rule'],bigl:['MakeBig',4,.85],biggl:['MakeBig',4,1.45],bigr:['MakeBig',5,.85],biggr:['MakeBig',5,1.45],buildrel:['BuildRel'],
    hbox:['HBox',0],text:['HBox'],fbox:['FBox'],framebox:['FrameBox'],makebox:['MakeBox'],not:['Not'],dots:['Dots'],matrix:['Matrix'],pmatrix:['Matrix','(',')'],cases:['Matrix','{','','left left',null,'.2em',null,true],
    numericmatrix:['Matrix',null,null,null,null,null,null,null,2],
    cr:['Cr'],'\\':['CrLaTeX'],newline:['CrLaTeX',true],hline:['HLine'],hdashline:['HLine','dashed'],hfill:['HFill'],newcolumntype:['NewColumnType'],
    begin:['BeginEnd'],end:['BeginEnd'],notag:['HandleNoTag'],label:['HandleLabel'],ref:['HandleRef'],eqref:['HandleRef',true],swap:['Macro','#2+#1',2],optional:['Macro','#1+#2',2,'d'],mathchoice:['MathChoice']
  };
  const commands=new CommandMap(name('commands'),Object.fromEntries(Object.entries(commandData).map(([n,[m,...args]])=>[n,[methods[m],...args]])));
  commands.add('aliasfrac',new Macro('frac',methods.Frac));
  commands.add('aliaslimits',new Macro('limits',methods.Limits,[true]));
  new CharacterMap(name('delimiters'),null,Object.fromEntries([...'().|[]{}'].map(c=>[c,c==='.'?'':c])));
  new CharacterMap('not_remap',null,{'=':'≠'});
  const environments={array:[methods.AlignedArray,false],plainarray:[methods.Array,false,null,null,'cc'],indentalign:[methods.IndentAlign,false],equation:[methods.Equation,false,true],eqnarray:[methods.EqnArray,false,false,true,'rcl','c','1em']};
  new EnvironmentMap(name('environments'),ParseMethods.environment,environments);
  const configuration=new ParserConfiguration([]);
  configuration.append(Configuration.local({items:Object.fromEntries(Object.values(BaseItems).map(Class=>[Class.prototype.kind,Class])),
    handler:{character:['characters','control','letters','numbers'].map(name),macro:[name('commands')],delimiter:[name('delimiters')],environment:[name('environments')]},
    fallback:{character:(p,c)=>p.Push(p.create('token','mo',{},c))}}));
  const options=new ParseOptions(configuration,[{mathStyle:'TeX',maxBuffer:10000,maxMacros:1000,maxTemplateSubtitutions:10000,tagSide:'right',tagIndent:'0.8em',baseURL:'',numberPattern:/^[0-9]+/,identifierPattern:/^[a-zA-Z]+/}]);
  options.nodeFactory.setMmlFactory(new MmlFactory());options.tags=new NoTags();options.tags.configuration=options;options.mathItem=new AbstractMathItem('',null);
  return options;
}
let cases=0;
function parse(source,options=fixture(),environment={}) {++cases;const parser=new TexParser(source,environment,options);return {root:parser.mml(),parser,options};}
function nodes(root,kind) {const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n);});return result;}
function visible(root) {return root.isToken?root.getText():root.childNodes.filter(Boolean).map(visible).join('');}
function attr(node,name) {return node.attributes.getExplicit(name);}
function fail(source,id,options) {assert.throws(()=>parse(source,options),e=>e.id===id,source);}
{
  const {root}=parse("{x_1^2}+y'''~|%ignored\nz");
  assert.equal(nodes(root,'msubsup').length,1);assert.equal(nodes(root,'msup').length,1);assert.ok(visible(root).includes('‴'));assert.ok(visible(root).includes('\u00a0'));assert.ok(!visible(root).includes('ignored'));
  assert.equal(nodes(root,'mo').find(n=>n.getText()==='|').getProperty('keep-attrs'),'stretchy');
}
for(const [source,id] of [['#','CantUseHash1'],['x^a^b','DoubleExponent'],['x_a_b','DoubleSubscripts'],["x^a'",'DoubleExponentPrime']])fail(source,id);
{
  const {root}=parse('\\mathrm{abc}{\\bf x}\\scriptstyle y\\small z\\sin t+\\lim_a^b\\nolimits');
  assert.ok(nodes(root,'mi').some(n=>n.getText()==='abc'&&attr(n,'mathvariant')==='normal'));assert.equal(nodes(root,'msubsup').length,1);assert.ok(visible(root).includes('\u2061'));
  assert.ok(nodes(root,'mstyle').some(n=>attr(n,'mathsize')==='0.85em'));
  assert.equal(nodes(parse('\\lim_a^b\\limits').root,'munderover').length,1);fail('x\\limits','MisplacedLimits');
  assert.throws(()=>parse('x\\aliaslimits'),e=>e.id==='MisplacedLimits'&&e.message==='\\aliaslimits is allowed only on operators');
}
{
  const {root}=parse('\\aliasfrac{a}{b}+{a\\over b}+{c\\above 2pt d}+{e\\atopwithdelims()f}+\\sqrt[\\leftroot{3}\\uproot{-2}n]{x}+\\root 3\\of y+\\sqrt\\frac12');
  assert.equal(nodes(root,'mfrac').length,5);assert.equal(nodes(root,'mroot').length,2);assert.equal(nodes(root,'msqrt').length,1);assert.ok(nodes(root,'mpadded').some(n=>attr(n,'width')==='+0.2em'));
  fail('\\leftroot{1}','MisplacedMoveRoot');fail('\\sqrt[\\leftroot{x}]{a}','IntegerArg');fail('\\sqrt[\\leftroot{1}\\leftroot{2}]{a}','MultipleMoveRoot');
}
{
  const {root}=parse('\\hat{x}+\\vec{y}+\\overline{a}+\\underbrace{b}+\\overset{a}{x}+\\underset{b}{x}+\\overunderset{a}{b}{x}+\\buildrel a\\over b+\\left(x\\middle|y\\right)+\\bigl[+\\mathrel{x}+\\mathop{\\rm abc}_a');
  assert.ok(nodes(root,'munderover').length>=5);assert.equal(nodes(root,'mover').length,1);assert.equal(nodes(root,'munder').length,1);
  assert.ok(nodes(root,'munderover').some(n=>n.childNodes.length===3&&n.childNodes[1]===null&&n.childNodes[2]));assert.ok(nodes(root,'mo').some(n=>attr(n,'minsize')==='1.2em'));
}
{
  const {root}=parse("\\mmlToken{mi}[mathvariant='bold', id=test]{x}+\\hbox{hello}+\\text{world}+\\fbox{a}+\\framebox[2em][r]{b}+\\makebox[3em][C]{c}+\\vbox{\\hsize=2em x}+\\parbox[t]{4em}{d}");
  assert.equal(nodes(root,'menclose').length,2);assert.ok(nodes(root,'mi').some(n=>attr(n,'id')==='test'&&n.getProperty('keep-attrs')==='mathvariant id'));
  assert.ok(nodes(root,'mpadded').some(n=>n.getProperty('vbox')==='bottom'&&attr(n,'width')==='2em'));
  fail('\\mmlToken{mrow}{x}','NotMathMLToken');fail('\\mmlToken{mi}[unknown=x]{a}','UnknownAttrForElement');fail('\\mmlToken{mi}[!=x]{a}','InvalidMathMLAttr');
}
{
  const {root}=parse('\\strut+\\phantom{x}+\\vphantom{y}+\\hphantom{z}+\\smash[t]{x}+\\llap{a}+\\rlap{b}+\\raise 2pt{x}+\\lower -3pt{y}+\\moveleft 1em{a}+\\moveright -2em{b}+\\Rule{1em}{2pt}{3pt}+\\Space{1em}{2pt}{3pt}+\\rule[-2pt]{1em}{3pt}');
  assert.equal(nodes(root,'mphantom').length,3);assert.ok(nodes(root,'mpadded').some(n=>attr(n,'lspace')==='-1width'));assert.ok(nodes(root,'mpadded').some(n=>attr(n,'voffset')==='+2pt'));
  assert.equal(nodes(root,'mspace').filter(n=>attr(n,'mathbackground')==='black').length,2);
}
{
  const {root}=parse('x\\,\\hskip 2pt\\kern 3pt\\nonscript\\,y\\timesbreak z\\allowbreak\\break x\\goodbreak y\\not=+\\dots+\\mathchoice{D}{T}{S}{Q}\\\\[2pt]z');
  assert.equal(nodes(root,'MathChoice').length,1);assert.ok(visible(root).includes('≠'));assert.ok(nodes(root,'mspace').some(n=>attr(n,'data-lineleading')==='2pt'));
}
{
  const {root}=parse('\\matrix{\\hline a\\hfill&b\\cr c&d\\\\[2pt]\\hdashline}+\\pmatrix{x&y}+\\cases{x&some text\\cr y&\\text{other}}');
  assert.ok(nodes(root,'mtable').length>=3);assert.equal(nodes(root,'mtr').length,5);assert.equal(nodes(root,'mtd').length,10);assert.ok(visible(root).includes('some text'));
  fail('\\hfill','UnsupportedHFill');fail('\\hline','Misplaced');fail('\\cases{x&a&b}','ExtraAlignTab');
}
{
  const {root}=parse('\\newcolumntype{C}{c}\\begin{array}[t]{Cl}\\breakAlign{t}{tc}a&b\\\\c&d\\end{array}+\\begin{plainarray}a&b\\end{plainarray}');
  const tables=nodes(root,'mtable');assert.equal(tables.length,2);assert.equal(attr(tables[0],'align'),'baseline 1');assert.equal(attr(tables[0],'data-break-align'),'top center');
  fail('\\newcolumntype{AB}{c}','BadColumnName');fail('\\newcolumntype{C}[x]{c}','PositiveIntegerArg');fail('\\breakAlign{c}{t}','BreakNotInArray');
}
{
  const options=fixture();
  const {root}=parse('\\begin{indentalign}[1em][2em][3em]{lcr}x\\end{indentalign}',options);assert.ok(nodes(root,'mstyle').some(n=>attr(n,'indentshift')==='2em'&&attr(n,'indentalignlast')==='right'));
  parse('\\begin{equation}a\\label{a}\\notag\\end{equation}',options);assert.equal(options.mathItem.display,true);assert.ok(options.tags.labels.a);
  assert.equal(nodes(parse('\\begin{eqnarray}a&=&b\\end{eqnarray}',options).root,'mtable').length,1);
  const refs=parse('\\ref{missing}+\\eqref{missing}',options);assert.equal(options.tags.redo,true);assert.ok(visible(refs.root).includes('???'));fail('\\begin{bad\\name}','InvalidEnv');
}
{
  const {root}=parse('\\Rule{1em}{2pt}{3pt}+\\rule{2em}{4pt}',fixture(),{color:''});assert.equal(nodes(root,'mspace').filter(n=>attr(n,'mathbackground')==='black').length,2);
  const matrix=parse('\\numericmatrix{a&b&1}').root;assert.equal(attr(nodes(matrix,'mtable')[0],'side'),2);
}
{
  assert.equal(visible(parse('\\swap{a}{b}+\\optional{x}+\\optional[q]{r}').root),'b+a+d+x+q+r');
  const options=fixture();parse('\\label{x}',options);options.tags.label='';fail('\\label{x}','MultipleLabel',options);options.options.ignoreDuplicateLabels=true;options.tags.label='';parse('\\label{x}',options);fail('\\label{y}','MultipleCommand',options);
  assert.equal(splitAlignArray(' t b m c '),'top bottom middle center');assert.throws(()=>splitAlignArray('x'),e=>e.id==='BadBreakAlign');assert.throws(()=>splitAlignArray('tt',1),e=>e.id==='TooManyAligns');
}
assert.deepEqual(members.filter(name=>!called.has(name)),[],'Every original method executes through real configured TexParser');
console.log(JSON.stringify({methods:called.size,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
