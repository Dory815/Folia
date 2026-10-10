// Root-owned prerequisite probe. Real pinned TeX profile, no native implementation.
'use strict';
const assert=require('node:assert/strict');
const {createReference}=require('./reference-profile.cjs');
const reference=createReference();
let observed=[];
reference.input.postFilters.add(({data})=>data.root.walkTree(node=>{
  if(node.kind==='mo')observed.push({text:node.getText(),stored_type:typeof node._texClass,
    stored_null:node._texClass===null,getter_undefined:node.texClass===undefined});
}),100);
for(const name of ['toString','constructor']){
  observed=[];
  const tex='\\mmlToken{mo}{'+name+'}';
  const svg=globalThis.FoliaMathJax.render(tex,true,16,1280);
  assert.deepEqual(observed,[{text:name,stored_type:'undefined',stored_null:false,getter_undefined:true}]);
  assert.ok(svg.includes('data-mml-node="mo"'));
  console.log(JSON.stringify({tex,observed,svg_has_mo:true}));
}
assert.equal(/foo$/.test('foo\n'),false);
assert.equal(/foo$/.test('foo\r\n'),false);
console.log('Five fixed no-flags source patterns require strict end assertions; raw class Missing differs from null.');
