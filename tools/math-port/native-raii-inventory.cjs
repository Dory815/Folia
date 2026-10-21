#!/usr/bin/env node
'use strict';

// Static inventory for the Native RAII math replacement. This is deliberately
// a proposal generator: it records observable source facts and labels uncertain
// disposition/provenance instead of asserting that names prove reuse.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');

function discoverRoot(start=__dirname) {
  let candidate=path.resolve(start);
  while(true) {
    if(fs.existsSync(path.join(candidate,'CMakeLists.txt'))&&fs.existsSync(path.join(candidate,'docs/math-port/HANDOFF.md')))return candidate;
    const parent=path.dirname(candidate);
    if(parent===candidate)throw new Error(`Could not discover repository root from ${start}`);
    candidate=parent;
  }
}
const root = discoverRoot();
const args=process.argv.slice(2);
const option=(name,fallback)=>{const i=args.indexOf(name);return i>=0&&args[i+1]?path.resolve(args[i+1]):fallback;};
const inventoryOutputRoot=path.join(root,'build/math-port/inventory');
const out=option('--report',path.join(inventoryOutputRoot,'native-raii-inventory.json'));
const manifestOut=option('--manifest',path.join(inventoryOutputRoot,'native-raii-files.json'));
const norm = p => p.split(path.sep).join('/');
const walk = dir => fs.readdirSync(dir, {withFileTypes:true}).flatMap(e => {
  const p = path.join(dir, e.name);
  return e.isDirectory() ? walk(p) : [p];
});
const sha = b => crypto.createHash('sha256').update(b).digest('hex');
const read = p => fs.readFileSync(path.join(root,p), 'utf8');
const rootCMake = read('CMakeLists.txt');
const list = dir => walk(path.join(root,dir)).map(p => norm(path.relative(root,p))).sort();
const isSourceFile=p=>/\.(ixx|cpp|hpp|h|cc|c|inc)$/.test(p);
const unresolvedRefs=(refs,known)=>[...refs].filter(p=>!known.has(p)).sort();
const trackedUnder = dir => list(dir);
function resolveModuleEdges(p,facts,providers,activeSources) {
  const found=[], unresolved=[];
  for(const i of facts.imports) {
    if(i.kind==='standard')continue;
    const name=i.kind==='partition'?`${facts.primaryModule}${i.module}`:i.module;
    const ps=providers.get(name)??[];
    if(ps.length===1)found.push({module:i.module,resolvedModule:name,file:ps[0],exported:i.exported,kind:i.kind});
    else if(activeSources.has(p))unresolved.push({file:p,module:i.module,resolvedModule:name,reason:ps.length?'duplicate-active-interface-providers':'no-registered-interface-provider'});
  }
  if(facts.isImplementationUnit&&facts.primaryModule) {
    const ps=providers.get(facts.primaryModule)??[];
    if(ps.length===1)found.push({module:facts.primaryModule,resolvedModule:facts.primaryModule,file:ps[0],exported:false,kind:'implicit-primary-interface'});
    else if(activeSources.has(p))unresolved.push({file:p,module:facts.primaryModule,resolvedModule:facts.primaryModule,reason:ps.length?'duplicate-active-primary-interface-providers':'implementation-unit-missing-primary-interface'});
  }
  return {edges:found,unresolved};
}
function resolveTargetSourceVariables(bodies,targetName,manifests) {
  const variables=new Set();
  for(const body of bodies)for(const m of body.matchAll(/\$\{([A-Za-z_][A-Za-z0-9_]*)\}/g))variables.add(m[1]);
  const resolved=[],unresolved=[];
  for(const variable of variables) {
    // These CMake built-ins expand path prefixes, not source-list contents.
    if(['CMAKE_CURRENT_SOURCE_DIR','CMAKE_CURRENT_BINARY_DIR','CMAKE_CURRENT_LIST_DIR'].includes(variable))continue;
    if(targetName==='folia_math'&&variable==='FOLIA_FONT_GENERATED_CPP'&&manifests.fontDependencies) {
      const manifest=manifests.fontDependencies;
      if(!Array.isArray(manifest.generated_cpp)) {
        unresolved.push({target:targetName,variable,expression:`\${${variable}}`,reason:'known-font-dependency-manifest-has-no-generated_cpp-array'});
        continue;
      }
      const generatedCpp=(manifest.generated_cpp??[]).filter(p=>/\.cpp$/.test(p));
      resolved.push({target:targetName,variable,expression:`\${${variable}}`,origin:manifests.fontDependenciesPath,
        sourceLock:manifest.source_lock??null,
        manifestSha256:manifests.fontDependenciesSha256,sourceDependencyCount:(manifest.source_dependencies??[]).length,
        manifestGeneratedCppCount:manifest.generated_cpp.length,
        excludedNonCppOutputs:manifest.generated_cpp.filter(p=>!/\.cpp$/.test(p)),generatedCpp});
    } else unresolved.push({target:targetName,variable,expression:`\${${variable}}`,reason:'source-list-variable-not-resolved-by-this-static-inventory'});
  }
  return {resolved,unresolved};
}
function runSelfTests() {
  const assert=require('node:assert/strict');
  assert.equal(isSourceFile('src/math/output/scene/builder.ixx'),true);
  assert.equal(isSourceFile('src/math/output/scene/builder.cpp'),true);
  assert.deepEqual(unresolvedRefs(new Set(['src/math/missing.ixx']),new Set(['src/math/present.ixx'])),['src/math/missing.ixx']);
  const providers=new Map([['fixture.primary',['primary.ixx']],['fixture.primary:node',['partition.ixx']]]);
  const impl=resolveModuleEdges('impl.cpp',{isImplementationUnit:true,primaryModule:'fixture.primary',imports:[{module:':node',kind:'partition',exported:false}]},providers,new Set(['impl.cpp']));
  assert.deepEqual(impl.edges.map(e=>[e.kind,e.file]),[['partition','partition.ixx'],['implicit-primary-interface','primary.ixx']]);
  assert.deepEqual(impl.unresolved,[]);
  const missing=resolveModuleEdges('active.ixx',{isImplementationUnit:false,primaryModule:'fixture.primary',imports:[{module:'fixture.old',kind:'named-module',exported:false}]},providers,new Set(['active.ixx']));
  assert.equal(missing.edges.length,0);
  assert.equal(missing.unresolved[0].reason,'no-registered-interface-provider');
  assert.equal(resolveModuleEdges('active.cpp',{isImplementationUnit:true,primaryModule:'fixture.absent',imports:[]},providers,new Set(['active.cpp'])).unresolved[0].reason,'implementation-unit-missing-primary-interface');
  const literalFixture=['literal.cpp'];
  const variableFixture=resolveTargetSourceVariables(['target_sources(folia_math PRIVATE literal.cpp ${FOLIA_FONT_GENERATED_CPP} ${UNKNOWN_SOURCE_LIST})'],'folia_math',
    {fontDependencies:{generated_cpp:['build/font/a.cpp','build/font/data.inc','build/font/b.cpp']},fontDependenciesPath:'fixture-dependencies.json',fontDependenciesSha256:'fixture-hash'});
  assert.deepEqual(variableFixture.resolved[0].generatedCpp,['build/font/a.cpp','build/font/b.cpp']);
  assert.equal(variableFixture.unresolved[0].variable,'UNKNOWN_SOURCE_LIST');
  assert.equal(variableFixture.resolved[0].generatedCpp.includes('literal.cpp'),false);
  assert.deepEqual([...literalFixture,...variableFixture.resolved[0].generatedCpp],['literal.cpp','build/font/a.cpp','build/font/b.cpp']);
  assert.equal(resolveTargetSourceVariables(['target_sources(folia_math PRIVATE ${FOLIA_FONT_GENERATED_CPP})'],'folia_math',
    {fontDependencies:{},fontDependenciesPath:'fixture-dependencies.json'}).unresolved[0].reason,'known-font-dependency-manifest-has-no-generated_cpp-array');
  const missingEnvelope=codemodelEnvelopeStatus({root,index:null,codemodel:null});
  assert.equal(missingEnvelope.status,'missing');
  assert.equal(missingEnvelope.authoritative,false);
  assert.equal(codemodelEnvelopeStatus({root,indexError:'bad json',codemodel:null}).status,'malformed');
  const wrongRoot=codemodelEnvelopeStatus({root,index:{reply:{'client-math-port':{'codemodel-v2':{jsonFile:'cm.json'}}}},
    codemodel:{paths:{source:path.resolve(root,'elsewhere'),build:path.resolve(root,'build/core')},configurations:[]}});
  assert.equal(wrongRoot.status,'wrong-source-root');
  assert.equal(wrongRoot.authoritative,false);
  assert.equal(codemodelEnvelopeStatus({root,index:{reply:{'client-math-port':{'codemodel-v2':{jsonFile:'cm.json'}}}},
    codemodel:{paths:{source:root},configurations:[]}}).status,'malformed');
  assert.equal(codemodelEnvelopeStatus({root,index:{reply:{'client-math-port':{'codemodel-v2':{jsonFile:'cm.json'}}}},
    codemodel:{paths:{source:root,build:path.join(root,'build/core')},configurations:{}}}).status,'malformed');
  const missingReader=readConfiguredCodemodel(root,{replyDir:path.join(root,'build/math-port/no-file-api-reader-fixture')});
  assert.equal(missingReader.status,'missing');
  assert.equal(missingReader.authoritative,false);
  const fixtureModel={paths:{source:'/fixture',build:'/fixture/build'}};
  const fixtureTargets=[
    {name:'folia_math',id:'folia_math::@fixture',sources:[
      {path:'src/math/a.ixx',exists:true},{path:'src/math/b.ixx',exists:true},{path:'src/math/a.cpp',exists:true}]},
    {name:'FoliaNativeMath',id:'FoliaNativeMath::@fixture',sources:[{path:'src/math/native_renderer_bridge.cpp',exists:true}]}
  ];
  const fixtureMap={modules:{'fixture.a':{},'fixture.b':{}}};
  const fixtureTexts={'src/math/a.ixx':'export module fixture.a;','src/math/b.ixx':'export module fixture.b;'};
  const evaluateFixture=(targets=fixtureTargets,map=fixtureMap,texts=fixtureTexts)=>evaluateConfiguredTargetData('/fixture',{
    indexFile:'fixture-index.json',codemodelFile:'fixture-codemodel.json',codemodel:fixtureModel,configuration:'Debug',targetFacts:targets,
    moduleMap:map,sourceText:p=>{if(!(p in texts))throw new Error(`unexpected fixture read: ${p}`);return texts[p];}});
  const fixtureConfigured=evaluateFixture();
  assert.equal(fixtureConfigured.status,'ready');
  assert.equal(fixtureConfigured.authoritative,true);
  assert.equal(fixtureConfigured.foliaMathSourceCount,3);
  assert.equal(fixtureConfigured.foliaMathInterfaceCount,2);
  assert.equal(fixtureConfigured.moduleMapCheck.exactMatch,true);
  assert.equal(fixtureConfigured.bridgeSourceMember,true);
  assert.deepEqual(fixtureConfigured.bridgeTargets,['FoliaNativeMath']);
  assert.ok(fixtureConfigured.allTargetMathMembership.some(t=>t.name==='FoliaNativeMath'&&t.paths.includes('src/math/native_renderer_bridge.cpp')));
  const missingGenerated=evaluateConfiguredTargetData('/fixture',{indexFile:'fixture-index.json',codemodelFile:'fixture-codemodel.json',
    codemodel:fixtureModel,configuration:'Debug',targetFacts:fixtureTargets,moduleMap:fixtureMap,
    generatedFontCpp:['build/font/missing.cpp'],sourceText:p=>fixtureTexts[p]});
  assert.equal(missingGenerated.status,'stale-inputs');
  assert.equal(missingGenerated.authoritative,false);
  const staleSummary=configuredCodemodelSummary({...missingGenerated,indexFile:'latest-index.json',codemodelFile:'latest-codemodel.json',
    missingFontInputs:['build/font/missing.cpp'],evidence:{index:{sha256:'fixture'}}});
  assert.equal(staleSummary.indexFile,'latest-index.json');
  assert.equal(staleSummary.missingFontInputs[0],'build/font/missing.cpp');
  assert.equal(staleSummary.evidence.index.sha256,'fixture');
  const generatedScan=moduleScanCandidates(['fixture-present.cpp','fixture-missing.cpp'],p=>p==='fixture-present.cpp');
  assert.deepEqual(generatedScan.paths,['fixture-present.cpp']);
  assert.deepEqual(generatedScan.missing,['fixture-missing.cpp']);
  const absentModuleMap=evaluateFixture(fixtureTargets,{});
  assert.equal(absentModuleMap.status,'malformed');
  assert.equal(absentModuleMap.authoritative,false);
  const malformedTarget=evaluateFixture([{name:'folia_math',sources:null}],fixtureMap);
  assert.equal(malformedTarget.status,'malformed');
  assert.equal(malformedTarget.authoritative,false);
  const missingSource=evaluateFixture([{...fixtureTargets[0],sources:[{path:'src/math/missing.ixx',exists:false}]},fixtureTargets[1]]);
  assert.equal(missingSource.status,'stale-inputs');
  assert.equal(missingSource.authoritative,false);
  const mismatchedMap=evaluateFixture(fixtureTargets,{modules:{'fixture.a':{},'fixture.other':{}}});
  assert.equal(mismatchedMap.status,'incomplete');
  assert.equal(mismatchedMap.authoritative,false);
  assert.equal(mismatchedMap.moduleMapCheck.exactMatch,false);
  const duplicateProvider=evaluateFixture([{...fixtureTargets[0],sources:[...fixtureTargets[0].sources,{path:'src/math/c.ixx',exists:true}]}],fixtureMap,
    {...fixtureTexts,'src/math/c.ixx':'export module fixture.a;'});
  assert.equal(duplicateProvider.status,'incomplete');
  assert.equal(duplicateProvider.moduleMapCheck.duplicateProviders.length,1);
  const undeclared=evaluateFixture([{...fixtureTargets[0],sources:[...fixtureTargets[0].sources,{path:'src/math/c.ixx',exists:true}]}],fixtureMap,
    {...fixtureTexts,'src/math/c.ixx':'module;'});
  assert.equal(undeclared.status,'incomplete');
  assert.equal(undeclared.moduleMapCheck.undeclaredInterfaceCount,1);
  const noBuildReader=readConfiguredCodemodel('/fixture',{replyDir:'/fixture/no-file-api-replies'});
  assert.equal(noBuildReader.status,'missing');
  assert.equal(noBuildReader.authoritative,false);
  assert.equal(noBuildReader.foliaMathSources,undefined);
  return 'passed 55 assertions (including isolated File API fixtures; no repository build inputs read)';
}
if(args.includes('--self-test')) { process.stdout.write(`${runSelfTests()}\n`); process.exit(0); }
const textFiles = list('src/math');
const sourceFiles = textFiles.filter(isSourceFile);
const fileIds = new Map(textFiles.map((p,i)=>[p,`F${String(i+1).padStart(3,'0')}`]));
function moduleScanCandidates(paths,exists= p=>fs.existsSync(path.join(root,p))) {
  const unique=[...new Set(paths)];
  return {paths:unique.filter(exists),missing:unique.filter(p=>!exists(p))};
}

function moduleFacts(p) {
  if (!/\.(ixx|cpp)$/.test(p)) return {declaration:null, isInterface:false, isImplementationUnit:false, primaryModule:null, partition:null, imports:[], forwardDeclarations:[]};
  const s = read(p), imports = [];
  for (const m of s.matchAll(/^\s*(export\s+)?import\s+([^;]+);/gm)) {
    const name = m[2].trim();
    imports.push({module:name, exported:!!m[1], kind:name.startsWith(':') ? 'partition' : name.startsWith('std') ? 'standard' : 'named-module'});
  }
  const interfaceDecl = s.match(/^\s*export\s+module\s+([^;]+);/m);
  const decl = interfaceDecl ?? s.match(/^\s*module\s+([^;]+);/m);
  const named = decl?.[1]?.trim() ?? null;
  const forwardDeclarations = [...s.matchAll(/^\s*(?:export\s+)?(?:class|struct|union)\s+([A-Za-z_][A-Za-z0-9_]*)\s*;/gm)].map(m=>m[1]);
  return {declaration:named, isInterface:!!interfaceDecl, isImplementationUnit:!!named&&!interfaceDecl, primaryModule:named?.includes(':') ? named.slice(0,named.indexOf(':')) : named,
    partition:named?.includes(':') ? named.slice(named.indexOf(':')+1) : null, imports, forwardDeclarations};
}

function configuredCodemodelSummary(c) {
  const summary={status:c.status,authoritative:c.authoritative,reason:c.reason??null};
  for(const key of ['indexFile','codemodelFile','generatedModuleMapPath','expectedSourceRoot','reportedSourceRoot','expectedBuildRoot','reportedBuildRoot',
    'actualReplyDir','expectedReplyDir','badPaths','missingInputs','newerInputs','missingFontInputs','extraFontInputs','moduleMapCheck','evidence','freshness'])
    if(c[key]!==undefined)summary[key]=c[key];
  if(c.authoritative)Object.assign(summary,{sourceRoot:c.sourceRoot,buildRoot:c.buildRoot,configuration:c.configuration,
    foliaMathSourceCount:c.foliaMathSourceCount,foliaMathInterfaceCount:c.foliaMathInterfaceCount,bridgeSourceMember:c.bridgeSourceMember,
    bridgeTargets:c.bridgeTargets,allTargetMathMembership:c.allTargetMathMembership,
    targets:c.targets.map(t=>({name:t.name,id:t.id,type:t.type,sourcePaths:t.sources.map(s=>s.path)})),foliaMathSources:c.foliaMathSources});
  return summary;
}
function codemodelEnvelopeStatus({index,indexError,codemodel,codemodelError,root}) {
  if(!index&&!indexError)return {status:'missing',authoritative:false,reason:'no-file-api-index'};
  if(indexError||codemodelError)return {status:'malformed',authoritative:false,reason:String(indexError??codemodelError)};
  if(!index?.reply?.['client-math-port']?.['codemodel-v2'])return {status:'malformed',authoritative:false,reason:'missing-client-math-port-codemodel-v2'};
  if(!Array.isArray(codemodel?.configurations)||typeof codemodel?.paths?.source!=='string'||!codemodel.paths.source.trim()||
      typeof codemodel?.paths?.build!=='string'||!codemodel.paths.build.trim())
    return {status:'malformed',authoritative:false,reason:'invalid-codemodel-shape'};
  const actual=path.resolve(root,codemodel.paths.source),expected=path.resolve(root);
  if(process.platform==='win32' ? actual.toLowerCase()!==expected.toLowerCase() : actual!==expected)
    return {status:'wrong-source-root',authoritative:false,expectedSourceRoot:expected,reportedSourceRoot:actual};
  const actualBuild=path.resolve(root,codemodel.paths.build),expectedBuild=path.resolve(root,'build/core');
  if(process.platform==='win32' ? actualBuild.toLowerCase()!==expectedBuild.toLowerCase() : actualBuild!==expectedBuild)
    return {status:'incomplete',authoritative:false,reason:'codemodel-build-root-does-not-match-local-core-module-map',expectedBuildRoot:expectedBuild,reportedBuildRoot:actualBuild};
  return {status:'candidate',authoritative:false,sourceRoot:actual};
}
function evaluateConfiguredTargetData(repoRoot, {indexFile,codemodelFile,codemodel,configuration,targetFacts,moduleMap,
    generatedFontCpp=[],indexTime=0,freshnessInputs=[],sourceText=p=>fs.readFileSync(path.join(repoRoot,p),'utf8')}) {
  const mathTarget=targetFacts.find(t=>t.name==='folia_math'&&!t.name.includes('@synth'));
  if(!mathTarget)return {status:'incomplete',authoritative:false,reason:'folia_math-target-not-present',indexFile,codemodelFile};
  if(!targetFacts.every(t=>Array.isArray(t.sources)))return {status:'malformed',authoritative:false,reason:'target-sources-not-array',indexFile,codemodelFile};
  if(!moduleMap||typeof moduleMap.modules!=='object'||moduleMap.modules===null||Array.isArray(moduleMap.modules))
    return {status:'malformed',authoritative:false,reason:'generated-module-map-has-no-modules-object',indexFile,codemodelFile};
  const badPaths=targetFacts.flatMap(t=>t.sources.filter(s=>!s.exists).map(s=>`${t.name}:${s.path}`));
  if(badPaths.length)return {status:'stale-inputs',authoritative:false,reason:'configured-target-source-paths-missing',indexFile,codemodelFile,badPaths};
  const newerInputs=freshnessInputs.filter(input=>input.mtimeMs>indexTime).map(input=>input.path);
  const expectedSet=new Set(generatedFontCpp.map(norm)), configuredSet=new Set(mathTarget.sources.map(s=>s.path));
  const missingFontInputs=[...expectedSet].filter(p=>!configuredSet.has(p));
  const extraFontInputs=mathTarget.sources.map(s=>s.path).filter(p=>p.startsWith('build/math-port/font-generated-native-raii/')&&p.endsWith('.cpp')&&!expectedSet.has(p));
  if(newerInputs.length||missingFontInputs.length||extraFontInputs.length)return {status:'stale-inputs',authoritative:false,
    reason:'configured-codemodel-predates-source-list-inputs-or-font-manifest',indexFile,codemodelFile,newerInputs,missingFontInputs,extraFontInputs};
  const interfaceRows=mathTarget.sources.filter(s=>s.path.endsWith('.ixx')).map(source=>{
    const body=sourceText(source.path), declaration=body.match(/^\s*export\s+module\s+([^;]+);/m)?.[1]?.trim()??null;
    return {path:source.path,module:declaration};
  });
  const providerMap=new Map();
  for(const row of interfaceRows)if(row.module)providerMap.set(row.module,[...(providerMap.get(row.module)??[]),row.path]);
  const moduleMapNames=Object.keys(moduleMap.modules??{}).sort(), providerNames=[...providerMap.keys()].sort();
  const duplicateProviders=[...providerMap].filter(([,ps])=>ps.length>1).map(([module,paths])=>({module,paths}));
  const moduleMapCheck={moduleMapCount:moduleMapNames.length,providerCount:providerNames.length,interfaceSourceCount:interfaceRows.length,
    undeclaredInterfaceCount:interfaceRows.filter(x=>!x.module).length,
    exactMatch:JSON.stringify(moduleMapNames)===JSON.stringify(providerNames)&&interfaceRows.every(x=>x.module)&&duplicateProviders.length===0,
    missingFromMap:providerNames.filter(n=>!moduleMapNames.includes(n)),missingProviders:moduleMapNames.filter(n=>!providerMap.has(n)),duplicateProviders};
  if(!moduleMapCheck.exactMatch)return {status:'incomplete',authoritative:false,reason:'configured-interface-declarations-do-not-match-generated-module-map',
    indexFile,codemodelFile,moduleMapCheck};
  const allMathTargets=targetFacts.map(t=>({name:t.name,id:t.id,paths:t.sources.filter(s=>s.path.startsWith('src/math/')).map(s=>s.path)})).filter(t=>t.paths.length);
  const bridgeTargets=allMathTargets.filter(t=>t.paths.includes('src/math/native_renderer_bridge.cpp')).map(t=>t.name);
  return {status:'ready',authoritative:true,sourceRoot:path.resolve(codemodel.paths.source),buildRoot:path.resolve(repoRoot,codemodel.paths.build),configuration,
    indexFile,codemodelFile,targets:targetFacts,foliaMathSourceCount:mathTarget.sources.length,foliaMathSources:mathTarget.sources,
    foliaMathInterfaceCount:interfaceRows.length,interfaceProviders:interfaceRows,moduleMapNames,moduleMapCheck,
    allTargetMathMembership:allMathTargets,bridgeTargets,bridgeSourceMember:bridgeTargets.length>0,
    freshness:{status:'mtime-current-at-read',indexMtimeUtc:new Date(indexTime).toISOString(),checkedCMakeInputs:freshnessInputs.map(x=>x.path),newerInputs:[],
      generatedFontCppCount:expectedSet.size,missingFontInputs:[],extraFontInputs:[],
      limitation:'Codemodel records the last configure result; it does not attest unchanged non-CMake glob contents or source file content hashes.'}};
}
function readConfiguredCodemodel(repoRoot, {generatedFontCpp=[],cmakeFiles=[],replyDir=path.join(repoRoot,'build/core/.cmake/api/v1/reply')}={}) {
  let indexes=[];
  try { indexes=fs.readdirSync(replyDir).filter(x=>/^index-.*\.json$/.test(x)).sort((a,b)=>fs.statSync(path.join(replyDir,b)).mtimeMs-fs.statSync(path.join(replyDir,a)).mtimeMs); }
  catch(error) { return {status:'missing',authoritative:false,reason:String(error)}; }
  if(!indexes.length)return {status:'missing',authoritative:false,reason:'no-file-api-index'};
  let index,indexFile,indexPath,ref;
  for(const candidate of indexes) {
    indexPath=path.join(replyDir,candidate);
    try { index=JSON.parse(fs.readFileSync(indexPath,'utf8')); }
    catch(error) { return {status:'malformed',authoritative:false,reason:`index-invalid:${error}`,indexFile:candidate}; }
    ref=index.reply?.['client-math-port']?.['codemodel-v2'];
    if(ref?.jsonFile){indexFile=candidate;break;}
  }
  if(!indexFile)return {status:'missing',authoritative:false,reason:'no-index-contains-client-math-port-codemodel-v2'};
  const codemodelFile=ref.jsonFile; let codemodel;
  try { codemodel=JSON.parse(fs.readFileSync(path.join(replyDir,codemodelFile),'utf8')); }
  catch(error) { return {status:'malformed',authoritative:false,reason:`codemodel-invalid:${error}`,indexFile,codemodelFile}; }
  const envelope=codemodelEnvelopeStatus({index,codemodel,root:repoRoot});
  if(envelope.status!=='candidate')return {...envelope,indexFile,codemodelFile};
  const configuration=codemodel.configurations.find(x=>x.name==='Debug');
  if(!configuration)return {status:'incomplete',authoritative:false,reason:'Debug-configuration-not-present',indexFile,codemodelFile};
  if(!Array.isArray(configuration.targets))return {status:'malformed',authoritative:false,reason:'configuration-targets-not-array',indexFile,codemodelFile};
  const indexTime=fs.statSync(indexPath).mtimeMs, targetFacts=[];
  try {
    for(const targetRef of configuration.targets??[]) {
      if(!targetRef||typeof targetRef.jsonFile!=='string'||!targetRef.jsonFile.trim())return {status:'malformed',authoritative:false,reason:'invalid-target-reference',indexFile,codemodelFile};
      const targetPath=path.join(replyDir,targetRef.jsonFile), target=JSON.parse(fs.readFileSync(targetPath,'utf8'));
      if(!target||!Array.isArray(target.sources))return {status:'malformed',authoritative:false,reason:`target-${targetRef.name}-sources-not-array`,indexFile,codemodelFile};
      const targetDirectory=target.paths?.source??'.', relative=path.relative(path.resolve(repoRoot),path.resolve(repoRoot,targetDirectory));
      if(relative==='..'||relative.startsWith(`..${path.sep}`)||path.isAbsolute(relative))return {status:'wrong-source-root',authoritative:false,
        reason:`target-${target.name}-source-root-outside-codemodel-root`,indexFile,codemodelFile};
      const sources=(target.sources??[]).map(source=>{
        const absolute=path.isAbsolute(source.path)?path.resolve(source.path):path.resolve(repoRoot,source.path);
        return {path:norm(path.relative(repoRoot,absolute)),exists:fs.existsSync(absolute),isGenerated:!!source.isGenerated,compileGroupIndex:source.compileGroupIndex??null};
      });
      const stat=fs.statSync(targetPath);
      targetFacts.push({name:target.name,id:target.id,type:target.type??null,reply:targetRef.jsonFile,sha256:sha(fs.readFileSync(targetPath)),
        mtimeUtc:stat.mtime.toISOString(),sources});
    }
  } catch(error) { return {status:'malformed',authoritative:false,reason:`target-reply-invalid:${error}`,indexFile,codemodelFile}; }
  if(!Array.isArray(configuration.directories)||!configuration.directories.every(d=>typeof d?.source==='string'))
    return {status:'malformed',authoritative:false,reason:'configuration-directories-invalid',indexFile,codemodelFile};
  const freshnessPaths=[...new Set([...configuration.directories.map(d=>path.join(repoRoot,d.source,'CMakeLists.txt')),
    ...cmakeFiles.map(p=>path.join(repoRoot,p)),path.join(repoRoot,'build/math-port/font-generated-native-raii/font-data-source-dependencies.json')])];
  const codemodelPath=path.join(replyDir,codemodelFile),indexStat=fs.statSync(indexPath),codemodelStat=fs.statSync(codemodelPath);
  const partialEvidence={index:{path:norm(path.relative(repoRoot,indexPath)),sha256:sha(fs.readFileSync(indexPath)),mtimeUtc:indexStat.mtime.toISOString()},
    codemodel:{path:norm(path.relative(repoRoot,codemodelPath)),sha256:sha(fs.readFileSync(codemodelPath)),mtimeUtc:codemodelStat.mtime.toISOString()},
    targets:targetFacts.map(t=>({name:t.name,path:norm(path.join('build/core/.cmake/api/v1/reply',t.reply)),sha256:t.sha256,mtimeUtc:t.mtimeUtc}))};
  const missingInputs=freshnessPaths.filter(p=>!fs.existsSync(p)).map(p=>norm(path.relative(repoRoot,p)));
  if(missingInputs.length)return {status:'incomplete',authoritative:false,reason:'codemodel-configure-inputs-missing',indexFile,codemodelFile,missingInputs,evidence:partialEvidence};
  const freshnessInputs=freshnessPaths.map(p=>({path:norm(path.relative(repoRoot,p)),mtimeMs:fs.statSync(p).mtimeMs}));
  const buildRoot=path.resolve(repoRoot,codemodel.paths.build);
  const expectedReplyDir=path.join(buildRoot,'.cmake/api/v1/reply');
  const actualReplyDir=path.resolve(replyDir);
  if(process.platform==='win32' ? actualReplyDir.toLowerCase()!==expectedReplyDir.toLowerCase() : actualReplyDir!==expectedReplyDir)
    return {status:'incomplete',authoritative:false,reason:'file-api-reply-directory-does-not-match-codemodel-build-root',expectedReplyDir,actualReplyDir,indexFile,codemodelFile};
  const moduleMapPath=path.join(buildRoot,'CMakeFiles/folia_math.dir/CXXModules.json'); let moduleMap;
  try { moduleMap=JSON.parse(fs.readFileSync(moduleMapPath,'utf8')); }
  catch(error) { return {status:'incomplete',authoritative:false,reason:`generated-module-map-invalid-or-missing:${error}`,indexFile,codemodelFile}; }
  const mapStat=fs.statSync(moduleMapPath);
  const evidence={...partialEvidence,
    generatedModuleMap:{path:norm(path.relative(repoRoot,moduleMapPath)),sha256:sha(fs.readFileSync(moduleMapPath)),mtimeUtc:mapStat.mtime.toISOString()},
  };
  const result=evaluateConfiguredTargetData(repoRoot,{indexFile,codemodelFile,codemodel,configuration:configuration.name,targetFacts,moduleMap,
    generatedFontCpp,indexTime:indexStat.mtimeMs,freshnessInputs,sourceText:p=>fs.readFileSync(path.join(repoRoot,p),'utf8')});
  return {...result,evidence,generatedModuleMapPath:norm(path.relative(repoRoot,moduleMapPath)),
    freshness:result.freshness??{status:result.status,newerInputs:result.newerInputs??[],limitation:'A rejected snapshot is not authoritative.'}};
}

function responsibility(p) {
  const n=p.toLowerCase();
  if(/(?:^|\/)(?:license-[^/]+|notice)\.txt$/.test(n))return {decision:'keep-license',responsibilityGroup:'third-party-license',namedOwner:'Root (license provenance)'};
  const group=n.includes('/dependencies/mhchem_parser/')?'math.tex.mhchem':n.includes('/input/tex/')?'math.tex':
    n.includes('/output/common/')?'math.common':n.includes('/output/scene/')?'math.scene':n.includes('/output/svg/')?'math.svg':
    n.includes('/fonts/')?'math.fonts':n.includes('/util/')?'math.util':n.includes('/core/mml_tree/')?'math.mml':
    n.includes('/core/tree/')?'math.tree':n.includes('/handlers/html/')?'math.html':n.includes('/adaptors/')||n.includes('/dom_')?'math.dom-adaptors':
    n.includes('/core/')?'math.core':n.endsWith('/native_renderer_bridge.cpp')?'math.c-abi-bridge':n.includes('/math.cpp')||n.includes('/math.ixx')?'math.renderer':'math.other';
  return {decision:'unreviewed',responsibilityGroup:group,namedOwner:'unassigned'};
}

function cmakeClosure() {
  const visited=new Set(), todo=['CMakeLists.txt'];
  while (todo.length) {
    const file=todo.pop();
    if (visited.has(file) || !fs.existsSync(path.join(root,file))) continue;
    visited.add(file);
    const body=read(file), base=path.posix.dirname(file);
    for (const m of body.matchAll(/^\s*add_subdirectory\s*\(\s*([^\s)]+)(?:\s+[^)]*)?\)/gm)) {
      const dir=path.posix.normalize(path.posix.join(base,m[1]));
      if (/[${}]/.test(dir)) continue;
      todo.push(path.posix.join(dir,'CMakeLists.txt'));
    }
  }
  return [...visited].sort();
}
const cmakeFiles = cmakeClosure();
const cmakeText = cmakeFiles.map(p => read(p)).join('\n');
const cmakeRefs = new Set([...cmakeText.matchAll(/(?:src\/math|tests\/math)\/[A-Za-z0-9_./-]+\.(?:ixx|cpp|hpp|h|c|cc)/g)].map(m => m[0]));
const mathRefs = new Set();
const targetSources = {};
for (const block of cmakeText.matchAll(/(?:target_sources|add_library|add_executable)\s*\(\s*([A-Za-z0-9_]+)/g)) {
  const rest = cmakeText.slice(block.index);
  let depth=0, end=0;
  for (; end<rest.length; end++) { if (rest[end]==='(') depth++; else if (rest[end]===')' && --depth===0) { end++; break; } }
  const body=rest.slice(0,end), target=block[1], files=[...body.matchAll(/(?:src\/math|tests\/math)\/[A-Za-z0-9_./-]+\.(?:ixx|cpp|hpp|h|c|cc)/g)].map(m=>m[0]);
  if (!targetSources[target]) targetSources[target]=new Set();
  for (const file of files) targetSources[target].add(file);
}
for (const p of targetSources.folia_math ?? []) if (p.startsWith('src/math/')) mathRefs.add(p);
const lexicalUnresolvedMathRefs=unresolvedRefs(mathRefs,new Set(fileIds.keys()));
let existingMathRefs=[...mathRefs].filter(p=>fileIds.has(p));
const testRefs = new Set([...cmakeRefs].filter(p => p.startsWith('tests/math/')));
const testGlobSources=list('tests').filter(p=>p.endsWith('.cpp')&&!p.startsWith('tests/platform/')&&!p.startsWith('tests/encoding/')&&!p.startsWith('tests/math/'));
if (!targetSources.FoliaTests) targetSources.FoliaTests=new Set();
for (const p of testGlobSources) targetSources.FoliaTests.add(p);
for (const p of testGlobSources) if (p.startsWith('tests/math/')) testRefs.add(p);
// Only registered CXX_MODULES interfaces may satisfy named imports. A primary
// implementation unit (module M;) contributes to M, but never supplies import M.
function targetBodies(targetName) {
  const bodies=[];
  const rx=/(?:target_sources|add_library|add_executable)\s*\(\s*([A-Za-z0-9_]+)/g;
  for (const match of cmakeText.matchAll(rx)) {
    if (match[1]!==targetName) continue;
    const rest=cmakeText.slice(match.index); let depth=0,end=0;
    for (;end<rest.length;end++) { if(rest[end]==='(')depth++; else if(rest[end]===')'&&--depth===0){end++;break;} }
    bodies.push(rest.slice(0,end));
  }
  return bodies;
}
let activeModuleInterfaces=new Set();
for (const body of targetBodies('folia_math')) {
  if (!/FILE_SET\s+CXX_MODULES\s+FILES/.test(body)) continue;
  for (const m of body.matchAll(/src\/math\/[A-Za-z0-9_./-]+\.ixx/g)) activeModuleInterfaces.add(m[0]);
}
const fontDependenciesPath='build/math-port/font-generated-native-raii/font-data-source-dependencies.json';
let fontDependencies=null,fontDependenciesSha256=null;
if(fs.existsSync(path.join(root,fontDependenciesPath))) {
  const bytes=fs.readFileSync(path.join(root,fontDependenciesPath));
  try { fontDependencies=JSON.parse(bytes.toString('utf8')); fontDependenciesSha256=sha(bytes); }
  catch(error) { fontDependencies={manifestError:String(error)}; }
}
const targetSourceExpressions=Object.keys(targetSources).map(target=>({target,bodies:targetBodies(target)}));
const variableResolution=targetSourceExpressions.map(({target,bodies})=>resolveTargetSourceVariables(bodies,target,
  {fontDependencies,fontDependenciesPath,fontDependenciesSha256}));
const resolvedSourceVariableInputs=variableResolution.flatMap(x=>x.resolved);
const unresolvedSourceVariableExpressions=variableResolution.flatMap(x=>x.unresolved);
const generatedFontCppPaths=resolvedSourceVariableInputs.flatMap(x=>x.generatedCpp);
const generatedFontSources=generatedFontCppPaths.map((p,index)=>{
  if(!fs.existsSync(path.join(root,p)))return {id:`GF${String(index+1).padStart(3,'0')}`,path:p,exists:false,origin:fontDependenciesPath,manifestSha256:fontDependenciesSha256};
  const bytes=fs.readFileSync(path.join(root,p));
  const facts=moduleFacts(p);
  return {id:`GF${String(index+1).padStart(3,'0')}`,path:p,exists:true,origin:fontDependenciesPath,
    manifestSha256:fontDependenciesSha256,sha256:sha(bytes),bytes:bytes.length,...facts};
});
const missingGeneratedFontSources=generatedFontSources.filter(x=>!x.exists).map(x=>x.path);
const missingGeneratedFoliaMathSources=missingGeneratedFontSources;
const configuredCodemodel=readConfiguredCodemodel(root,{generatedFontCpp:generatedFontCppPaths,cmakeFiles});
const configuredTargetsBySource=new Map();
const configuredFoliaMathSources=new Set();
if(configuredCodemodel.authoritative) {
  for(const target of configuredCodemodel.targets)for(const source of target.sources)if(source.path.startsWith('src/math/')) {
    if(!configuredTargetsBySource.has(source.path))configuredTargetsBySource.set(source.path,[]);
    configuredTargetsBySource.get(source.path).push(target.name);
  }
  for(const source of configuredCodemodel.foliaMathSources)if(source.path.startsWith('src/math/'))configuredFoliaMathSources.add(source.path);
  existingMathRefs=[...configuredFoliaMathSources].sort();
  activeModuleInterfaces=new Set(configuredCodemodel.foliaMathSources.filter(s=>s.path.endsWith('.ixx')).map(s=>s.path));
}
const interfaceProviders=new Map();
for (const p of activeModuleInterfaces) {
  if (!fileIds.has(p)) continue;
  const f=moduleFacts(p);
  if (!f.isInterface || !f.declaration) continue;
  if (!interfaceProviders.has(f.declaration)) interfaceProviders.set(f.declaration,[]);
  interfaceProviders.get(f.declaration).push(p);
}
const candidateInterfaceProviders=new Map();
for (const p of sourceFiles) {
  const f=moduleFacts(p);
  if (f.isInterface&&f.declaration&&!activeModuleInterfaces.has(p)) {
    if(!candidateInterfaceProviders.has(f.declaration))candidateInterfaceProviders.set(f.declaration,[]);
    candidateInterfaceProviders.get(f.declaration).push(p);
  }
}
const duplicateActiveInterfaceProviders=[...interfaceProviders].filter(([,ps])=>ps.length>1).map(([module,files])=>({module,files}));
const cmakeModuleMapPath='build/core/CMakeFiles/folia_math.dir/CXXModules.json';
let cmakeModuleMap=null;
if(fs.existsSync(path.join(root,cmakeModuleMapPath))) {
  try { const bytes=fs.readFileSync(path.join(root,cmakeModuleMapPath)); const parsed=JSON.parse(bytes.toString('utf8')); cmakeModuleMap={path:cmakeModuleMapPath,sha256:sha(bytes),bytes:bytes.length,modules:Object.keys(parsed.modules??{}).sort()}; }
  catch(error) { cmakeModuleMap={path:cmakeModuleMapPath,error:String(error)}; }
}
const activeInterfaceNames=[...interfaceProviders.keys()].sort();
const cmakeInterfaceNames=cmakeModuleMap?.modules??[];
const lexicalInterfaceMapCheck={available:!!cmakeModuleMap&&!cmakeModuleMap.error,authoritative:false,status:'lexical-candidate-only-no-configured-codemodel',
  generatedModuleCount:cmakeInterfaceNames.length,registeredInterfaceCount:activeInterfaceNames.length,
  matches:null,
  missingFromGenerated:activeInterfaceNames.filter(x=>!cmakeInterfaceNames.includes(x)),
  unexpectedInGenerated:cmakeInterfaceNames.filter(x=>!activeInterfaceNames.includes(x))};
const interfaceMapCheck=configuredCodemodel.authoritative
  ? {available:true,authoritative:true,status:'configured-target-providers-compared-to-generated-module-map',
      generatedModuleCount:configuredCodemodel.moduleMapCheck.moduleMapCount,registeredInterfaceCount:configuredCodemodel.moduleMapCheck.providerCount,
      matches:configuredCodemodel.moduleMapCheck.exactMatch,missingFromGenerated:configuredCodemodel.moduleMapCheck.missingFromMap,
      unexpectedInGenerated:configuredCodemodel.moduleMapCheck.missingProviders,duplicateProviders:configuredCodemodel.moduleMapCheck.duplicateProviders}
  : lexicalInterfaceMapCheck;
const edges = {};
const unresolvedActiveImports=[];
const activeSourceSet=new Set([...existingMathRefs,...generatedFontCppPaths]);
const moduleScan=moduleScanCandidates([...sourceFiles,...generatedFontCppPaths]);
const moduleSourcePaths=moduleScan.paths;
for (const p of moduleSourcePaths) {
  const facts = moduleFacts(p);
  const resolved=resolveModuleEdges(p,facts,interfaceProviders,activeSourceSet);
  edges[p]=resolved.edges;
  unresolvedActiveImports.push(...resolved.unresolved);
}
const forwardDeclarationRisks = [];
for (const p of sourceFiles) {
  const source = moduleFacts(p);
  for (const symbol of source.forwardDeclarations) {
    for (const edge of edges[p] ?? []) {
      const targetText = read(edge.file);
      if (new RegExp(`\\b${symbol}\\b`).test(targetText)) forwardDeclarationRisks.push({file:p,symbol,importedModule:edge.module,importedFile:edge.file,
        note:'Textual symbol occurrence in imported module; review declaration visibility and module ownership (conservative flag).'});
    }
  }
}
const active = new Set(activeSourceSet);
const queue = [...active];
while (queue.length) for (const e of edges[queue.shift()] ?? []) if (!active.has(e.file)) { active.add(e.file); queue.push(e.file); }

const provenanceDocs = trackedUnder('docs/math-port').filter(p=>p.endsWith('.md'));
const provenanceLines = provenanceDocs.flatMap(doc=>read(doc).split(/\r?\n/).map((line,i)=>({document:doc,line:i+1,text:line})));
const sourcePathRx = /(?:\.reference\/MathJax-src\/)?(?:ts\/)[A-Za-z0-9_./-]+\.(?:ts|js)/g;
function sourceAnnotations(p,facts) {
  if (/\/(?:LICENSE-[^/]+|NOTICE)\.txt$/i.test(p)) return [];
  let moduleName=facts.declaration;
  if (!moduleName && p.endsWith('.cpp') && fs.existsSync(path.join(root,p.slice(0,-4)+'.ixx'))) moduleName=moduleFacts(p.slice(0,-4)+'.ixx').declaration;
  const out=[];
  for (const record of provenanceLines) {
    const line=record.text;
    if (!line.includes(p) && !(moduleName && line.includes(moduleName))) continue;
    const sources=[...new Set(line.match(sourcePathRx)??[])];
    if (!sources.length) continue;
    const cells=line.split('|').map(c=>c.trim().replace(/^`|`$/g,''));
    let sourceHash=null, hashEvidence='';
    const tsIndex=cells.findIndex(c=>/^\.reference\/MathJax-src\/ts\//.test(c)||/^ts\//.test(c));
    if (tsIndex>=0 && /^[a-f0-9]{64}$/i.test(cells[tsIndex+1]??'')) {
      sourceHash=cells[tsIndex+1].toLowerCase(); hashEvidence='adjacent table cell';
    } else {
      const hashes=[...new Set(line.match(/\b[a-f0-9]{64}\b/gi)??[])];
      if (sources.length===1 && hashes.length===1) { sourceHash=hashes[0].toLowerCase(); hashEvidence='single hash on exact mapping line'; }
    }
    out.push({document:record.document,line:record.line,sourcePaths:sources,sourceSha256:sourceHash,hashEvidence,
      status:sourceHash?'annotated':'path-only-or-ambiguous'});
  }
  if (/\.(?:cpp|ixx|inc)$/.test(p)) {
    for (const [i,line] of read(p).split(/\r?\n/).slice(0,12).entries()) {
      const sources=[...new Set(line.match(/[A-Za-z0-9_./-]+\.ts\b/g)??[])];
      const hashes=[...new Set(line.match(/\b[a-f0-9]{64}\b/gi)??[])];
      if (sources.length||hashes.length) out.push({document:p,line:i+1,sourcePaths:sources,sourceSha256:hashes.length===1?hashes[0].toLowerCase():null,
        hashEvidence:hashes.length===1?'source-file header':'',status:sources.some(s=>s.includes('ts/'))&&hashes.length===1?'header-annotated':'path-or-hash-incomplete'});
    }
  }
  return out;
}

const files = textFiles.map(p => {
  const {decision,responsibilityGroup,namedOwner:owner} = responsibility(p), facts = moduleFacts(p), bytes = fs.readFileSync(path.join(root,p));
  const annotations=sourceAnnotations(p,facts);
  const exactSources=[...new Map(annotations.filter(a=>a.sourceSha256).flatMap(a=>a.sourcePaths.filter(s=>s.includes('ts/')).map(s=>[`${s}:${a.sourceSha256}`,{path:s,sha256:a.sourceSha256,document:a.document,line:a.line}]))).values()];
  const annotationIndexStatus=exactSources.length===1?'one-note-index-entry':exactSources.length>1?'multiple-note-index-entries':'no-exact-note-index-entry';
  const configuredTargets=configuredTargetsBySource.get(p)??[];
  return {id:fileIds.get(p),path:p,sha256:sha(bytes),bytes:bytes.length,kind:path.extname(p).slice(1),...facts,
    directlyListedByFoliaMath:configuredCodemodel.authoritative?configuredFoliaMathSources.has(p):null,
    lexicallyListedByFoliaMath:mathRefs.has(p),activeModuleImportClosure:configuredCodemodel.authoritative?active.has(p):null,
    mentionedByCMake:cmakeRefs.has(p),configuredTargets,
    directCMakeTargets:[...new Set([...Object.entries(targetSources).filter(([,sources])=>sources.has(p)).map(([target])=>target),...configuredTargets])],
    disposition:decision,nativeResponsibility:responsibilityGroup,responsibilityGroup,namedOwner:owner,
    decisionStatus:decision==='keep-license'?'license-preservation':'unreviewed',sourceAnnotations:annotations,
    sourceAnnotationIndexStatus:annotationIndexStatus,sourceAnnotationNote:'Index heuristic only; not algorithm equivalence, complete Source mapping, or disposition evidence.',
    confidence:'responsibility grouping and annotation index only; no file disposition or algorithm-equivalence conclusion'};
});
const indexedSourceFiles=files.filter(f=>sourceFiles.includes(f.path)&&f.sourceAnnotationIndexStatus!=='no-exact-note-index-entry');

const fontLockPath='docs/math-port/evidence/font-data-source-lock.json';
const fontLock=JSON.parse(read(fontLockPath));
const generatedFontSourceCandidates=fontLock.outputs.filter(x=>/^build\/math-port\/font-generated\/newcm_dynamic_setup_[A-Za-z0-9_]+\.cpp$/.test(x.path))
  .map((x,i)=>({id:`G${String(i+1).padStart(2,'0')}`,target:'folia_math',path:x.path,sha256:x.sha256,bytes:x.bytes,lock:fontLockPath}));
for(const unit of generatedFontSources) {
  const resolved=edges[unit.path]??[];
  unit.resolvedImports=resolved.map(e=>({writtenImport:e.module,resolvedModule:e.resolvedModule,file:e.file,kind:e.kind}));
  unit.unresolvedImports=unresolvedActiveImports.filter(e=>e.file===unit.path);
}
const generatedSourceRows=generatedFontSources.map(unit=>({path:unit.path,decisionStatus:'unreviewed',responsibilityGroup:'math.fonts.generated-newcm',
  namedOwner:'unassigned',configuredTargetMember:configuredCodemodel.authoritative&&configuredCodemodel.foliaMathSources.some(s=>s.path===unit.path),
  availableSourceProvenance:[{document:fontDependenciesPath,manifestSha256:fontDependenciesSha256,
    sourceLock:fontDependencies?.source_lock??null,status:'dependency-manifest note only; no per-unit pinned TS mapping or algorithm-equivalence assertion'}]}));
const lexicalUnresolvedMathSourceVariables=unresolvedSourceVariableExpressions.filter(x=>x.target==='folia_math');
const unresolvedMathSourceVariables=configuredCodemodel.authoritative?[]:lexicalUnresolvedMathSourceVariables;
const fullResolvedCMakeMathSourceCount=configuredCodemodel.authoritative?configuredCodemodel.foliaMathSourceCount:
  existingMathRefs.length+generatedFontSources.filter(x=>x.exists).length;

const targets = configuredCodemodel.authoritative?configuredCodemodel.targets.map(t=>t.name):[...cmakeText.matchAll(/add_(?:library|executable)\s*\(\s*([A-Za-z0-9_]+)/g)].map(m=>m[1]);
const portScripts = trackedUnder('tools/math-port').filter(p=>p.endsWith('.cjs'));
const jsOnlyContractCandidates = portScripts.filter(p=>/(?:mutation-test|\b(?:native-identities|option-array|option-filter|property-symbol|sparse-(?:tree|node)|callable-receiver)|(?:prototype|descriptor|identity|context-property)).*\.cjs$/i.test(p));
const tests = {
  currentCMakeMathTestSources:configuredCodemodel.authoritative?configuredCodemodel.targets.flatMap(t=>t.sources
    .filter(s=>s.path.startsWith('tests/math/')&&s.path.endsWith('.cpp')).map(s=>({target:t.name,path:s.path}))):null,
  lexicalCandidateMathTestSources:[...testRefs].sort(),
  javascriptContractReviewCandidates:jsOnlyContractCandidates,
  retirementPlan:[
    {category:'retire-js-only-contract-tests',rule:'Retire only contracts for withdrawn JS identities/prototypes/descriptors/call mutation/sparse holes/JS coercion, Context projection, and host/object-heap mechanics. The candidate filename list is a review queue, not a claim every file has been read; decide per script and record rationale.'},
    {category:'rewrite-mathematical-coverage',rule:'Preserve math assertions from mixed scripts in typed C++ tests and Source-locked outputs: parser/package behavior, MML structure/attributes, CommonWrapper/BBox/placement, fonts, SVG bytes, errors, and persistent semantic state. The CMake-linked tests list is the current compiled baseline, not the future acceptance suite.'},
    {category:'retain-development-oracles',rule:'CJS scripts may remain as offline Source oracles/data generators; they are not linked into or executed by native rendering.'},
    {category:'single-target',rule:'Root replaces the current folia_math source graph once with the reviewed graph. Retire old target inputs and tests as batches; add no adapter, alternate target, native2 option, or legacy fallback.'}
  ]
};
const revision = null;
const report = {
  schema:'folia.math.native-raii.inventory.v1',generatedAt:new Date().toISOString(),root:'.',baseRevision:revision,
  scope:{allSrcMathFiles:textFiles.length,codeSources:sourceFiles.length,moduleUnits:sourceFiles.filter(p=>p.endsWith('.ixx')).length,
    implementationUnits:sourceFiles.filter(p=>p.endsWith('.cpp')).length,otherSourceFiles:sourceFiles.filter(p=>! /\.(ixx|cpp)$/.test(p)).length,
    currentCMakeLiteralFoliaMathReferences:mathRefs.size,existingCMakeDirectFoliaMathFiles:existingMathRefs.length,
    currentCMakeResolvedFoliaMathSourceFiles:fullResolvedCMakeMathSourceCount,
    unresolvedFoliaMathSourceVariableExpressions:unresolvedMathSourceVariables.length,
    missingGeneratedFoliaMathSources:missingGeneratedFontSources.length,
    unresolvedCMakeDirectFoliaMathFiles:configuredCodemodel.authoritative?0:null,lexicalCandidateUnresolvedMathReferences:lexicalUnresolvedMathRefs.length,
    moduleImportClosureFiles:configuredCodemodel.authoritative?active.size:null,
    filesOutsideFoliaMathModuleClosure:configuredCodemodel.authoritative?textFiles.filter(p=>!configuredFoliaMathSources.has(p)&&!active.has(p)).length:null,
    allTargetCMakeMentions:cmakeRefs.size,generatedFoliaMathFontSources:generatedFontSources.length,
    lockedButInactiveFontShardCandidates:generatedFontSourceCandidates.length,
    sourceMembershipAuthority:configuredCodemodel.status},
  graph:{cmakeFiles,cmakeMathSources:[...existingMathRefs].sort().map(p=>fileIds.get(p)),unresolvedCMakeMathSources:configuredCodemodel.authoritative?[]:null,
    lexicalCandidateUnresolvedMathReferences:lexicalUnresolvedMathRefs,
    cmakeMathSourceCountBasis:configuredCodemodel.authoritative?'Configured File API Debug folia_math target sources; generated font source membership cross-checked against dependency manifest.':`Incomplete lexical candidate only (${configuredCodemodel.status}); no configured-target or unresolved-import conclusion.`,
    fullResolvedCMakeMathSources:[...existingMathRefs].sort().map(p=>fileIds.get(p)).concat(generatedFontSources.filter(x=>x.exists).map(x=>x.path)),
    resolvedSourceVariableInputs:resolvedSourceVariableInputs.map(input=>({...input,
      generatedCpp:generatedFontSources.map(({path,exists,sha256,bytes,declaration,isInterface,isImplementationUnit,primaryModule,partition,imports,resolvedImports,unresolvedImports})=>
        ({path,exists,sha256,bytes,declaration,isInterface,isImplementationUnit,primaryModule,partition,imports,resolvedImports,unresolvedImports}))})),
    unresolvedSourceVariableExpressions:configuredCodemodel.authoritative?[]:null,
    lexicalCandidateUnresolvedSourceVariableExpressions:unresolvedSourceVariableExpressions,missingGeneratedFoliaMathSources,
    lexicalCandidateMathSourceIdsByTarget:Object.fromEntries(Object.entries(targetSources).map(([k,v])=>[k,[...v].sort().map(p=>fileIds.get(p)).filter(Boolean)])),
    configuredCodemodel:configuredCodemodelSummary(configuredCodemodel),
    lexicalCandidatePathsOutsideMathInventoryByTarget:Object.fromEntries(Object.entries(targetSources).map(([k,v])=>[k,[...v].filter(p=>!fileIds.has(p))]).filter(([,v])=>v.length)),
    generatedSources:generatedFontSources,
    priorFontSourceLockCandidatesWhenInactive:generatedFontSourceCandidates.map(({id,path,sha256,bytes,lock})=>({id,path,sha256,bytes,lock})),
    dynamicCMakeSourceExpressions:{resolved:resolvedSourceVariableInputs.map(x=>({target:x.target,expression:x.expression,manifest:x.origin,manifestSha256:x.manifestSha256,cppCount:x.generatedCpp.length})),unresolved:unresolvedSourceVariableExpressions},
    edgeTupleColumns:['fromFileId','writtenImport','resolvedFileIdOrNull','kind','exported'],
    activeModuleInterfaces:[...activeModuleInterfaces].sort().map(p=>fileIds.has(p)?fileIds.get(p):p),
    activeInterfaceProviders:Object.fromEntries([...interfaceProviders].map(([name,ps])=>[name,ps.map(p=>fileIds.get(p))])),
    duplicateActiveInterfaceProviders, candidateInterfaceProviders:Object.fromEntries([...candidateInterfaceProviders].map(([name,ps])=>[name,ps.map(p=>fileIds.get(p))])),
    implicitPrimaryInterfaceEdges:moduleSourcePaths.filter(p=>activeSourceSet.has(p)&&moduleFacts(p).isImplementationUnit)
      .flatMap(p=>(edges[p]??[]).filter(e=>e.kind==='implicit-primary-interface').map(e=>[fileIds.get(p)??p,fileIds.get(e.file)??e.file])),
    unresolvedActiveImports:configuredCodemodel.authoritative?unresolvedActiveImports:[],lexicalCandidateUnresolvedImports:unresolvedActiveImports,
    importResolutionAuthority:configuredCodemodel.status,cmakeModuleMapPath,cmakeModuleMapEvidence:cmakeModuleMap,interfaceMapCheck,
    moduleImportEdges:moduleSourcePaths.flatMap(p=>moduleFacts(p).imports.map(i=>{
      const resolved=edges[p].find(e=>e.module===i.module);
      return [fileIds.get(p)??p,i.module,resolved?(fileIds.get(resolved.file)??resolved.file):null,i.kind,i.exported];
    })),
    forwardDeclarationTupleColumns:['fileId','symbol','importedModule','importedFileId'],
    forwardDeclarationRisks:forwardDeclarationRisks.map(r=>[fileIds.get(r.file),r.symbol,r.importedModule,fileIds.get(r.importedFile)]),
    fileIdNote:'Each tuple references the id field in files; unresolved imports have null target ids.'},tests:{...tests,observedCMakeTargets:[...new Set(targets)]},files,
  limitations:['Configured membership comes from the latest matching CMake File API Debug codemodel only when its source root matches this repository, target replies parse, input mtimes do not postdate the index, source paths exist, and generated font .cpp membership matches the dependency manifest. Otherwise configured membership is unavailable and the literal CMake scan is an incomplete lexical candidate, not a configuration error. The File API cannot attest non-CMake glob contents or source-file content hashes after configure.',
    'The separate lexical CMake scan follows literal add_subdirectory paths and literal source paths; it does not expand foreach, evaluate conditions, or resolve arbitrary source-list variables. FOLIA_FONT_GENERATED_CPP is resolved from its dependency manifest as an independent candidate input.',
    'The configured mathematical test list comes from all Debug File API target sources under tests/math when available. The separate lexical test scan is a candidate only; historical mathematical test migration remains incomplete.',
    'Only configured folia_math .ixx sources supply active module providers. Implementation units are not import providers; they receive an implicit edge to their primary interface. Lexical fallback providers and unresolved imports are candidate notes only, not compiler/configuration errors. Textual includes are not resolved.',
    'Forward-declaration flags are conservative textual name checks against imported module units, not compiler visibility diagnostics.',
    'Source annotations are a note index harvested from Markdown/header text by path/module and TS path/hash patterns. They do not establish algorithm equivalence, complete Source mapping, or a pass/fail provenance gate.',
    'Responsibilities are narrow filename/location group labels. All non-license file decisions remain unreviewed; active membership does not prove behavioral coverage and inactive membership does not mean dead code.',
    `The ${jsOnlyContractCandidates.length} JavaScript-named scripts are review candidates only; names do not establish their assertions or disposition.`],
  sourceProvenanceGate:{status:'note-index-only',indexedSourceFiles:indexedSourceFiles.length,totalSourceFiles:sourceFiles.length,unindexedSourceFiles:files.filter(f=>sourceFiles.includes(f.path)&&f.sourceAnnotationIndexStatus==='no-exact-note-index-entry').map(f=>f.path),
    limitation:'No completeness or algorithm-equivalence claim; this is not an acceptance gate.'}
};
const compact={schema:'folia.math.native-raii.decision-manifest.v1',snapshotRevision:revision,
  scope:report.scope,limitations:[...report.limitations,'The inventory reader did not configure or build; it reads existing File API and module-map replies.'],sourceProvenanceGate:{...report.sourceProvenanceGate,unindexedSourceFiles:undefined},
  cmakeModuleMapEvidence:cmakeModuleMap,interfaceMapCheck,generatedSourceUnits:generatedSourceRows,
  configuredCodemodel:report.graph.configuredCodemodel,
  resolvedSourceVariableInputs:resolvedSourceVariableInputs.map(({target,variable,expression,origin,sourceLock,manifestSha256,sourceDependencyCount,manifestGeneratedCppCount,excludedNonCppOutputs,generatedCpp})=>({target,variable,expression,origin,sourceLock,manifestSha256,sourceDependencyCount,manifestGeneratedCppCount,excludedNonCppOutputs,generatedCppCount:generatedCpp.length})),
  unresolvedSourceVariableExpressions:report.graph.unresolvedSourceVariableExpressions,
  lexicalCandidateUnresolvedSourceVariableExpressions:unresolvedSourceVariableExpressions,
  files:files.map(f=>({path:f.path,decisionStatus:f.decisionStatus,responsibilityGroup:f.responsibilityGroup,
    namedOwner:f.namedOwner,configuredTargets:f.configuredTargets,directlyListedByFoliaMath:f.directlyListedByFoliaMath,
    sourceAnnotationIndexStatus:f.sourceAnnotationIndexStatus,sourceAnnotationIndex:f.sourceAnnotations.map(a=>({document:a.document,line:a.line,sourcePaths:a.sourcePaths,sourceSha256:a.sourceSha256,status:a.status}))}))};
fs.mkdirSync(path.dirname(out),{recursive:true});
fs.mkdirSync(path.dirname(manifestOut),{recursive:true});
fs.writeFileSync(out,JSON.stringify(report,null,2)+'\n');
fs.writeFileSync(manifestOut,JSON.stringify(compact,null,2)+'\n');
