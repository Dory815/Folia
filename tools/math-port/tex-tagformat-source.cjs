'use strict';
// Original Tagformat/Tags and real TeX equations, labels and references.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for(const [file,hash]of [
 ['.reference/MathJax-src/ts/input/tex/tagformat/TagFormatConfiguration.ts','fdb658f76cd044e64b233b999e216b93720c83ca2d0ede3b033d5759f4755a42'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/tagformat/TagFormatConfiguration.js','54cb07c08095308bd78d232998366afe0053d74abfe1829cdf2b640921ab29bd'],
 ['.reference/MathJax-src/ts/input/tex/Tags.ts','5a73a8d16f1d3c87ece167fc06dac0c323ea05a519e5ee5ea9aadbb76cef9a5d'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/Tags.js','e027c7edb6f7207f3b3ce7e7203dcfc763219b1fa37a3b39925ae83c1beef896']
])assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),hash,file);
const reference=require('./reference-profile.cjs').createReference();assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {TagsFactory,AllTags}=load('input/tex/Tags.js'),{Configuration}=load('input/tex/Configuration.js');
const {TagFormatConfiguration}=load('input/tex/tagformat/TagFormatConfiguration.js');
const defaults=TagFormatConfiguration.options.tagformat;
assert.equal(defaults.number(7),'7');assert.deepEqual(defaults.tag('X'),['(','X',')']);assert.equal(Boolean(defaults.ref),false);assert.equal(defaults.id('a b\u00a0c'),'mjx-eqn:a_b_c');assert.equal(defaults.url('a% /?α😀','/doc'),'/doc#a%25%20%2F%3F%CE%B1%F0%9F%98%80');
const custom={number:n=>'N'+(n+10),tag:t=>['[',t,']'],ref:t=>'ref:'+t,id:id=>'id:'+id,url:(id,base)=>base+'/to/'+id};
let parentCalls=0;
// Match the pinned CJS target: its generated subclasses call their parent with apply.
function ParentTags(){AllTags.apply(this,arguments)}
Object.setPrototypeOf(ParentTags,AllTags);ParentTags.prototype=Object.create(AllTags.prototype);ParentTags.prototype.constructor=ParentTags;
Object.assign(ParentTags.prototype,{
 formatNumber(n){return 'P'+n},formatTag(t){return 'parent:'+t},formatId(id){return 'parent:'+id},formatUrl(id,base){return base+'!'+id},
 finalize(node,env){++parentCalls;const result=AllTags.prototype.finalize.call(this,node,env);result.attributes.set('data-parent','yes');return result}
});
Configuration.create('tagformat-parent',{tags:{parent:ParentTags}});
let cases=0;
function fixture(options={},selected=true,parent=false){
 let error='';TagsFactory.setDefault('none');const jax=new TeX({packages:['base','ams',...(parent?['tagformat-parent']:[]),...(selected?['tagformat']:[])],tags:'all',...options,formatError:(j,e)=>{error=e.id;return j.formatError(e)}});jax.setMmlFactory(new MmlFactory());
 return {jax,compile(source,display=true){++cases;error='';const tree=jax.compile(new AbstractMathItem(source,jax,display),reference.document);return {tree,error}}};
}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n)});return result;}
function text(root){let s='';root.walkTree(n=>{if(n.isToken)s+=n.getText()});return s;}
function tags(root){return nodes(root,'mlabeledtr').map(row=>row.childNodes[0]);}
function refs(root){const result=[];root.walkTree(n=>{if(n.attributes?.get('class')==='MathJax_ref')result.push(n)});return result;}
function ok(f,source,display=true){const result=f.compile(source,display);assert.equal(result.error,'',source);return result.tree;}
{
 const f=fixture({baseURL:'/doc'}),root=ok(f,'x\\label{a}');assert.deepEqual(tags(root).map(text),['(1)']);assert.equal(tags(root)[0].attributes.get('id'),'mjx-eqn:a');assert.equal(f.jax.parseOptions.tags.allLabels.a.tag,'1');
 let r=refs(ok(f,'\\ref{a}+\\eqref{a}',false));assert.deepEqual(r.map(text),['1','(1)']);assert.deepEqual(r.map(n=>n.attributes.get('href')),['/doc#mjx-eqn%3Aa','/doc#mjx-eqn%3Aa']);
 assert.equal(f.jax.parseOptions.tags.allCounter,1);assert.equal(tags(ok(f,'y'))[0].attributes.get('id'),'mjx-eqn:2');f.jax.reset(7);assert.deepEqual(tags(ok(f,'z')).map(text),['(8)']);
}
{
 const f=fixture({tagformat:custom,baseURL:'/doc'});let root=ok(f,'x\\label{a}');assert.deepEqual(tags(root).map(text),['[N11]']);assert.equal(tags(root)[0].attributes.get('id'),'id:a');
 let r=refs(ok(f,'\\ref{a}+\\eqref{a}',false));assert.deepEqual(r.map(text),['N11','ref:N11']);assert.equal(r[0].attributes.get('href'),'/doc/to/id:a');
 f.jax.parseOptions.options.tagformat={...custom,number:n=>'U'+n,tag:t=>'<'+t+'>',ref:''};root=ok(f,'y\\label{b}');assert.deepEqual(tags(root).map(text),['<U2>']);assert.equal(nodes(tags(root)[0],'mtext').length,1);assert.equal(text(refs(ok(f,'\\eqref{b}',false))[0]),'<U2>');
 root=ok(f,'x\\tag{X}');assert.deepEqual(tags(root).map(text),['<X>']);root=ok(f,'x\\tag*{X}');assert.deepEqual(tags(root).map(text),['X']);
 f.jax.parseOptions.options.tagformat=undefined;assert.throws(()=>f.jax.parseOptions.tags.formatTag('X'),TypeError);f.jax.parseOptions.options.tagformat={...defaults};assert.equal(text(tags(ok(f,'z'))[0]),'(3)');
}
const modes=[['none','x',0],['none','\\begin{equation}x\\end{equation}',0],['none','x\\tag{E}',1],['all','x',1],['all','\\begin{equation*}x\\end{equation*}',0],['ams','x',0],['ams','\\begin{equation}x\\end{equation}',1],['ams','\\begin{align}x&=y\\\\a&=b\\end{align}',2]];
for(const [mode,source,count]of modes)assert.equal(tags(ok(fixture({tags:mode}),source)).length,count,mode+source);
{
 const f=fixture({tags:'unregistered'});assert.equal(tags(ok(f,'x')).length,0);assert.deepEqual(tags(ok(f,'x\\tag{E}')).map(text),['(E)']);
}
const urls=[['a b','mjx-eqn:a_b','/doc#mjx-eqn%3Aa_b'],['α😀','mjx-eqn:α😀','/doc#mjx-eqn%3A%CE%B1%F0%9F%98%80'],['a%/:?#[]()','mjx-eqn:a%/:?#[]()','/doc#mjx-eqn%3Aa%25%2F%3A%3F%23%5B%5D()'],['a\u00a0b','mjx-eqn:a_b','/doc#mjx-eqn%3Aa_b']];
for(const [label,id,url]of urls){const f=fixture({baseURL:'/doc'});assert.equal(tags(ok(f,'x\\label{'+label+'}'))[0].attributes.get('id'),id);assert.equal(refs(ok(f,'\\eqref{'+label+'}',false))[0].attributes.get('href'),url);}
{
 const a=fixture({tagformat:custom}),b=fixture();assert.deepEqual(tags(ok(a,'x')).map(text),['[N11]']);assert.deepEqual(tags(ok(b,'x')).map(text),['(1)']);assert.deepEqual(tags(ok(a,'y')).map(text),['[N12]']);
 const absent=fixture({},false);absent.jax.parseOptions.options.tagformat=custom;assert.deepEqual(tags(ok(absent,'x')).map(text),['(1)']);
}
{
 const f=fixture({tags:'parent'},true,true);assert.deepEqual(tags(ok(f,'x')).map(text),['(1)']);assert.ok(parentCalls>0);assert.equal(nodes(ok(f,'y'),'mtable')[0].attributes.get('data-parent'),'yes');
 const absent=fixture({tags:'parent'},false,true);assert.deepEqual(tags(ok(absent,'x')).map(text),['parent:P1']);assert.equal(absent.jax.parseOptions.tags.formatUrl('x','base'),'base!x');
}
const errors=[['\\tag','MissingArgFor'],['x\\tag{X}\\tag{Y}','MultipleCommand'],['x\\label{a}\\label{b}','MultipleCommand'],['\\unknown','UndefinedControlSequence']];
for(const [source,id]of errors){const f=fixture();assert.equal(f.compile(source).error,id,source);assert.deepEqual(tags(ok(f,'x\\label{after}')).map(text),['(1)']);}
{
 const f=fixture();ok(f,'x\\label{a}');assert.equal(f.compile('y\\label{a}').error,'MultipleLabel');ok(f,'z\\label{after}');
}
console.log(JSON.stringify({callbacks:5,modes:modes.length,urlCases:urls.length,errorCases:errors.length+1,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
