// Deliberately corrupt one recorded representation at a time. Keeping the raw
// serialization unchanged ensures the structured comparator itself catches it.
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const {compare,read}=require('./reference-compare.cjs');
const root=path.resolve(__dirname,'../..');
const dir=path.join(root,'build/math-port-reference');
const corpus=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/reference-corpus.json')));
const snapshots=new Map(corpus.cases.map(c=>[c.name,read(path.join(dir,c.name+'.json.gz'))]));
for (const [name,snapshot] of snapshots) assert.deepEqual(compare(snapshot,snapshot),[],name+' baseline');
function domFind(node,predicate) {
  if (predicate(node)) return node;
  for (const child of node.children||[]) {const found=domFind(child,predicate); if (found) return found;}
  return null;
}
const mutations=[];
function mutation(name,caseName,change,expectedPath) {
  const expected=snapshots.get(caseName); assert.ok(expected,caseName);
  const actual=structuredClone(expected); change(actual);
  const differences=compare(expected,actual);
  assert.ok(differences.length,name+' was not detected');
  assert.ok(differences.some(d=>expectedPath.test(d.path)),name+' detected the wrong representation: '+JSON.stringify(differences));
  mutations.push({name,case:caseName,detected_at:differences[0].path});
}
mutation('missing_second_fragment','regression_inline_fragments',s=>s.result.fragments.splice(1,1),/\.result\.fragments$/);
mutation('wrong_variant','regression_inline_fragments',s=>{
  const w=s.layout_runs.flatMap(r=>r.wrappers).find(w=>typeof w.state.variant==='string');assert.ok(w);w.state.variant='incorrect-variant';
},/\.state\.variant$/);
mutation('shifted_glyph','regression_inline_fragments',s=>{
  const p=s.placements.find(p=>p.method==='placeChar'&&typeof p.x==='number');assert.ok(p);p.x+=0.001;
},/\.placements\[\d+\]\.x$/);
mutation('wrong_rule_thickness','regression_fraction_rule',s=>{
  const rect=domFind(s.result.fragments[0].tree,n=>n.kind==='rect');assert.ok(rect);rect.attributes.height=String(Number(rect.attributes.height)+10);
},/\.result\.fragments\[0\]\.tree.*\.attributes\.height$/);
mutation('wrong_nested_viewport_clip','regression_vertical_clip',s=>{
  const svg=domFind(s.result.fragments[0].tree,n=>n.kind==='svg'&&n!==s.result.fragments[0].tree&&Object.hasOwn(n.attributes,'viewBox'));assert.ok(svg);svg.attributes.viewBox='0 0 1 1';
},/\.result\.fragments\[0\]\.tree.*\.attributes\.viewBox$/);
mutation('wrong_macro_scope_result','regression_group_macro',s=>{
  // Change the post-group expansion only, leaving the scoped expansion and
  // entire SVG byte string intact; the semantic tree must independently fail.
  const texts=s.parsed_mml.filter(n=>n.kind==='text'&&typeof n.text==='string');
  assert.deepEqual(texts.map(n=>n.text),['b','+','a'],'Fixture must expose scoped b then restored a');
  texts.at(-1).text='b';
},/\.parsed_mml\[\d+\]\.text$/);
mutation('dropped_mml_attribute','regression_fraction_rule',s=>{
  const node=s.parsed_mml.find(n=>n.attributes?.explicit?.names.length);assert.ok(node);
  const key=node.attributes.explicit.names[0];delete node.attributes.explicit.values[key];node.attributes.explicit.names.shift();
},/\.parsed_mml.*\.attributes\.explicit/);
mutation('wrong_bbox','regression_fraction_rule',s=>{s.layout_runs[0].wrappers[0].bbox.w+=0.001;},/\.bbox\.w$/);
mutation('dropped_postfilter','regression_fraction_rule',s=>s.postfilters.pop(),/\.postfilters$/);
mutation('wrong_diagnostic','regression_double_script',s=>{s.diagnostics[0].id='IncorrectError';},/\.diagnostics\[0\]\.id$/);
// The tolerance applies to approved numerical geometry only.
const expected=snapshots.get('regression_fraction_rule'),near=structuredClone(expected);
near.layout_runs[0].wrappers[0].bbox.w+=1e-12;
assert.deepEqual(compare(expected,near),[],'documented geometry tolerance');
mutation('non_geometry_numbers_exact','regression_fraction_rule',s=>{s.postfilters[0].priority+=1e-12;},/\.priority$/);
console.log(JSON.stringify({baselines:snapshots.size,geometry_tolerance:true,discrete_numbers_exact:true,mutations},null,2));
