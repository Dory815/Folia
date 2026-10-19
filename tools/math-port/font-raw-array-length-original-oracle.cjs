#!/usr/bin/env node
'use strict';

// Source-only witness for raw FontData sizeVariants/stretchVariants field
// values. The dynamic record stores the actual result of `.length`; FontData
// does not coerce it to a number while constructing the extension record.
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
const {FontData} = require(path.join(root,
  'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/output/common/FontData.js'));

function describe(value) {
  if (value === undefined) return {$:'undefined'};
  if (value === null) return null;
  if (typeof value === 'number' && Number.isNaN(value)) return {$:'NaN'};
  if (typeof value === 'number' && Object.is(value, -0)) return {$:'-0'};
  if (typeof value === 'object') return {$:Array.isArray(value)?'array':'object'};
  return value;
}

const cases = {};
function run(name, setFields) {
  class ProbeFont extends FontData {}
  ProbeFont.dynamicExtensions = new Map();
  const font = new ProbeFont();
  const reads = [];
  setFields(font, reads);
  const before = [...ProbeFont.dynamicExtensions.keys()];
  let result;
  try {
    result = font.addExtension({name:`probe-${name}`});
  } catch (error) {
    cases[name] = {
      reads,
      before,
      after:[...ProbeFont.dynamicExtensions.keys()],
      error:{name:error.name,message:error.message},
    };
    return;
  }
  const record = ProbeFont.dynamicExtensions.get(`probe-${name}`);
  cases[name] = {
    reads,
    before,
    after:[...ProbeFont.dynamicExtensions.keys()],
    sizeN:describe(record.sizeN),
    stretchN:describe(record.stretchN),
    result:describe(result),
  };
}

run('number_fields',font=>{font.sizeVariants=17;font.stretchVariants=false;});
run('string_fields',font=>{font.sizeVariants='abc';font.stretchVariants='';});
run('size_getter_string_length',(font,reads)=>{
  font.sizeVariants={get length(){reads.push('size.length');return '04';}};
  font.stretchVariants={get length(){reads.push('stretch.length');return 6;}};
});
run('size_getter_undefined_length',(font,reads)=>{
  font.sizeVariants={get length(){reads.push('size.length');return undefined;}};
  font.stretchVariants={get length(){reads.push('stretch.length');return undefined;}};
});
run('null_size_field',font=>{font.sizeVariants=null;font.stretchVariants=[];});
run('null_stretch_field',font=>{font.sizeVariants=[];font.stretchVariants=null;});

const output={source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e',cases};
const target=path.join(root,'build/math-port/font-raw-array-length-original.json');
fs.mkdirSync(path.dirname(target),{recursive:true});
fs.writeFileSync(target,`${JSON.stringify(output,null,2)}\n`);
process.stdout.write(`${path.relative(root,target)}\n`);
