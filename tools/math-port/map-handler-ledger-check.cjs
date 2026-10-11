// Root reviewed complete Source inventory, including object-literal methods.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),file=path.join(root,'docs/math-port/evidence/p20a-map-handler-members.json');
const source='ts/input/tex/MapHandler.ts',fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'member-ledger.cjs'),source],{encoding:'utf8'}));
const ts=require(path.join(root,'build/math-port-tooling/node_modules/typescript')),bytes=fs.readFileSync(path.join(root,'.reference/MathJax-src',source));
const ast=ts.createSourceFile(source,bytes.toString('utf8'),ts.ScriptTarget.Latest,true,ts.ScriptKind.TS);
const variable=ast.statements.flatMap(s=>ts.isVariableStatement(s)?Array.from(s.declarationList.declarations):[]).find(d=>d.name.getText(ast)==='MapHandler');
assert.ok(variable&&ts.isObjectLiteralExpression(variable.initializer));
fresh.files[0].object_methods=variable.initializer.properties.map(m=>{assert.ok(ts.isMethodDeclaration(m)&&m.body);return {owner:'MapHandler',name:m.name.getText(ast),line:ast.getLineAndCharacterOfPosition(m.getStart(ast)).line+1,end_line:ast.getLineAndCharacterOfPosition(m.end).line+1,member_sha256:crypto.createHash('sha256').update(m.getText(ast)).digest('hex')};});
const cpp='src/math/input/tex/map_handler.cpp:',ixx='src/math/input/tex/map_handler.ixx:';
function target(m){
 if(m.owner==='MapHandler')return cpp+'MapHandler::'+(m.name==='register'?'registerMap':m.name);
 if(m.owner==='<module>')return m.name==='maps'?cpp+'mapRegistry':ixx+'MapHandler';
 if(m.name==='FALLBACK')return cpp+'SubHandler::FALLBACK';
 if(m.name==='_configuration')return ixx+'SubHandler::configuration_';
 if(m.name==='_fallback')return ixx+'SubHandler::fallback_';
 if(m.owner==='SubHandlers'&&m.name==='map')return ixx+'SubHandlers::map_';
 return (m.name==='keys'?ixx:cpp)+m.owner+'::'+m.name;
}
assert.equal(fresh.files[0].members.length,21);assert.deepEqual(fresh.files[0].object_methods.map(m=>m.name),['register','getMap']);
if(process.argv.includes('--freeze-reviewed')){
 fresh.status='locally_accepted_complete_map_handler_not_concrete_token_maps_parser_or_renderer';
 for(const m of [...fresh.files[0].members,...fresh.files[0].object_methods]){m.target=target(m);m.review='root_review_passed';m.evidence='docs/math-port/evidence/p20a-map-handler-acceptance.md';}
 fresh.files[0].native_adaptation='Root-frozen TokenMap protocol with raw Property map keys, actual prioritized-list records, native MethodConfig, RTTI CharacterMap family and retained live Map iterator. Concrete TokenMap algorithms and regexp engine remain unaccepted.';
 fs.writeFileSync(file,JSON.stringify(fresh,null,2)+'\n');
}
const accepted=JSON.parse(fs.readFileSync(file));assert.equal(accepted.files.length,1);
const a=accepted.files[0],f=fresh.files[0];assert.equal(a.source,f.source);assert.equal(a.source_sha256,f.source_sha256);assert.equal(a.members.length,f.members.length);assert.equal(a.object_methods.length,2);
for(const group of ['members','object_methods'])for(let i=0;i<f[group].length;i++){const m=f[group][i],entry=a[group][i];for(const[k,v]of Object.entries(m))if(!['target','review','evidence'].includes(k))assert.deepEqual(entry[k],v);assert.equal(entry.review,'root_review_passed');assert.equal(entry.target,target(m));assert.ok(fs.existsSync(path.join(root,entry.target.split(':')[0])));assert.ok(fs.existsSync(path.join(root,entry.evidence)));}
console.log(JSON.stringify({source_files:1,inventoried_members:21,object_methods:2,status:'reviewed_pinned_inventory_matches'}));
