#!/usr/bin/env node
'use strict';

// Pinned Source witnesses for FontData field replacement and aliases. These
// invoke the shipped CJS FontData methods directly; observations are derived
// from the resulting objects, not reconstructed from expected values.
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
const {FontData} = require(path.join(root,
  'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/output/common/FontData.js'));

function describe(value, seen = new Map()) {
  if (value === undefined) return {$:'undefined'};
  if (value === null || typeof value === 'string' || typeof value === 'boolean') return value;
  if (typeof value === 'number') {
    if (Number.isNaN(value)) return {$:'NaN'};
    if (!Number.isFinite(value)) return {$:value < 0 ? '-Infinity' : 'Infinity'};
    if (Object.is(value, -0)) return {$:'-0'};
    return value;
  }
  if (typeof value === 'symbol') return {$:'symbol', description:value.description ?? null};
  if (seen.has(value)) return {$:'ref', id:seen.get(value)};
  const id = seen.size + 1;
  seen.set(value, id);
  if (Array.isArray(value)) {
    const items = Array.from({length:value.length}, (_, i) =>
      Object.hasOwn(value, i) ? describe(value[i], seen) : {$:'hole'});
    const extras = Reflect.ownKeys(value).filter((key) =>
      key !== 'length' && !(typeof key === 'string' && /^(0|[1-9]\d*)$/.test(key) && Number(key) < value.length));
    return {$:'array', id, items, extras:extras.map((key) => [describe(key, seen), describe(value[key], seen)])};
  }
  if (value instanceof Map) return {$:'map', id, entries:[...value].map(([k,v])=>[describe(k,seen),describe(v,seen)])};
  const keys = Reflect.ownKeys(value);
  return {$:'object', id, entries:keys.map((key) => {
    const descriptor = Object.getOwnPropertyDescriptor(value,key);
    if ('value' in descriptor) return [describe(key,seen),describe(descriptor.value,seen),descriptor.enumerable];
    return [describe(key,seen),{$:'accessor',get:!!descriptor.get,set:!!descriptor.set},descriptor.enumerable];
  })};
}

function makeFontClass() {
  class ProbeFontData extends FontData {}
  ProbeFontData.dynamicExtensions = new Map();
  return ProbeFontData;
}

const output = {source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e', cases:{}};

{
  const F = makeFontClass();
  const font = new F();
  const incoming = new Array(3);
  incoming[1] = 'only-index-one';
  font.addExtension({name:'sparse-length-3-index-1',sizeVariants:incoming});
  output.cases.plainArrayReplacementCopiesEnumerableIndexes = {
    sourceLength:incoming.length,
    sourceEnumerableKeys:Object.keys(incoming),
    resultLength:font.sizeVariants.length,
    resultEnumerableKeys:Object.keys(font.sizeVariants),
    result:describe(font.sizeVariants),
  };
}

{
  const F = makeFontClass();
  const font = new F();
  const reads = [];
  for (const name of ['set', 'get', 'values']) {
    const method = Map.prototype[name];
    Object.defineProperty(method, 'call', {
      configurable:true,
      get() { reads.push(`${name}.call`); throw new Error('unexpected Function.call lookup'); }
    });
  }
  let request;
  try {
    font.addExtension({name:'direct-map-call', ranges:[['arrow.js', {}, []]]}, 'direct-map-call');
    const dynamic = F.dynamicExtensions.get('direct-map-call');
    request = font.dynamicFileName(dynamic.files['arrow.js']);
    const values = [...F.dynamicExtensions.values()];
    output.cases.mapMemberCallsDoNotReadFunctionCall = {
      request, valuesCount:values.length, mapSize:F.dynamicExtensions.size, reads
    };
  } finally {
    for (const name of ['set', 'get', 'values']) {
      delete Map.prototype[name].call;
    }
  }
}

{
  const F = makeFontClass();
  const font = new F();
  font.sizeVariants = ['old-a','old-b','old-tail'];
  const oldAlias = font.sizeVariants;
  const nested = {leaf:[1, {value:2}]};
  const incoming = [];
  incoming.length = 4;
  incoming[0] = 'new-a';
  incoming[2] = nested;
  const extra = {kept:true};
  incoming.extra = extra;
  const symbolKey = Symbol('font-field');
  incoming[symbolKey] = {symbolValue:9};
  const extensionResult = font.addExtension({name:'plain-replacement',sizeVariants:incoming});
  const applied = font.sizeVariants;
  output.cases.plainArrayIntoExistingArray = {
    extensionResult:describe(extensionResult),
    sameFieldAsOldAlias:applied===oldAlias,
    oldAlias:describe(oldAlias),
    currentField:describe(applied),
    inputAfterCall:describe(incoming),
    nestedIdentityPreserved:nested===applied[2],
    extraIdentityPreserved:extra===applied.extra,
    symbolKeys:Reflect.ownKeys(applied).filter((x)=>typeof x==='symbol').map((x)=>x.description),
  };
}

{
  const F = makeFontClass();
  const font = new F();
  const removed = {token:'remove'};
  const kept = {token:'keep'};
  const appended = {token:'append'};
  font.sizeVariants = [removed,kept,removed];
  const oldAlias = font.sizeVariants;
  font.addExtension({name:'append-remove',sizeVariants:{'[-]':[removed],'[+]':[appended]}});
  output.cases.appendRemoveArrayPatch = {
    oldAliasStillCurrent:font.sizeVariants===oldAlias,
    oldAlias:describe(oldAlias),
    current:describe(font.sizeVariants),
    removedByStrictIdentity:font.sizeVariants.includes(removed),
    keptIdentity:font.sizeVariants[0]===kept,
    appendedIdentity:font.sizeVariants[1]===appended,
  };
}

{
  const F = makeFontClass();
  const font = new F();
  const oldAlias = font.sizeVariants;
  font.sizeVariants = null;
  let laterUse;
  try { font.addExtension({name:'raw-field-use',sizeVariants:[]}); }
  catch (error) { laterUse = {name:error.name,message:error.message}; }
  output.cases.rawAssignmentAndDeferredFailure = {
    assignmentAccepted:font.sizeVariants===null,
    oldAliasUnchanged:font.sizeVariants!==oldAlias && Array.isArray(oldAlias),
    postOperationValue:describe(font.sizeVariants),
    laterUse:laterUse ?? null,
    dynamicExtensionKeys:[...F.dynamicExtensions.keys()],
  };
}

{
  const F = makeFontClass();
  const first = {name:'first'};
  const replaced = {name:'replaced'};
  F.dynamicExtensions.set('first',first);
  F.dynamicExtensions.set('replace-me',{name:'old'});
  const mapAlias = F.dynamicExtensions;
  F.dynamicExtensions.set('last',{name:'last'});
  const beforeKeys = [...F.dynamicExtensions.keys()];
  F.dynamicExtensions.set('replace-me',replaced);
  output.cases.dynamicExtensionMapSet = {
    mapIdentityPreserved:F.dynamicExtensions===mapAlias,
    keysBefore:beforeKeys,
    keysAfter:[...F.dynamicExtensions.keys()],
    replacedValueIdentity:F.dynamicExtensions.get('replace-me')===replaced,
    firstValueIdentity:F.dynamicExtensions.get('first')===first,
    entries:describe(F.dynamicExtensions),
  };
}

{
  const F = makeFontClass();
  const font = new F();
  const reads=[];
  const sizes=[];
  Object.defineProperty(sizes,'0',{enumerable:true,configurable:true,get(){reads.push('size[0]');return 'from-getter';}});
  sizes.length=2;
  Object.defineProperty(sizes,'extra',{enumerable:true,configurable:true,get(){reads.push('size.extra');return 17;}});
  font.addExtension({name:'getter-order',sizeVariants:sizes});
  output.cases.enumerableGetterMergeOrder = {
    reads,
    result:describe(font.sizeVariants),
    source:describe(sizes),
    sourceKeys:Reflect.ownKeys(sizes).map((key)=>typeof key==='symbol'?`Symbol(${key.description})`:key),
  };
}

{
  const F = makeFontClass();
  const first = {name:'first'};
  F.dynamicExtensions.set('first',first);
  const oldMap = F.dynamicExtensions;
  const replacement = new Map([
    ['keep',{name:'keep'}],
    ['replace-me',{name:'old'}],
    ['tail',{name:'tail'}],
  ]);
  const replacementValue = {name:'new'};
  F.dynamicExtensions = replacement;
  const assignedSameMap = F.dynamicExtensions === replacement;
  const keysBefore = [...replacement.keys()];
  F.dynamicExtensions.set('replace-me',replacementValue);
  output.cases.dynamicExtensionsRawMapReplacement = {
    sourceDescriptor:(() => {
      const d = Object.getOwnPropertyDescriptor(F,'dynamicExtensions');
      return {writable:d.writable,enumerable:d.enumerable,configurable:d.configurable};
    })(),
    assignedSameMap,
    oldMapStillHasFirst:oldMap.get('first')===first,
    oldMapKeys:[...oldMap.keys()],
    replacementIdentityPreserved:F.dynamicExtensions===replacement,
    keysBefore,
    keysAfter:[...F.dynamicExtensions.keys()],
    replacedValueIdentity:F.dynamicExtensions.get('replace-me')===replacementValue,
    replacementValue:describe(F.dynamicExtensions.get('replace-me')),
  };
}

{
  const F = makeFontClass();
  const font = new F();
  const reads=[];
  font.sizeVariants = {get length(){reads.push('size.length');return 4;}};
  font.stretchVariants = {get length(){reads.push('stretch.length');return 6;}};
  const beforeKeys=[...F.dynamicExtensions.keys()];
  const result=font.addExtension({name:'raw-length-getters'});
  output.cases.rawArrayFieldGetterOrder = {
    reads,
    beforeKeys,
    afterKeys:[...F.dynamicExtensions.keys()],
    sizeN:F.dynamicExtensions.get('raw-length-getters').sizeN,
    stretchN:F.dynamicExtensions.get('raw-length-getters').stretchN,
    result:describe(result),
  };
}

const target = path.join(root,'build/math-port/font-field-replacement-original.json');
fs.mkdirSync(path.dirname(target),{recursive:true});
fs.writeFileSync(target,`${JSON.stringify(output,null,2)}\n`);
process.stdout.write(`${path.relative(root,target)}\n`);
