#!/usr/bin/env node
'use strict';

// Observe the pinned CommonJS SVG instance-extension path. The extension
// modules export data records; each record is applied through the already
// constructed output Jax and its FontData instance.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const packages = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const sourceLockPath = path.join(root, 'tools/math-port/common-layout-reference/profile-source-lock.json');
const sourceLock = JSON.parse(fs.readFileSync(sourceLockPath, 'utf8'));
const pinnedScopeHash = '9705d389538b2fc8bf782feea4372ace639b3727304f2971937bbcfb0042a0d1';
if (sourceLock.schema !== 1 || sourceLock.scopeHash !== pinnedScopeHash)
  throw new Error('Pinned FontData extension observation scope does not match the profile source lock');
const consumedModules = [
  'src/cjs/output/svg.js',
  'mathjax-newcm-font/cjs/svg.js',
  'mathjax-bboldx-font-extension/cjs/svg.js',
  'mathjax-bbm-font-extension/cjs/svg.js',
  'mathjax-dsfont-font-extension/cjs/svg.js',
];
const sourceFiles = consumedModules.map(file => {
  const locked = sourceLock.dependencies.find(item => item.file === file);
  if (!locked) throw new Error(`Source lock is missing observed CJS module ${file}`);
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(packages, file))).digest('hex');
  if (actual !== locked.sha256) throw new Error(`Pinned CJS hash mismatch for ${file}: ${actual}`);
  return {file, sha256:actual};
});
const {SVG} = require(path.join(packages, 'src/cjs/output/svg.js'));
const {MathJaxNewcmFont} = require(path.join(packages, 'mathjax-newcm-font/cjs/svg.js'));

const extensions = [
  ['bboldx', 'MathJaxBboldxFontExtension'],
  ['bbm', 'MathJaxBbmFontExtension'],
  ['dsfont', 'MathJaxDsfontFontExtension'],
];

function describe(value) {
  if (value === undefined) return {$:'undefined'};
  if (value === null || typeof value !== 'object') return value;
  if (Array.isArray(value)) {
    return {$:'array',length:value.length,slots:Array.from({length:value.length},(_,i)=>
      Object.hasOwn(value,i)?describe(value[i]):{$:'hole'})};
  }
  if (value instanceof Map) return {$:'map',entries:[...value].map(([key,item])=>[describe(key),describe(item)])};
  return {$:'object',entries:Object.keys(value).map(key=>[key,describe(value[key])])};
}

function ownKeys(value) {
  return value && typeof value === 'object' ? Object.keys(value) : [];
}

function sourceState(output, data, extensionName, result) {
  const font = output.font;
  const constructor = font.constructor;
  const dynamic = constructor.dynamicExtensions.get(data.name);
  const variantNames = (data.variants && data.variants['[+]'] || []).map(entry=>entry[0]);
  const smpNames = ownKeys(data.variantSmp);
  const cssNames = ownKeys(data.cssFonts);
  const optionNames = ownKeys(data.options);
  const parameterNames = ownKeys(data.parameters);
  const delimiterNames = ownKeys(data.delimiters);
  const charNames = ownKeys(data.chars);
  const sourceMap = Object.fromEntries(smpNames.map(key=>[key,describe(constructor.VariantSmp[key])]));
  const addedGlyphs = Object.fromEntries(charNames.map(name=>[
    name,Object.fromEntries(Object.keys(data.chars[name]).map(code=>[
      code,describe(font.getChar(name,Number(code))),
    ])),
  ]));
  const addedDelimiters = Object.fromEntries(delimiterNames.map(code=>[
    code,describe(font.getDelimiter(Number(code))),
  ]));
  const cssMap = Object.fromEntries(cssNames.map(key=>[key,describe(font.cssFontMap[key])]));
  const optionMap = Object.fromEntries(optionNames.map(key=>[key,describe(font.options[key])]));
  const parameterMap = Object.fromEntries(parameterNames.map(key=>[key,describe(font.params[key])]));
  const variants = Object.fromEntries(variantNames.map(key=>[key,{
    exists:Object.hasOwn(font.variant,key),
    cacheID:describe(font.variant[key] && font.variant[key].cacheID),
  }]));
  return {
    extensionName,
    dynamicKeys:[...constructor.dynamicExtensions.keys()],
    dynamicRecord:dynamic ? {
      name:dynamic.name,prefix:dynamic.prefix,sizeN:dynamic.sizeN,
      stretchN:dynamic.stretchN,fileKeys:Object.keys(dynamic.files),
    } : null,
    sizeVariants:describe(font.sizeVariants),
    stretchVariants:describe(font.stretchVariants),
    addedVariantNames:variantNames,
    addedVariants:variants,
    addedSmp:sourceMap,
    addedGlyphs,
    addedDelimiters,
    addedCssFonts:cssMap,
    addedOptions:optionMap,
    addedParameters:parameterMap,
    delimiterNames,
    charNames,
    result:describe(result),
    resultLength:Array.isArray(result)?result.length:{$:'non-array',type:typeof result},
  };
}

const output = new SVG({fontData:MathJaxNewcmFont,fontCache:'none',exFactor:.5,
  displayOverflow:'linebreak',linebreaks:{inline:true}});
const initialClassMap = MathJaxNewcmFont.dynamicExtensions;
const initialClassKeys = [...initialClassMap.keys()];
const sourceRows = [];
const nativeEquivalent = [];

for (const [name,exportName] of extensions) {
  const classKeysBeforeImport = [...MathJaxNewcmFont.dynamicExtensions.keys()];
  const extensionPath = path.join(packages,`mathjax-${name}-font-extension/cjs/svg.js`);
  const exported = require(extensionPath)[exportName];
  const classKeysAfterImport = [...MathJaxNewcmFont.dynamicExtensions.keys()];
  const reads = [];
  const observed = new Proxy(exported,{
    get(target,key,receiver){
      reads.push(typeof key==='symbol'?`Symbol(${key.description||''})`:String(key));
      return Reflect.get(target,key,receiver);
    },
  });
  const result = output.addExtension(observed);
  const record = sourceState(output,exported,name,result);
  sourceRows.push({
    name,exportName,reads,classKeysBeforeImport,classKeysAfterImport,
    importChangedClassMap:classKeysAfterImport.join('\0')!==classKeysBeforeImport.join('\0'),
    result:describe(result),
  });
  nativeEquivalent.push(record);
}

const result = {
  source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e',
  sourceScope:{scopeHash:sourceLock.scopeHash,corpusSha256:sourceLock.corpusSha256,files:sourceFiles},
  operation:'new SVG, then output.addExtension(data) in pinned profile order',
  initialClassKeys,
  sourceRows,
  nativeEquivalent,
};
const target = path.join(root,'build/math-port/font-instance-extension-original.json');
fs.mkdirSync(path.dirname(target),{recursive:true});
fs.writeFileSync(target,`${JSON.stringify(result,null,2)}\n`);
process.stdout.write(`${path.relative(root,target)}\n`);
