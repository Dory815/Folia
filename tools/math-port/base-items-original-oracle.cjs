// Independent MathJax 4.1.3 BaseItems observations. This is an oracle only:
// it runs the pinned package and never loads native code.
'use strict';
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const tsPath = 'ts/input/tex/base/BaseItems.ts';
const cjsPath = 'cjs/input/tex/base/BaseItems.js';
const tsHash = '0c0d8448f3577be6642c95a412682310362819d2d383ab83b817d2888e4e5f0c';
const cjsHash = '5310d0429b7b62d405ae9b93c8d21ad17b787e5eb3fd1f01439c62f2094f7d1a';
const source = path.join(root, '.reference/MathJax-src', tsPath);
const installedTS = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src', tsPath);
const cjs = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src', cjsPath);
const bytes = fs.readFileSync(source);
assert.equal(crypto.createHash('sha256').update(bytes).digest('hex'), tsHash);
assert.equal(fs.readFileSync(installedTS, 'utf8').replaceAll('\r\n', '\n'), bytes.toString('utf8').replaceAll('\r\n', '\n'));
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(cjs)).digest('hex'), cjsHash);

const load = name => require(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs', name));
const { TeX } = load('input/tex.js');
const { MmlFactory } = load('core/MmlTree/MmlFactory.js');
load('input/tex/ams/AmsConfiguration.js');
const itemClasses = load('input/tex/base/BaseItems.js');
const StackItemFactory = load('input/tex/StackItemFactory.js').default;
const { BaseItem } = load('input/tex/StackItem.js');
function clean(value, seen = new WeakSet()) {
  if (value === undefined) return { $: 'undefined' };
  if (value === null) return null;
  if (typeof value === 'number') return Number.isNaN(value) ? { $: 'NaN' } : Object.is(value, -0) ? { $: '-0' } : value;
  if (typeof value !== 'object') return value;
  if (seen.has(value)) return { $: 'cycle' };
  seen.add(value);
  if (Array.isArray(value)) return { $: 'array', items: Array.from({ length: value.length }, (_, i) =>
    Object.hasOwn(value, i) ? clean(value[i], seen) : { $: 'hole' }) };
  const result = {};
  for (const key of Reflect.ownKeys(value)) result[typeof key === 'symbol' ? `symbol:${key.description || ''}` : key] = clean(value[key], seen);
  return result;
}
function tree(node, seen = new WeakSet()) {
  if (node == null) return node;
  if (seen.has(node)) return { $: 'cycle' };
  seen.add(node);
  const attrs = node.attributes?.getAllAttributes?.() || {};
  return { kind: node.kind, text: node.text ?? null, attrs: clean(attrs), properties: clean(node.getAllProperties?.() || {}),
    children: (node.childNodes || []).map(child => tree(child, seen)) };
}
function observe(name, tex, display = false, packages = ['base']) {
  let result = null, error = null;
  try {
    const jax = new TeX({ packages, tags: 'base' });
    jax.setMmlFactory(new MmlFactory());
    const parser = new (load('input/tex/TexParser.js').default)(tex, {}, jax.parseOptions);
    result = { mml: tree(parser.mml()), parser: { i: parser.i, string: parser.string, macroCount: parser.macroCount },
      tags: { counter: jax.parseOptions.tags.counter, currentTag: clean(jax.parseOptions.tags.currentTag),
        labels: clean(jax.parseOptions.tags.labels), ids: clean(jax.parseOptions.tags.ids) } };
  } catch (e) {
    error = { name: e.name || e.constructor.name, id: e.id ?? null, message: String(e.message) };
  }
  return { name, tex, display, error, result };
}
const cases = [
  ['array-basic', String.raw`\begin{array}{cc}a&b\\c&d\end{array}`],
  ['array-left-center-right', String.raw`\begin{array}{lcr}a&b&c\\d&e&f\end{array}`],
  ['array-one-column', String.raw`\begin{array}{c}a\\b\end{array}`],
  ['array-empty-cell', String.raw`\begin{array}{cc}&b\\c&\end{array}`],
  ['array-empty', String.raw`\begin{array}{cc}\end{array}`],
  ['array-hline-top', String.raw`\begin{array}{c}\hline a\\b\end{array}`],
  ['array-hline-bottom', String.raw`\begin{array}{c}a\\\hline\end{array}`],
  ['array-double-hline', String.raw`\begin{array}{c}\hline\hline a\\b\end{array}`],
  ['array-dashed-hline', String.raw`\begin{array}{c}\hdashline a\\b\end{array}`],
  ['array-hfill-left', String.raw`\begin{array}{c}\hfill a\end{array}`],
  ['array-hfill-right', String.raw`\begin{array}{c}a\hfill\end{array}`],
  ['array-hfill-both', String.raw`\begin{array}{c}\hfill a\hfill\end{array}`],
  ['array-rowcolor', String.raw`\begin{array}{c}\rowcolor{red}a\\b\end{array}`],
  ['array-extra-column-at-end', String.raw`\begin{array}{c@{x}}a\\b\end{array}`],
  ['array-extra-column-between', String.raw`\begin{array}{c@{x}c}a&b\\c&d\end{array}`],
  ['array-column-insertions', String.raw`\begin{array}{>{x}c<{y}}a\end{array}`],
  ['array-column-frame', String.raw`\begin{array}{|c|}a\\b\end{array}`],
  ['array-double-frame', String.raw`\begin{array}{||c||}a\end{array}`],
  ['array-dashed-frame', String.raw`\begin{array}{:c:}a\end{array}`],
  ['array-width-and-spacing', String.raw`\begin{array}{c@{\quad}c}a&b\end{array}`],
  ['array-scriptlevel', String.raw`\begin{smallmatrix}a&b\\c&d\end{smallmatrix}`, false, ['base', 'ams']],
  ['array-delimiters', String.raw`\begin{array}{c}a\\b\end{array}`],
  ['array-braced-ampersand', String.raw`\begin{array}{c}a\{b&c\}\end{array}`],
  ['array-nested-array', String.raw`\begin{array}{c}\begin{array}{cc}a&b\end{array}\end{array}`],
  ['array-require-close', String.raw`\label{eq:x}`],
  ['matrix-basic', String.raw`\begin{matrix}a&b\\c&d\end{matrix}`, false, ['base', 'ams']],
  ['pmatrix-basic', String.raw`\begin{pmatrix}a&b\\c&d\end{pmatrix}`, false, ['base', 'ams']],
  ['bmatrix-basic', String.raw`\begin{bmatrix}a&b\\c&d\end{bmatrix}`, false, ['base', 'ams']],
  ['cases-basic', String.raw`\begin{cases}x&x>0\\-x&x\le 0\end{cases}`, false, ['base', 'ams']],
  ['equation-numbered', String.raw`\begin{equation}x=1\end{equation}`],
  ['equation-label-reference', String.raw`\begin{equation}x=1\label{eq:x}\end{equation}\eqref{eq:x}`],
  ['equation-star', String.raw`\begin{equation*}x=1\end{equation*}`],
  ['array-invalid-close', String.raw`\begin{array}{c}a\end{matrix}`],
  ['array-missing-end', String.raw`\begin{array}{c}a`],
  ['left-right-array', String.raw`\left(\begin{array}{cc}a&b\\c&d\end{array}\right)`],
  ['over-array-boundary', String.raw`{a\over b}`],
  ['prime-single', String.raw`x'`],
  ['prime-double', String.raw`x''`],
  ['script-subscript', String.raw`x_1`],
  ['script-superscript-prime', String.raw`x^{2}'`],
  ['not-operator', String.raw`\not=`],
  ['dots-ldots-cdots', String.raw`a+\ldots+b+\cdots+c`],
  ['named-function', String.raw`\sin x`],
  ['style-display', String.raw`\displaystyle x`],
  ['position-raise-lower', String.raw`\raise{2pt}{x}+\lower{1pt}{y}`],
  ['nonscript-space', String.raw`x\nonscript\,y`],
  ['left-middle-right', String.raw`\left(x\middle|y\right)`],
  ['ams-alignment-mstyle', String.raw`\begin{align}x&=y\end{align}`, false, ['base', 'ams']],
];
const classMetadata = Object.fromEntries(Object.entries(itemClasses).map(([name, constructor]) => {
  const kind = Object.getOwnPropertyDescriptor(constructor.prototype, 'kind');
  const prototype = Object.getOwnPropertyDescriptor(constructor, 'prototype');
  return [name, { length: constructor.length, ownPrototypeKind: !!kind,
    prototypeKindGetter: typeof kind?.get === 'function', prototypeKind: constructor.prototype.kind,
    prototypeDescriptor: prototype && { writable: prototype.writable, enumerable: prototype.enumerable, configurable: prototype.configurable },
    kindDescriptor: kind && { enumerable: kind.enumerable, configurable: kind.configurable },
    functionName: constructor.name, isFunction: typeof constructor === 'function',
    ownErrors: Object.hasOwn(constructor, 'errors'),
    errorPrototypeIsBase: !!constructor.errors && Object.getPrototypeOf(constructor.errors) === BaseItem.errors }];
}));
const factoryConstructor = StackItemFactory;
const factoryPrototype = Object.getOwnPropertyDescriptor(factoryConstructor, 'prototype');
const stackItemFactoryMetadata = { functionName: factoryConstructor.name,
  length: factoryConstructor.length, isFunction: typeof factoryConstructor === 'function',
  staticDefaultItems: Object.keys(factoryConstructor.DefaultStackItems),
  prototypeDescriptor: factoryPrototype && { writable: factoryPrototype.writable,
    enumerable: factoryPrototype.enumerable, configurable: factoryPrototype.configurable },
  sourcePrototypeChain: [factoryConstructor.prototype.constructor.name,
    Object.getPrototypeOf(factoryConstructor.prototype).constructor.name] };
const constructorArguments = {
  StartItem: [() => ({ isInner: false })], StopItem: [], OpenItem: [], CloseItem: [],
  NullItem: [], PrimeItem: [], SubsupItem: [], OverItem: [],
  LeftItem: [() => '('], Middle: [() => '|', () => 'red'],
  RightItem: [() => ')', () => 'blue'], BreakItem: [() => 'newline', () => true],
  BeginItem: [], EndItem: [], StyleItem: [], PositionItem: [], CellItem: [],
  MmlItem: [], FnItem: [], NotItem: [], NonscriptItem: [], DotsItem: [],
  ArrayItem: [], EqnArrayItem: [() => 'fixture-tag', () => true, () => false],
  EquationItem: [() => 'fixture-tag'], MstyleItem: [() => ({ mathcolor: 'red' }), () => 'fixture-style'],
};
const constructors = Object.fromEntries(Object.entries(itemClasses).map(([name, Constructor]) => {
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const args = [jax.parseOptions.itemFactory, ...(constructorArguments[name] || []).map(value => value())];
  const item = new Constructor(...args);
  return [name, { kind: item.kind, isOpen: item.isOpen, isClose: item.isClose,
    isFinal: item.isFinal, copyEnv: item.copyEnv, env: clean(item.env),
    ownKeys: Reflect.ownKeys(item).map(String), properties: clean(item._properties),
    global: clean(item.global), fields: name === 'ArrayItem' || name === 'EqnArrayItem' ? {
      table: clean(item.table), row: clean(item.row), frame: clean(item.frame), hfill: clean(item.hfill),
      arraydef: clean(item.arraydef), cstart: clean(item.cstart), cend: clean(item.cend),
      cextra: clean(item.cextra), atEnd: clean(item.atEnd), ralign: clean(item.ralign),
      breakAlign: clean(item.breakAlign), templateSubs: clean(item.templateSubs),
    } : name === 'MstyleItem' ? { attrList: clean(item.attrList) } : {}}];
}));
// Compare ordinary constructor-owned object identity and mutation visibility,
// not just the cleaned values above. Source allocates these records per item
// but MstyleItem retains its supplied PropertyList by reference.
const constructorIdentity = (() => {
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const first = factory.create('array'), second = factory.create('array');
  const arrayFields = ['table', 'row', 'frame', 'hfill', 'arraydef', 'cstart', 'cend',
    'cextra', 'ralign', 'breakAlign'];
  const fresh = Object.fromEntries(arrayFields.map(key => [key, first[key] !== second[key]]));
  first.breakAlign.cell = 'top';
  first.frame.push(['left', 'solid']);
  const attrList = { mathcolor: 'red' };
  const style = new itemClasses.MstyleItem(factory, attrList, 'identity');
  attrList.mathcolor = 'blue';
  const name = style.getName();
  return { fresh, firstBreakAlign: clean(first.breakAlign), secondBreakAlign: clean(second.breakAlign),
    firstFrame: clean(first.frame), secondFrame: clean(second.frame),
    mstyleAttributeMutationVisible: style.attrList === attrList,
    mstyleAttributeValueAfterMutation: style.attrList.mathcolor,
    mstyleName: clean(name), arrayAtEnd: [clean(first.atEnd), clean(second.atEnd)],
    parserInitiallyUndefined: first.parser === undefined && second.parser === undefined };
})();
const directMethods = [];
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const nullItem = factory.create('null');
  const openItem = factory.create('open');
  const fn = factory.create('fn');
  fn.Push(new MmlFactory().create('mi'));
  const results = [
    ['start', factory.create('start', {isInner: false}).checkItem(nullItem)],
    ['open', openItem.checkItem(nullItem)],
    ['begin', factory.create('begin').checkItem(nullItem)],
    ['style', factory.create('style').checkItem(nullItem)],
    ['position', factory.create('position').checkItem(nullItem)],
    ['nonscript', factory.create('nonscript').checkItem(nullItem)],
    ['fn-open', fn.checkItem(openItem)],
    ['not-open', factory.create('not').checkItem(openItem)],
    ['dots-open', factory.create('dots').checkItem(openItem)],
    ['subsup-open', factory.create('subsup').checkItem(openItem)],
  ];
  directMethods.push({ name: 'base-item-checkitem-fallbacks', results: results.map(([kind, result]) => ({
    kind, sharedSuccess: result === BaseItem.success,
    tuple: Array.isArray(result) ? { success: result[1], items: result[0]?.map(item => item.kind) ?? null } : null,
  })) });
  const linebreak = factory.create('break', 'newline', true);
  const output = linebreak.checkItem(nullItem);
  directMethods.push({ name: 'break-item-default-inserts-mspace', tuple: {
    success: output[1], items: output[0].map(item => item.kind),
    node: tree(output[0][0].First),
  } });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const open = factory.create('open');
  const child = new MmlFactory().create('mi');
  open.Push(child);
  const over = factory.create('over');
  const reduction = open.checkItem(over);
  const num = over.getProperty('num');

  const cellOpen = factory.create('open');
  const cell = factory.create('cell');
  cell.setProperty('linebreak', true);
  const linebreakResult = cellOpen.checkItem(cell);

  const trace = [];
  const errorCell = factory.create('cell');
  Object.defineProperty(errorCell._properties, 'linebreak', { configurable: true, get() { trace.push('linebreak'); return false; } });
  Object.defineProperty(errorCell._properties, 'name', { configurable: true, get() { trace.push('name'); return 'cell-name'; } });
  let misplaced = null;
  try { factory.create('open').checkItem(errorCell); }
  catch (e) { misplaced = { name: e.name, id: e.id, message: String(e.message) }; }

  const receiver = factory.create('null');
  const finalItem = factory.create('mml');
  const finalNode = new MmlFactory().create('mi');
  finalItem.Push(finalNode);
  const finalResult = receiver.checkItem(finalItem);

  let leftRight = null;
  try {
    const result = factory.create('left', '(').checkItem(factory.create('right', ')', 'blue'));
    leftRight = { success: result[1], items: result[0].map(item => item.kind), tree: tree(result[0][0].First) };
  } catch (e) { leftRight = { name: e.name, id: e.id, message: String(e.message) }; }
  directMethods.push({ name: 'base-item-checkitem-reduction-errors-and-order',
    reduction: { success: reduction === BaseItem.success, openSize: open.Size(), overName: over.getName(), num: tree(num), child: tree(child) },
    linebreak: { fail: linebreakResult === BaseItem.fail, openSize: cellOpen.Size() },
    misplaced, trace, final: { fail: finalResult === BaseItem.fail, size: receiver.Size(), top: tree(receiver.First) },
    leftRight });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  let error = null;
  try { factory.create('position').checkItem(factory.create('stop')); }
  catch (e) { error = { name: e.name, id: e.id, message: String(e.message) }; }
  directMethods.push({ name: 'position-item-close-missing-box', error });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const left = new MmlFactory().create('mi');
  const center = new MmlFactory().create('mn');
  const right = new MmlFactory().create('mo');
  const position = factory.create('position').setProperty('move', 'horizontal')
    .setProperty('left', left).setProperty('right', right);
  const final = factory.create('mml');
  final.Push(center);
  const result = position.checkItem(final);
  directMethods.push({ name: 'position-item-horizontal-final-expansion', success: result[1],
    items: result[0].map(item => item.kind), trees: result[0].map(item => tree(item.First)),
    centerRetained: result[0][1] === final, positionSize: position.Size() });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const style = factory.create('style').setProperty('styles', { mathcolor: 'red', displaystyle: true });
  style.Push(new MmlFactory().create('mi'));
  const result = style.checkItem(factory.create('stop'));
  directMethods.push({ name: 'style-item-close-wraps-content', success: result[1],
    items: result[0].map(item => item.kind), tree: tree(result[0][0].First), retainedStop: result[0][1].kind });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const item = factory.create('mstyle', { mathcolor: 'red' }, 'fixture-style');
  item.Push(new MmlFactory().create('mi'));
  const end = factory.create('end').setProperty('name', 'fixture-style');
  const result = item.checkItem(end);
  directMethods.push({ name: 'mstyle-matching-end-wraps-content', success: result[1],
    items: result[0].map(entry => entry.kind), tree: tree(result[0][0].First) });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const item = factory.create('equation', 'fixture-tag');
  let error = null;
  try { item.checkItem(factory.create('stop')); }
  catch (e) { error = { name: e.name, id: e.id, message: String(e.message) }; }
  directMethods.push({ name: 'equation-item-stop-throws-missing-end', error,
    currentTag: clean(jax.parseOptions.tags.currentTag), counter: jax.parseOptions.tags.counter });
}
{
  const Constructor = itemClasses.NullItem;
  const receiver = Object.create(Constructor.prototype);
  const result = Reflect.apply(Constructor, receiver, []);
  directMethods.push({ name: 'null-item-constructor-call-on-prototype-object',
    returnsReceiver: result === receiver, ownKeys: Reflect.ownKeys(receiver).map(String),
    kind: receiver.kind, isOpen: receiver.isOpen, isClose: receiver.isClose,
    isFinal: receiver.isFinal, copyEnv: receiver.copyEnv,
    factory: clean(receiver.factory), global: clean(receiver.global),
    properties: clean(receiver._properties) });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const first = factory.create('over');
  const second = factory.create('over');
  let error = null;
  try { first.checkItem(second); }
  catch (e) { error = { name: e.name, id: e.id, message: String(e.message) }; }
  directMethods.push({ name: 'over-double-over', kind: first.kind,
    isClose: first.isClose, defaultName: first.getName(), error });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const item = jax.parseOptions.itemFactory.create('array');
  item.table.push(new MmlFactory().create('mtr'), new MmlFactory().create('mtr'));
  item.arraydef.rowspacing = '1';
  item.addRowSpacing('0.5em');
  const rowspacing = item.arraydef.rowspacing;
  item.arraydef.scriptlevel = 1;
  item.breakAlign.table = 'top';
  item.setProperty('arrayPadding', '1em');
  const mml = item.createMml();
  directMethods.push({ name: 'array-rowspacing-and-create-mml-side-effects',
    rowspacing, scriptlevelAfterDelete: Object.hasOwn(item.arraydef, 'scriptlevel'),
    tree: tree(mml), tableLength: item.table.length });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const frame = (sides) => {
    const item = factory.create('array');
    item.table.push(new MmlFactory().create('mtr'));
    item.frame.push(...sides.map(([name, style]) => [name, style]));
    return tree(item.createMml());
  };
  directMethods.push({ name: 'array-handle-frame-branches',
    partialSolid: frame([['top', 'solid'], ['right', 'solid']]),
    allSolid: frame([['top', 'solid'], ['right', 'solid'], ['bottom', 'solid'], ['left', 'solid']]),
    mixed: frame([['top', 'dashed'], ['left', 'solid']]) });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const item = jax.parseOptions.itemFactory.create('eqnarray', 'fixture-tag', true, false);
  let error = null;
  try { item.EndEntry(); }
  catch (e) { error = { name: e.name, message: String(e.message) }; }
  directMethods.push({ name: 'eqnarray-endentry-missing-columnalign', error,
    rowLength: item.row.length, stackSize: item.Size(), columnalign: item.arraydef.columnalign });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  let error = null;
  try { factory.create('open').checkItem(factory.create('stop')); }
  catch (e) { error = { name: e.name, id: e.id, message: String(e.message) }; }
  directMethods.push({ name: 'open-stop-inherited-class-error', error });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const errors = itemClasses.SubsupItem.errors;
  const originalSup = errors.sup;
  errors.sup = ['MissingOpenForSup', 'Missing open brace for superscript %1', 'extra-argument'];
  const item = factory.create('subsup').setProperty('position', 2);
  let error = null;
  try { item.checkItem(factory.create('null')); }
  catch (e) { error = { name: e.name, id: e.id, message: String(e.message) }; }
  const mutatedErrors = clean(errors.sup);
  errors.sup = originalSup;
  directMethods.push({ name: 'subsup-error-splice-extra-arguments', error, mutatedErrors });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const begin = factory.create('begin').setProperty('name', 'alpha');
  const end = factory.create('end').setProperty('name', 'beta');
  let error = null;
  try { begin.checkItem(end); }
  catch (e) { error = { name: e.name, id: e.id, message: String(e.message) }; }
  directMethods.push({ name: 'begin-end-name-mismatch', beginName: begin.getName(),
    endName: end.getName(), error });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const open = factory.create('open');
  const close = factory.create('close');
  open.Push(new MmlFactory().create('mi'));
  const result = open.checkItem(close);
  const replacement = result[0][0];
  directMethods.push({ name: 'open-close-wraps-texatom', result: {
    kind: replacement.kind, size: replacement.nodes.length,
    node: tree(replacement.First), success: result[1],
  } });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const item = jax.parseOptions.itemFactory.create('array');
  directMethods.push({ name: 'array-default-storage', kind: item.kind,
    isOpen: item.isOpen, copyEnv: item.copyEnv,
    tableLength: item.table.length, rowLength: item.row.length,
    frame: clean(item.frame), hfill: clean(item.hfill),
    arraydef: clean(item.arraydef), breakAlign: clean(item.breakAlign) });
  item.EndEntry();
  directMethods.push({ name: 'array-end-entry-empty', rowLength: item.row.length,
    rowCellKind: item.row[0]?.kind || null, rowCellChildren: item.row[0]?.childNodes?.length ?? null,
    stackSize: item.Size(), hfill: clean(item.hfill) });
  const aligned = jax.parseOptions.itemFactory.create('array');
  aligned.ralign[0] = ['middle', '60%', 'center'];
  aligned.EndEntry();
  directMethods.push({ name: 'array-end-entry-ralign',
    rowCell: tree(aligned.row[0]), stackSize: aligned.Size(), hfill: clean(aligned.hfill) });
  const labeled = jax.parseOptions.itemFactory.create('array');
  labeled.row.push(new MmlFactory().create('mi'));
  labeled.setProperty('isLabeled', true);
  labeled.breakAlign.row = 'baseline';
  labeled.atEnd = true;
  labeled.EndRow();
  directMethods.push({ name: 'array-end-row-labeled-break-align',
    tableRow: tree(labeled.table[0]), rowLength: labeled.row.length,
    isLabeled: labeled.getProperty('isLabeled'), atEnd: labeled.atEnd,
    breakAlign: clean(labeled.breakAlign) });
  const lines = jax.parseOptions.itemFactory.create('array');
  lines.table.push(new MmlFactory().create('mtr'), new MmlFactory().create('mtr'));
  lines.arraydef.rowlines = 'solid none';
  lines.arraydef.rowspacing = '1';
  lines.setProperty('rowspacing', 0.5);
  lines.checkLines();
  directMethods.push({ name: 'array-check-lines-padding',
    rowlines: lines.arraydef.rowlines, rowspacing: lines.arraydef.rowspacing,
    frame: clean(lines.frame), tableLength: lines.table.length });
  const liveLines = jax.parseOptions.itemFactory.create('array');
  liveLines.table.push(new MmlFactory().create('mtr'));
  const rowlineTrace = [];
  let rowlineReads = 0;
  Object.defineProperty(liveLines.arraydef, 'rowlines', { configurable: true,
    get() { rowlineTrace.push(`get${++rowlineReads}`); return rowlineReads === 1 ? 'first' : undefined; } });
  let rowlineError = null;
  try { liveLines.checkLines(); }
  catch (e) { rowlineError = { name: e.name, message: String(e.message) }; }
  directMethods.push({ name: 'array-check-lines-live-rowlines-get',
    error: rowlineError, trace: rowlineTrace, frame: clean(liveLines.frame) });
  const shortCircuit = jax.parseOptions.itemFactory.create('array');
  shortCircuit.parser = new (load('input/tex/TexParser.js').default)('x', {}, jax.parseOptions);
  shortCircuit.cstart[0] = 'prefix';
  const extraTrace = [];
  Object.defineProperty(shortCircuit.cextra, '0', { configurable: true,
    get() { extraTrace.push('cextra0'); return undefined; } });
  Object.defineProperty(shortCircuit.cextra, '1', { configurable: true,
    get() { extraTrace.push('cextra1'); return true; } });
  shortCircuit.StartEntry();
  directMethods.push({ name: 'array-start-entry-cextra-short-circuit',
    trace: extraTrace, start: shortCircuit.cstart[0], parserString: shortCircuit.parser.string, parserIndex: shortCircuit.parser.i,
    atEnd: shortCircuit.atEnd });
  const partial = jax.parseOptions.itemFactory.create('array');
  partial.Push(new MmlFactory().create('mi'));
  partial.hfill = null;
  let partialError = null;
  try { partial.EndEntry(); }
  catch (e) { partialError = { name: e.name, message: String(e.message) }; }
  directMethods.push({ name: 'array-end-entry-null-hfill-partial', error: partialError,
    rowLength: partial.row.length, stackSize: partial.Size() });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const prime = factory.create('prime');
  const item = factory.create('mml');
  const mml = new MmlFactory();
  const first = mml.create('mi');
  const second = mml.create('mn');
  prime.Push(first, second);
  const result = prime.checkItem(item);
  directMethods.push({ name: 'prime-empty-script-wraps-two-live-stack-nodes',
    kind: result[0][0].kind, node: tree(result[0][0]),
    retained: result[0][1] === item, success: result[1] });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const item = factory.create('subsup').setProperty('position', 2);
  let error = null;
  try { item.checkItem(factory.create('null')); }
  catch (e) { error = { name: e.name, id: e.id, message: String(e.message) }; }
  directMethods.push({ name: 'subsup-missing-open-sup-error', error,
    position: item.getProperty('position'), stackSize: item.nodes.length });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const child = new MmlFactory().create('mi');
  const item = factory.create('mml', child);
  directMethods.push({ name: 'inherited-base-constructor-forwards-mml-child',
    size: item.Size(), firstIsOriginal: item.First === child, first: tree(item.First) });
}
{
  const jax = new TeX({ packages: ['base'], tags: 'base' });
  jax.setMmlFactory(new MmlFactory());
  const factory = jax.parseOptions.itemFactory;
  const mml = new MmlFactory();
  const token = mml.create('mo', {}, [mml.create('text').setText('x')]);
  const item = factory.create('mml');
  item.Push(token);
  const result = factory.create('not').checkItem(item);
  directMethods.push({ name: 'not-item-explicit-overlay-mutates-token-in-place',
    resultKind: result[0][0].kind, retained: result[0][0] === item,
    node: tree(item.First), success: result[1] });
}
{
  const values = [
    ['missing', undefined, false], ['null', null, true], ['zero', 0, true], ['false', false, true],
  ];
  for (const [label, value, supplied] of values) {
    const jax = new TeX({ packages: ['base'], tags: 'base' });
    jax.setMmlFactory(new MmlFactory());
    const factory = jax.parseOptions.itemFactory;
    const mml = new MmlFactory();
    const top = mml.create('mrow', {}, [mml.create('mi'), mml.create('mo')]);
    const subsup = factory.create('subsup').setProperty('position', 1);
    if (supplied) subsup.setProperty('movesupsub', value);
    subsup.Push(top);
    const incoming = factory.create('mml');
    incoming.Push(mml.create('mn'));
    subsup.checkItem(incoming);
    directMethods.push({ name: `subsup-movesupsub-nullish-${label}`,
      value: clean(top.getProperty('movesupsub')), node: tree(top) });
  }
}
{
  const values = [
    ['missing', undefined, false], ['null', null, true], ['zero', 0, true], ['false', false, true],
  ];
  for (const [label, value, supplied] of values) {
    const jax = new TeX({ packages: ['base'], tags: 'base' });
    jax.setMmlFactory(new MmlFactory());
    const factory = jax.parseOptions.itemFactory;
    const mml = new MmlFactory();
    const over = factory.create('over').setProperty('num', mml.create('mi'));
    if (supplied) over.setProperty('thickness', value);
    const result = over.checkItem(factory.create('close'));
    directMethods.push({ name: `over-thickness-nullish-${label}`,
      value: clean(result[0][0].First.attributes.get('linethickness')),
      node: tree(result[0][0].First) });
  }
}

const output = { version: '4.1.3', source: { tsPath, tsHash, cjsPath, cjsHash },
  classMetadata, stackItemFactoryMetadata, constructors, constructorIdentity, directMethods,
  cases: cases.map(([name, tex, display, packages]) => observe(name, tex, display, packages)) };
const out = process.argv[2] || path.join(root, 'build/math-port/base-items-original.json');
fs.mkdirSync(path.dirname(out), { recursive: true });
fs.writeFileSync(out, JSON.stringify(output, null, 2) + '\n');
console.log(`wrote ${output.cases.length} pinned BaseItems observations`);
