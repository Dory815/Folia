'use strict';
// Development reference: genuine TeX/BaseConfiguration/ParseOptions/MML classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
assert.equal(hash('.reference/MathJax-src/ts/input/tex/base/BaseConfiguration.ts'),'578d2c6b03f2f3e902476a0b73a5323b12262215fcbdc831a1135c1d9c1737ad');
assert.equal(hash('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/base/BaseConfiguration.js'),'0feedc60ac600b647cf2a1437c7244196a5aec04715f4a301fe6e9ec09722c57');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {BaseConfiguration,BaseTags}=load('input/tex/base/BaseConfiguration.js');
const {TeX}=load('input/tex.js');
const TexParser=load('input/tex/TexParser.js').default;
const {MmlFactory}=load('core/MmlTree/MmlFactory.js');
const {AbstractMathItem}=load('core/MathItem.js');
let cases=0;
function fixture(options={}) {
  const jax=new TeX({packages:['base'],...options});jax.setMmlFactory(new MmlFactory());++cases;
  return {jax,options:jax.parseOptions,
    parse:(source,environment={})=>new TexParser(source,environment,jax.parseOptions).mml(),
    compile:(source,display=false)=>jax.compile(new AbstractMathItem(source,jax,display),reference.document)};
}
const all=(root,kind)=>{const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n);});return result;};
const content=root=>root.isToken?root.getText():root.childNodes.filter(Boolean).map(content).join('');
const attr=(node,name)=>node.attributes.get(name);
{
  const f=fixture(),config=BaseConfiguration;
  assert.equal(config.name,'base');assert.equal(Object.keys(config.items).length,26);assert.equal(config.options.maxMacros,1000);
  assert.deepEqual(config.handler.character,['command','special']);assert.deepEqual(config.handler.macro,['delimiter','macros','lcGreek','ucGreek','mathchar0mi','mathchar0mo','mathchar7']);
  assert.equal(config.postprocessors.length,1);assert.equal(config.postprocessors[0][1],-4);
  assert.equal(f.jax.configuration.handlers.get('character').applicable('x').name,'letter');assert.equal(f.jax.configuration.handlers.get('character').applicable('2').name,'digit');
  const tags=new config.tags.base();tags.configuration=f.options;assert.equal(tags instanceof BaseTags,true);assert.deepEqual(tags.formatRef('3'),['(','3',')']);
}
{
  const f=fixture(),parsed=f.parse('12{,}345.6 .7 , -*`'),numbers=all(parsed,'mn');
  assert.equal(numbers.length,2);assert.equal(numbers[0].getText(),'12,345.6');assert.equal(numbers[1].getText(),'.7');assert.ok(content(parsed).includes('−∗‘'));assert.ok(all(parsed,'mo').some(n=>n.getProperty('fixStretchy')===true));
  assert.throws(()=>f.parse('\\unknowncs'),e=>e.id==='UndefinedControlSequence'&&e.message==='Undefined control sequence \\unknowncs');
  assert.throws(()=>f.parse('\\begin{unknownenv}'),e=>e.id==='UnknownEnv'&&e.message==="Unknown environment 'unknownenv'");
  const error=f.compile('\\unknowncs');assert.equal(all(error,'merror').length,1);assert.ok(content(error).includes('Undefined control sequence \\unknowncs'));
}
{
  const f=fixture(),root=f.parse('中-',{font:'bold'}),ids=all(root,'mi'),ops=all(root,'mo');assert.equal(ids.length,1);assert.equal(attr(ids[0],'mathvariant'),'normal');assert.equal(attr(ops[0],'mathvariant'),'bold');
  const forced=fixture({initialLetter:/z/});let greek=all(forced.parse('α',{italicFont:'bold-italic'}),'mi');assert.equal(greek.length,1);assert.equal(attr(greek[0],'mathvariant'),'bold-italic');
  greek=all(forced.parse('Γ',{italicFont:'bold-italic',font:'bold'}),'mi');assert.equal(attr(greek[0],'mathvariant'),'bold');
}
for(const useString of [false,true]) {
  const expression=/^[A-F]+/;const f=fixture({initialLetter:/[x-z]/,initialDigit:/[A-F]/,digits:useString?'^[A-F]+':expression});
  const root=f.parse('ABCx'),numbers=all(root,'mn');assert.equal(numbers.length,1);assert.equal(numbers[0].getText(),'ABC');assert.equal(all(root,'mi').length,1);
  if(!useString)assert.equal(f.options.options.numberPattern===expression,true);
  assert.equal(f.jax.configuration.handlers.get('character').applicable('A').name,'digit');
}
{
  const f=fixture(),scripts=f.compile('x_{a\\nonscript\\,b\\nonscript\\hskip 2pt c}');assert.equal(all(scripts,'mspace').length,0);
  const text=f.compile('a\\nonscript\\,b\\nonscript\\hskip 2pt c'),spaces=all(text,'mspace');assert.equal(spaces.length,2);assert.ok(spaces.some(n=>attr(n,'width')==='2pt'));assert.equal(f.options.getList('mspace').length,2);
}
{
  const f=fixture(),factory=f.options.nodeFactory,space=factory.create('node','mspace'),style=factory.create('node','mstyle',[space]),wrapped=factory.create('node','mrow',[style]),direct=factory.create('node','mspace'),root=factory.create('node','math',[wrapped,direct]);
  f.options.root=root;wrapped.attributes.set('scriptlevel',1);direct.attributes.set('scriptlevel',1);f.options.addNode('nonscript',wrapped);f.options.addNode('nonscript',direct);
  BaseConfiguration.postprocessors[0][0]({math:new AbstractMathItem('',f.jax),document:reference.document,data:f.options});
  assert.equal(all(root,'mspace').length,0);assert.equal(f.options.nodeLists.mspace.length,0);assert.equal(f.options.nodeLists.mstyle.length,0);assert.equal(f.options.nodeLists.mrow.length,0);
}
console.log(JSON.stringify({cases,baseItems:26,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
