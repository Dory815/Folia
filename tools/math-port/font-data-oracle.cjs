#!/usr/bin/env node
'use strict';

// Read-only original-result oracle for the pinned 4.1.3 font classes.  Native
// tests consume the checked-in source/count ledger and this tool's build-only
// JSON output; no JavaScript runs in the renderer.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const inputs = [
  ['@mathjax/mathjax-newcm-font', 'cjs/svg.js', 'MathJaxNewcmFont'],
  ['@mathjax/mathjax-bbm-font-extension', 'cjs/svg.js', 'MathJaxBbmFontExtension'],
  ['@mathjax/mathjax-bboldx-font-extension', 'cjs/svg.js', 'MathJaxBboldxFontExtension'],
  ['@mathjax/mathjax-dsfont-font-extension', 'cjs/svg.js', 'MathJaxDsfontFontExtension'],
];
const normalize = (value) => {
  if (value === undefined) return {$:'undefined'};
  if (value === null || typeof value === 'string' || typeof value === 'boolean') return value;
  if (typeof value === 'number') {
    if (Number.isNaN(value)) return {$:'NaN'};
    if (!Number.isFinite(value)) return {$:value < 0 ? '-Infinity' : 'Infinity'};
    if (Object.is(value, -0)) return {$:'-0'};
    return value;
  }
  if (Array.isArray(value)) return Array.from({length:value.length},(_,i)=>
    Object.hasOwn(value,i) ? normalize(value[i]) : {$:'hole'});
  const out = {};
  for (const key of Object.keys(value).sort()) out[key] = normalize(value[key]);
  return out;
};
const hash = (value) => crypto.createHash('sha256').update(JSON.stringify(normalize(value))).digest('hex');
const tableStats = (FontClass) => ({
  variantNames:Object.keys(FontClass.defaultChars),
  glyphCount:Object.values(FontClass.defaultChars).reduce((sum,map)=>sum+Object.keys(map).length,0),
  glyphTableSha256:hash(FontClass.defaultChars),
  delimiters:Object.keys(FontClass.defaultDelimiters).length,
  delimiterTableSha256:hash(FontClass.defaultDelimiters),
  cacheIds:normalize(FontClass.variantCacheIds||{}),
  options:normalize(FontClass.OPTIONS),
  variants:normalize(FontClass.defaultVariants),
  cssFonts:normalize(FontClass.defaultCssFonts),
  variantSmp:normalize(FontClass.VariantSmp),
  accentMap:normalize(FontClass.defaultAccentMap),
  moMap:normalize(FontClass.defaultMoMap),
  mnMap:normalize(FontClass.defaultMnMap),
  sizeVariants:normalize(FontClass.defaultSizeVariants),
  stretchVariants:normalize(FontClass.defaultStretchVariants),
  parameters:normalize(FontClass.defaultParams),
  dynamicFiles:Object.keys(FontClass.dynamicFiles||{}),
  dynamicExtensions:[...(FontClass.dynamicExtensions||new Map()).entries()].map(([name,data])=>({
    name,prefix:data.prefix,sizeN:data.sizeN,stretchN:data.stretchN,files:Object.keys(data.files||{}),
  })),
});
const results=[];
for (const [packageName, relative, exportName] of inputs) {
  const packageDir=path.join(packageRoot,packageName.slice('@mathjax/'.length));
  const manifest=JSON.parse(fs.readFileSync(path.join(packageDir,'package.json'),'utf8'));
  if(manifest.version!=='4.1.3')throw new Error(`expected ${packageName}@4.1.3, got ${manifest.version}`);
  const source=path.join(packageDir,relative), bytes=fs.readFileSync(source);
  const exported=require(source)[exportName];
  const extension=packageName.includes('-extension');
  let instance=null;
  if(!extension) instance=new exported();
  const data=extension?exported:exported;
  const chars=extension?data.chars:exported.defaultChars;
  const delimiters=extension?(data.delimiters||{}):exported.defaultDelimiters;
  results.push({
    package:packageName, version:manifest.version, source:relative,
    sourceSha256:crypto.createHash('sha256').update(bytes).digest('hex'),
    extension,
    variantCount:extension?Object.keys(chars).length:Object.keys(chars).length,
    glyphCount:Object.values(chars).reduce((sum,map)=>sum+Object.keys(map).length,0),
    delimiterCount:Object.keys(delimiters).length,
    variantNames:Object.keys(chars),
    glyphTableSha256:hash(chars), delimiterTableSha256:hash(delimiters),
    variants: extension ? normalize(data.variants) : normalize(exported.defaultVariants),
    cacheIds: extension ? normalize(data.cacheIds||{}) : normalize(exported.variantCacheIds||{}),
    dynamicFileNames: extension ? Object.keys(data.ranges||{}) : Object.keys(exported.dynamicFiles||{}),
    samples: extension ? {} : {
      asciiA:normalize(instance.getChar('normal',65)),
      minus:normalize(instance.getChar('normal',0x2212)),
      missing:normalize(instance.getChar('normal',0x10ffff)),
      doubleStruckA:normalize(instance.getChar('double-struck',65)),
      family:instance.getFamily('serif'),
      missingCssFont:normalize(instance.getCssFont('missing')),
      remappedAccent:normalize(instance.getRemappedChar('accent',0x2c6)),
      delimiterParen:normalize(instance.getDelimiter(0x28)),
      sizeVariantParen:normalize(instance.getSizeVariant(0x28,0)),
      stretchVariantsParen:normalize(instance.getStretchVariants(0x28)),
    },
    ...(!extension ? {instance:{
      options:normalize(instance.options),
      parameters:normalize(instance.params),
      sizeVariants:normalize(instance.sizeVariants),
      stretchVariants:normalize(instance.stretchVariants),
      cssFamilyPrefix:instance.cssFamilyPrefix,
      cssFontPrefix:instance.cssFontPrefix,
      cssFonts:normalize(instance.cssFontMap),
      remaps:normalize(instance.remapChars),
      variantNames:Object.keys(instance.variant),
      cacheIds:Object.fromEntries(Object.keys(instance.variant).map(name=>[name,normalize(instance.variant[name].cacheID)])),
      variantRecordIdentity:instance.getVariant('normal')===instance.variant.normal,
      sharedCharacterRecords:{
        normalBoldZero:instance.getChar('normal',48)===instance.getChar('bold',48),
        normalItalicZero:instance.getChar('normal',48)===instance.getChar('italic',48),
        normalBoldA:instance.getChar('normal',65)===instance.getChar('bold',65),
        normalBold8213:instance.getChar('normal',8213)===instance.getChar('bold',8213),
        missingIsUndefined:instance.getChar('normal',0x10ffff)===undefined,
      },
      cssQueries:{normal:normalize(instance.getCssFont('normal')),unknown:normalize(instance.getCssFont('missing')),
        family:instance.getFamily('serif')},
      styles:{initial:normalize(instance.styles),setAndRead:(()=>{const style={test:'identity'};instance.styles=style;return {value:normalize(instance.styles),sameIdentity:instance.styles===style};})()},
    }} : {}),
  });
}
// Exercise the original synchronous-load path with the package's own shard
// modules. The callback only maps the Source URL to the pinned CJS file.
const {MathJaxNewcmFont}=require(path.join(packageRoot,'mathjax-newcm-font/cjs/svg.js'));
const mathjax=require(path.join(packageRoot,'src/cjs/mathjax.js')).mathjax;
const previousSynchronous=mathjax.asyncIsSynchronous;
const previousLoad=mathjax.asyncLoad;
mathjax.asyncIsSynchronous=true;
mathjax.asyncLoad=(url)=>{
  const file=path.basename(String(url));
  const shard=path.join(packageRoot,'mathjax-newcm-font/cjs/svg/dynamic',file);
  if(!fs.existsSync(shard))throw new Error(`no original NewCM shard for ${url}`);
  require(shard);
};
try {
  const loaded=new MathJaxNewcmFont();
  const dynamic=loaded.getChar('normal',0xC0);
  results[0].samples.dynamicLatinAfterLoad=normalize(dynamic);
  loaded.loadDynamicFilesSync();
  results[0].dynamicTables={
    chars:Object.fromEntries(Object.keys(loaded.variant).map(name=>[name,normalize(loaded.variant[name].chars)])),
    delimiters:normalize(loaded.delimiters),
  };
  results[0].samples.dynamicFileSetup=Object.keys(MathJaxNewcmFont.dynamicFiles).map(name=>({name,failed:MathJaxNewcmFont.dynamicFiles[name].failed}));
  results[0].classExtensionMerges=[];
  results[0].classExtensionMerges.push({name:'initial',state:tableStats(MathJaxNewcmFont)});
  for(const [packageName,relative,exportName] of inputs.slice(1)){
    const source=path.join(packageRoot,packageName.slice('@mathjax/'.length),relative);
    const Extension=require(source)[exportName];
    const before=tableStats(MathJaxNewcmFont);
    const result=MathJaxNewcmFont.addExtension(Extension);
    results[0].classExtensionMerges.push({name:packageName,before,returnValue:normalize(result),state:tableStats(MathJaxNewcmFont)});
  }
} finally {
  mathjax.asyncIsSynchronous=previousSynchronous;
  mathjax.asyncLoad=previousLoad;
}
const output={source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e',results};
const destination=path.join(root,'build/math-port/font-original.json');
fs.mkdirSync(path.dirname(destination),{recursive:true});
fs.writeFileSync(destination,JSON.stringify(output,null,2)+'\n','utf8');
console.log(`${path.relative(root,destination)}: ${results.length} pinned font sources`);
for(const item of results)console.log(`${item.package}: variants=${item.variantCount}, glyphs=${item.glyphCount}, delimiters=${item.delimiterCount}, sha256=${item.sourceSha256}`);
