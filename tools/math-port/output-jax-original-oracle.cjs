'use strict';

// Root-owned observations of the pinned original OutputJax base class.
// Production C++ and native observer output never participate in this oracle.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const version = JSON.parse(fs.readFileSync(path.join(packageRoot, 'package.json'))).version;
if (version !== '4.1.3') throw new Error('Expected @mathjax/src 4.1.3');
const sourceFile = 'ts/core/OutputJax.ts';
const digest = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const sourceHash = digest(path.join(sourceRoot, sourceFile));
if (sourceHash !== 'e4932ba7a5d8f782022fa0ab3c8a8bc05a3072704cd8802213fdadb2722789a3')
  throw new Error('Pinned OutputJax.ts changed');
const {AbstractOutputJax} = require(path.join(packageRoot, 'cjs/core/OutputJax.js'));
function plain(value) {
  if (value === undefined) return {$: 'undefined'};
  if (typeof value === 'number' && !Number.isFinite(value))
    return {$: Number.isNaN(value) ? 'NaN' : value < 0 ? '-Infinity' : 'Infinity'};
  if (typeof value === 'number' && Object.is(value, -0)) return {$: '-0'};
  if (Array.isArray(value)) return Array.from({length:value.length}, (_, i) =>
    Object.hasOwn(value, i) ? plain(value[i]) : {$:'hole'});
  if (value && typeof value === 'object') return Object.fromEntries(
    Object.keys(value).map(key => [key, plain(value[key])]));
  return value;
}
const records = [];
function observe(name, action) {
  let actual;
  try { actual = {value:plain(action())}; }
  catch (error) { actual = {error:{name:error.name, message:error.message}}; }
  records.push({name,actual});
}
observe('base-defaults', () => {
  const jax = new AbstractOutputJax();
  return {name:jax.name, options:jax.options, adaptor:jax.adaptor,
    pre:jax.preFilters.items, post:jax.postFilters.items};
});
observe('derived-options-and-live-name', () => {
  class Derived extends AbstractOutputJax {}
  Derived.NAME = 'scene';
  Derived.OPTIONS = {...AbstractOutputJax.OPTIONS, scale:1, nested:{x:1,y:2}};
  const jax = new Derived({scale:2,nested:{x:3}});
  const before = jax.name;
  Derived.NAME = 'renamed';
  return {before, after:jax.name, options:jax.options,
    optionsDistinct:jax.options !== Derived.OPTIONS,
    nestedDistinct:jax.options.nested !== Derived.OPTIONS.nested,
    defaults:Derived.OPTIONS};
});
observe('set-adaptor-preserves-identity', () => {
  const jax = new AbstractOutputJax(), adaptor = {tag:'host'};
  const results = [jax.setAdaptor(adaptor)];
  const same = jax.adaptor === adaptor;
  adaptor.tag = 'mutated';
  const observed = jax.adaptor.tag;
  results.push(jax.setAdaptor(null));
  return {results, same, observed, current:jax.adaptor};
});
observe('base-lifecycle-and-page-return-values', () => {
  const jax = new AbstractOutputJax(), document = {id:3};
  return {initialize:jax.initialize(), reset:jax.reset('a',7),
    metrics:jax.getMetrics(document), sheet:jax.styleSheet(document),
    page:jax.pageElements(document), hasTypeset:typeof jax.typeset,
    hasEscaped:typeof jax.escaped};
});
observe('filter-same-record-and-live-data', () => {
  const jax = new AbstractOutputJax(), math = {}, document = {}, data = {n:1};
  let first, identity = [];
  jax.preFilters.add(args => {first=args;identity.push(args.math===math,args.document===document,args.data===data);args.data.n=2;});
  jax.preFilters.add(args => {identity.push(args===first);args.data={n:3};});
  const result = jax.executeFilters(jax.preFilters,math,document,data);
  return {identity, original:data, result, returnedLive:result===first.data};
});
observe('filter-order-and-false-stop', () => {
  const jax = new AbstractOutputJax(), trace = [];
  jax.preFilters.add(args=>{trace.push('late');args.data.push('late');},10);
  jax.preFilters.add(args=>{trace.push('early');args.data.push('early');},-5);
  jax.preFilters.add(args=>{trace.push('stop');args.data.push('stop');return false;},0);
  jax.preFilters.add(args=>{trace.push('equal');args.data.push('equal');},0);
  return {trace, data:jax.executeFilters(jax.preFilters,{},null,[])};
});
observe('configured-filter-definition-and-priority', () => {
  const trace = [];
  const jax = new AbstractOutputJax({preFilters:[
    [args=>{trace.push('second');args.data.push(2);},4],
    [args=>{trace.push('first');args.data.push(1);},-3]
  ],postFilters:[args=>{trace.push('post');args.data.push(3);} ]});
  const data = jax.executeFilters(jax.preFilters,{},null,[]);
  return {trace, result:jax.executeFilters(jax.postFilters,{},null,data)};
});
observe('filter-undefined-replacement', () => {
  const jax = new AbstractOutputJax();
  jax.postFilters.add(args=>{args.data=undefined;});
  return jax.executeFilters(jax.postFilters,{},undefined,{n:1});
});
observe('filter-thrown-error-propagates', () => {
  const jax = new AbstractOutputJax();
  jax.preFilters.add(()=>{throw new TypeError('original-filter-probe');});
  return jax.executeFilters(jax.preFilters,{},null,{});
});
observe('unknown-option-source-warning', () => {
  const warnings = [], original = console.warn;
  console.warn = (...args)=>warnings.push(args.map(String).join(' '));
  try {const jax = new AbstractOutputJax({unknown:4});return {options:jax.options,warnings};}
  finally {console.warn=original;}
});
observe('null-options', () => new AbstractOutputJax(null).options);
observe('null-filter-definition', () => new AbstractOutputJax({preFilters:null}).preFilters.items);
observe('instances-do-not-share-filter-lists', () => {
  const a = new AbstractOutputJax(), b = new AbstractOutputJax();
  a.preFilters.add(()=>{});
  return {preDistinct:a.preFilters!==b.preFilters, postDistinct:a.postFilters!==b.postFilters,
    firstCount:a.preFilters.items.length, secondCount:b.preFilters.items.length};
});
observe('prototype-method-inventory', () => Object.getOwnPropertyNames(AbstractOutputJax.prototype).map(name=>{
  const descriptor=Object.getOwnPropertyDescriptor(AbstractOutputJax.prototype,name);
  return {name, enumerable:descriptor.enumerable, configurable:descriptor.configurable,
    writable:descriptor.writable, getter:typeof descriptor.get, value:typeof descriptor.value};
}));
process.stdout.write(JSON.stringify({version, sourceFile, sourceHash,
  cjsHash:digest(path.join(packageRoot,'cjs/core/OutputJax.js')), records},null,2)+'\n');
