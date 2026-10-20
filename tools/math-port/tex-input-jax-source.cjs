'use strict';
// Development witness for the original input jax lifecycle, using actual classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for(const [file,sha] of [
  ['.reference/MathJax-src/ts/input/tex.ts','151d3c73697bfbf874001549f6e0c80dbebeabba2b36ab50501a4ffd7753bf06'],
  ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex.js','ef2852b197173f373e62326016e980aef01ffc838f5f766ab6076262f944eada'],
  ['.reference/MathJax-src/ts/core/InputJax.ts','99af0b13253a27a3c3db639f5cc6531580166aa4e569bc225fb7327257696948'],
  ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/InputJax.js','e2e330b4ba637c93bc3d2411e59129d681f865be7ad76297e8ab79d8f0f01f7a']
]) assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),sha,file);
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js');
const {AbstractMathItem}=load('core/MathItem.js');
const {Configuration}=load('input/tex/Configuration.js');
const {NodeFactory}=load('input/tex/NodeFactory.js');
const TexError=load('input/tex/TexError.js').default;
let cases=0;
function fixture(options={}) {
  const jax=new TeX({packages:['base'],...options});jax.setMmlFactory(new MmlFactory());++cases;
  return {jax,compile:(source,display=true)=>jax.compile(new AbstractMathItem(source,jax,display),reference.document)};
}
function all(root,kind){const nodes=[];root.walkTree(n=>{if(n.kind===kind)nodes.push(n);});return nodes;}
function text(root){return root.isToken?root.getText():root.childNodes.filter(Boolean).map(text).join('');}
{
  const f=fixture(),r=f.compile('\\mathbb{R}+\\frac{1}{\\sqrt[3]{x}}+x^2');
  assert.equal(f.jax.name,'TeX');assert.equal(f.jax.processStrings,true);
  assert.equal(f.jax.options.maxBuffer,5120);assert.equal(f.jax.options.maxTemplateSubtitutions,10000);
  assert.equal(f.jax.postFilters.items.length,8);assert.equal(r.kind,'math');assert.equal(r.attributes.get('display'),'block');
  for(const kind of ['mfrac','mroot','msup'])assert.equal(all(r,kind).length,1);
  assert.equal(all(r,'msubsup').length,0);
  assert.ok(all(r,'mi').some(n=>n.getText()==='R'&&n.attributes.get('mathvariant')==='double-struck'));
  assert.equal(f.jax.parseOptions.error,false);assert.equal(f.jax.parseOptions.parsers.length,0);
}
{
  const mark=(d,s)=>d.data.options.baseURL+=s;
  Configuration.create('test-filter-order',{
    config:(_c,j)=>{j.parseOptions.options.baseURL+='config;';j.preFilters.add(d=>mark(d,'config-pre;'));},
    preprocessors:[d=>mark(d,'package-pre;')],postprocessors:[d=>mark(d,'package-post;')]
  });
  const f=fixture({packages:['base','test-filter-order'],preFilters:[d=>{mark(d,'user-pre;');d.math.math='x^2';}],postFilters:[d=>mark(d,'user-post;')]});
  const r=f.compile('replaced',false);assert.equal(all(r,'msup').length,1);assert.equal(r.attributes.get('data-latex'),'x^2');
  assert.equal(f.jax.parseOptions.options.baseURL,'config;user-pre;config-pre;package-pre;user-post;package-post;');
}
{
  const f=fixture(),r=f.compile('\\notACommand');
  assert.equal(f.jax.parseOptions.error,true);assert.equal(all(r,'merror').length,1);assert.ok(text(r).includes('Undefined control sequence'));
  // Source retains the parser whose constructor threw until clear(). C++ uses
  // its accepted parser-scope RAII contract and leaves no dangling parser borrow.
  assert.equal(f.jax.parseOptions.parsers.length,1);
  f.jax.options.formatError=(j,e)=>j.parseOptions.nodeFactory.create('token','mtext',{},e.message.slice(0,10));
  const custom=f.compile('\\notACommand');assert.equal(all(custom,'merror').length,0);assert.equal(all(custom,'mtext').length,1);
  const fail=()=>{throw Error('preprocessor exception');};f.jax.preFilters.add(fail);
  assert.throws(()=>f.compile('x'),/preprocessor exception/);f.jax.preFilters.remove(fail);
  assert.equal(all(f.compile('x'),'mi').length,1);assert.equal(f.jax.parseOptions.error,false);
  assert.equal(text(f.jax.formatError(new TexError('Multiline','first\nsecond'))),'first');
  for(const [message,expected] of [
    ['first\nsecond\nthird','first\nthird'],
    ['first\nsecond\rthird','first\rthird'],
    ['first\nsecond\u2028third','first\u2028third'],
    ['first\nsecond\u2029third','first\u2029third'],
    ['first\rsecond','first\rsecond'],['\n','']
  ])assert.equal(text(f.jax.formatError(new TexError('Multiline',message))),expected);
}
{
  const f=fixture(),r=f.compile('\\hsize{5em} x');
  assert.equal(r.attributes.get('maxwidth'),'5em');assert.equal(r.attributes.get('overflow'),'linebreak');
  f.jax.postFilters.add(d=>{d.data.options.baseURL+='stop;';return false;},-10);
  f.jax.postFilters.add(d=>{d.data.options.baseURL+='unreached;';},-9);f.compile('x');
  assert.equal(f.jax.parseOptions.options.baseURL,'stop;');
}
{
  const f=fixture();let count;
  for(let i=0;i<64;i++){
    f.jax.setMmlFactory(new MmlFactory());const r=f.compile('x_i+\\frac{1}{2}');let size=0;r.walkTree(()=>++size);
    if(i===0)count=size;assert.equal(size,count);assert.equal(f.jax.parseOptions.getList('msub').length,1);assert.equal(f.jax.parseOptions.parsers.length,0);
  }
  f.jax.reset(9);assert.equal(f.jax.parseOptions.root,null);assert.deepEqual(Object.keys(f.jax.parseOptions.nodeLists),[]);assert.equal(f.jax.parseOptions.tags.allCounter,9);
}
{
  const f=fixture();
  Configuration.create('test-dynamic',{
    init:c=>{c.options.maxMacros++;},config:(_c,j)=>{j.parseOptions.options.baseURL+='dynamic;';},options:{maxMacros:99},
    nodes:{node:(factory,kind,children,def,text)=>NodeFactory.createNode(factory,kind,children,{...def,'data-created':'dynamic'},text)},
    preprocessors:[[d=>{d.data.options.baseURL+='package-pre;';},2]]
  });
  f.jax.configuration.add('test-dynamic',f.jax,{maxMacros:123});
  assert.equal(f.jax.parseOptions.options.maxMacros,123);assert.equal(f.jax.configuration.options.maxMacros,100);
  assert.equal(f.jax.parseOptions.options.baseURL,'dynamic;');const r=f.compile('x');
  assert.equal(r.attributes.get('data-created'),'dynamic');assert.equal(f.jax.parseOptions.options.baseURL,'dynamic;package-pre;');
  assert.throws(()=>f.jax.configuration.add('absent',f.jax));
}
console.log(JSON.stringify({cases,methods:['constructor','compile','setMmlFactory','reset','formatError','config','add'],compiled_source_sha256:reference.provenance.compiled_source_sha256}));
