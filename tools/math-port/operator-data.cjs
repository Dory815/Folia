// Root-owned pristine source data generation; no source algorithm bodies.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),npm=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const source='ts/core/MmlTree/OperatorDictionary.ts',runtime='cjs/core/MmlTree/OperatorDictionary.js',sha=x=>crypto.createHash('sha256').update(x).digest('hex');
const lockfile=path.join(root,'docs/math-port/evidence/p11-source-lock.json');
const lock=JSON.parse(fs.readFileSync(lockfile,'utf8'));
assert.equal(process.version,lock.node);assert.equal(sha(fs.readFileSync(path.join(root,'.reference/MathJax-src',source))),lock.sha256);
assert.equal(fs.readFileSync(path.join(npm,source),'utf8').replaceAll('\r\n','\n'),fs.readFileSync(path.join(root,'.reference/MathJax-src',source),'utf8').replaceAll('\r\n','\n'));
const resolved=require.resolve('@mathjax/src/js/core/MmlTree/OperatorDictionary.js',{paths:[path.join(root,'tools/mathjax-bundle')]});assert.equal(fs.realpathSync(resolved),fs.realpathSync(path.join(npm,runtime)));assert.equal(sha(fs.readFileSync(resolved)),lock.runtime_sha256);
const D=require(resolved),nodes=[],ids=new Map();
function visit(value){if(value===null||typeof value!=='object')return;if(ids.has(value))return;assert.equal(Object.getPrototypeOf(value),Array.isArray(value)?Array.prototype:Object.prototype);ids.set(value,nodes.length);nodes.push(value);for(const key of Object.keys(value))visit(value[key]);}
for(const name of ['MO','RANGES','MMLSPACING','OPTABLE'])visit(D[name]);
// Escaped UTF-8 bytes avoid UCN restrictions and adjacent hexadecimal greed.
const bytes=s=>Array.from(Buffer.from(s,'utf8'),c=>'\\'+c.toString(8).padStart(3,'0')).join('');
const str=s=>'std::string("'+bytes(s)+'",'+Buffer.byteLength(s,'utf8')+')';
function property(v){if(v===undefined)return 'Missing{}';if(v===null)return 'PropertyNull{}';if(typeof v==='object')return 'nodes['+ids.get(v)+']';if(typeof v==='string')return str(v);if(typeof v==='number'){assert.ok(Number.isFinite(v));return String(v)+'.0';}if(typeof v==='boolean')return String(v);throw Error('Unbound data');}
const out=['// Generated from pristine pinned OperatorDictionary by operator-data.cjs.', '// MathJax 4.1.3 source: Apache-2.0; see LICENSE-MathJax.txt.', 'module folia.math.core.mml_tree.operator_dictionary;','import std;','namespace folia::math::mml::operator_dict {namespace {','struct State {PropertyList mo,optable;PropertyArray ranges,spacing;State(){','std::vector<Property> nodes('+nodes.length+');'];
for(let i=0;i<nodes.length;i++){const v=nodes[i];out.push(Array.isArray(v)?'{PropertyArray a;a.resize('+v.length+');nodes['+i+']=a;}':'nodes['+i+']=PropertyList{};');}
for(let i=0;i<nodes.length;i++){out.push('{auto record=PropertyList::record(nodes['+i+']).value();');for(const key of Object.keys(nodes[i]))out.push('record.setOwn('+str(key)+','+property(nodes[i][key])+');');out.push('}');}
for(const [name,field,array]of [['MO','mo',false],['RANGES','ranges',true],['MMLSPACING','spacing',true],['OPTABLE','optable',false]])out.push(field+'='+ (array?'PropertyArray::array':'PropertyList::object')+'(nodes['+ids.get(D[name])+']).value();');
out.push('}};State& state(){static State value;return value;}','}','PropertyList& MO(){return state().mo;}','PropertyArray& RANGES(){return state().ranges;}','PropertyArray& MMLSPACING(){return state().spacing;}','PropertyList& OPTABLE(){return state().optable;}','}');
const output=out.join('\n')+'\n',target=path.join(root,'src/math/core/mml_tree/operator_dictionary_data.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
module.exports={D,nodes,ids,lock,str};
if(require.main===module)console.log(JSON.stringify({graphNodes:nodes.length,tableEntries:Object.values(D.OPTABLE).reduce((n,t)=>n+Object.keys(t).length,0),mode:process.argv.includes('--check')?'check':'generate'}));
