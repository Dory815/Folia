'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

const root = path.resolve(__dirname, '../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const sourceRoot = path.join(root, '.reference/MathJax-src/ts/output');
assert.equal(JSON.parse(fs.readFileSync(path.join(packageRoot, 'src/package.json'))).version, '4.1.3');
const sourceHashes = {
  commonTs: '3686a8e803e26ae59d03c096ac2e6e5b1ab1332a5c4bcaa083cb9a02c2c61096',
  svgTs: '557acc55adc39e849864c61a92ac9e253bce5c13fbfd16603f143cf9c85ee9e5',
  commonCjs: 'b8a58d983e465f21ed36574472140bfd65e2696bb72f6c5647e0a2ae47eb1bd9',
  svgCjs: '1495e23a9da0bf6b0e8b44d71f54271fa12e902c11444e4f18cdd749ad3b800e',
};
const sha = filename => crypto.createHash('sha256').update(fs.readFileSync(filename)).digest('hex');
for (const [key, file] of Object.entries({
  commonTs: path.join(sourceRoot, 'common.ts'),
  svgTs: path.join(sourceRoot, 'svg.ts'),
  commonCjs: path.join(packageRoot, 'src/cjs/output/common.js'),
  svgCjs: path.join(packageRoot, 'src/cjs/output/svg.js'),
})) assert.equal(sha(file), sourceHashes[key], `${key} pinned source hash`);

async function main() {
  const cjsRoot = path.join(packageRoot, 'src/cjs');
  const { mathjax } = require(path.join(cjsRoot, 'mathjax.js'));
  const { TeX } = require(path.join(cjsRoot, 'input/tex.js'));
  const { SVG } = require(path.join(cjsRoot, 'output/svg.js'));
  const { CommonOutputJax } = require(path.join(cjsRoot, 'output/common.js'));
  const { liteAdaptor } = require(path.join(cjsRoot, 'adaptors/liteAdaptor.js'));
  const { RegisterHTMLHandler } = require(path.join(cjsRoot, 'handlers/html.js'));
  const adaptor = liteAdaptor({fontSize: 16});
  RegisterHTMLHandler(adaptor);
  const baseGetter = Object.getOwnPropertyDescriptor(
    CommonOutputJax.prototype, 'forceInlineBreaks').get;

  async function observe(inline, display) {
    const output = new SVG({fontCache: 'none', linebreaks: {inline}});
    const input = new TeX({packages: ['base']});
    const document = mathjax.document('', {InputJax: input, OutputJax: output});
    await document.convertPromise('a+b', {display, em: 16, ex: 8, containerWidth: 400});
    const rootNode = output.math.root;
    const first = rootNode.childNodes[0];
    let operator;
    rootNode.walkTree(node => { if (node.kind === 'mo' && !operator) operator = node; });
    const inlineMarked = !!rootNode.getProperty('inlineMarked');
    const inlineForced = rootNode.getProperty('inlineForced');
    const value = {
      inline,
      display,
      baseCommonForceGetter: baseGetter.call(output),
      svgForceGetter: output.forceInlineBreaks,
      inlineMarked,
      inlineForced: inlineForced === undefined ? null : inlineForced,
      breakable: !!operator?.getProperty('breakable'),
      forcebreak: !!operator?.getProperty('forcebreak'),
      firstKind: first?.kind || null,
    };
    return value;
  }

  async function observeTransitions() {
    const output = new SVG({fontCache: 'none', linebreaks: {inline: true}});
    const input = new TeX({packages: ['base']});
    const document = mathjax.document('', {InputJax: input, OutputJax: output});
    await document.convertPromise('a+b', {display: false, em: 16, ex: 8,
      containerWidth: 400});
    const read = () => {
      const rootNode = output.math.root;
      let operator;
      rootNode.walkTree(node => { if (node.kind === 'mo' && !operator) operator = node; });
      return {
        svgForceGetter: output.forceInlineBreaks,
        inlineMarked: !!rootNode.getProperty('inlineMarked'),
        inlineForced: rootNode.getProperty('inlineForced') ?? null,
        breakable: !!operator?.getProperty('breakable'),
        forcebreak: !!operator?.getProperty('forcebreak'),
      };
    };
    const states = [read()];
    output.math.display = true;
    output.toDOM(output.math, adaptor.node('span'), document);
    states.push(read());
    output.math.display = false;
    output.options.linebreaks.inline = false;
    output.toDOM(output.math, adaptor.node('span'), document);
    states.push(read());
    return states;
  }

  const cases = [
    await observe(true, false),
    await observe(false, false),
    await observe(true, true),
  ];
  assert.deepEqual(cases.map(c => [c.baseCommonForceGetter, c.svgForceGetter,
    c.inlineMarked, c.inlineForced, c.breakable, c.forcebreak]), [
    [false, true, true, true, true, true],
    [false, false, false, null, false, false],
    [false, true, false, null, false, false],
  ]);
  const transitions = await observeTransitions();
  assert.deepEqual(transitions.map(s => [s.svgForceGetter, s.inlineMarked,
    s.inlineForced, s.breakable, s.forcebreak]), [
    [true, true, true, true, true],
    [true, false, null, false, false],
    [false, false, null, false, false],
  ]);
  const expected = path.join(root, 'tests/math/fixtures/common-source/inline-policy.expected.json');
  assert.deepEqual({cases, transitions}, JSON.parse(fs.readFileSync(expected, 'utf8')));
  const evidence = path.join(root, 'build/math-port/common-source/inline-policy.json');
  fs.mkdirSync(path.dirname(evidence), {recursive: true});
  fs.writeFileSync(evidence, JSON.stringify({cases, transitions}, null, 2) + '\n');
  console.log('pinned Source SVG inline policy and transitions passed (3 cases + 2 transitions)');
}

main().catch(error => { console.error(error.stack); process.exitCode = 1; });
