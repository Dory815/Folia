'use strict';
// Development witness: original extpfeil package and actual TeX/MML classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
assert.equal(digest('.reference/MathJax-src/ts/input/tex/extpfeil/ExtpfeilConfiguration.ts'),'7dded3444fecd39520046ab4ccf3b9f0e9c397338e2b2056bdc074758a202cce');
assert.equal(digest('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/extpfeil/ExtpfeilConfiguration.js'),'e51f5adac0bf1485dc10a327b30bdb5bb54b1064c692f0ee855b805a4b5941a4');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {NodeFactory}=load('input/tex/NodeFactory.js');
const {AmsMethods}=load('input/tex/ams/AmsMethods.js');
const {NewcommandConfig}=load('input/tex/newcommand/NewcommandConfiguration.js');
const {ExtpfeilConfiguration}=load('input/tex/extpfeil/ExtpfeilConfiguration.js');
assert.deepEqual(ExtpfeilConfiguration.handler.macro,['extpfeil']);assert.equal(ExtpfeilConfiguration.config===NewcommandConfig,true);
let cases=0;
function fixture(groups=false){let error='',message='';const jax=new TeX({packages:groups?['base','begingroup','newcommand','extpfeil']:['base','extpfeil'],formatError:(jax,e)=>{error=e.id;message=e.message;return jax.formatError(e);}});jax.setMmlFactory(new MmlFactory());return {jax,compile(source){++cases;error='';message='';const root=jax.compile(new AbstractMathItem(source,jax,false),reference.document);return {root,error,message};}};}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n);});return result;}
function ok(f,source){const result=f.compile(source);assert.equal(result.error,'',source);return result.root;}
function arrow(root,glyph){const node=nodes(root,'mo').find(n=>n.getText()===glyph);assert.ok(node);assert.equal(node.attributes.get('stretchy'),true);return node;}
const mappings=[['xtwoheadrightarrow','↠',0x21a0,12,16,'+1.556em','0.667em'],['xtwoheadleftarrow','↞',0x219e,17,13,'+1.667em','0.944em'],['xmapsto','↦',0x21a6,6,7,'+0.722em','0.333em'],['xlongequal','=',0x3d,7,7,'+0.778em','0.389em'],['xtofrom','⇄',0x21c4,12,12,'+1.333em','0.667em']];
{
 const f=fixture(),map=f.jax.parseOptions.handlers.retrieve('extpfeil');
 for(const [name,glyph,code,left,right,width,lspace]of mappings){const entry=map.lookup(name);assert.equal(entry.func===AmsMethods.xArrow,true);assert.deepEqual(entry.args,[code,left,right]);
   for(const suffix of ['{a}','[b]{a}','[]{}']){const root=ok(f,'\\'+name+suffix);arrow(root,glyph);const padded=nodes(root,'mpadded');assert.equal(padded.length,suffix==='[b]{a}'?2:1);for(const pad of padded){assert.equal(pad.attributes.get('width'),width);assert.equal(pad.attributes.get('lspace'),lspace);}}
 }
 assert.equal(typeof map.lookup('Newextarrow').func,'function');
}
{
 const f=fixture();ok(f,'\\Newextarrow{\\custom}{5,10}{8594}');arrow(ok(f,'\\custom[b]{a}'),'→');f.jax.reset(9);arrow(ok(f,'\\custom{a}'),'→');
 ok(f,'\\Newextarrow{\\custom}{6,7}{0X21A6}');arrow(ok(f,'\\custom{a}'),'↦');
 ok(f,'\\Newextarrow{\\!}{01,002}{0x2190}');arrow(ok(f,'\\!{a}'),'←');
 ok(f,'\\Newextarrow{\\α}{1,2}{8594}');arrow(ok(f,'\\α{a}'),'→');
}
{
 const f=fixture(true);ok(f,'\\Newextarrow{\\custom}{1,2}{0x2192}');ok(f,'\\begingroup\\Newextarrow{\\custom}{3,4}{0x2190}');arrow(ok(f,'\\custom{a}'),'←');arrow(ok(f,'\\endgroup\\custom{a}'),'→');
 ok(f,'\\begingroup\\global\\Newextarrow{\\custom}{1,2}{0x21A6}\\endgroup');arrow(ok(f,'\\custom{a}'),'↦');
 ok(f,'\\let\\saved\\custom\\Newextarrow{\\custom}{1,2}{0x2190}');arrow(ok(f,'\\saved{a}'),'↦');arrow(ok(f,'\\custom{a}'),'←');
 const failure=f.compile('\\let\\define\\Newextarrow\\define{bad}{bad}{bad}');assert.equal(failure.error,'NewextarrowArg1');assert.equal(failure.message,'First argument to \\Newextarrow must be a control sequence name');
}
{
 const f=fixture(),requests=[];f.jax.parseOptions.nodeFactory.set('token',(factory,kind,def,text)=>{if(kind==='mo')requests.push({text,stretchy:def.stretchy,texClass:def.texClass});return NodeFactory.createToken(factory,kind,def,text);});
 ok(f,'\\Newextarrow{\\custom}{5,10}{8594}\\custom{a}');assert.deepEqual(requests,[{text:'→',stretchy:true,texClass:0}]);
}
for(const [source,id]of [
 ['\\Newextarrow{bad}{bad}{bad}','NewextarrowArg1'],['\\Newextarrow{\\foo}{bad}{bad}','NewextarrowArg2'],['\\Newextarrow{\\foo}{1,2}{bad}','NewextarrowArg3'],
 ['\\Newextarrow{bad}','MissingArgFor'],['\\Newextarrow{bad}{bad}','MissingArgFor'],['\\Newextarrow{\\foo}{1, 2}{8594}','NewextarrowArg2'],
 ['\\Newextarrow{\\foo}{-1,2}{8594}','NewextarrowArg2'],['\\Newextarrow{\\foo}{1,2}{0xZZ}','NewextarrowArg3'],['\\Newextarrow{\\foo}{1,2}{-1}','NewextarrowArg3'],
 ['\\Newextarrow{\\ab1}{1,2}{8594}','NewextarrowArg1'],['\\Newextarrow{\\😀}{1,2}{8594}','NewextarrowArg1']
]) {const f=fixture();assert.equal(f.compile(source).error,id,source);ok(f,'a+b');}
{
 const f=fixture();ok(f,'\\Newextarrow{\\invalid}{1,2}{1114112}');assert.throws(()=>f.compile('\\invalid{a}'),RangeError);arrow(ok(f,'\\xmapsto{a}'),'↦');
}
console.log(JSON.stringify({commands:6,arrows:5,cases,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
