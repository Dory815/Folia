// Independent MathJax 4.1.3 CJS observations for the base TeX package.
// This is an oracle only: it loads the pinned upstream package and never loads native code.
'use strict';
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');

const root = path.resolve(__dirname, '../..');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const locks = {
  'ts/input/tex/base/BaseConfiguration.ts': '578d2c6b03f2f3e902476a0b73a5323b12262215fcb d c831a1135c1d9c1737ad'.replaceAll(' ', ''),
  'ts/input/tex/base/BaseMethods.ts': '7336240c8f9c5037f9fdcd5a84ea079312854f1ef3f406aeb5a8b6a8ff765cda',
  'ts/input/tex/base/BaseMappings.ts': '08dada0ed7fab0134da2c3a341e30637959ebb730512835b9482d7c78d72adae',
};
const cjsLocks = {
  'input/tex/base/BaseConfiguration.js': '0feedc60ac600b647cf2a1437c7244196a5aec04715f4a301fe6e9ec09722c57',
  'input/tex/base/BaseMethods.js': 'b8d5c74e01f85ae391ec8d16ffcd216a4ca4267bd8ffa091c5bfe1462ad7b491',
  'input/tex/base/BaseMappings.js': '3522fc3fbb50af2f65442be3b2fbb2623824620899ebe775a87b885928c4acce',
};
const files = [];
for (const [source, expected] of Object.entries(locks)) {
  const bytes = fs.readFileSync(path.join(sourceRoot, source));
  const hash = crypto.createHash('sha256').update(bytes).digest('hex');
  assert.equal(hash, expected, `${source} pinned TS changed`);
  const cjs = source.slice(3).replace(/\.ts$/, '.js');
  const cjsBytes = fs.readFileSync(path.join(packageRoot, 'cjs', cjs));
  const cjsHash = crypto.createHash('sha256').update(cjsBytes).digest('hex');
  assert.equal(cjsHash, cjsLocks[cjs], `${cjs} pinned CJS changed`);
  assert.equal(fs.readFileSync(path.join(packageRoot, source), 'utf8').replaceAll('\r\n', '\n'),
    bytes.toString('utf8').replaceAll('\r\n', '\n'), `${source} installed TS differs`);
  files.push({ source, sha256: hash, cjs, cjs_sha256: cjsHash });
}

const load = name => require(path.join(packageRoot, 'cjs', name));
const TexParser = load('input/tex/TexParser.js').default;
const { TeX } = load('input/tex.js');
const { MmlFactory } = load('core/MmlTree/MmlFactory.js');
const BaseMethods = load('input/tex/base/BaseMethods.js').default;
const { BaseConfiguration } = load('input/tex/base/BaseConfiguration.js');
const { MapHandler } = load('input/tex/MapHandler.js');
const rows = [];
function plain(value, seen = new WeakSet()) {
  if (value === undefined) return { $: 'undefined' };
  if (value === null) return null;
  if (typeof value === 'number') return Number.isNaN(value) ? { $: 'NaN' } : Object.is(value, -0) ? { $: '-0' } : value;
  if (typeof value === 'function') return { $: 'function', name: value.name || '', length: value.length };
  if (typeof value !== 'object') return value;
  if (seen.has(value)) return { $: 'cycle' };
  seen.add(value);
  if (Array.isArray(value)) return { $: 'array', items: Array.from({ length: value.length }, (_, i) =>
    Object.hasOwn(value, i) ? plain(value[i], seen) : { $: 'hole' }) };
  const out = {};
  for (const key of Reflect.ownKeys(value)) out[typeof key === 'symbol' ? `symbol:${key.description || ''}` : key] = plain(value[key], seen);
  return out;
}
function mml(node, seen = new WeakSet()) {
  if (node == null) return node;
  if (seen.has(node)) return { $: 'cycle' };
  seen.add(node);
  const attrs = node.attributes?.getAllAttributes?.() || {};
  const props = node.getAllProperties?.() || {};
  return { kind: node.kind, text: node.text ?? null, attrs: plain(attrs), properties: plain(props),
    children: (node.childNodes || []).map(child => mml(child, seen)) };
}
function error(fn) {
  try { return { result: fn(), error: null }; }
  catch (e) { return { result: null, error: { name: e.name || e.constructor.name, id: e.id ?? null, message: String(e.message) } }; }
}
function record(name, tex, display = false) {
  const result = error(() => {
    const jax = new TeX({ packages: ['base'], tags: 'base' });
    jax.setMmlFactory(new MmlFactory());
    const parser = new TexParser(tex, {}, jax.parseOptions);
    const tree = parser.mml();
    return { tree: mml(tree), parser: { i: parser.i, string: parser.string, macroCount: parser.macroCount },
      tags: { counter: jax.parseOptions.tags.counter, currentTag: plain(jax.parseOptions.tags.currentTag),
        labels: plain(jax.parseOptions.tags.labels), ids: plain(jax.parseOptions.tags.ids) } };
  });
  rows.push({ name, tex, display, ...result });
}

function methodMeta(value) {
  return typeof value === 'function' ? { name: value.name || '', length: value.length } : plain(value);
}
function tokenMap(name) {
  const map = MapHandler.getMap(name);
  if (!map) return null;
  const entries = map.map instanceof Map ? [...map.map.entries()].map(([key, value]) => [key, plain(value)]) : null;
  const regex = map._regExp ? { source: map._regExp.source, flags: map._regExp.flags } : null;
  return { class: map.constructor.name, name: map.name, parser: methodMeta(map.parser), regex, entries };
}

// Representative branches from the base character, command, environment and
// error handlers. This is a differential corpus proposal, not exhaustive coverage.
for (const [name, tex] of [
  ['plain-identifiers', 'x+y=12'],
  ['greek-symbols', '\\alpha+\\Gamma'],
  ['fraction-root', '\\frac{a+b}{\\sqrt{x}}'],
  ['subscript-superscript-prime', "x_i^2\\prime"],
  ['large-op-limits', '\\sum_{i=1}^n i'],
  ['left-right-fences', '\\left(\\frac{a}{b}\\right)'],
  ['text-and-font', '\\mathbf{A}+\\text{hello}'],
  ['named-operator', '\\sin x+\\lim_{n\\to\\infty}'],
  ['spacing-and-kern', 'a\\quad b\\,c\\!d'],
  ['accent-and-over-under', '\\widehat{x}+\\underbrace{a+b}_{n}'],
  ['array-environment', '\\begin{array}{cc}a&b\\\\c&d\\end{array}'],
  ['array-rules-and-align', '\\begin{array}{|r|c|}x&y\\\\1&2\\end{array}'],
  ['mathchoice', '\\mathchoice{D}{T}{S}{SS}'],
  ['primitive-fraction-and-choose', 'a\\over b+{n\\choose k}'],
  ['negated-operator-and-middle', '\\left(a\\not\\in b\\middle|c\\right)'],
  ['rule-and-box', '\\rule{1em}{2pt}+\\hbox{abc}'],
  ['font-and-size', '{\\mathsf{A}+\\mathtt{b}}+{\\Large x}'],
  ['big-delimiters', '\\bigl(a\\bigr)+\\Bigl[b\\Bigr]'],
  ['over-under-accents', '\\overline{x}+\\underbrace{a+b}_{n}+\\overset{!}{=}'],
  ['style-and-phantom', '\\displaystyle{x+\\phantom{y}}+\\smash[b]{z}'],
  ['lap-raise-and-space', '\\llap{a}b+\\raise1ex{x}+\\hspace{1em}y'],
  ['break-controls', 'a\\allowbreak+b\\nobreak c'],
  ['spacing-tilde', 'a~b\\enspace c\\mkern2mu d'],
  ['dots-and-primes', 'x\\dots+y\\prime+z\u2032'],
  ['root-movement', '\\sqrt[\\leftroot{2}\\uproot{1}3]{x}'],
  ['named-functions-and-operators', '\\arctan x+\\det A+\\gcd(a,b)'],
  ['mml-token-and-break-align', '\\mmlToken{mi}{abc}+\\breakAlign{tc}'],
  ['references-and-labels', 'x\\label{eq:a}+y\\ref{eq:a}'],
  ['undefined-control-sequence', '\\notARealCommand'],
  ['missing-environment-end', '\\begin{array}{cc}x'],
]) record(name, tex);

const jax = new TeX({ packages: ['base'] });
const baseConfig = BaseConfiguration;
const moduleSurface = {
  baseMethods: Object.fromEntries(Object.keys(BaseMethods).map(key => [key, methodMeta(BaseMethods[key])])),
  baseConfiguration: {
    name: baseConfig.name,
    keys: Reflect.ownKeys(baseConfig).map(String),
    handlers: plain(baseConfig.handlers), fallbacks: plain(baseConfig.fallbacks),
    items: Object.fromEntries(Object.entries(baseConfig.items || {}).map(([key, value]) => [key, methodMeta(value)])),
    options: Object.fromEntries(Object.entries(baseConfig.options || {}).map(([key, value]) => [key, plain(value)])),
    tags: Object.fromEntries(Object.entries(baseConfig.tags || {}).map(([key, value]) => [key, methodMeta(value)])),
    preprocessors: plain(baseConfig.preprocessors), postprocessors: plain(baseConfig.postprocessors),
  },
  mappings: ['letter','digit','command','special','lcGreek','ucGreek','mathchar0mi','mathchar0mo',
    'mathchar7','delimiter','macros','environment','remap','not_remap'].map(tokenMap),
  activeHandlerKeys: [...jax.parseOptions.handlers.keys()],
};
const output = { version: '4.1.3', files, moduleSurface, cases: rows };
const out = process.argv[2] || path.join(root, 'build/math-port/base-package-original.json');
fs.mkdirSync(path.dirname(out), { recursive: true });
fs.writeFileSync(out, JSON.stringify(output, null, 2) + '\n');
console.log(`wrote ${rows.length} upstream base-package observations to ${path.relative(root, out)}`);
