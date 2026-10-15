'use strict';
// Pins module-state scope for the actual CJS utility module. The locked
// package/map oracle remains untouched.
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const {createRequire} = require('node:module');
const root = path.resolve(__dirname, '../../..');
const cjsRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const load = name => require(path.join(cjsRoot, name));
load('input/tex/bussproofs/BussproofsConfiguration.js');
const utilPath = path.join(cjsRoot, 'input/tex/bussproofs/BussproofsUtil.js');
const source = fs.readFileSync(utilPath, 'utf8');
const requireFromUtil = createRequire(utilPath);
function freshUtilityModule() {
  const module = {exports: {}};
  const evaluate = vm.runInNewContext(`(function(exports, require, module) { ${source}\n})`, {console});
  evaluate(module.exports, requireFromUtil, module);
  return module.exports;
}
const sourceRealmA = require(utilPath);
const sameEnvironment = require(utilPath);
const sourceRealmB = freshUtilityModule();
const {TeX} = load('input/tex.js');
const {MmlFactory} = load('core/MmlTree/MmlFactory.js');
const {SVG} = load('output/svg.js');
const {liteAdaptor} = load('adaptors/liteAdaptor.js');
const {RegisterHTMLHandler} = load('handlers/html.js');
const {mathjax} = load('mathjax.js');
const {AbstractMathItem} = load('core/MathItem.js');
const {MathJaxNewcmFont} = require(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/mathjax-newcm-font/cjs/svg.js'));
const adaptor = liteAdaptor({fontSize: 16}); RegisterHTMLHandler(adaptor);
const input = new TeX({packages: ['base', 'bussproofs']});
const outputJax = new SVG({fontData: MathJaxNewcmFont, fontCache: 'none', exFactor: 0.5});
const documentA = mathjax.document('', {InputJax: input, OutputJax: outputJax});
const documentB = mathjax.document('', {InputJax: input, OutputJax: outputJax});
function parse(sourceText, document) {
  const jax = new TeX({packages: ['base', 'bussproofs']}); jax.setMmlFactory(new MmlFactory());
  const item = new AbstractMathItem(sourceText, jax, true); jax.compile(item, document);
  return {jax, item, document};
}
function balanceError(module, parsed) {
  try {
    module.balanceRules({math: parsed.item, document: parsed.document, data: parsed.jax.parseOptions});
    return {name: null, message: null};
  } catch (error) { return {name: error.name || error.constructor.name, message: String(error.message)}; }
}
const proof = String.raw`\begin{prooftree}\AxiomC{A}\UnaryInfC{B}\UnaryInfC{C}\end{prooftree}`;
const first = parse(proof, documentA), second = parse(proof, documentB);
sourceRealmA.saveDocument({document: documentA});
sourceRealmA.saveDocument({document: documentB});
sourceRealmA.clearDocument({document: documentA});
const sameEnvironmentAfterClear = balanceError(sameEnvironment, second);
sourceRealmA.saveDocument({document: documentA});
sourceRealmB.saveDocument({document: documentB});
sourceRealmA.clearDocument({document: documentA});
const independentEnvironmentAfterClear = balanceError(sourceRealmB, second);
const result = {
  source: 'MathJax-src 4.1.3 BussproofsUtil.js',
  sameEnvironmentUsesSameExports: sourceRealmA === sameEnvironment,
  independentModuleHasDistinctExports: sourceRealmA !== sourceRealmB,
  sameEnvironmentAfterOtherJaxClear: sameEnvironmentAfterClear,
  independentEnvironmentAfterOtherModuleClear: independentEnvironmentAfterClear,
};
const destination = path.join(root, 'build/math-port/bussproofs-module-scope-original.json');
fs.writeFileSync(destination, `${JSON.stringify(result, null, 2)}\n`);
console.log(JSON.stringify({...result, output: path.relative(root, destination)}));




