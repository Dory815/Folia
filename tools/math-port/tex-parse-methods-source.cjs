'use strict';
// Execute the original methods through actual TexParser/Map/NodeFactory classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
assert.equal(hash('.reference/MathJax-src/ts/input/tex/ParseMethods.ts'),'1cdc4c6828078cd40863269864cdd6b7e5d22024e2ed36f4eeafb673781a5a70');
assert.equal(hash('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/ParseMethods.js'),'2af91730bf39c97e86169c68cdc8c1054c05c58216f60a2f2d8e54ec75294851');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const Methods=load('input/tex/ParseMethods.js').default;
const {ParseUtil}=load('input/tex/ParseUtil.js');
const {Configuration,ParserConfiguration}=load('input/tex/Configuration.js');
const {RegExpMap,CharacterMap,CommandMap,EnvironmentMap}=load('input/tex/TokenMap.js');
const {Token,Macro}=load('input/tex/Token.js');
const ParseOptions=load('input/tex/ParseOptions.js').default;
const TexParser=load('input/tex/TexParser.js').default;
const {NoTags}=load('input/tex/Tags.js');
const {MmlFactory}=load('core/MmlTree/MmlFactory.js');
const {NodeFactory}=load('input/tex/NodeFactory.js');
const Base=load('input/tex/base/BaseItems.js');
assert.deepEqual(Object.keys(Methods).sort(),['variable','digit','controlSequence','lcGreek','ucGreek','mathchar0mi','mathchar0mo','mathchar7','delimiter','environment'].sort());
let count=0;
function fixture(method,values={},token=null,extra={}) {
  const name='parse-method-check-'+(++count);
  if(token) new CharacterMap(name,method,{}).add('a',token);
  else new RegExpMap(name,method,/[\s\S]/);
  const configuration=new ParserConfiguration([]);
  configuration.append(Configuration.local({items:Object.fromEntries(Object.values(Base).map(Class=>[Class.prototype.kind,Class])),
    handler:{character:[name],...extra.handler},fallback:extra.fallback||{}}));
  const options=new ParseOptions(configuration,[{mathStyle:'TeX',tagSide:'right',tagIndent:'0.8em',...values}]);
  options.nodeFactory.setMmlFactory(new MmlFactory());options.tags=new NoTags();options.tags.configuration=options;
  return options;
}
const first=parser=>{const root=parser.mml();return root.isToken?root:root.childNodes[0];};
for(const [source,style,expected] of [['x','TeX',undefined],['é','French','italic'],['Α','French','normal'],['α','ISO','italic'],['𝛼','upright',undefined],['ϰ','upright','normal'],['ϱ','upright',undefined]]) {
  const options=fixture(Methods.variable,{mathStyle:style}),parser=new TexParser(source,{},options),node=first(parser);
  assert.equal(node.kind,'mi');assert.equal(node.getText(),source);assert.equal(node.attributes.getExplicit('mathvariant'),expected);
}
for(const font of ['bold','-tex-calligraphic','']) {
  const parser=new TexParser('x',{font},fixture(Methods.variable)),node=first(parser);
  assert.equal(node.attributes.getExplicit('mathvariant'),font||undefined);
  assert.equal(!!node.getProperty('ignore-variant'),font.startsWith('-'));
}
assert.equal(ParseUtil.isLatinOrGreekChar('!é'),true);assert.equal(ParseUtil.isLatinOrGreekChar('ᾧ'),true);
assert.equal(ParseUtil.isLatinOrGreekChar('𝛼'),false);assert.equal(ParseUtil.isLatinOrGreekChar('ϱ'),false);
for(const emptyFont of [false,true]) {
  const expression=/^[a-z]+/g;expression.lastIndex=50;
  const options=fixture(Methods.variable),observations=[];
  options.nodeFactory.setCreators({token:(factory,kind,definition,text)=>{
    observations.push(definition.autoOP);return NodeFactory.createToken(factory,kind,definition,text);
  }});
  const parser=new TexParser('abc',{font:emptyFont?'':'normal',multiLetterIdentifiers:expression,noAutoOP:true},options),node=parser.mml();
  assert.equal(node.isToken?1:node.childNodes.length,emptyFont?3:1);assert.equal((node.isToken?node:node.childNodes[0]).getText(),emptyFont?'a':'abc');
  assert.deepEqual(observations,emptyFont?[undefined,undefined,undefined]:[false]);
  assert.equal(expression.lastIndex,emptyFont?50:0);
}
{
  const parser=new TexParser('𝛼',{multiLetterIdentifiers:/^z+/},fixture(Methods.variable));
  assert.equal(parser.i,3);assert.equal(first(parser).getText(),'𝛼');
}
for(const flags of ['','g','y','gy']) {
  const numberPattern=new RegExp('^[0-9]+(?:\\{,\\}[0-9]+)?(?:\\.[0-9]+)?',flags);
  numberPattern.lastIndex=flags==='y'?0:99;
  const parser=new TexParser('12{,}345.6',{font:'bold'},fixture(Methods.digit,{numberPattern})),node=first(parser);
  assert.equal(node.kind,'mn');assert.equal(node.getText(),'12,345.6');assert.equal(node.attributes.getExplicit('mathvariant'),'bold');
  assert.equal(parser.i,10);assert.equal(numberPattern.lastIndex,flags==='y'?10:flags===''?99:0);
}
{
  const options=fixture(Methods.digit,{numberPattern:/^[0-9]+|^\.[0-9]+/},null,
    {fallback:{character:(p,c)=>p.Push(p.create('token','mo',{},c))}});
  const node=first(new TexParser('.',{},options));assert.equal(node.kind,'mo');assert.equal(node.getText(),'.');
}
for(const [method,text,style,font,expected] of [['lcGreek','α','TeX','bold','italic'],['lcGreek','α','upright','bold','normal'],['ucGreek','Γ','TeX','','normal'],['ucGreek','Γ','ISO','','italic'],['ucGreek','Γ','ISO','bold','bold']]) {
  const options=fixture(Methods[method],{mathStyle:style},new Token('a',text));
  const node=first(new TexParser('a',{font},options));assert.equal(node.getText(),text);assert.equal(node.attributes.getExplicit('mathvariant'),expected);
}
for(const supplied of [false,true]) for(const method of ['mathchar0mi','mathchar7']) {
  const options=fixture(Methods[method],{},new Token('a','R',supplied?{}:null));
  const node=first(new TexParser('a',{},options));
  assert.equal(node.attributes.getExplicit('mathvariant'),supplied?undefined:method==='mathchar0mi'?'italic':'normal');
}
{
  const options=fixture(Methods.mathchar7,{},new Token('a','R',{mathvariant:'double-struck'}));
  assert.equal(first(new TexParser('a',{font:'bold'},options)).attributes.getExplicit('mathvariant'),'bold');
}
{
  const options=fixture(Methods.mathchar0mo,{},new Token('a','∑',{stretchy:true,largeop:true}));
  const observations=[];
  options.nodeFactory.setCreators({token:(factory,kind,definition,text)=>{
    assert.equal(definition.fixStretchy,undefined);const node=NodeFactory.createToken(factory,kind,definition,text);
    observations.push(node.getProperty('fixStretchy'));return node;
  }});
  const parser=new TexParser('a',{},options),root=parser.mml(),node=root.isToken?root:root.childNodes[0];options.root=root;
  assert.equal(node.getProperty('fixStretchy'),true);assert.deepEqual(observations,[undefined]);
  assert.equal(node.attributes.getExplicit('stretchy'),false);assert.equal(node.attributes.getExplicit('largeop'),true);
  assert.equal(options.getList('fixStretchy')[0]===node,true);
}
for(const supplied of [false,true]) {
  const options=fixture(Methods.delimiter,{},new Token('a','|',supplied?{stretchy:true,fence:true}:null));
  const node=first(new TexParser('a',{},options));assert.equal(node.kind,'mo');
  assert.equal(node.attributes.getExplicit('stretchy'),supplied);assert.equal(node.attributes.getExplicit('fence'),supplied);
  assert.equal(node.getProperty('keep-attrs'),'stretchy');
}
{
  const commands=new CommandMap('parse-method-command',{});
  commands.add('alias',new Macro('original',(p,name,text)=>{
    assert.equal(p.currentCS,'\\alias');assert.equal(name,'\\original');p.Push(p.create('token','mi',{},text));
  },['R']));
  const options=fixture(Methods.controlSequence,{},null,{handler:{macro:['parse-method-command']}});
  const parser=new TexParser('\\alias',{},options),root=parser.mml();
  assert.equal(parser.currentCS,'');assert.equal(root.childNodes.length,1);assert.equal(root.childNodes[0].getText(),'R');
  assert.equal(root.attributes.get('data-latex'),'\\alias');
}
{
  const body=(parser,begin,...args)=>{
    assert.equal(begin.kind,'begin');assert.equal(begin.getProperty('name'),args.length?'valuesActual':'emptyActual');
    assert.deepEqual(args,args.length?['payload',false,2]:[]);return parser.create('token','mi',{},'E');
  };
  const environments=new EnvironmentMap('parse-method-environment',Methods.environment,{});
  environments.add('empty',new Macro('emptyActual',body,[]));
  environments.add('values',new Macro('valuesActual',body,[null,'payload',false,2]));
  new CommandMap('parse-method-environment-call',{empty:p=>p.parse('environment',[p,'empty']),values:p=>p.parse('environment',[p,'values'])});
  const options=fixture(Methods.controlSequence,{},null,{handler:{macro:['parse-method-environment-call'],environment:['parse-method-environment']}});
  for(const source of ['\\empty','\\values'])assert.equal(first(new TexParser(source,{},options)).getText(),'E');
}
console.log(JSON.stringify({status:'passed',methods:10,fixtures:count,compiledProfile:reference.provenance.compiled_source_sha256,
  limit:'Actual original ParseMethods and configured parser collaboration. Full native BaseMethods, package bindings, input-jax filters, SVG and Folia remain unverified.'}));
