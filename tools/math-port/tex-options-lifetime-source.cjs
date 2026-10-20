'use strict';
// Actual scope38 TeX input and SVG pipeline; native option ownership contract.
const assert = require('node:assert/strict');
const {createReference} = require('./reference-profile.cjs');
const ref = createReference();
assert.equal(ref.provenance.compiled_source_sha256,
  'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const options = ref.input.parseOptions.options;
const errors = [];
const originalFormatError = ref.input.options.formatError;
ref.input.options.formatError = (jax,error) => {
  errors.push(error.id);
  return originalFormatError(jax,error);
};
function convert(tex) { ref.document.convert(tex,{em:16,ex:8,containerWidth:1280}); }
assert.equal(options.maxBuffer,5120);
convert(String.raw`\setOptions{maxBuffer=false}x`);
assert.deepEqual(errors,['TeXOptionNotSettable']);
assert.equal(options.maxBuffer,5120);
convert(String.raw`\setOptions[physics]{italicdiff=true}x`);
assert.equal(options.physics.italicdiff,true);
ref.input.reset();
assert.equal(ref.input.parseOptions.options,options);
assert.equal(options.physics.italicdiff,true);
const style = ref.input.parseOptions.mathStyle;
assert.equal(style('A',false),'');
convert(String.raw`\setOptions{mathStyle=ISO,tagSide=left}x`);
assert.equal(options.mathStyle,'ISO');
assert.equal(options.tagSide,'left');
assert.equal(ref.input.parseOptions.mathStyle,style);
assert.equal(ref.input.parseOptions.mathStyle('A',false),'');
ref.input.reset();
assert.equal(options.tagSide,'left');
convert(String.raw`\setOptions[physics]{italicdiff=false}x`);
assert.equal(options.physics.italicdiff,false);
assert.deepEqual(errors,['TeXOptionNotSettable']);
console.log(JSON.stringify({status:'passed',observations:5,
  maxBuffer:'false in allowOptions is permission denial, not an allowed option value',
  effectiveOptions:'persist across compile and reset; current values are not initial defaults',
  mathStyle:'resolver is captured at ParseOptions construction; setting its option does not replace the resolver',
  source:ref.provenance.upstream,compiledSourceHash:ref.provenance.compiled_source_sha256,
  limit:'Pinned Source lifecycle behavior only; native TeX/configuration integration remains incomplete.'}));
