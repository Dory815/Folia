'use strict';
// Source-only witness for the duplicate NewCM SVG cache ID. It invokes the
// pinned SVG wrapper's placeChar and SVG typesetSvg plus FontCache methods.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const { spawnSync } = require('node:child_process');

const root = path.resolve(__dirname, '../..');
const packages = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const cjs = path.join(packages, 'src/cjs');
const lock = JSON.parse(fs.readFileSync(path.join(root,
  'tools/math-port/common-layout-reference/profile-source-lock.json'), 'utf8'));
const hash = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const pinned = new Map(lock.dependencies.map(dep => [dep.file, dep.sha256]));
const sourceFiles = [
  'src/cjs/output/svg/FontCache.js',
  'src/cjs/output/svg/Wrapper.js',
  'src/cjs/output/svg.js',
  'mathjax-newcm-font/cjs/svg.js',
  'mathjax-bbm-font-extension/cjs/svg.js',
  'mathjax-bboldx-font-extension/cjs/svg.js',
];
for (const rel of sourceFiles) {
  const expected = pinned.get(rel);
  assert.ok(expected, `source lock is missing ${rel}`);
  assert.equal(hash(path.join(packages, rel)), expected, `pinned source changed: ${rel}`);
}
for (const pkg of ['src', 'mathjax-newcm-font', 'mathjax-bbm-font-extension', 'mathjax-bboldx-font-extension']) {
  assert.equal(JSON.parse(fs.readFileSync(path.join(packages, pkg, 'package.json'), 'utf8')).version,
    '4.1.3', `unexpected ${pkg} version`);
}

const { MathJaxNewcmFont } = require(path.join(packages, 'mathjax-newcm-font/cjs/svg.js'));
const bbm = require(path.join(packages, 'mathjax-bbm-font-extension/cjs/svg.js')).MathJaxBbmFontExtension;
const bboldx = require(path.join(packages, 'mathjax-bboldx-font-extension/cjs/svg.js')).MathJaxBboldxFontExtension;
const { FontCache } = require(path.join(cjs, 'output/svg/FontCache.js'));
const { SVG } = require(path.join(cjs, 'output/svg.js'));
const { SvgWrapper } = require(path.join(cjs, 'output/svg/Wrapper.js'));
const { liteAdaptor } = require(path.join(cjs, 'adaptors/liteAdaptor.js'));

// Match OutputJax.addExtension order from the locked profile (bboldx, then bbm).
MathJaxNewcmFont.addExtension(bboldx);
MathJaxNewcmFont.addExtension(bbm);
const font = new MathJaxNewcmFont();
const variants = ['-bbm-bold', '-bboldx'];
const codepoint = 0x21;
const sourceGlyphs = Object.fromEntries(variants.map(variant => {
  const tuple = font.getChar(variant, codepoint);
  assert.ok(Array.isArray(tuple), `Source glyph missing: ${variant}/U+21`);
  return [variant, {
    cache_id: font.getVariant(variant).cacheID,
    metrics: tuple.slice(0, 3),
    path_data: tuple[3].p,
  }];
}));
assert.equal(sourceGlyphs['-bbm-bold'].cache_id, 'NCM-MB-BB');
assert.equal(sourceGlyphs['-bboldx'].cache_id, 'NCM-MB-BB');
assert.notEqual(sourceGlyphs['-bbm-bold'].path_data, sourceGlyphs['-bboldx'].path_data);

// Probe the two install orders in isolated processes because addExtension
// mutates the static NewCM class. This verifies extension order cannot change
// either collision participant's cache identity or selected glyph bytes.
const orderProbe = String.raw`
const path = require('node:path');
const packages = process.argv[1], order = JSON.parse(process.argv[2]);
const F = require(path.join(packages, 'mathjax-newcm-font/cjs/svg.js')).MathJaxNewcmFont;
const extensions = {
  bboldx: require(path.join(packages, 'mathjax-bboldx-font-extension/cjs/svg.js')).MathJaxBboldxFontExtension,
  bbm: require(path.join(packages, 'mathjax-bbm-font-extension/cjs/svg.js')).MathJaxBbmFontExtension,
};
for (const name of order) F.addExtension(extensions[name]);
const font = new F();
process.stdout.write(JSON.stringify(Object.fromEntries(['-bbm-bold', '-bboldx'].map(variant => {
  const tuple = font.getChar(variant, 0x21);
  return [variant, { cache_id: font.getVariant(variant).cacheID, path_data: tuple[3].p }];
}))));
`;
function probeExtensionOrder(order) {
  const child = spawnSync(process.execPath, ['-e', orderProbe, packages, JSON.stringify(order)], { encoding: 'utf8' });
  assert.equal(child.status, 0, `extension-order probe failed: ${child.stderr}`);
  return JSON.parse(child.stdout);
}
const extensionOrderChecks = {
  profile_bboldx_then_bbm: probeExtensionOrder(['bboldx', 'bbm']),
  reversed_bbm_then_bboldx: probeExtensionOrder(['bbm', 'bboldx']),
};
assert.deepEqual(extensionOrderChecks.profile_bboldx_then_bbm, extensionOrderChecks.reversed_bbm_then_bboldx);
assert.deepEqual(extensionOrderChecks.profile_bboldx_then_bbm,
  Object.fromEntries(Object.entries(sourceGlyphs).map(([variant, value]) => [variant,
    { cache_id: value.cache_id, path_data: value.path_data }])));

function createSession(mode) {
  const adaptor = liteAdaptor();
  const jax = {
    font,
    options: { fontCache: mode, useXlink: false, localID: null },
    adaptor,
    svg: (kind, properties = {}, children = []) => adaptor.node(kind, properties, children),
    text: text => adaptor.text(text),
  };
  const fontCache = new FontCache(jax);
  jax.fontCache = fontCache;
  if (mode === 'global') fontCache.clearCache();
  return { mode, adaptor, jax, fontCache };
}

function emitOne(session, variant) {
  const { mode, adaptor, jax, fontCache } = session;
  const svg = adaptor.node('svg', {}, []);
  const group = adaptor.node('g', {}, []);
  adaptor.append(svg, group);
  const wrapper = Object.create(SvgWrapper.prototype);
  Object.defineProperties(wrapper, {
    font: { value: font }, jax: { value: jax }, adaptor: { value: adaptor },
    variant: { value: variant }, utext: { value: '' }, dx: { value: 0 },
    bbox: { value: { rscale: 1 } },
  });
  wrapper.toSVG = parents => SvgWrapper.prototype.placeChar.call(wrapper,
    codepoint, 0, 0, parents[0], variant);
  // Call Source's actual output lifecycle: local mode clears the cache and
  // inserts the defs into each math SVG before calling wrapper.toSVG.
  SVG.prototype.typesetSvg.call(jax, wrapper, svg, group);
  return {
    svg: adaptor.serializeXML(svg),
    defs: mode === 'global' ? adaptor.serializeXML(fontCache.getCache()) : null,
    cache: [...fontCache.cache.entries()],
  };
}

function emitCompound(session, order) {
  const { mode, adaptor, jax, fontCache } = session;
  const svg = adaptor.node('svg', {}, []);
  const group = adaptor.node('g', {}, []);
  adaptor.append(svg, group);
  const wrapper = Object.create(SvgWrapper.prototype);
  Object.defineProperties(wrapper, {
    font: { value: font }, jax: { value: jax }, adaptor: { value: adaptor },
    variant: { value: order[0] }, utext: { value: '' }, dx: { value: 0 },
    bbox: { value: { rscale: 1 } },
  });
  // One typesetSvg invocation represents one math item and therefore one
  // SVG. Exercise both semantic variants in that same SVG/cache lifetime.
  wrapper.toSVG = parents => order.map(variant =>
    SvgWrapper.prototype.placeChar.call(wrapper, codepoint, 0, 0, parents[0], variant));
  SVG.prototype.typesetSvg.call(jax, wrapper, svg, group);
  return { svg: adaptor.serializeXML(svg), defs: mode === 'global' ? adaptor.serializeXML(fontCache.getCache()) : null,
    cache: [...fontCache.cache.entries()] };
}

const witness = {
  schema: 1,
  source: {
    package_version: '4.1.3',
    cache_key_method: 'pinned SVG FontCache.cachePath via SvgWrapper.placeChar',
    source_files: Object.fromEntries(sourceFiles.map(rel => [rel, pinned.get(rel)])),
    source_glyphs: sourceGlyphs,
    extension_order_checks: extensionOrderChecks,
  },
  cases: {},
};
for (const mode of ['none', 'local']) {
  witness.cases[mode] = {};
  for (const order of [variants, [...variants].reverse()]) {
    const key = order.join(' then ');
    const session = createSession(mode);
    witness.cases[mode][key] = order.map(variant => ({
      variant,
      ...emitOne(session, variant),
    }));
  }
  witness.cases[mode].one_svg = {};
  for (const order of [variants, [...variants].reverse()]) {
    const key = order.join(' then ');
    witness.cases[mode].one_svg[key] = {
      variants: order,
      ...emitCompound(createSession(mode), order),
    };
  }
}
// Also capture the actual shared-cache first-writer result in both orders.
// The serializer is the same Source wrapper/FontCache path; only the cache is
// deliberately kept across math items, unlike fontCache='local'.
for (const order of [variants, [...variants].reverse()]) {
  const key = order.join(' then ');
  const adaptor = liteAdaptor();
  const jax = { font, options: { fontCache: 'global', useXlink: false, localID: null }, adaptor,
    svg: (kind, properties = {}, children = []) => adaptor.node(kind, properties, children),
    text: text => adaptor.text(text) };
  jax.fontCache = new FontCache(jax);
  jax.fontCache.clearCache();
  const defs = jax.fontCache.getCache();
  witness.cases.global = witness.cases.global || {};
  witness.cases.global[key] = order.map(variant => {
    const svg = adaptor.node('svg', {}, []), group = adaptor.node('g', {}, []);
    adaptor.append(svg, group);
    const wrapper = Object.create(SvgWrapper.prototype);
    Object.defineProperties(wrapper, {
      font: { value: font }, jax: { value: jax }, adaptor: { value: adaptor },
      variant: { value: variant }, utext: { value: '' }, dx: { value: 0 },
      bbox: { value: { rscale: 1 } },
    });
    wrapper.toSVG = parents => SvgWrapper.prototype.placeChar.call(wrapper,
      codepoint, 0, 0, parents[0], variant);
    SVG.prototype.typesetSvg.call(jax, wrapper, svg, group);
    return { variant, svg: adaptor.serializeXML(svg), defs: adaptor.serializeXML(defs),
      cache: [...jax.fontCache.cache.entries()] };
  });
}

for (const mode of ['none', 'local']) {
  for (const [orderKey, items] of Object.entries(witness.cases[mode])) {
    if (orderKey === 'one_svg') continue;
    assert.deepEqual(items.map(item => item.variant), orderKey.split(' then '));
    for (const [index, item] of items.entries()) {
      const expectedPath = `M${sourceGlyphs[item.variant].path_data}Z`;
      if (mode === 'none') assert.ok(item.svg.includes(`<path data-c="21" d="${expectedPath}"`));
      else assert.ok(item.svg.includes(`<path id="MJX-${index + 1}-NCM-MB-BB-21" d="${expectedPath}"`));
    }
  }
  for (const [orderKey, item] of Object.entries(witness.cases[mode].one_svg)) {
    const order = orderKey.split(' then ');
    assert.deepEqual(item.variants, order);
    assert.equal((item.svg.match(/data-c="21"/g) || []).length, 2,
      `one SVG should contain both glyph placements for ${orderKey}`);
    for (let index = 0; index < order.length; index++) {
      const pathData = sourceGlyphs[order[index]].path_data;
      if (mode === 'none') {
        assert.ok(item.svg.includes(`<path data-c="21" d="M${pathData}Z"`),
          `inline path missing for ${order[index]} in ${orderKey}`);
      } else {
        const winner = sourceGlyphs[order[0]].path_data;
        assert.ok(item.svg.includes(`<path id="MJX-1-NCM-MB-BB-21" d="M${winner}Z"`),
          `local cache should retain first path for ${orderKey}`);
        assert.ok(item.svg.includes('href="#MJX-1-NCM-MB-BB-21"'),
          `local cache use missing for ${order[index]} in ${orderKey}`);
      }
    }
    if (mode === 'local') {
      assert.equal((item.svg.match(/<path id="MJX-1-NCM-MB-BB-21"/g) || []).length, 1);
      assert.equal((item.svg.match(/href="#MJX-1-NCM-MB-BB-21"/g) || []).length, 2);
    }
  }
}
for (const [orderKey, items] of Object.entries(witness.cases.global)) {
  assert.deepEqual(items.map(item => item.variant), orderKey.split(' then '));
  const firstPath = `M${sourceGlyphs[items[0].variant].path_data}Z`;
  assert.ok(items[0].defs.includes(`d="${firstPath}"`));
  assert.equal(items[1].defs, items[0].defs);
  assert.ok(items.every(item => item.svg.includes('href="#MJX-NCM-MB-BB-21"')));
}

const outputIndex = process.argv.indexOf('--output');
if (outputIndex >= 0 && !process.argv[outputIndex + 1]) throw new Error('--output requires a path');
const dest = outputIndex >= 0 ? path.resolve(process.argv[outputIndex + 1]) : path.join(root, 'build/math-port/font-cache-source-witness.json');
fs.writeFileSync(dest, JSON.stringify(witness, null, 2) + '\n');
console.log(JSON.stringify({
  output: path.relative(root, dest),
  cache_ids: Object.fromEntries(Object.entries(sourceGlyphs).map(([k, v]) => [k, v.cache_id])),
  paths_distinct: sourceGlyphs['-bbm-bold'].path_data !== sourceGlyphs['-bboldx'].path_data,
  modes: Object.fromEntries(Object.entries(witness.cases).map(([mode, cases]) => [mode, Object.keys(cases)])),
}));
