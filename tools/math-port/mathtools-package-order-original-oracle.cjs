'use strict';

// Focused Source observation for the Folia package-order override.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const cjsRoot =
    path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const hash = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const load = name => require(path.join(cjsRoot, 'cjs', name));
const {TeX} = load('input/tex.js');
const {MmlFactory} = load('core/MmlTree/MmlFactory.js');
const {AbstractMathItem} = load('core/MathItem.js');
const {MapHandler} = load('input/tex/MapHandler.js');
load('input/tex/ams/AmsConfiguration.js');
load('input/tex/extpfeil/ExtpfeilConfiguration.js');
load('input/tex/mathtools/MathtoolsConfiguration.js');

function plain(value, seen = new WeakSet()) {
  if (value === undefined)
    return {$ : 'undefined'};
  if (value === null || typeof value === 'string' || typeof value === 'boolean')
    return value;
  if (typeof value === 'number')
    return Number.isNaN(value)    ? {$ : 'NaN'}
           : Object.is(value, -0) ? {$ : '-0'}
           : Number.isFinite(value)
               ? value
               : {$ : value < 0 ? '-Infinity' : 'Infinity'};
  if (typeof value === 'function')
    return {$ : 'function', name : value.name || '', length : value.length};
  if (typeof value !== 'object')
    return String(value);
  if (seen.has(value))
    return {$ : 'cycle'};
  seen.add(value);
  if (Array.isArray(value))
    return Array.from({length : value.length},
                      (_, i) => Object.hasOwn(value, i) ? plain(value[i], seen)
                                                        : {$ : 'hole'});
  if (value instanceof Map)
    return {
      $ : 'map',
      entries : [...value ].map(([ k, v ]) => [plain(k, seen), plain(v, seen)])
    };
  const result = {};
  for (const key of Reflect.ownKeys(value))
    result[typeof key === 'symbol' ? `symbol:${key.description || ''}` : key] =
        plain(value[key], seen);
  return result;
}
function tree(node) {
  return node == null ? null : {
    kind : node.kind,
    attrs : plain(node.attributes?.getAllAttributes?.() ?? {}),
    properties : plain(node.getAllProperties?.() ?? {}),
    ...(typeof node.getText === 'function' ? {text : node.getText()} : {}),
    children : (node.childNodes || []).map(tree)
  };
}
const packages = [ 'base', 'ams', 'extpfeil', 'mathtools' ];
const inputs = [ String.raw`\xmapsto{f}`, String.raw`\xmapsto[below]{above}` ];
const cases = inputs.map((input, index) => {
  let jax = null, result = null, error = null;
  try {
    jax = new TeX({packages});
    jax.setMmlFactory(new MmlFactory());
    result = tree(jax.compile(new AbstractMathItem(input, jax, true), {}));
  } catch (caught) {
    error = {
      name : caught.name || caught.constructor.name,
      id : caught.id ?? null,
      message : String(caught.message)
    };
  }
  return {
    name : `folia-xmapsto-${index}`,
    input,
    options : {},
    result,
    error,
    packageData : jax ? plain(jax.parseOptions.packageData) : null,
    activeOptions : jax ? plain(jax.parseOptions.options.mathtools) : null,
    activeTags : jax ? plain(jax.parseOptions.options.tags) : null
  };
});
const map = MapHandler.getMap('mathtools-macros');
const entry = map?.map?.get('xmapsto');
const output = {
  schema : 1,
  source : 'MathJax 4.1.3 fb987178c2d279a99b0db5ea02e751691703955e',
  packages,
  override : plain(entry),
  cases
};
const destination =
    path.join(root, 'build/math-port/mathtools-package-order-original.json');
fs.mkdirSync(path.dirname(destination), {recursive : true});
const bytes = Buffer.from(`${JSON.stringify(output, null, 2)}\n`);
fs.writeFileSync(destination, bytes);
process.stdout.write(`${JSON.stringify({
  cases : cases.length,
  packages,
  oracleSha256 : hash(bytes),
  output : path.relative(root, destination)
})}\n`);
