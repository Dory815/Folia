'use strict';
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
const bundle = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const load = name => require(path.join(bundle, name));
load('input/tex/base/BaseConfiguration.js');
load('input/tex/boldsymbol/BoldsymbolConfiguration.js');
const { TeX } = load('input/tex.js');
const { AbstractMathItem } = load('core/MathItem.js');
const { MmlFactory } = load('core/MmlTree/MmlFactory.js');
const jax = new TeX({ packages: ['base', 'boldsymbol'], tags: 'base' });
jax.setMmlFactory(new MmlFactory());
const result = jax.compile(new AbstractMathItem(String.raw`\boldsymbol{\text{abc}+\mathbb{R}}`, jax, true), {});
function walk(node, out = []) {
  if (!node) return out;
  out.push({ kind: node.kind, text: node.getText?.() ?? null,
    mathvariant: node.attributes?.get('mathvariant'),
    fixBold: node.getProperty?.('fixBold') });
  for (const child of node.childNodes || []) walk(child, out);
  return out;
}
process.stdout.write(JSON.stringify(walk(result), null, 2) + '\n');
