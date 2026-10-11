// Complete source-generated string tables, preserving source names and order.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,ConfigurationType,HandlerType,TexConstant,hashes}=require('./tex-token-reference.cjs');
function literal(v){if(typeof v==='string')return JSON.stringify(v);assert.equal(typeof v,'object');return 'PropertyList{'+Object.entries(v).map(([k,x])=>'{'+JSON.stringify(k)+','+literal(x)+'}').join(',')+'}';}
const outputs=[['handler_types','HandlerTypes',[['ConfigurationType',ConfigurationType],['HandlerType',HandlerType]]],['tex_constants','TexConstants',[['TexConstant',TexConstant]]]];
for(const [module,name,values] of outputs){const sha=hashes.find(h=>h.source.endsWith('/'+name+'.ts')).sha256;const text='// Generated from complete pinned '+name+'.ts by tex-token-constants.cjs.\n// MathJax 4.1.3 Apache-2.0; source SHA256 '+sha+'\nexport module folia.math.input.tex.'+module+';\nexport import folia.math.core.tree.property;\nimport std;\nexport namespace folia::math::tex {\n'+values.map(([n,v])=>'inline PropertyList '+n+'='+literal(v)+';').join('\n')+'\n}\n';const target=path.join(root,'src/math/input/tex',module+'.ixx');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8'),text);else fs.writeFileSync(target,text);}
console.log(JSON.stringify({tables:3,entries:Object.keys(ConfigurationType).length+Object.keys(HandlerType).length+Object.values(TexConstant).reduce((n,v)=>n+Object.keys(v).length,0),mode:process.argv.includes('--check')?'check':'generate'}));
