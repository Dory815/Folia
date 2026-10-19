'use strict';
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
const modules = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const BaseMethods = require(path.join(modules, 'input/tex/base/BaseMethods.js')).default;
const { TeX } = require(path.join(modules, 'input/tex.js'));
const { AbstractMathItem } = require(path.join(modules, 'core/MathItem.js'));
const { MmlFactory } = require(path.join(modules, 'core/MmlTree/MmlFactory.js'));

function run(name, args) {
  const mathItem = {};
  const parser = {
    configuration: { mathItem },
    stack: {
      env: {},
      Top() { return { First: false, isKind(kind) { return kind === 'start'; }, getProperty() { return undefined; } }; }
    },
    Push(item) { this.pushed = item; },
    itemFactory: {
      create(kind, numbered) {
        const item = { kind, numbered, properties: {} };
        item.setProperty = function(key, value) { this.properties[key] = value; return this; };
        return item;
      }
    }
  };
  const begin = { getName() { return 'equation'; } };
  const result = BaseMethods.Equation(parser, begin, true, ...args);
  return { name, display: mathItem.display, environmentDisplay: parser.stack.env.display,
    result: { kind: result.kind, numbered: result.numbered, properties: result.properties } };
}

const rows = [
  run('omitted', []),
  run('undefined', [undefined]),
  run('false', [false]),
  run('null', [null])
];
const jax = new TeX({ packages: ['base'], tags: 'base' });
jax.setMmlFactory(new MmlFactory());
const rootNode = jax.compile(new AbstractMathItem(String.raw`\begin{equation}x=1\end{equation}`, jax, false), {});
rows.push({ name: 'actual-tex-compile-equation-default', display: rootNode.attributes.get('display'),
  treeKind: rootNode.kind, equationDefault: rootNode.attributes.get('display') === 'block' });
process.stdout.write(JSON.stringify(rows, null, 2) + '\n');
