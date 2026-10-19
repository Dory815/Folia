'use strict';
// Pinned MathJax RenderList observations. This exercises the real CJS class;
// it does not replace RenderList with a model used only by this oracle.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const digest = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const locks = {
  'cjs/core/MathDocument.js': '3a7dad24d15a1137fa0bd8394a42284c690830c751102b6a31785a0f77eee1db',
  'cjs/util/PrioritizedList.js': '787bd21f3634312af666917d8f140145864f98d09ede1bdde3c957d28ec2bc25'
};
for (const [file, hash] of Object.entries(locks)) {
  const actual = digest(path.join(packageRoot, file));
  if (actual !== hash) throw new Error(`${file} hash drift: ${actual}`);
}
const { RenderList } = require(path.join(packageRoot, 'cjs/core/MathDocument.js'));

function output() {
  const cases = [];
  {
    const trace = [];
    const doc = Object.fromEntries(['low', 'same', 'high'].map(name => [name, function () {
      trace.push(`${name}:${this === doc}`);
      return false;
    }]));
    const list = RenderList.create({ high: [8, 'high'], low: [2, 'low'], same: [2, 'same'], skip: [0, 'skip'] });
    list.renderDoc(doc);
    cases.push({ name: 'stable-priority-and-zero', trace, ids: ['low', 'same', 'high', 'skip'].map(id => [id, list.findID(id) !== null]) });
  }
  {
    const trace = [];
    const math = Object.fromEntries(['two', 'five', 'eight'].map(name => [name, function (doc) {
      trace.push(`${name}:${this === math}:${doc === document}`);
      return false;
    }]));
    const document = {};
    const list = RenderList.create({ two: [2, 'two', true], five: [5, 'five', true], eight: [8, 'eight', false] });
    list.renderMath(math, document, 5);
    const afterStart = trace.slice();
    trace.length = 0;
    list.renderConvert(math, document, 5);
    cases.push({ name: 'start-and-convert-end', afterStart, conversion: trace });
  }
  {
    const trace = [];
    const list = RenderList.create({
      first: [1, function () { trace.push('stop'); return true; }, () => false],
      second: [2, function () { trace.push('later'); return false; }, () => false]
    });
    const doc = {};
    list.renderDoc(doc, 0);
    cases.push({ name: 'early-stop', trace });
  }
  {
    const trace = [];
    const actions = {};
    actions.direct = [3,
      function (doc) { trace.push(`doc:${this === data}:${doc === document}`); return false; },
      function (math, doc) { trace.push(`math:${this === data}:${math === item}:${doc === document}`); return false; }
    ];
    const list = RenderList.create(actions);
    const data = list.findID('direct');
    const item = {};
    const document = {};
    list.renderDoc(document);
    list.renderMath(item, document);
    cases.push({ name: 'direct-function-receiver', trace });
  }
  {
    const trace = [];
    const list = RenderList.create({ first: [1, 'first'], second: [2, 'second'] });
    const doc = {
      first() { trace.push('first'); list.add({ id: 'late', renderDoc: () => { trace.push('late'); return false; }, renderMath: () => false, convert: true }, 3); return false; },
      second() { trace.push('second'); return false; }
    };
    list.renderDoc(doc);
    cases.push({ name: 'mutation-during-iteration', trace });
  }
  {
    const list = RenderList.create({ fail: [1, 'fail'] });
    let error = 'none';
    try { list.renderDoc({ fail() { throw new TypeError('render-fault'); } }); }
    catch (e) { error = `${e.name}:${e.message}`; }
    cases.push({ name: 'callback-error', error });
  }
  return { schema: 1, cases };
}

const result = output();
const destination = process.argv[2];
if (destination) fs.writeFileSync(destination, `${JSON.stringify(result, null, 2)}\n`);
else process.stdout.write(`${JSON.stringify(result, null, 2)}\n`);
