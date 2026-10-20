'use strict';
// Original Braket methods/items/maps, exercised through real TeX/MML classes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
for(const [name,ts,cjs]of [
 ['BraketItems','aa7cb6994db49db972516221ad3fb08ba0c18c6eabaea06440bb1525271f0105','0750ac6d769dd3ff14488b8a51218f000b812f51d637bb9d65c892aef565fb3e'],
 ['BraketMethods','608d674b35a0bfd7153f9f5b66c740a615f6362d8a20d8d1b3fe7b68ea26759e','c20cb54ca55d1b96d35de30491cd3daf8f7177952b4012bea3d2c936c9e64f9d'],
 ['BraketMappings','05087a346704a2aaccbda87433f16717e12173d09d168e8419078e0973ec408d','1152d0239e8bee22d930865a30ff77acac3d8b28e63c5f424052bbd5fcec774e'],
 ['BraketConfiguration','3b70b93e668c2e2cafc254ebb95397f08fb6c62d386ce679c208df4daed18733','1c9458a3c4965a298c41ef18f734beb3fb3709d75f287617fcdd2bdeae9ee569']
])for(const [file,hash]of [['.reference/MathJax-src/ts/input/tex/braket/'+name+'.ts',ts],['tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/braket/'+name+'.js',cjs]])assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),hash,file);
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const load=file=>require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs',file));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js'),{AbstractMathItem}=load('core/MathItem.js');
const {NodeFactory}=load('input/tex/NodeFactory.js'),{BraketItem}=load('input/tex/braket/BraketItems.js');
const {BaseItem}=load('input/tex/StackItem.js');
const methods=load('input/tex/braket/BraketMethods.js').default,base=load('input/tex/base/BaseMethods.js').default;
const {BraketConfiguration}=load('input/tex/braket/BraketConfiguration.js');
assert.deepEqual(Object.keys(methods),['Braket','Bar','Macro']);assert.equal(methods.Macro===base.Macro,true);
assert.equal(BraketConfiguration.priority,3);assert.equal(BraketConfiguration.items.braket===BraketItem,true);
let cases=0;
function fixture(aliases=false,selected=true){let error='',message='';const jax=new TeX({packages:['base',...(selected?['braket']:[]),...(aliases?['newcommand']:[])],formatError:(j,e)=>{error=e.id;message=e.message;return j.formatError(e)}});jax.setMmlFactory(new MmlFactory());return {jax,compile(source){++cases;error='';message='';const tree=jax.compile(new AbstractMathItem(source,jax,false),reference.document);return {tree,error,message}}};}
function nodes(root,kind){const result=[];root.walkTree(n=>{if(n.kind===kind)result.push(n)});return result;}
function text(root){const result=[];root.walkTree(n=>{if(n.isToken)result.push(n.getText())});return result.join('');}
function bars(root){return nodes(root,'mo').filter(n=>n.attributes.getExplicit('data-braketbar')===true);}
function ok(f,source){const r=f.compile(source);assert.equal(r.error,'',source);return r.tree;}
const commands=[
 ['bra','{x}','⟨x|'],['ket','{x}','|x⟩'],['braket','{x}','⟨x⟩'],['set','{x}','{x}'],
 ['Bra','{x}','⟨x|'],['Ket','{x}','|x⟩'],['Braket','{x}','⟨x⟩'],['Set','{x}','{x}'],
 ['ketbra','{x}{y}','|x⟩⟨y|'],['Ketbra','{x}{y}','|x⟩⟨y|']
];
{
 const f=fixture(),map=f.jax.parseOptions.handlers.retrieve('Braket-macros');
 for(const [command,args,expected]of commands){assert.ok(map.lookup(command));assert.equal(text(ok(f,'\\'+command+args)),expected);}
 assert.equal(map.lookup('|').func===methods.Bar,true);assert.equal(f.jax.parseOptions.handlers.retrieve('Braket-characters').lookup('|').func===methods.Bar,true);
 assert.equal(fixture(false,false).compile('\\Braket{x}').error,'UndefinedControlSequence');
 for(const command of ['braket','set','Braket','Set'])for(const source of ['\\'+command+' x','\\'+command+'{}'])assert.equal(bars(ok(f,source)).length,0);
}
const barCases=[
 ['\\braket{a|b|c}',2,'||',false],['\\set{a|b|c}',2,'||',false],
 ['\\Braket{a|b|c}',2,'||',true],['\\Set{a|b|c}',1,'|',true],
 ['\\Braket{a||b}',1,'‖',true],['\\braket{a||b}',1,'‖',false],
 ['\\Braket{a\\|b}',1,'‖',true],['\\braket{a\\|b}',1,'‖',false],
 ['\\Braket{a|{bc}|d}',2,'||',true],
 ['\\Braket{a|\\braket{b|c}|d}',3,'|||',null],
 ['\\Set{a|b||c}',1,'|',true]
];
{
 const f=fixture();for(const [source,count,glyphs,stretchy]of barCases){const root=ok(f,source),b=bars(root);assert.equal(b.length,count,source);assert.equal(b.map(n=>n.getText()).join(''),glyphs,source);if(stretchy!==null)for(const node of b)assert.equal(node.attributes.get('stretchy'),stretchy);}
 for(const source of ['|','||','\\|','{a|b}','\\left|x\\right|'])assert.equal(bars(ok(f,source)).length,0);
 const set=ok(f,'\\Set{x|y}');assert.deepEqual(nodes(set,'mspace').map(n=>n.attributes.get('width')),['0.167em','0.167em']);
 const font=ok(f,'\\mathbf{\\Braket{x|{\\mathit{y}}|z}}w');assert.deepEqual(nodes(font,'mi').map(n=>[n.getText(),n.attributes.get('mathvariant')]),[['x','bold'],['y','-tex-mathit'],['z','bold'],['w','italic']]);
}
const fractions=[
 ['\\Braket{a\\over b|c}',[['a','b']]],['\\Braket{a|b\\over c|d}',[['b','c']]],
 ['\\Braket{a|b\\over c}',[['b','c']]],
 ['\\Braket{a\\over b|c\\over d|e\\over f}',[['a','b'],['c','d'],['e','f']]],
 ['\\Set{a\\over b|c\\over d}',[['a','b'],['c','d']]],
 ['\\braket{a\\over b|c}',[['a','b|c']]]
];
{
 const f=fixture();for(const [source,expected]of fractions){const root=ok(f,source);assert.deepEqual(nodes(root,'mfrac').map(n=>n.childNodes.map(text)),expected,source);}
}
{
 const f=fixture(),requests=[];
 f.jax.parseOptions.nodeFactory.set('token',(factory,kind,def,t)=>{const node=NodeFactory.createToken(factory,kind,def,t);if(kind==='mo'||kind==='mspace')requests.push({kind,text:t,def:{...def},node});return node});
 const root=ok(f,'\\braket{a|b}'),fixed=requests.filter(r=>['|','⟨','⟩'].includes(r.text));
 assert.deepEqual(fixed.map(r=>[r.text,r.def.texClass]),[['|',0],['⟨',4],['⟩',5]]);
 assert.deepEqual(fixed[0].def,{stretchy:false,'data-braketbar':true,texClass:0});
 for(const r of fixed.slice(1))assert.deepEqual(r.def,{fence:true,stretchy:false,symmetric:true,texClass:r.text==='⟨'?4:5});
 assert.equal(bars(root)[0].attributes.get('stretchy'),false);assert.equal(bars(root)[0]===fixed[0].node,true);
 const atoms=[];f.jax.parseOptions.nodeFactory.set('node',(factory,kind,children,def,t)=>{if(kind==='TeXAtom'&&[4,5].includes(def.texClass))atoms.push({children:[...children],class:def.texClass});return NodeFactory.createNode(factory,kind,children,def,t)});
 const stretch=ok(f,'\\Braket{a|b}');assert.deepEqual(atoms.map(r=>[r.class,r.children.length]),[[5,0],[4,0]]);assert.equal(bars(stretch)[0].getProperty('texClass'),2);
 ok(f,'\\Set{a|b}');const spaces=requests.filter(r=>r.kind==='mspace');assert.deepEqual(spaces.map(r=>[r.text,r.def.width]),[[undefined,'0.167em'],[undefined,'0.167em']]);
 // Default empty token text is discarded by the original empty mspace node.
 for(const space of spaces){assert.equal(space.node.childNodes.length,0);assert.equal(space.node.getText(),'');}
}
{
 const f=fixture(),factory=f.jax.parseOptions.itemFactory,nodeFactory=f.jax.parseOptions.nodeFactory;
 const item=factory.create('braket');item.setProperties({open:'⟨',close:'⟩'});
 const x=nodeFactory.create('token','mi',{},'x');assert.equal(item.checkItem(factory.create('mml',x))===BaseItem.fail,true);
 const close=factory.create('close').setProperty('braketbar',true);assert.equal(item.checkItem(close)===BaseItem.fail,true);assert.equal(item.Size(),0);assert.equal(item.barNodes.length,1);assert.equal(item.barNodes[0]===x,true);
 const y=nodeFactory.create('token','mi',{},'y');item.checkItem(factory.create('mml',y));assert.equal(item.toMml(false)===y,true);
 const result=item.checkItem(factory.create('close'));assert.equal(result[1],true);assert.equal(result[0][0].kind,'mml');
 const single=factory.create('braket');single.setProperties({single:true,open:'{',close:'}'});
 const z=nodeFactory.create('token','mi',{},'z'),replay=single.checkItem(factory.create('mml',z));assert.equal(replay[1],true);assert.equal(replay[0][0].kind,'mrow');
}
const errors=[
 ...commands.map(([command])=>['\\'+command,'MissingArgFor']),
 ['\\Braket{a|b','MissingCloseBrace'],['\\Braket{\\unknown|b}','UndefinedControlSequence'],
 ['\\Braket{a|{b|c}|d}','ExtraCloseMissingOpen'],
 ['\\Braket{x\\right)}','MissingLeftExtraRight'],['\\Braket{x&y}','Misplaced'],
 ['\\Braket{a\\over b\\over c}','AmbiguousUseOf'],['\\Ketbra{x}','MissingArgFor']
];
for(const [source,id]of errors){const f=fixture();assert.equal(f.compile(source).error,id,source);assert.equal(bars(ok(f,'\\Braket{x|y}')).length,1);}
{
 const f=fixture(true);ok(f,'\\let\\saved\\Braket\\let\\savedBar\\|\\let\\savedBra\\bra');
 assert.equal(bars(ok(f,'\\saved{x\\savedBar y}'))[0].getText(),'‖');assert.equal(text(ok(f,'\\savedBra{x}')),'⟨x|');
 const r=f.compile('\\saved');assert.equal(r.error,'MissingArgFor');assert.equal(r.message,'Missing argument for \\saved');
 ok(f,'\\saved{x|y}');
}
console.log(JSON.stringify({commands:11,characterMaps:1,methods:2,cases,barCases:barCases.length,fractionCases:fractions.length,errorCases:errors.length,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
