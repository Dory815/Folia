'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../../..');
const tsPath = path.join(root, '.reference/MathJax-src/ts/input/tex/Tags.ts');
const cjsPath = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/Tags.js');
const hashes = {
  ts: crypto.createHash('sha256').update(fs.readFileSync(tsPath)).digest('hex'),
  cjs: crypto.createHash('sha256').update(fs.readFileSync(cjsPath)).digest('hex'),
};
assert.equal(hashes.ts, '5a73a8d16f1d3c87ece167fc06dac0c323ea05a519e5ee5ea9aadbb76cef9a5d');
assert.equal(hashes.cjs, 'e027c7edb6f7207f3b3ce7e7203dcfc763219b1fa37a3b39925ae83c1beef896');
const source = require(cjsPath);
const methodRecord = (prototype) => Object.fromEntries(Object.getOwnPropertyNames(prototype)
  .filter((key) => key !== 'constructor' && typeof Object.getOwnPropertyDescriptor(prototype, key).value === 'function')
  .map((key) => {
    const descriptor = Object.getOwnPropertyDescriptor(prototype, key);
    return [key, { name: descriptor.value.name, length: descriptor.value.length,
      writable: descriptor.writable, enumerable: descriptor.enumerable, configurable: descriptor.configurable }];
  }));
const expected = { start:3,end:0,tag:2,notag:0,formatUrl:2,formatTag:1,formatRef:1,formatId:1,
  formatNumber:1,autoTag:0,clearTag:0,getTag:1,makeTag:0,makeId:0,resetTag:0,reset:1,
  startEquation:1,finishEquation:1,finalize:2 };
const base = methodRecord(source.AbstractTags.prototype);
for (const [name, length] of Object.entries(expected)) {
  assert.equal(base[name]?.length, length, `AbstractTags.${name} length`);
  assert.equal(base[name]?.name, '', `AbstractTags.${name} name`);
  assert.deepEqual({writable:base[name].writable,enumerable:base[name].enumerable,configurable:base[name].configurable},
    {writable:true,enumerable:true,configurable:true}, `AbstractTags.${name} descriptor`);
}
const accessorRecord = (prototype, name) => {
  const d = Object.getOwnPropertyDescriptor(prototype, name);
  return { enumerable:d.enumerable, configurable:d.configurable,
    get:d.get && {name:d.get.name,length:d.get.length}, set:d.set && {name:d.set.name,length:d.set.length} };
};
const accessors = {
  env:accessorRecord(source.AbstractTags.prototype,'env'),
  noTag:accessorRecord(source.AbstractTags.prototype,'noTag'),
  label:accessorRecord(source.AbstractTags.prototype,'label'),
};
assert.deepEqual(accessors,{env:{enumerable:false,configurable:true,get:{name:'get',length:0},set:undefined},
  noTag:{enumerable:false,configurable:true,get:{name:'get',length:0},set:undefined},
  label:{enumerable:false,configurable:true,get:{name:'get',length:0},set:{name:'set',length:1}}});
const prototypeConstructors = Object.fromEntries(['AbstractTags','NoTags','AllTags'].map((name) => {
  const ctor = source[name]; const d = Object.getOwnPropertyDescriptor(ctor.prototype,'constructor');
  return [name,{name:ctor.name,length:ctor.length,constructorName:d.value.name,
    writable:d.writable,enumerable:d.enumerable,configurable:d.configurable}];
}));
assert.equal(prototypeConstructors.AbstractTags.enumerable,false);
assert.equal(prototypeConstructors.NoTags.enumerable,true);
assert.equal(prototypeConstructors.AllTags.enumerable,true);
const none = new source.NoTags();
const enTag = Object.getOwnPropertyDescriptor(none,'enTag');
const result = { hashes, methods:base, accessors, prototypeConstructors,
  NoTags:{autoTag:source.NoTags.prototype.autoTag.length,getTag:source.NoTags.prototype.getTag.length},
  AllTags:{finalize:source.AllTags.prototype.finalize.length},
  enTag:{name:enTag.value.name,length:enTag.value.length,writable:enTag.writable,
    enumerable:enTag.enumerable,configurable:enTag.configurable} };
assert.deepEqual(result.NoTags,{autoTag:0,getTag:0});
assert.deepEqual(result.AllTags,{finalize:2});
assert.deepEqual({name:result.enTag.name,length:result.enTag.length,writable:result.enTag.writable,
  enumerable:result.enTag.enumerable,configurable:result.enTag.configurable},
  {name:'',length:2,writable:true,enumerable:true,configurable:true});
process.stdout.write(JSON.stringify(result,null,2)+'\n');
