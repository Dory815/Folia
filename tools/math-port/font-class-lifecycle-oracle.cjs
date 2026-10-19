#!/usr/bin/env node
'use strict';

// Source-only probes for FontData class identity, constructor dispatch,
// own-property creation, and static-method receiver behavior.  This executes
// the pinned bundled CJS without replacing its helpers or methods.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const sourcePackageRoot = path.join(packageRoot, 'src');
const packageVersion = JSON.parse(fs.readFileSync(path.join(sourcePackageRoot, 'package.json'))).version;
if (packageVersion !== '4.1.3') throw new Error(`Expected @mathjax/src 4.1.3, got ${packageVersion}`);
const { FontData } = require(path.join(packageRoot, 'src/cjs/output/common/FontData.js'));
const { SvgFontData } = require(path.join(packageRoot, 'src/cjs/output/svg/FontData.js'));
const newcmRoot = path.join(packageRoot, 'mathjax-newcm-font');
const newcmEntry = path.join(newcmRoot, 'cjs/svg.js');
const newcmPackageVersion = JSON.parse(fs.readFileSync(path.join(newcmRoot, 'package.json'))).version;
const newcmCjsSha256 = crypto.createHash('sha256').update(fs.readFileSync(newcmEntry)).digest('hex');
if (newcmCjsSha256 !== '709d6e85113ef44645ad802b8424508d5f836dd61e4d1f2797b5e011a9693734')
  throw new Error(`Pinned NewCM CJS changed: ${newcmCjsSha256}`);
const { MathJaxNewcmFont } = require(newcmEntry);
const normalize = (value, seen = new Map()) => {
  if (value === undefined) return { $: 'undefined' };
  if (value === null) return null;
  if (typeof value === 'number' && !Number.isFinite(value))
    return { $: Number.isNaN(value) ? 'NaN' : value < 0 ? '-Infinity' : 'Infinity' };
  if (typeof value === 'function') return { $: 'function', name: value.name, length: value.length };
  if (typeof value !== 'object') return value;
  if (seen.has(value)) return { $ref: seen.get(value) };
  const id = seen.size;
  seen.set(value, id);
  if (Array.isArray(value)) {
    const items = [];
    for (let i = 0; i < value.length; i++)
      items.push(Object.hasOwn(value, i) ? normalize(value[i], seen) : { $: 'hole' });
    return { $id: id, $array: items };
  }
  const result = { $id: id, $object: Object.create(null) };
  for (const key of Reflect.ownKeys(value)) {
    if (typeof key !== 'string') continue;
    const desc = Object.getOwnPropertyDescriptor(value, key);
    result.$object[key] = desc && 'value' in desc
      ? normalize(desc.value, seen)
      : { $: 'accessor', get: desc?.get ? normalize(desc.get, seen) : null,
          set: desc?.set ? normalize(desc.set, seen) : null };
  }
  return result;
};
const describe = (object, key) => {
  const d = Object.getOwnPropertyDescriptor(object, key);
  if (!d) return null;
  return 'value' in d
    ? { value: normalize(d.value), writable: d.writable, enumerable: d.enumerable, configurable: d.configurable }
    : { get: d.get ? normalize(d.get) : null, set: d.set ? normalize(d.set) : null,
        enumerable: d.enumerable, configurable: d.configurable };
};
const descriptorShape = (object, key) => {
  const d = Object.getOwnPropertyDescriptor(object, key);
  if (!d) return null;
  if (!('value' in d)) return { get: !!d.get, set: !!d.set,
    enumerable: d.enumerable, configurable: d.configurable };
  const value = d.value;
  return { valueType: value === null ? 'null' : Array.isArray(value) ? 'array' : typeof value,
    writable: d.writable, enumerable: d.enumerable, configurable: d.configurable };
};
const summarize = value => {
  if (Array.isArray(value)) return { type: 'array', length: value.length };
  if (value instanceof Map) return { type: 'Map', size: value.size };
  if (value && typeof value === 'object') return { type: 'object', keys: Object.keys(value).slice(0, 12), keyCount: Object.keys(value).length };
  return normalize(value);
};
const observe = fn => {
  try { return { value: normalize(fn()) }; }
  catch (e) { return { error: { name: e?.name || typeof e, message: e?.message || String(e) } }; }
};
const sourceFiles = [
  '.reference/MathJax-src/ts/output/common/FontData.ts',
  '.reference/MathJax-src/ts/output/svg/FontData.ts',
];
const cjsFiles = [
  'src/cjs/output/common/FontData.js',
  'src/cjs/output/svg/FontData.js',
];
const results = {
  source: Object.fromEntries(sourceFiles.map(file => [file, {
    sha256: crypto.createHash('sha256').update(fs.readFileSync(path.join(root, file))).digest('hex'),
    cjsFile: file.replace(/^\.reference\/MathJax-src\/ts\//, 'cjs/').replace(/\.ts$/, '.js'),
  }])),
  cjs: Object.fromEntries(cjsFiles.map(file => [file,
    crypto.createHash('sha256').update(fs.readFileSync(path.join(packageRoot, file))).digest('hex')])),
  newcm: { packageVersion:newcmPackageVersion, file:'mathjax-newcm-font/cjs/svg.js', sha256:newcmCjsSha256 },
  classes: {},
};

for (const [name, ctor] of [['FontData', FontData], ['SvgFontData', SvgFontData], ['NewCM', MathJaxNewcmFont]]) {
  results.classes[name] = {
    name: ctor.name,
    length: ctor.length,
    ownKeys: Reflect.ownKeys(ctor).filter(k => typeof k === 'string'),
    prototypeKeys: Reflect.ownKeys(ctor.prototype).filter(k => typeof k === 'string'),
    constructorParent: Object.getPrototypeOf(ctor).name,
    prototypeParent: Object.getPrototypeOf(ctor.prototype)?.constructor?.name,
    // Capture every own descriptor, not only the fields exercised by the
    // constructor probes below. Native registration must bind the complete
    // source class surface on the canonical constructor/prototype objects.
    staticDescriptors: Object.fromEntries(Reflect.ownKeys(ctor)
      .filter(key => typeof key === 'string')
      .map(key => [key, describe(ctor, key)])),
    prototypeDescriptors: Object.fromEntries(Reflect.ownKeys(ctor.prototype)
      .filter(key => typeof key === 'string')
      .map(key => [key, describe(ctor.prototype, key)])),
    static: Object.fromEntries(['OPTIONS', 'JAX', 'NAME', 'charOptions', 'addExtension',
      'defineDynamicFiles', 'dynamicSetup'].map(key => [key, describe(ctor, key)])),
  };
}

// The real base constructor captures this.CLASS once, then makes these
// virtual calls in source order.  The first override poisons the instance's
// constructor property; later constructor phases must still use the captured
// class' defaults.
{
  const trace = [];
  const poison = function Poisoned() {};
  class DispatchProbe extends MathJaxNewcmFont {
    static OPTIONS = { ...MathJaxNewcmFont.OPTIONS, dynamicPrefix: 'captured-prefix' };
    static defaultVariants = [['normal'], ['probe-variant']];
    defineCssFonts(...args) { trace.push(['defineCssFonts', args.map(summarize)]); this.constructor = poison; }
    createVariants(...args) { trace.push(['createVariants', args.map(summarize)]); }
    defineDelimiters(...args) { trace.push(['defineDelimiters', args.map(summarize)]); }
    defineChars(...args) { trace.push(['defineChars', args.map(summarize)]); }
    defineRemap(...args) { trace.push(['defineRemap', args.map(summarize)]); }
    defineDynamicCharacters(...args) { trace.push(['defineDynamicCharacters', args.map(summarize)]); }
  }
  const instance = new DispatchProbe({ dynamicPrefix: 'instance-prefix' });
  results.constructorDispatch = {
    trace,
    classGetterAfterPoison: observe(() => instance.CLASS?.name),
    ownKeys: Reflect.ownKeys(instance),
    ownDescriptors: Object.fromEntries(Reflect.ownKeys(instance).filter(k => typeof k === 'string')
      .map(key => [key, descriptorShape(instance, key)])),
    capturedOption: instance.options.dynamicPrefix,
    ownConstructor: descriptorShape(instance, 'constructor'),
  };
}

// Verify the class receiver used by inherited SVG static methods and the
// aliasing of class-level objects.  These probes intentionally use ordinary
// methods on a derived class, as Source does, rather than stubbing the base.
{
  const trace = [];
  class StaticProbe extends SvgFontData {
    static JAX = 'ProbeJax';
    static defaultSizeVariants = ['base-size'];
    static defaultStretchVariants = ['base-stretch'];
    static defineDynamicFiles(...args) { trace.push(['defineDynamicFiles', args]); return super.defineDynamicFiles(...args); }
    static adjustDelimiters(...args) { trace.push(['adjustDelimiters', args]); return super.adjustDelimiters(...args); }
  }
  StaticProbe.dynamicExtensions = new Map();
  StaticProbe.variantCacheIds = { inherited: 'base-cache' };
  StaticProbe.OPTIONS = { ...SvgFontData.OPTIONS, probeOption: 'old' };
  const cacheStorage = StaticProbe.variantCacheIds;
  const optionStorage = StaticProbe.OPTIONS;
  const cacheIds = { probe: 'cache-probe' };
  const extension = { name: 'class-probe', ranges: [], cacheIds,
    options: { probeOption: 'merged' },
    delimiters: { 41: { dir: 'V', variants: [-1] } } };
  results.staticDispatch = observe(() => {
    StaticProbe.addExtension(extension);
    return {
      trace,
      sameCacheIdsObject: StaticProbe.variantCacheIds === cacheStorage,
      sameOptionsObject: StaticProbe.OPTIONS === optionStorage,
      options: StaticProbe.OPTIONS,
      cacheIds: StaticProbe.variantCacheIds,
      extension: StaticProbe.dynamicExtensions.get('class-probe'),
      ownStaticDescriptors: Object.fromEntries(['JAX', 'OPTIONS', 'dynamicExtensions', 'addExtension']
        .map(key => [key, describe(StaticProbe, key)])),
    };
  });
}

// Instance addExtension intentionally routes through the current constructor
// and through overridable instance methods. Record the dispatch order while
// delegating each call to the original implementation.
{
  const trace=[];
  class InstanceProbe extends MathJaxNewcmFont {
    defineCssFonts(...args) { trace.push(['defineCssFonts', args.map(summarize)]); return super.defineCssFonts(...args); }
    createVariants(...args) { trace.push(['createVariants', args.map(summarize)]); return super.createVariants(...args); }
    defineDelimiters(...args) { trace.push(['defineDelimiters', args.map(summarize)]); return super.defineDelimiters(...args); }
    defineChars(...args) { trace.push(['defineChars', args.map(summarize)]); return super.defineChars(...args); }
    defineRemap(...args) { trace.push(['defineRemap', args.map(summarize)]); return super.defineRemap(...args); }
    defineDynamicCharacters(...args) { trace.push(['defineDynamicCharacters', args.map(summarize)]); return super.defineDynamicCharacters(...args); }
  }
  const font=new InstanceProbe();trace.length=0;
  const ranges=[['instance-probe.js',{normal:[0xE123]},[]]];
  const cacheBefore=InstanceProbe.VariantSmp;
  const result=observe(() => font.addExtension({
    name:'instance-probe', options:{dynamicPrefix:'instance-extension'}, parameters:{quad:2},
    sizeVariants:{append:['probe-size']}, stretchVariants:{append:['probe-stretch']},
    variantSmp:{'probe-smp':[1]}, cssFonts:{normal:['probe-family',false,false]},
    variants:{append:[['probe-variant']]}, delimiters:{41:{dir:'V',variants:[-1]}},
    chars:{normal:{0xE123:[1,0,.5,{p:'M1'}]}}, accentMap:{65:'A'},moMap:{66:'B'},mnMap:{67:'C'},ranges,
  }));
  results.instanceExtensionDispatch={result,trace,variantSmpReplaced:InstanceProbe.VariantSmp!==cacheBefore,
    ranges,extensionNames:[...InstanceProbe.dynamicExtensions.keys()]};
}

// SVG charOptions must modify exactly the supplied tuple and preserve option
// identity for an already populated fourth slot.
{
  const options = { original: true };
  const map = { 65: [1, 0, .5], 66: [1, 0, .5, options] };
  results.charOptions = {
    created: observe(() => SvgFontData.charOptions(map, 65)),
    tupleAfterCreate: normalize(map[65]),
    existingIsSame: SvgFontData.charOptions(map, 66) === options,
    sameTupleAfterExisting: map[66][3] === options,
    invalid: observe(() => SvgFontData.charOptions({ 67: { p: 'not tuple' } }, 67)),
  };
}

// Instance descriptors and live aliases across the Source accessors/methods.
{
  const font = new MathJaxNewcmFont();
  const normal = font.getVariant('normal');
  const tuple = [1, 0, .5];
  font.defineChars('normal', { 0xE123: tuple });
  results.instanceGraph = {
    ownKeys: Reflect.ownKeys(font),
    descriptors: Object.fromEntries(Reflect.ownKeys(font).filter(k => typeof k === 'string')
      .map(key => [key, descriptorShape(font, key)])),
    classAccessor: font.CLASS === MathJaxNewcmFont,
    variantRecordIsReturnedRecord: font.getVariant('normal') === normal,
    charIsSameTuple: font.getChar('normal', 0xE123) === tuple,
    styles: describe(Object.getPrototypeOf(font), 'styles'),
    cssFamilyPrefix: font.getFamily('probe-family'),
  };
}

const destination = path.join(root, 'build/math-port/font-class-lifecycle-original.json');
fs.mkdirSync(path.dirname(destination), { recursive: true });
fs.writeFileSync(destination, JSON.stringify({
  packageVersion,
  newcmPackageVersion,
  source: 'MathJax 4.1.3 pinned CJS; FontData/SvgFontData class and owner observations',
  results,
}, null, 2) + '\n');
console.log(`${path.relative(root, destination)}: pinned FontData class/lifetime source observations`);
