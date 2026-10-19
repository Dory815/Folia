#!/usr/bin/env node
'use strict';

// Exercise MathJax's actual NewCM module requests through Node's package
// resolver and the pinned NewCM dynamic CJS modules. This records module
// evaluation order and the real glyph tables after the Source setup callback.
const fs = require('node:fs');
const path = require('node:path');
const {execFileSync} = require('node:child_process');
const {createRequire} = require('node:module');
const root = path.resolve(__dirname, '../..');
const at = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const {MathJaxNewcmFont: Font} = require(path.join(at, 'mathjax-newcm-font/cjs/svg.js'));
const mathjax = require(path.join(at, 'src/cjs/mathjax.js')).mathjax;
const sourceRequire = createRequire(path.join(at, 'src/cjs/util/asyncLoad/node.js'));

const normalize = (value) => {
  if (value === undefined) return {$:'undefined'};
  if (value === null || typeof value === 'string' || typeof value === 'boolean') return value;
  if (typeof value === 'number') {
    if (Number.isNaN(value)) return {$:'NaN'};
    if (!Number.isFinite(value)) return {$:value < 0 ? '-Infinity' : 'Infinity'};
    if (Object.is(value, -0)) return {$:'-0'};
    return value;
  }
  if (Array.isArray(value)) return Array.from({length:value.length}, (_, i) =>
    Object.hasOwn(value, i) ? normalize(value[i]) : {$:'hole'});
  const out = {};
  for (const key of Object.keys(value).sort()) out[key] = normalize(value[key]);
  return out;
};
const aliasCacheWitness = (first) => {
  const script = String.raw`
    const path = require('node:path');
    const {createRequire} = require('node:module');
    const root = process.argv[1];
    const first = process.argv[2];
    const at = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
    const sourceRequire = createRequire(path.join(at, 'src/cjs/util/asyncLoad/node.js'));
    const {MathJaxNewcmFont: Font} = require(path.join(at, 'mathjax-newcm-font/cjs/svg.js'));
    const file = Font.dynamicFiles.arrows;
    const js = '@mathjax/mathjax-newcm-font/js/svg/dynamic/arrows.js';
    const cjs = '@mathjax/mathjax-newcm-font/cjs/svg/dynamic/arrows.js';
    const initialSetup = file.setup;
    const before = typeof file.setup;
    const firstExports = sourceRequire(first === 'js' ? js : cjs);
    const afterFirst = typeof file.setup;
    const changedAfterFirst = file.setup !== initialSetup;
    const setupAfterFirst = file.setup;
    const secondExports = sourceRequire(first === 'js' ? cjs : js);
    const afterSecond = typeof file.setup;
    process.stdout.write(JSON.stringify({
      first, before, afterFirst, afterSecond, changedAfterFirst,
      stableAfterSecond:file.setup === setupAfterFirst,
      resolvedSame:sourceRequire.resolve(js) === sourceRequire.resolve(cjs),
      moduleIdentitySame:firstExports === secondExports,
      canonicalModule:'@mathjax/mathjax-newcm-font/' + path.relative(
        path.join(at, 'mathjax-newcm-font'), sourceRequire.resolve(js)).replace(/\\/g, '/'),
      cached:require('node:module')._cache[sourceRequire.resolve(js)] ===
        require('node:module')._cache[sourceRequire.resolve(cjs)]
    }));
  `;
  return JSON.parse(execFileSync(process.execPath, ['-e', script, root, first], {encoding:'utf8'}));
};
const sourcePrefix = Font.OPTIONS.dynamicPrefix;
const fileEntries = Object.entries(Font.dynamicFiles);
const trace = [];
const oldLoad = mathjax.asyncLoad;
const oldSync = mathjax.asyncIsSynchronous;
mathjax.asyncIsSynchronous = true;
mathjax.asyncLoad = (request) => {
  trace.push({phase:'request', request});
  const result = sourceRequire(request);
  const file = fileEntries.find(([, value]) =>
    `${sourcePrefix}/${value.file.replace(/(\.js)?$/, '.js')}` === request)?.[1];
  trace.push({phase:'evaluated', request, setup:typeof file?.setup});
  return result;
};

try {
  const font = new Font();
  const before = fileEntries.map(([key, file]) => ({key, setup:typeof file.setup, promise:!!file.promise}));
  font.loadDynamicFilesSync();
  const after = fileEntries.map(([key, file]) => ({key, setup:typeof file.setup, promise:!!file.promise, failed:!!file.failed}));
  const samples = {};
  for (const [name, codepoint] of [
    ['arrows', 0x219C], ['symbols', 0x2200], ['monospace', 0x1D400], ['sans-serif', 0x1D5D4],
  ]) samples[name] = normalize(font.getChar('normal', codepoint));
  const loadedGlyphCount = Object.values(font.variant)
    .reduce((sum, variant) => sum + Object.keys(variant.chars).length, 0);
  const output = {
    source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e',
    sourcePrefix,
    initialModuleCount:fileEntries.length,
    before,
    requests:trace,
    after,
    loadedGlyphCount,
    samples,
    aliasCache:[aliasCacheWitness('js'),aliasCacheWitness('cjs')],
  };
  const target = path.join(root, 'build/math-port/newcm-font-module-provider-original.json');
  fs.mkdirSync(path.dirname(target), {recursive:true});
  fs.writeFileSync(target, JSON.stringify(output, null, 2) + '\n');
  process.stdout.write(`${path.relative(root, target)}: ${trace.length} module events, ${output.loadedGlyphCount} glyphs\n`);
} finally {
  mathjax.asyncLoad = oldLoad;
  mathjax.asyncIsSynchronous = oldSync;
}
