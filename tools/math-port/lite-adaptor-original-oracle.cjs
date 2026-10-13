'use strict';

// Root-owned observations. Only pinned upstream CJS produces the actual records.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const version = JSON.parse(fs.readFileSync(path.join(packageRoot, 'package.json'))).version;
if (version !== '4.1.3') throw new Error('Expected @mathjax/src 4.1.3');
const locks = {
  'ts/core/DOMAdaptor.ts':'fc4bc991ab2e9a35fd2693a38ff6915286e549d884591922a5d7a716630e8af6',
  'ts/adaptors/liteAdaptor.ts':'6aded576671ae8cf4b1146cc82c793628fa593abdc00d05e3e8880c53fe6a214',
  'ts/adaptors/NodeMixin.ts':'c28ccea622471176271cf688de41ae0b3c83f8acb8c44c66fbdd66517d101872',
  'ts/adaptors/lite/Element.ts':'d3fc9147fa7bf658fd2eb523784edc0940f66c6223d61bdd3b9d7f6b5e53bfeb',
  'ts/adaptors/lite/Text.ts':'542c2534c5a4b663fe261c84e30d3fdac59196e362ee7f9c33d4f0faa25d0d59',
  'ts/adaptors/lite/Document.ts':'eeb005e7c8c65be48e57b77da7a0e05f0b36b8718a0c8f4d46830d6984e80221',
  'ts/adaptors/lite/List.ts':'627e5f65fac939f54bdf7a6c6cac143c303762405c732069ccebfa4720bb55d2',
  'ts/adaptors/lite/Window.ts':'4c97e935379a8305420ccdfcbec67b99ac64d75f6191d95c1e67494e563fcfdc',
  'ts/adaptors/lite/Parser.ts':'f3c15592f359cdca1a3c2729c230f8de862e8b67be5ae8f1d2d387e8482fe5d9'
};
const digest = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const sources = Object.entries(locks).map(([file,hash]) => {
  if (digest(path.join(sourceRoot,file)) !== hash) throw new Error(`Pinned source changed: ${file}`);
  const cjs = 'cjs/' + file.slice(3,-3) + '.js';
  return {file,hash,cjs,cjsHash:digest(path.join(packageRoot,cjs))};
});
const load = file => require(path.join(packageRoot,'cjs',file));
const {AbstractDOMAdaptor} = load('core/DOMAdaptor.js');
const {liteAdaptor,LiteBase,LiteAdaptor} = load('adaptors/liteAdaptor.js');
const {NodeMixin} = load('adaptors/NodeMixin.js');
const {LiteElement} = load('adaptors/lite/Element.js');
const {LiteText,LiteComment} = load('adaptors/lite/Text.js');
const {LiteDocument} = load('adaptors/lite/Document.js');
const {LiteList} = load('adaptors/lite/List.js');
const records = [];
function plain(value) {
  if (value === undefined) return {$:'undefined'};
  if (typeof value === 'number' && !Number.isFinite(value))
    return {$:Number.isNaN(value)?'NaN':value<0?'-Infinity':'Infinity'};
  if (typeof value === 'number' && Object.is(value,-0)) return {$:'-0'};
  if (Array.isArray(value)) return Array.from({length:value.length},(_,i) =>
    Object.hasOwn(value,i)?plain(value[i]):{$:'hole'});
  if (value && typeof value === 'object') return Object.fromEntries(
    Object.keys(value).map(key => [key,plain(value[key])]));
  return value;
}
function observe(name,action) {
  try {records.push({name,actual:{value:plain(action())}});}
  catch(error) {records.push({name,actual:{error:{name:error.name,message:error.message}}});}
}
function tree(node) {
  if (!node) return node;
  if (node.kind === '#document') return {kind:node.kind,type:node.type,root:tree(node.root)};
  if (node.kind === '#text' || node.kind === '#comment') return {kind:node.kind,value:node.value};
  return {kind:node.kind,attributes:Object.entries(node.attributes),children:node.children.map(tree)};
}
function caught(action) {
  try {return {value:action()};}
  catch(error) {return {error:{name:error.name,message:error.message}};}
}
observe('constructors-and-default-host-state',() => {
  const base = new LiteBase(), a = liteAdaptor(), b = liteAdaptor({fontSize:20});
  return {baseDocument:base.document,document:a.document,baseCanMeasure:base.canMeasureNodes,
    canMeasure:a.canMeasureNodes,options:a.options,custom:b.options,distinct:a.options!==b.options,
    windowDocument:a.window.document.defaultView===a.window,
    listClass:a.window.NodeList===a.window.HTMLCollection,
    fragmentClass:a.window.NodeList===a.window.DocumentFragment};
});
observe('element-constructor-shallow-copy-and-parent',() => {
  const nested = {a:1}, attrs = {nested,id:'x'}, text = new LiteText('a'), children = [text];
  const n = new LiteElement('p',attrs,children);
  attrs.id='y';children.push(new LiteText('b'));nested.a=2;
  return {attrsDistinct:n.attributes!==attrs,childrenDistinct:n.children!==children,
    nestedSame:n.attributes.nested===nested,parent:n.parent,childParent:text.parent===n,
    styles:n.styles,node:tree(n)};
});
observe('document-and-window-identities',() => {
  const a=liteAdaptor(), d=a.createDocument();a.document=d;
  return {kind:d.kind,type:a.doctype(),root:a.root()===d.root,head:a.head()===d.head,
    body:a.body()===d.body,headParent:d.head.parent===d.root,bodyParent:d.body.parent===d.root,
    defaultView:d.defaultView,headExplicit:a.head(a.window.document)===a.window.document.head,tree:tree(d)};
});
observe('abstract-node-style-and-property-input',() => {
  const a=liteAdaptor(), payload={x:1}, text=a.text('z');
  const n=a.node('span',{id:7,style:{'font-size':'18px',color:'red'},properties:{payload,flag:true}},[text],'ignored');
  return {tree:tree(n),same:a.getProperty(n,'payload')===payload,flag:a.getProperty(n,'flag'),
    propertyAttribute:a.hasAttribute(n,'payload'),style:a.getStyle(n,'font-size'),parent:text.parent===n};
});
observe('append-moves-and-reorders-live-child',() => {
  const a=liteAdaptor(), x=a.node('x'), y=a.node('y'), p=a.node('p',{},[x,y]), q=a.node('q');
  const first=a.append(p,x), second=a.append(q,y);
  return {firstSame:first===x,secondSame:second===y,p:tree(p),q:tree(q),xParent:x.parent===p,yParent:y.parent===q};
});
observe('insert-detaches-even-without-old-parent',() => {
  const a=liteAdaptor(), x=a.node('x'), y=a.node('y'), p=a.node('p',{},[x]), q=a.node('q',{},[y]);
  const result=a.insert(x,y), before=tree(q);
  a.insert(x,a.node('orphan'));
  return {result,before,p:tree(p),q:tree(q),parent:x.parent};
});
observe('remove-attached-and-never-attached',() => {
  const a=liteAdaptor(), x=a.text('x'), p=a.node('p',{},[x]), y=a.node('y');
  const r=a.remove(x), s=a.remove(y);
  return {same:r===x,orphanSame:s===y,xParent:x.parent,yParent:y.parent,p:tree(p)};
});
observe('replace-retains-new-node-in-old-parent-array',() => {
  const a=liteAdaptor(), old=a.node('old'), fresh=a.node('fresh'), p=a.node('p',{},[old]), q=a.node('q',{},[fresh]);
  const result=a.replace(fresh,old);
  return {returnedOld:result===old,p:tree(p),q:tree(q),bothArrays:p.children[0]===q.children[0],
    freshParent:fresh.parent===p,oldParent:old.parent};
});
observe('replace-orphan-is-no-op',() => {
  const a=liteAdaptor(), old=a.node('old'), fresh=a.node('fresh'), p=a.node('p',{},[fresh]);
  const result=a.replace(fresh,old);
  return {returnedOld:result===old,p:tree(p),oldParent:old.parent,freshParent:fresh.parent===p};
});
observe('clone-shallow-attributes-without-custom-properties',() => {
  const a=liteAdaptor(), nested={x:1}, n=a.node('p',{},[a.node('b'),a.text('t')]);
  n.attributes.nested=nested;a.setStyle(n,'color','red');a.setProperty(n,'payload',nested);
  const c=a.clone(n,false);
  return {same:c===n,attrsDistinct:c.attributes!==n.attributes,nestedSame:c.attributes.nested===nested,
    parent:c.parent,styles:c.styles,payload:a.getProperty(c,'payload'),clone:tree(c),original:tree(n)};
});
observe('clone-deep-text-and-comment-parent-slots',() => {
  const a=liteAdaptor(), n=a.node('p',{},[a.text('t'),a.comment('<!--c-->'),a.node('b',{},[a.text('v')])]), c=a.clone(n);
  return {tree:tree(c),parent:c.parent,textParent:c.children[0].parent,commentParent:c.children[1].parent,
    elementParent:c.children[2].parent===c,innerTextParent:c.children[2].children[0].parent,
    distinct:c.children.map((x,i)=>x!==n.children[i])};
});
observe('split-uses-utf16-slice-and-negative-index',() => {
  const a=liteAdaptor(), t=a.text('A\u{1F600}B'), p=a.node('p',{},[t]);
  const right=a.split(t,2), last=a.split(right,-1);
  return {tree:tree(p),rightParent:right.parent===p,lastParent:last.parent===p,
    indices:[a.childIndex(t),a.childIndex(right),a.childIndex(last)]};
});
observe('navigation-copy-and-empty-slots',() => {
  const a=liteAdaptor(), x=a.node('x'), t=a.text('t'), y=a.node('y'), p=a.node('p',{},[x,t,y]), empty=a.node('e');
  const copy=a.childNodes(p);copy.pop();
  return {first:a.firstChild(p)===x,last:a.lastChild(p)===y,next:a.next(x)===t,previous:a.previous(y)===t,
    noNext:a.next(y),noPrevious:a.previous(x),orphanNext:a.next(empty),orphanIndex:a.childIndex(empty),
    emptyFirst:a.firstChild(empty),emptyLast:a.lastChild(empty),outOfBounds:a.childNode(p,3),
    originalLength:p.children.length,copyLength:copy.length};
});
observe('contains-self-and-ancestor-chain',() => {
  const a=liteAdaptor(), t=a.text('t'), child=a.node('b',{},[t]), p=a.node('p',{},[child]);
  return {self:a.contains(p,p),text:a.contains(p,t),reverse:a.contains(child,p),
    missing:a.contains(p,null),orphan:a.contains(p,a.node('o'))};
});
observe('value-and-comment-delimiters',() => {
  const a=liteAdaptor();
  return {text:a.value(a.text('a')),element:a.value(a.node('p')),
    comments:['<!--x-->','<!DOCTYPE html>','plain','<!--a\nb-->'].map(s=>a.value(a.comment(s)))};
});
observe('textcontent-skips-comments-and-keeps-nested-text',() => {
  const a=liteAdaptor(), n=a.node('p',{},[a.text('a'),a.comment('<!--b-->'),a.node('span',{},[a.text('c&d')])]);
  return {content:a.textContent(n),inner:a.innerHTML(n),outer:a.outerHTML(n),xml:a.serializeXML(n)};
});
observe('attributes-coercion-order-namespaces-and-presence',() => {
  const a=liteAdaptor(), n=a.node('svg');
  a.setAttribute(n,'zero',0);a.setAttribute(n,'false',false);a.setAttribute(n,'missing',undefined);
  a.setAttribute(n,'x:href','A&B','http://www.w3.org/1999/xlink');a.setAttribute(n,'zero',2);
  const before=a.allAttributes(n);a.removeAttribute(n,'false');
  return {before,after:a.allAttributes(n),hasMissing:a.hasAttribute(n,'missing'),hasFalse:a.hasAttribute(n,'false'),
    missing:a.getAttribute(n,'absent'),xml:a.serializeXML(n),html:a.outerHTML(n)};
});
observe('xml-default-namespace-does-not-mutate-attributes',() => {
  const a=liteAdaptor(), n=a.node('svg',{title:'"<&>'},[a.node('path',{d:'M0 0'}),a.node('br')]);
  const xml=a.serializeXML(n), html=a.outerHTML(n);
  return {xml,html,attributes:a.allAttributes(n),inner:a.innerHTML(n)};
});
observe('classes-space-splitting-versus-allclasses-normalization',() => {
  const a=liteAdaptor(), n=a.node('p',{'class':'  a  a b\tc '});
  const before={all:a.allClasses(n),empty:a.hasClass(n,''),tab:a.hasClass(n,'b\tc')};
  a.addClass(n,'a');a.addClass(n,'d');a.removeClass(n,'a');
  return {before,raw:a.getAttribute(n,'class'),all:a.allClasses(n),stillA:a.hasClass(n,'a')};
});
observe('styles-lazy-read-write-and-attribute-reset',() => {
  const a=liteAdaptor(), n=a.node('p'), absent=a.getStyle(n,'color'), cachedAbsent=n.styles;
  a.setAttribute(n,'style','color: red; padding: 1px 2px');const initial=n.styles;
  const color=a.getStyle(n,'color'), first=n.styles;a.setStyle(n,'margin-top','3px');
  const written=a.allStyles(n);a.setAttribute(n,'style','color: blue');const reset=n.styles;
  return {absent,cachedAbsent,initial,color,written,reset,newColor:a.getStyle(n,'color'),
    newCache:n.styles!==first,raw:a.allStyles(n)};
});
observe('remove-style-attribute-retains-existing-style-cache',() => {
  const a=liteAdaptor(), n=a.node('p',{style:'color: red'});a.getStyle(n,'color');const cache=n.styles;
  a.removeAttribute(n,'style');
  return {has:a.hasAttribute(n,'style'),raw:a.allStyles(n),sameCache:n.styles===cache,color:a.getStyle(n,'color')};
});
observe('insert-rules-replaces-array-with-unparented-text',() => {
  const a=liteAdaptor(), t=a.text('old'), n=a.node('style',{},[t]);
  a.insertRules(n,['a { color: red; }','b { margin: 0; }']);
  return {tree:tree(n),oldParent:t.parent===n,newParent:n.children[0].parent,css:a.cssText(n),
    nonStyleCss:a.cssText(a.node('p',{},[a.text('x')]))};
});
observe('serialization-pcdata-and-self-closing-tags',() => {
  const a=liteAdaptor();
  return ['style','script','textarea','option','div','br'].map(kind=>{
    const n=a.node(kind,{},[a.text('<&>'),a.comment('<!--c-->')]);
    return {kind,inner:a.innerHTML(n),html:a.outerHTML(n),xml:a.serializeXML(n)};
  });
});
observe('serializer-escaping-functions',() => {
  const p=liteAdaptor().parser,s='"<&>\n';
  return {html:p.protectHTML(s),attribute:p.protectAttribute(s,false),xmlAttribute:p.protectAttribute(s,true)};
});
observe('parse-fragment-entities-comments-and-self-close',() => {
  const a=liteAdaptor(), d=a.parse('<p id="a">x&amp;&#x1F600;<!--c--><br/><b title="&quot;">y</b></p>');
  return {tree:tree(d),body:a.outerHTML(a.body(d)),xml:a.serializeXML(a.body(d)),
    text:a.textContent(a.body(d)),assigned:a.document===d,document:a.document};
});
observe('parse-full-document-pcdata-and-doctype',() => {
  const a=liteAdaptor(), d=a.parse('<!DOCTYPE html><html><head><title>A<&></title></head><body><p>B</p></body></html>');
  return {tree:tree(d),type:a.doctype(d),head:a.outerHTML(a.head(d)),body:a.outerHTML(a.body(d)),
    rootParent:d.root.parent,headParent:d.head.parent===d.root,bodyParent:d.body.parent===d.root};
});
observe('parse-head-body-fragments-and-incomplete-tags',() => {
  const a=liteAdaptor();
  return ['<head><title>x</title></head>','<body><p>x</p></body>','x<p id="a"','<?xml version="1.0"?><svg><path/></svg>']
    .map(input=>({input,result:caught(()=>tree(a.parse(input)))}));
});
observe('selectors-root-inclusion-order-stop-and-namespace',() => {
  const a=liteAdaptor(), d=a.parse('<div id="root" class="hit"><span id="a" class="hit" data-k="v"><span id="b" class="hit"></span></span><span id="c" class="hit" data-k="v"></span></div>');
  const n=a.getElement('#root',d), ids=list=>list.map(x=>a.getAttribute(x,'id'));
  return {tags:ids(a.tags(n,'span')),stop:ids(a.tags(n,'span',null,2)),namespace:ids(a.tags(n,'span','svg')),
    classes:ids(a.elementsByClass(n,'hit')),attributes:ids(a.elementsByAttribute(n,'data-k','v')),
    firstClass:a.getElement('.hit',n)===n,rootTag:a.getElement('div',n)===n,
    attribute:a.getAttribute(a.getElement('[data-k="v"]',n),'id'),unsupported:a.getElement('div span',n),
    noId:a.getElement('#absent',n),noClass:a.getElement('.absent',n),noTag:a.getElement('absent',n)};
});
observe('getelements-keeps-order-duplicates-and-list-identity',() => {
  const a=liteAdaptor(), d=a.parse('<p id="a" class="hit"></p><p id="b" class="hit"></p>'), x=a.getElement('#a',d), y=a.getElement('#b',d);
  const input=[x], list=new LiteList(input);input.push(y);list.append(y);
  const selected=a.getElements(['#a','.hit','[id="b"]',x,[y],list,'p x'],d);
  return {ids:selected.map(n=>a.getAttribute(n,'id')),listCopy:list.nodes!==input,
    listValues:list.nodes.map(n=>a.getAttribute(n,'id')),identity:selected[0]===x};
});
observe('litelist-iterator-actual-next-receiver-error',() => {
  const list=new LiteList([new LiteElement('p')]);
  return {arrayFrom:caught(()=>Array.from(list)),next:caught(()=>list[Symbol.iterator]().next())};
});
observe('nodemixin-default-measurement-unicode-boundaries',() => {
  const a=liteAdaptor();
  const inputs=['','abc','a\u{1F600}b','\u1100\u115F\u1160','\u303E\u303F\u3040',
    '\u3247\u3248\u3250','\uA4C6\uA4C7','\u{1B000}\u{1B001}\u{1B002}',
    '\u{1F200}\u{1F251}\u{1F252}','\u{20000}\u{3FFFD}\u{3FFFE}','\uD800\uDC00\uD800','a\u0301'];
  return inputs.map(text=>{const n=a.node('span',{},[a.text(text)]);return {text,size:a.nodeSize(n),
    sizeOtherEm:a.nodeSize(n,100,true),fontSize:a.fontSize(n),fontFamily:a.fontFamily(n),bbox:a.nodeBBox(n)};});
});
observe('nodemixin-custom-live-options-ignore-css-measurement',() => {
  const a=liteAdaptor({fontSize:22,fontFamily:'native-test',cjkCharWidth:2,unknownCharWidth:0.4,unknownCharHeight:1.2});
  const n=a.node('span',{style:'font-size: 90px; font-family: Other'},[a.text('a\u4E00\u{20000}')]);
  const before=a.nodeSize(n);a.options.unknownCharWidth=0.7;
  return {before,after:a.nodeSize(n,22),fontSize:a.fontSize(n),fontFamily:a.fontFamily(n),options:a.options};
});
observe('nodemixin-false-flags-use-base-methods',() => {
  const Mixed=NodeMixin(LiteBase,{badCSS:false,badSizes:false}), a=new Mixed(null,{}), n=a.node('p',{},[a.text('abc')]);
  return {options:a.options,canMeasure:a.canMeasureNodes,fontSize:a.fontSize(n),fontFamily:a.fontFamily(n),
    size:a.nodeSize(n,10),bbox:a.nodeBBox(n)};
});
observe('nodemixin-derived-live-class-defaults',() => {
  class Derived extends LiteAdaptor {}
  Derived.OPTIONS={...LiteAdaptor.OPTIONS,fontSize:25};
  const a=new Derived(null,{fontFamily:'A'});Derived.OPTIONS.fontSize=30;
  const b=new Derived(null,{});
  return {first:a.options,second:b.options,firstFont:a.fontSize(null),secondFont:b.fontSize(null),
    distinct:a.options!==Derived.OPTIONS};
});
observe('prototype-and-static-member-inventory',() => {
  const inventory=cls=>Object.getOwnPropertyNames(cls.prototype).map(name=>{
    const d=Object.getOwnPropertyDescriptor(cls.prototype,name);
    return {name,enumerable:d.enumerable,configurable:d.configurable,writable:d.writable,
      getter:typeof d.get,value:typeof d.value};
  });
  return {abstract:inventory(AbstractDOMAdaptor),base:inventory(LiteBase),adaptor:inventory(LiteAdaptor),
    mixin:inventory(Object.getPrototypeOf(LiteAdaptor)),element:inventory(LiteElement),text:inventory(LiteText),
    comment:inventory(LiteComment),document:inventory(LiteDocument),list:inventory(LiteList),
    cjkSource:LiteAdaptor.cjkPattern.source,cjkFlags:LiteAdaptor.cjkPattern.flags};
});
process.stdout.write(JSON.stringify({version,sources,records,
  uncovered:['createWorker asynchronous node:worker_threads lifecycle','browser adaptors','full HTML conformance beyond LiteParser source']},null,2)+'\n');
