// Independent method-focused MathJax 4.1.3 TeX-to-MML corpus.
// This loads pinned CJS only and never imports native code.
'use strict';
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const locks = {
  'ts/input/tex/base/BaseMethods.ts': '7336240c8f9c5037f9fdcd5a84ea079312854f1ef3f406aeb5a8b6a8ff765cda',
  'input/tex/base/BaseMethods.js': 'b8d5c74e01f85ae391ec8d16ffcd216a4ca4267bd8ffa091c5bfe1462ad7b491',
  'ts/input/tex/base/BaseConfiguration.ts': '578d2c6b03f2f3e902476a0b73a5323b12262215fcb d c831a1135c1d9c1737ad'.replaceAll(' ', ''),
  'input/tex/base/BaseConfiguration.js': '0feedc60ac600b647cf2a1437c7244196a5aec04715f4a301fe6e9ec09722c57',
};
const files = [];
for (const [file, expected] of Object.entries(locks)) {
  const isSource = file.startsWith('ts/');
  const base = isSource ? sourceRoot : path.join(packageRoot, 'cjs');
  const hash = crypto.createHash('sha256').update(fs.readFileSync(path.join(base, file))).digest('hex');
  if (hash !== expected) throw new Error(`pinned input changed: ${file} (${hash})`);
  files.push({ file, sha256: hash });
}
const load = name => require(path.join(packageRoot, 'cjs', name));
const TexParser = load('input/tex/TexParser.js').default;
const { TeX } = load('input/tex.js');
const { MmlFactory } = load('core/MmlTree/MmlFactory.js');
const BaseMethods = load('input/tex/base/BaseMethods.js').default;
// These modules register their real configurations in the shared Source handler.
// The per-case `packages` list below still controls what each TeX instance loads.
load('input/tex/ams/AmsConfiguration.js');
load('input/tex/configmacros/ConfigMacrosConfiguration.js');
load('input/tex/newcommand/NewcommandConfiguration.js');
load('input/tex/boldsymbol/BoldsymbolConfiguration.js');
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
function observe(name, tex, display = false, packages = ['base']) {
  try {
    const jax = new TeX({ packages, tags: 'base' });
    jax.setMmlFactory(new MmlFactory());
    const parser = new TexParser(tex, {}, jax.parseOptions);
    return { name, tex, display, packages, result: { tree: mml(parser.mml()), parser: { i: parser.i, string: parser.string,
      macroCount: parser.macroCount }, methodNames: Object.keys(BaseMethods) } };
  } catch (e) {
    return { name, tex, display, packages, error: { name: e.name || e.constructor.name, id: e.id ?? null, message: String(e.message) } };
  }
}
const cases = [
  ['bar-tilde-vec', String.raw`\bar{x}+\tilde{y}+\vec{z}+\overline{w}`],
  ['font-variants', String.raw`\mathbb{R}+\mathbf{x}+\mathsf{A}+\mathtt{b}`],
  ['style-switches', String.raw`\displaystyle{x+\scriptstyle y}+\textstyle z`],
  ['spacing-family', String.raw`\hspace{1em}x\kern2mu y\quad z\enspace w`],
  ['break-controls', String.raw`a\allowbreak b\nobreak c\\d`],
  ['fraction-variants', String.raw`a\atop b+\binom{n}{k}+\genfrac{[}{]}{0pt}{}{n}{k}`],
  ['root-variants', String.raw`\sqrt[3]{x}+\sqrt[\uproot{2}\leftroot{1}4]{x}`],
  ['over-under-family', String.raw`\stackrel{!}{=}+\overset{a}{b}+\underset{c}{d}`],
  ['operator-classes', String.raw`\mathbin{+}+\mathrel{=}+\mathop{f}`],
  ['boxes', String.raw`\boxed{x}+\fbox{y}+\framebox{z}+\makebox[2em][c]{q}`],
  ['smash-phantom-lap', String.raw`\strut x+\phantom{y}+\smash[t]{z}+\llap{w}`],
  ['big-delimiter-sizes', String.raw`\bigl(a\bigr)+\Bigl[b\Bigr]+\biggl\{c\biggr\}`],
  ['not-and-dots', String.raw`x\not\in y+\dots+\ldots+\cdots`],
  ['array-matrix', String.raw`\begin{matrix}a&b\\c&d\end{matrix}+\begin{array}{|r|c|}x&y\\1&2\end{array}`],
  ['array-row-controls', String.raw`\begin{array}{c}\hline a\\\hfill b\end{array}`],
  ['aligned-break-align', String.raw`\begin{aligned}a&=b\breakAlign{tc}\\c&=d\end{aligned}`],
  ['new-column-type', String.raw`\newcolumntype{C}{c}\begin{array}{C}x\end{array}`],
  ['mml-token', String.raw`\mmlToken{mi}{abc}+\mmlToken{mo}[mathvariant=normal]{+}`],
  ['horizontal-size', String.raw`\hsize=3em x+\vbox{a\\b}`],
  ['paragraph-box', String.raw`\parbox{2em}{ab cd}`],
  ['macro-definition', String.raw`\newcommand{\foo}[1]{#1+#1}\foo{x}`],
  ['labels-and-reference', String.raw`\label{eq:a}x+\ref{eq:a}+\eqref{eq:a}`],
  ['equation-environment', String.raw`\begin{equation}x=1\label{eq:x}\end{equation}`],
  ['cases-and-array-close', String.raw`\begin{cases}x&x>0\\-x&x<0\end{cases}`],
  ['mathchoice', String.raw`\mathchoice{D}{T}{S}{SS}`],
  ['primitive-over-with-delimiters', String.raw`{a\overwithdelims()b}+{a\above2pt b}`],
  ['font-command-set', String.raw`\mathrm{d}x+\mathcal{F}+\mathfrak{g}+\boldsymbol{\alpha}`],
  ['move-and-raise', String.raw`\moveleft2mu{x}+\moveright3mu{y}+\raise1ex{z}+\lower1ex{q}`],
  ['ref-with-missing-label', String.raw`\ref{missing}`],
  ['malformed-array', String.raw`\begin{array}{cc}x`],
  ['undefined-command', String.raw`\notARealCommand`],
  ['ams-matrix', String.raw`\begin{matrix}a&b\\c&d\end{matrix}`, ['base', 'ams']],
  ['ams-aligned-break-align', String.raw`\begin{aligned}a&=b\\c&=d\end{aligned}`, ['base', 'ams']],
  ['ams-cases', String.raw`\begin{cases}x&x>0\\-x&x<0\end{cases}`, ['base', 'ams']],
  ['ams-binom-and-boldsymbol', String.raw`\binom{n}{k}+\boldsymbol{\alpha}`, ['base', 'ams', 'boldsymbol']],
  ['ams-eqref', String.raw`\eqref{missing}`, ['base', 'ams']],
  ['configmacros-newcommand', String.raw`\newcommand{\foo}[1]{#1+#1}\foo{x}`, ['base', 'newcommand', 'configmacros']],
];
const output = { version: '4.1.3', files, methodNames: Object.keys(BaseMethods), cases: cases.map(([name, tex, packages]) =>
  observe(name, tex, false, packages || ['base'])) };
const out = process.argv[2] || path.join(root, 'build/math-port/base-methods-original.json');
fs.mkdirSync(path.dirname(out), { recursive: true });
fs.writeFileSync(out, JSON.stringify(output, null, 2) + '\n');
console.log(`wrote ${output.cases.length} method-focused upstream observations to ${path.relative(root, out)}`);
