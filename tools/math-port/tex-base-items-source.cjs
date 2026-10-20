'use strict';
// Actual pinned BaseItems, ColumnParser, ParseOptions, TexParser, tags and MML.
// No parser/tag/node factory stand-ins. All expected observations are fixed.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
let root = __dirname;
while (!fs.existsSync(path.join(root, '.reference/MathJax-src/ts/input/tex/base/BaseItems.ts'))) {
  const parent = path.dirname(root); if (parent === root) throw Error('repository root not found'); root = parent;
}
const {createReference} = require(path.join(root, 'tools/math-port/reference-profile.cjs'));
const locks = {
  '.reference/MathJax-src/ts/input/tex/base/BaseItems.ts':'0c0d8448f3577be6642c95a412682310362819d2d383ab83b817d2888e4e5f0c',
  '.reference/MathJax-src/ts/input/tex/ColumnParser.ts':'4bffbf64ef23395d551d0e190043222bfffe7fd71e2417e3fb8008c5f3377d5c',
  'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/base/BaseItems.js':'5310d0429b7b62d405ae9b93c8d21ad17b787e5eb3fd1f01439c62f2094f7d1a',
  'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/ColumnParser.js':'9d09d0b921ce50255facfe303d7771309ad71ea473388167535d5acf711feae9'
};
for (const [file,hash] of Object.entries(locks)) assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex'),hash,file);
const ref = createReference();
assert.equal(ref.provenance.compiled_source_sha256, 'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const cjs = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const Base = require(path.join(cjs, 'input/tex/base/BaseItems.js'));
const {BaseConfiguration} = require(path.join(cjs, 'input/tex/base/BaseConfiguration.js'));
const StackItemFactory = require(path.join(cjs, 'input/tex/StackItemFactory.js')).default;
const TexParser = require(path.join(cjs, 'input/tex/TexParser.js')).default;
const {ColumnParser} = require(path.join(cjs, 'input/tex/ColumnParser.js'));
const options = ref.input.parseOptions;
options.nodeFactory.setMmlFactory(ref.document.mmlFactory);
const factory = new StackItemFactory(BaseConfiguration.items);
factory.configuration = options;
const node = (kind, text) => options.nodeFactory.create('node',kind,[],{},text === undefined ? undefined : options.nodeFactory.create('text',text));
const item = (kind,...args) => factory.create(kind,...args);
const shape = n => ({kind:n.kind, ...(n.isToken || n.kind === 'text' ? {text:n.getText()} : {}), children:n.childNodes.filter(Boolean).map(shape)});
const check = (frame,incoming) => frame.checkItem(incoming);
const errorId = action => { try { action(); assert.fail('expected TexError'); } catch (e) { return e.id; } };
const observations = {};
const parser = new TexParser('', {}, options); // real active request/parser scope

// Construct each actual class through the original factory catalog.
const order = ['start','stop','open','close','null','prime','subsup','over','left','middle','right','break','begin','end','style','position','cell','mml','fn','not','nonscript','dots','array','eqnarray','equation','mstyle'];
assert.deepEqual(Object.keys(BaseConfiguration.items), order);
observations.classes = [];
for (const kind of order) {
  const args = {
    start:[{isInner:true}], prime:[node('mi','x'),node('mo','′')], subsup:[node('msubsup')], left:['('], middle:['|','red'], right:[')','blue'],
    break:['newline',false], mml:[node('mi','x')], fn:[node('mi','sin')], eqnarray:['eqnarray',false,true], equation:[false], mstyle:[{mathcolor:'red'},'color']
  }[kind] || [];
  const frame = item(kind,...args);
  assert.equal(frame.kind,kind);
  observations.classes.push([kind,frame.isOpen,frame.isClose,frame.isFinal]);
  if (kind === 'eqnarray' || kind === 'equation') options.tags.end();
}
assert.equal(observations.classes.length,26);

let opening = item('open'); opening.Push(node('mi','x'));
let closed = check(opening,item('close'));
assert.equal(closed[0][0].First.kind,'TeXAtom');
assert.equal(errorId(() => check(item('open'),item('stop'))),'ExtraOpenMissingClose');
assert.equal(errorId(() => check(item('left','('),item('stop'))),'ExtraLeftMissingRight');
let begin = item('begin').setProperty('name','matrix');
assert.equal(errorId(() => check(begin,item('end').setProperty('name','array'))),'EnvBadEnd');
assert.equal(errorId(() => check(begin,item('stop'))),'EnvMissingEnd');

// Upper positions differ for msup/mover versus msubsup/munderover.
observations.primes = [];
for (const [kind,position] of [['msup',1],['msubsup',2],['mover',1],['munderover',2]]) {
  const base = node(kind); base.setChildren([node('mi','x')]);
  const prime = node('mo','′'), next = item('stop');
  const result = check(item('prime',base,prime),next);
  assert.equal(result[0][0],base); assert.equal(base.childNodes[position],prime); assert.equal(result[0][1],next);
  observations.primes.push([kind,position]);
}
let scripted = node('msubsup'); scripted.setChildren([node('mi','x')]);
let subsup = item('subsup',scripted).setProperties({position:1,primes:node('mo','′'),movesupsub:false});
check(subsup,item('mml',node('mi','i')));
assert.equal(scripted.childNodes[1].getText(),'i'); assert.equal(scripted.childNodes[2].getText(),'′');
assert.equal(scripted.getProperty('movesupsub'),false);
assert.equal(errorId(() => check(item('subsup',node('msubsup')).setProperty('position',2),item('stop'))),'MissingScript');

let over = item('over').setProperties({num:node('mi','a'),thickness:0}); over.Push(node('mi','b'));
let fraction = check(over,item('close'))[0][0].First;
assert.equal(fraction.kind,'mfrac'); assert.equal(fraction.attributes.get('linethickness'),0);
assert.equal(fraction.attributes.get('data-latex-item'),'\\over');
assert.equal(errorId(() => check(item('over'),item('over'))),'AmbiguousUseOf');
let style = item('style').setProperty('styles',{mathcolor:'red'}); style.Push(node('mi','x'));
assert.equal(check(style,item('stop'))[0][0].First.attributes.get('mathcolor'),'red');
let vertical = item('position').setProperties({move:'vertical',dh:'1em',dd:'-1em'});
let padded = check(vertical,item('mml',node('mi','x')))[0][0].First;
assert.equal(padded.attributes.get('height'),'1em'); assert.equal(padded.attributes.get('depth'),'-1em');
assert.equal(errorId(() => check(item('position').setProperty('name','\\raise'),item('close'))),'MissingBoxFor');
let fn = item('fn',node('mi','sin')); assert.equal(check(fn,item('mml',node('mi','x')))[0][1].getText(),'\u2061');
let neg = node('mo','='); check(item('not'),item('mml',neg)); assert.equal(neg.getText(),'≠');
let combined = node('mo','x'); check(item('not'),item('mml',combined)); assert.equal(combined.getText(),'x\u0338');
let nonscript = item('mml',node('mspace')); check(item('nonscript'),nonscript); assert.ok(options.nodeLists.nonscript.includes(nonscript.First));
let ldots = node('mo','…'), cdots = node('mo','⋯');
let dots = item('dots').setProperties({ldots,cdots}); assert.equal(check(dots,item('mml',node('mo','+')))[0][0],cdots);
let breaker = item('break','newline',false), plus = item('mml',node('mo','+'));
check(breaker,plus); assert.equal(plus.First.attributes.get('linebreak'),'newline');
let mstyle = item('mstyle',{mathcolor:'red'},'color'); mstyle.Push(node('mi','x'));
assert.equal(check(mstyle,item('end').setProperty('name','color'))[0][0].kind,'mstyle');
let equation = item('equation',false); equation.Push(node('mi','x'));
assert.equal(check(equation,item('end').setProperty('name','equation'))[0][0].getText(),'x');

// Column-parser mathematical attributes and actual ArrayItem vector aliases.
const columns = new ColumnParser();
const cases = [
  ['lcr',{columnalign:'left center right'}],
  ['p{2em}m{3em}b{4em}',{columnalign:'left left left',columnwidth:'2em 3em 4em'}],
  ['lp{2em}',{columnalign:'left left',columnwidth:'auto 2em'}],
  ['P{2em}M{3em}B{4em}',{columnalign:'left left left',columnwidth:'2em 3em 4em'}],
  ['w{R}{2em}W{c}{3em}',{columnalign:'right center',columnwidth:'2em 3em'}],
  ['|l:r|',{columnalign:'left right',columnlines:'dashed'}],
  ['l@{+}r',{columnalign:'left center right',columnspacing:'0 0'}],
  ['ll@{+}l',{columnalign:'left left center left',columnspacing:'1em 0 0'}],
  ['ll|l',{columnalign:'left left left',columnlines:'none solid'}],
  ['!{x}l!{y}',{columnalign:'center left center',columnspacing:'.5em .5em .5em','data-array-padding':'.5em .5em'}],
  ['*{2}{lc}',{columnalign:'left center left center'}],
  ['{{l}}',{columnalign:'left'}],
  [' \nl ',{columnalign:'left'}],
];
observations.columns = [];
for (const [template,expected] of cases) {
  let array = item('array'); columns.process(parser,template,array);
  for (const [key,value] of Object.entries(expected)) assert.equal(array.arraydef[key],value,template+': '+key);
  observations.columns.push({template,arraydef:array.arraydef,frame:array.frame,cstart:array.cstart,cend:array.cend,cextra:array.cextra,ralign:array.ralign});
}
for (const [template,id] of [['z','BadPreamToken'],['p','MissingArgForColumn'],['p{2}','MissingColumnDimOrUnits'],['p{2em','MissingCloseBrace'],['*{01}{l}','ColArgNotNum']])
  assert.equal(errorId(() => columns.process(parser,template,item('array'))),id,template);
columns.columnHandler.q = state => columns.macroColumn(state,'>{#1}l',1);
let expanded = item('array'); columns.process(parser,'q{prefix}',expanded); assert.equal(expanded.cstart[0],'prefix');
columns.MAXCOLUMNS = 1; assert.equal(errorId(() => columns.process(parser,'lll',item('array'))),'MaxColumns'); columns.MAXCOLUMNS = 10000;
let scanArray = item('array'); scanArray.parser = parser;
parser.string = String.raw`\hline {a&b}\begin{array}x&y\end{array}&tail`; parser.i = 0;
assert.deepEqual(scanArray.getEntry(),['\\hline',' {a&b}\\begin{array}x&y\\end{array}','&',true]);
assert.equal(parser.string,'&tail');
parser.string = 'a&tail'; parser.i = 0; scanArray.cstart = ['x']; scanArray.cend = ['y']; scanArray.StartEntry();
assert.equal(parser.string,'xay&tail');
let rowArray = item('array'); rowArray.arraydef = {rowspacing:'4pt',rowlines:'solid dashed'};
rowArray.Push(node('mi','a')); rowArray.EndEntry(); rowArray.EndRow();
rowArray.Push(node('mi','b')); rowArray.EndEntry(); rowArray.EndRow();
rowArray.checkLines(); rowArray.addRowSpacing('2pt');
assert.deepEqual(rowArray.frame,[['bottom','dashed']]); assert.equal(rowArray.arraydef.rowlines,'solid');
observations.rowspacing = rowArray.arraydef.rowspacing;
assert.equal(observations.rowspacing,'4pt 0.6em');
let framed = item('array'); framed.frame = [['left','solid'],['right','solid']];
assert.equal(framed.createMml().kind,'menclose');
let dashed = item('array'); dashed.frame = [['top','dashed'],['left','solid']];
assert.equal(dashed.createMml().attributes.get('data-frame-styles'),'dashed none none solid');
let eqn = item('eqnarray','align',false,true); eqn.arraydef = {columnalign:'right left',columnwidth:'auto auto',columnspacing:'0 1em'};
for (let i=0;i<3;i++) { eqn.Push(node('mi','x')); eqn.EndEntry(); } eqn.EndRow(); eqn.EndTable();
assert.equal(eqn.arraydef.columnalign,'right left right'); assert.equal(eqn.arraydef.columnspacing,'0 1em');
assert.equal(eqn.table[0].childNodes[1].childNodes[0].childNodes[0].attributes.get('indentshift'),'2em');
parser.mml();

// Real full-profile parser for array template scanning, fences and equation tags.
observations.parses = [];
for (const source of [String.raw`x'+y_1`,String.raw`\left(a\middle|b\right)`,String.raw`\begin{array}{>{x}l<{y}r}a&b\\c&d\end{array}`,String.raw`\begin{equation}x=1\end{equation}`]) {
  ref.document.convert(source,{display:true,em:16,ex:8,containerWidth:1280});
  assert.equal(options.error,false,source);
  observations.parses.push({source,mml:shape(options.root)});
}
process.stdout.write(JSON.stringify({ok:true,sourceHashes:locks,observations},null,2)+'\n');
