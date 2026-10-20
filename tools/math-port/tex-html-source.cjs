'use strict';
// Original HTML TeX methods and actual MML/NodeFactory metadata behavior.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for(const [file,sha]of [
 ['.reference/MathJax-src/ts/input/tex/html/HtmlMethods.ts','128fcaa123ed0ea01c1ce228c1038120e6ecee90ce770b1814eedb986fd573a8'],
 ['.reference/MathJax-src/ts/input/tex/html/HtmlConfiguration.ts','f1223a3359611a6fd79a364c5b3b0c7fef31e17dcbc28fcbaafbb315d00b0ca0'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/html/HtmlMethods.js','fabb2f71a057bf3c6f3f52e23ebd0ea93fbe744500cf3e6792c41dd8c37b8234'],
 ['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/html/HtmlConfiguration.js','284d5341c6b8055e540e2a286956ece2deb83925945f204d268d6ece25b408f0']
]) assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),sha,file);
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {NodeFactory}=load('input/tex/NodeFactory.js'),methods=load('input/tex/html/HtmlMethods.js').default;
const {HtmlConfiguration}=load('input/tex/html/HtmlConfiguration.js');
assert.deepEqual(HtmlConfiguration.handler.macro,['html_macros']);
let cases=0;
function fixture(aliases=false,selected=true){
 let error='',message='';
 const jax=new TeX({packages:['base',...(selected?['html']:[]),...(aliases?['newcommand']:[])],formatError(j,e){error=e.id;message=e.message;return j.formatError(e)}});
 jax.setMmlFactory(new MmlFactory());
 return {jax,compile(s){++cases;error='';message='';const tree=jax.compile(new AbstractMathItem(s,jax,false),reference.document);return {tree,error,message}}};
}
function ok(f,s){const r=f.compile(s);assert.equal(r.error,'',s);return r.tree;}
function nodes(root,key){const result=[];root.walkTree(n=>{if(n.attributes?.getExplicit(key)!==undefined)result.push(n);});return result;}
function one(root,key){const n=nodes(root,key);assert.equal(n.length,1,key);return n[0];}
const f=fixture(),map=f.jax.parseOptions.handlers.retrieve('html_macros');
for(const [name,method]of [['data','Data'],['href','Href'],['class','Class'],['style','Style'],['cssId','Id']])
 assert.equal(map.lookup(name).func===methods[method],true);
assert.equal(fixture(false,false).compile('\\href{a}{x}').error,'UndefinedControlSequence');one(ok(f,'\\href{a}{x}'),'href');
for(const [content,kind,count]of [['x','mi',1],['xy','mrow',2],['x+y','mrow',3],['','mrow',0],['\\frac{a}{b}','mfrac',2]]) {
 const n=one(ok(f,'\\href{url?x=1&y=2#z}{'+content+'}'),'href');assert.equal(n.kind,kind);assert.equal(n.childNodes.length,count);assert.equal(n.attributes.get('href'),'url?x=1&y=2#z');
}
{
 const n=one(ok(f,'\\cssId{outer}{\\href{target}{xy}}'),'id');assert.equal(n.kind,'mrow');assert.equal(n.attributes.get('href'),'target');assert.equal(n.attributes.get('id'),'outer');
}
{
 const n=one(ok(f,'\\data{item={a,b=c},flag,falseval=false,raw=12,raw=13,trail=x\\ ,=empty}{xy}'),'data-item');
 assert.equal(n.kind,'mrow');
 for(const [key,value]of [['item','a,b=c'],['flag',true],['falseval',false],['raw','13'],['trail','x\\ '],['','empty']])assert.equal(n.attributes.get('data-'+key),value);
}
for(const key of ['dash-name','under_score','colon:name','a<b','日本語','a😀b','\ud800'])
 assert.equal(one(ok(f,'\\data{'+key+'=value}{x}'),'data-'+key).attributes.get('data-'+key),'value');
const invalid=['a b','a"b',"a'b",'a>b','a/b','a\tb',...['\0','\x1f','\x7f','\x9f'].map(c=>'a'+c+'b')];
for(let c=0xfdd0;c<=0xfdef;++c)invalid.push(String.fromCodePoint(c));
for(let p=0;p<=16;++p)invalid.push(String.fromCodePoint((p<<16)+0xfffe),String.fromCodePoint((p<<16)+0xffff));
for(const key of invalid){const r=f.compile('\\data{'+key+'=x}{y}');assert.equal(r.error,'InvalidHTMLAttr');assert.equal(r.message,'Invalid HTML attribute: data-'+key);}
ok(f,'\\data{item=x}{y}');
assert.equal(one(ok(f,'\\class{outer}{\\class{inner}{xy}}'),'class').attributes.get('class'),'inner outer');
assert.equal(one(ok(f,'\\class{outer}{\\mmlToken{mi}[class=true]{x}}'),'class').attributes.get('class'),'true outer');
assert.equal(one(ok(f,'\\cssId{outer}{\\cssId{inner}{x}}'),'id').attributes.get('id'),'outer');
for(const [oldStyle,newStyle,result]of [
 ['color:red','width:1em','color:red; width:1em'],['a;b','xy','a;b xy'],
 ['x;','abc','x;; abc'],['x;','','x;; '],['','color:red','color:red']
]) assert.equal(one(ok(f,'\\style{'+newStyle+'}{\\style{'+oldStyle+'}{x}}'),'style').attributes.get('style'),result);
{
 const f=fixture(),requests=[];
 f.jax.parseOptions.nodeFactory.set('node',(factory,kind,children,def,text)=>{requests.push(kind);return NodeFactory.createNode(factory,kind,children,def,text);});
 assert.equal(one(ok(f,'\\class{outer}{\\href{target}{xy}}'),'class').attributes.get('href'),'target');assert.equal(requests.filter(k=>k==='mrow').length,1);
}
{
 const f=fixture(true);ok(f,'\\let\\saved\\href');assert.equal(one(ok(f,'\\saved{target}{x}'),'href').attributes.get('href'),'target');
 assert.equal(f.compile('\\data{bad key=x}{\\unknown}').error,'UndefinedControlSequence');
 assert.equal(f.compile('\\data{bad key=x}{y}').error,'InvalidHTMLAttr');
 for(const cmd of ['data','href','class','style','cssId'])assert.equal(f.compile('\\'+cmd+'{value}').error,'MissingArgFor');
 assert.throws(()=>f.compile('\\style{color:red}{\\mmlToken{mi}[style=true]{x}}'),TypeError);ok(f,'\\href{target}{y}');
}
console.log(JSON.stringify({methods:5,maps:1,cases,invalidNames:invalid.length,compiled_source_sha256:reference.provenance.compiled_source_sha256}));