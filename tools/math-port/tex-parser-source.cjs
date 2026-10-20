'use strict';
// Original parser/request/tag collaborations; native RAII cleanup is tested in C++.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const hashes={
  TexParser:'9afdfff070ec7ed7115a1afe97048b3d1f1befae5b75e8ec963e0456685ee167',
  ParseOptions:'ab60a07dbb24040c0fb9bd67b942755ffc5b0b18980abf1938736d8d90b22209',
  Tags:'5a73a8d16f1d3c87ece167fc06dac0c323ea05a519e5ee5ea9aadbb76cef9a5d',
  StackItem:'ac138d321e1efbc54afbd961a4b6bfa24f7d0923eddba1c8d891dee9424602b5',
  Stack:'36be7583b14111961367e9e3f1dd5911456d235b096a2fcee62f438d37c7e11a',
  StackItemFactory:'229bf0f6bd58642bff86102bb8fb6fd2b9a61d3f52ad31ce738f3f2cefa2e405',
  NodeUtil:'bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2',
  ParseUtil:'b43646071997f88f27df51610c7972a8cb5791ea76194e7318d17d1a2f81f76a',
  TokenMap:'0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e'
};
for(const [name,expected] of Object.entries(hashes))assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,`.reference/MathJax-src/ts/input/tex/${name}.ts`))).digest('hex'),expected,name);
const ref=require('./reference-profile.cjs').createReference();
assert.equal(ref.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const base=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const load=name=>require(path.join(base,name));
const {Configuration,ParserConfiguration}=load('input/tex/Configuration.js');
const {RegExpMap}=load('input/tex/TokenMap.js');
const {CommandMap,MacroMap}=load('input/tex/TokenMap.js');
const {Macro}=load('input/tex/Token.js');
const Base=load('input/tex/base/BaseItems.js');
const ParseOptions=load('input/tex/ParseOptions.js').default;
const TexParser=load('input/tex/TexParser.js').default;
const {AbstractTags,NoTags}=load('input/tex/Tags.js');
const {MmlFactory}=load('core/MmlTree/MmlFactory.js');
const {AbstractMathItem}=load('core/MathItem.js');
const Stack=load('input/tex/Stack.js').default;
new RegExpMap('literal-test',(parser,char)=>parser.Push(parser.create('token','mi',{},char)),/[\s\S]/);
const configuration=new ParserConfiguration([]);
configuration.append(Configuration.local({items:Object.fromEntries(Object.values(Base).map(Class=>[Class.prototype.kind,Class])),handler:{character:['literal-test']}}));
const options=new ParseOptions(configuration,[{mathStyle:'TeX',tagSide:'right',tagIndent:'0.8em'}]);
options.nodeFactory.setMmlFactory(new MmlFactory());options.tags=new NoTags();options.tags.configuration=options;
// These callbacks run against the real parser, not an object with a currentCS field.
{
  const parser=new TexParser('',{},options);parser.currentCS='previous';
  const calls=[];
  const commands=new CommandMap('command-request-check',{});
  commands.add('alias',new Macro('definition',(p,name,arg)=>{
    commands.remove('alias');calls.push(p.currentCS,name,arg);
  },['bound']));
  assert.equal(commands.parse([parser,'alias']),true);
  assert.deepEqual(calls,['\\alias','\\definition','bound']);
  assert.equal(commands.contains('alias'),false);assert.equal(parser.currentCS,'previous');
  assert.equal(commands.parse([parser,'absent']),null);
  commands.add('missing',new Macro('missing',null));
  assert.equal(commands.parse([parser,'missing']),null);assert.equal(parser.currentCS,'previous');
  commands.add('stop',new Macro('stop',()=>false));
  assert.equal(commands.parse([parser,'stop']),false);assert.equal(parser.currentCS,'previous');
  const raw=new MacroMap('raw-request-check',{raw:(p,name)=>{calls.push(p.currentCS,name);}});
  calls.length=0;raw.parse([parser,'raw']);assert.deepEqual(calls,['previous','raw']);
  let failed=false;
  commands.add('inner',new Macro('inner',p=>{calls.push(p.currentCS);if(failed)throw new Error('command failure');}));
  commands.add('outer',new Macro('outer',p=>{calls.push(p.currentCS);commands.parse([p,'inner']);calls.push(p.currentCS);}));
  calls.length=0;commands.parse([parser,'outer']);
  assert.deepEqual(calls,['\\outer','\\inner','\\outer']);assert.equal(parser.currentCS,'previous');
  failed=true;assert.throws(()=>commands.parse([parser,'outer']),/command failure/);
  assert.equal(parser.currentCS,'\\inner');parser.mml();
}
const shape=node=>node.kind==='text'?'text('+node.getText()+')':node.kind+'('+node.childNodes.map(shape).join(',')+')';
for(const [source,expected] of [['','inferredMrow()'],['abc','inferredMrow(mi(text(a)),mi(text(b)),mi(text(c)))'],['α𝛼','inferredMrow(mi(text(α)),mi(text(𝛼)))']]){
  const parser=new TexParser(source,{},options);assert.equal(options.parsers.length,1);
  const node=parser.mml();assert.equal(shape(node),expected);assert.equal(node.attributes.get('data-latex')||'',source);assert.equal(options.parsers.length,0);
}
const nf=options.nodeFactory;
const live=nf.create('token','mi',{},'x'),dead=nf.create('token','mi',{},'x');
options.root=nf.create('node','mrow',[live]);
options.addNode('postprocess',live);options.addNode('postprocess',live);options.addNode('postprocess',dead);
assert.equal(options.getList('postprocess').length,2);assert.equal(live.getProperty('in-lists'),'postprocess,postprocess');
options.removeFromList('postprocess',[live]);assert.equal(options.getList('postprocess').length,1);
options.root.removeChild(live);assert.equal(options.getList('postprocess').length,0);
options.options.maxBuffer=123;options.clear();assert.equal(options.options.maxBuffer,123);assert.equal(options.root,null);assert.deepEqual(options.nodeLists,{});
const tags=new AbstractTags();tags.configuration=options;
tags.label='outer';tags.start('align',true,true);tags.autoTag();tags.autoTag();assert.equal(tags.counter,1);assert.equal(tags.currentTag.tag,'1');
tags.start('split',false,false);tags.label='inner';tags.notag();tags.end();assert.equal(tags.env,'align');assert.equal(tags.label,'outer');assert.equal(tags.history[0].labelId,'inner');
tags.clearTag();assert.equal(tags.currentTag.tag,null);assert.equal(tags.currentTag.noTag,false);assert.equal(tags.label,'outer');
tags.end();assert.equal(tags.history.length,2);assert.equal(tags.label,'outer');
tags.redo=true;tags.refUpdate=true;tags.resetTag();assert.equal(tags.redo,false);assert.equal(tags.refUpdate,false);assert.equal(tags.history.length,0);
const math=new AbstractMathItem('x',null,true);
tags.reset(7);tags.startEquation(math);tags.autoTag();tags.labels.label={tag:'8',id:'mjx-eqn:label'};tags.ids['mjx-eqn:label']=true;tags.redo=true;tags.finishEquation(math);
assert.equal(tags.allCounter,8);assert.equal(math.inputData.recompile.counter,7);assert.equal(tags.allLabels.label.tag,'8');
tags.startEquation(math);assert.equal(tags.refUpdate,true);assert.equal(tags.counter,7);tags.autoTag();tags.finishEquation(math);assert.equal(tags.allCounter,8);
assert.deepEqual(tags.formatRef('A'),['(','A',')']);assert.equal(tags.formatUrl('α #😀','page'),'page#%CE%B1%20%23%F0%9F%98%80');
assert.throws(()=>tags.formatUrl('\uD800',''),URIError);
const content=nf.create('token','mi',{},'x'),tag=nf.create('node','mtd');const table=tags.enTag(content,tag);
assert.equal(table.kind,'mtable');const row=table.childNodes[0];assert.equal(row.kind,'mlabeledtr');assert.equal(row.childNodes[0],tag);assert.equal(row.childNodes[1].childNodes[0].childNodes[0],content);
assert.equal(table.attributes.get('side'),'right');assert.equal(table.attributes.get('minlabelspacing'),'0.8em');
const stack=new Stack(options.itemFactory,{font:'normal'},true);
const opening=options.itemFactory.create('open');stack.Push(opening);stack.env.color='red';
const popped=stack.Pop();assert.equal(popped.env.color,'red');assert.equal(stack.env.color,undefined);assert.equal(stack.env.font,'normal');
const begin=options.itemFactory.create('begin');stack.Push(begin);stack.Push(begin);assert.equal(stack.height,3);assert.equal(stack.Top(),stack.Top(2));
stack.Top().env.boldsymbol=true;assert.equal(stack.Top(2).env.boldsymbol,true);stack.Pop();stack.Pop();
const fractionStack=new Stack(options.itemFactory,{},true);
const numerator=nf.create('token','mi',{},'a'),denominator=nf.create('token','mi',{},'b');
fractionStack.Push(numerator);fractionStack.Push(options.itemFactory.create('over'));fractionStack.Push(denominator);fractionStack.Push(options.itemFactory.create('stop'));
assert.equal(fractionStack.height,1);assert.equal(fractionStack.Top().kind,'mml');
const fraction=fractionStack.Top().First;assert.equal(fraction.kind,'mfrac');assert.equal(fraction.childNodes[0]===numerator,true);assert.equal(fraction.childNodes[1]===denominator,true);
console.log(JSON.stringify({status:'passed',sourceFiles:9,compiledProfile:ref.provenance.compiled_source_sha256,
  limit:'Original configurable parser, lists and tag state/table construction. Native exception cleanup and indexed slot ownership are C++ invariants. Full native packages, nested tag TeX, postfilters and SVG integration remain separate acceptance requirements.'}));
