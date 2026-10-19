#!/usr/bin/env node
'use strict';

// Static inventory for the Native RAII math replacement. This is deliberately
// a proposal generator: it records observable source facts and labels uncertain
// disposition/provenance instead of asserting that names prove reuse.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const cp = require('node:child_process');

const root = path.resolve(__dirname, path.basename(__dirname) === 'inventory' ? '../../../../..' : '../..');
const args=process.argv.slice(2);
const option=(name,fallback)=>{const i=args.indexOf(name);return i>=0&&args[i+1]?path.resolve(args[i+1]):fallback;};
const out=option('--report',path.join(root,'build/math-port/native-raii-inventory.json'));
const manifestOut=option('--manifest',path.join(root,'docs/math-port/native-raii-files.json'));
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
  return 'passed 8 assertions';
}
if(args.includes('--self-test')) { process.stdout.write(`${runSelfTests()}\n`); process.exit(0); }
const trackedUnder = dir => cp.execFileSync('git',['ls-files','--',dir],{cwd:root,encoding:'utf8'}).split(/\r?\n/).filter(Boolean);
const textFiles = list('src/math');
const sourceFiles = textFiles.filter(isSourceFile);
const fileIds = new Map(textFiles.map((p,i)=>[p,`F${String(i+1).padStart(3,'0')}`]));

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

function disposition(p) {
  const n = p.toLowerCase();
  if (/\/license-[^/]+\.txt$|\/notice\.txt$/.test(n))
    return ['keep','Preserve the required third-party license/notice text with the retained source/data.','Root (license provenance)'];
  if (n.includes('/dependencies/mhchem_parser/'))
    return ['rewrite','Native typed chemistry parser/helper algorithms used by the pinned mhchem package; verify exact upstream source mapping and behavior.','session-parser'];
  if (/src\/math\/(output\/common|fonts)\/(font_data_options|font_data_bindings|font_data_surface|bbox_bindings|common_bindings|wrapper_class_bindings|factory_context_fields|newcm_font_data_surface)\./.test(n))
    return ['remove','Context/Value/identity projection surface withdrawn; its typed service/data behavior belongs in the native repository, Scene, or serializer contract.','common-font-audit'];
  if (/src\/math\/output\/common\/line_bbox\./.test(n))
    return ['rewrite','Typed line-indent/BBox math using native node and layout contracts.','common-font-audit'];
  if (/src\/math\/output\/common\/font_data\./.test(n))
    return ['rewrite','Canonical typed immutable FontRepository and FontFaceData service, with no Property/Context representation.','common-font-audit'];
  if (/src\/math\/util\/regexp_host\./.test(n))
    return ['rewrite','Typed Unicode regular-expression service for the native math profile.','raii-regex'];
  if (/src\/math\/fonts\/newcm_font_(data|module_provider)\./.test(n))
    return ['rewrite','Typed immutable NewCM provider/installer using locked data; generated payload comes from the audited generator.','common-font-audit'];
  if (/src\/math\/fonts\/svg_extensions\./.test(n))
    return ['rewrite','Rewrite extension maps as typed values owned by common font face data; only immutable table payload may be reused.','common-font-audit'];
  if (/src\/math\/core\/mml_tree\/(?:operator_dictionary_data|mo_pattern_data)\.cpp$/.test(n))
    return n.endsWith('mo_pattern_data.cpp')
      ? ['regenerate','Regenerate the immutable MathJax operator-pattern table from its pinned source and reviewed generator.','mml']
      : ['rewrite','Generated operator definitions currently instantiate legacy Property graphs; emit typed immutable operator records instead.','mml'];
  if (/src\/math\/input\/tex\/textcomp\/mapping_data\.inc$/.test(n))
    return ['regenerate','Regenerate the locked TextcompMappings table as data-only output.','session-parser'];
  if (/src\/math\/util\/(?:unicode_normalization_data\.ixx|js_case_data\.cpp)$/.test(n))
    return ['regenerate','Regenerate the immutable Unicode mapping table from its pinned Unicode data inputs.','common-font-audit'];
  if (/src\/math\/output\/common\/(wrapper|wrapper_factory|output_jax|linebreak_visitor)\.|src\/math\/output\/common\/wrappers\//.test(n))
    return ['rewrite','Typed CommonWrapper/factory and per-family measurement/placement; move family algorithms into output/common/placement/<family>.','common-font-audit'];
  if (/src\/math\/output\/scene\/wrappers\//.test(n))
    return ['rewrite','Transfer mathematical family placement to output/common/placement/<family>; Scene only receives completed owned geometry.','common-font-audit'];
  if (/\.(txt|inc)$/.test(n) && (/font|operator|pattern|unicode|entity|data/.test(n)))
    return ['keep','Immutable mathematical/font/Unicode/operator data; verify provenance and representation before retaining.','common-font-audit'];
  if (/src\/math\/(?:util\/.*_data\.inc|fonts\/.*_data\.inc|dependencies\/mhchem_parser\/.*_data\.inc)$/.test(n))
    return ['keep','Immutable table payload; retain as data only after its exact Source mapping/license and import-free representation are confirmed.','common-font-audit'];
  if (/context|binding|identity|projection|property|object|option_(types|storage|object|array|iterator)|callable|runtime_modules|lite|dom_adaptor|xml_dom|(?:^|\/)core\/tree\//.test(n))
    return ['remove','Withdrawn runtime object graph, DOM/adaptor projection, or host-only compatibility layer; mathematical behavior must move to typed native owners where needed.','root-retirement'];
  if (/(^|\/)host(?:[./]|$)|_host\./.test(n))
    return ['remove','Withdrawn host projection/Context bridge; port any required mathematical behavior to the owning typed module.','root-retirement'];
  if (n.includes('/output/svg/')) return ['rewrite','Source-compatible SVG serializer/metadata consumer over completed immutable Scene; no layout or mutable MML dependency.','svg'];
  if (n.includes('/output/scene/')) return ['rewrite','Replace duplicate wrapper/identity projection with owned immutable Scene and one layout pass; placement ownership pending common audit.','scene'];
  if (n.includes('/output/common/') || n.includes('/fonts/') || n.includes('/output/common')) return ['rewrite','Typed CommonWrapper/BBox/font/placement service; one measurement and placement pass, audited per file before reuse.','common-font-audit'];
  if (n.includes('/core/mml_tree/')) return ['rewrite','Typed MML node family, attributes, factory, and document-local tree operations.','mml'];
  if (n.includes('/input/tex/')) return ['rewrite','Native typed TeX parser/package behavior and explicit renderer session state.','session-parser'];
  if (n.includes('/util/')) return ['rewrite','Keep only pure mathematical/text utilities after transitive import audit; replace JS coercion/object helpers with typed domain APIs.','common-font-audit'];
  if (n.includes('/input/tex/')) return ['rewrite','Native typed TeX parser/package behavior and explicit renderer session state.','session-parser'];
  if (n.includes('/core/')) return ['rewrite','Renderer/document lifecycle and typed orchestration around request-owned MML, Scene, and C ABI.','Root (pipeline integration)'];
  if (n.includes('/handlers/html/')) return ['rewrite','Owned HTML and annotation content staging, measured into the immutable Scene.','scene'];
  if (n.endsWith('/native_renderer_bridge.cpp')) return ['rewrite','Existing plain C ABI bridge to the single folia_math target; keep platform-specific integration outside the core pipeline.','Root (pipeline integration)'];
  if (n.includes('/math.cpp') || n.includes('/math.ixx')) return ['rewrite','Public Renderer facade and request-pipeline orchestration only.','Root (pipeline integration)'];
  return ['rewrite','Assign to the named Root pipeline owner for canonical-path decomposition and target retirement review.','Root (pipeline integration)'];
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
const unresolvedMathRefs=unresolvedRefs(mathRefs,new Set(fileIds.keys()));
const existingMathRefs=[...mathRefs].filter(p=>fileIds.has(p));
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
const activeModuleInterfaces=new Set();
for (const body of targetBodies('folia_math')) {
  if (!/FILE_SET\s+CXX_MODULES\s+FILES/.test(body)) continue;
  for (const m of body.matchAll(/src\/math\/[A-Za-z0-9_./-]+\.ixx/g)) activeModuleInterfaces.add(m[0]);
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
const interfaceMapCheck={available:!!cmakeModuleMap&&!cmakeModuleMap.error,
  generatedModuleCount:cmakeInterfaceNames.length,registeredInterfaceCount:activeInterfaceNames.length,
  matches:!!cmakeModuleMap&&!cmakeModuleMap.error&&JSON.stringify(activeInterfaceNames)===JSON.stringify(cmakeInterfaceNames),
  missingFromGenerated:activeInterfaceNames.filter(x=>!cmakeInterfaceNames.includes(x)),
  unexpectedInGenerated:cmakeInterfaceNames.filter(x=>!activeInterfaceNames.includes(x))};
const edges = {};
const unresolvedActiveImports=[];
const activeSourceSet=new Set(existingMathRefs);
for (const p of sourceFiles) {
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
const active = new Set(existingMathRefs);
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
  const [decision,responsibility,owner] = disposition(p), facts = moduleFacts(p), bytes = fs.readFileSync(path.join(root,p));
  const annotations=sourceAnnotations(p,facts);
  const exactSources=[...new Map(annotations.filter(a=>a.sourceSha256).flatMap(a=>a.sourcePaths.filter(s=>s.includes('ts/')).map(s=>[`${s}:${a.sourceSha256}`,{path:s,sha256:a.sourceSha256,document:a.document,line:a.line}]))).values()];
  const pinnedSource=exactSources.length===1
    ? `MathJax 4.1.3 ${exactSources[0].path} sha256:${exactSources[0].sha256} (annotated in ${exactSources[0].document}:${exactSources[0].line})`
    : exactSources.length>1
      ? 'multiple annotated Source inputs; see sourceAnnotations for paths, hashes, and evidence lines'
      : /\/LICENSE-[^/]+\.txt$|\/NOTICE\.txt$/i.test(p)
        ? 'Third-party license/notice file; preserve alongside associated source/data.'
        : /src\/math\/fonts\/newcm_font_|src\/math\/fonts\/svg_extensions\./.test(p)
          ? 'docs/math-port/evidence/font-data-source-lock.json (NewCM payload lock; exact native-unit mapping unresolved)'
          : 'unresolved: exact pinned MathJax 4.1.3 source path/hash not established by this static inventory';
  return {id:fileIds.get(p),path:p,sha256:sha(bytes),bytes:bytes.length,kind:path.extname(p).slice(1),...facts,
    directlyListedByFoliaMath:mathRefs.has(p),activeModuleImportClosure:active.has(p),mentionedByCMake:cmakeRefs.has(p),
    directCMakeTargets:Object.entries(targetSources).filter(([,files])=>files.has(p)).map(([target])=>target),
    disposition:decision,nativeResponsibility:responsibility,sourceAnnotations:annotations,pinnedSource,namedOwner:owner,
    confidence:'proposed; review disposition and source provenance before promotion'};
});
const mappedSourceFiles=files.filter(f=>sourceFiles.includes(f.path)&&f.pinnedSource.startsWith('MathJax 4.1.3 '));

const fontLockPath='docs/math-port/evidence/font-data-source-lock.json';
const fontLock=JSON.parse(read(fontLockPath));
const generatedFontSourceCandidates=fontLock.outputs.filter(x=>/^build\/math-port\/font-generated\/newcm_dynamic_setup_[A-Za-z0-9_]+\.cpp$/.test(x.path))
  .map((x,i)=>({id:`G${String(i+1).padStart(2,'0')}`,target:'folia_math',path:x.path,sha256:x.sha256,bytes:x.bytes,lock:fontLockPath}));
const activeFontShardGeneration=/FOLIA_MATH_FONT_SHARD_SOURCES/.test(cmakeText);
const generatedFontSources=activeFontShardGeneration?generatedFontSourceCandidates:[];

const targets = [...cmakeText.matchAll(/add_(?:library|executable)\s*\(\s*([A-Za-z0-9_]+)/g)].map(m=>m[1]);
const portScripts = trackedUnder('tools/math-port').filter(p=>p.endsWith('.cjs'));
const jsOnlyContractCandidates = portScripts.filter(p=>/(?:mutation-test|\b(?:native-identities|option-array|option-filter|property-symbol|sparse-(?:tree|node)|callable-receiver)|(?:prototype|descriptor|identity|context-property)).*\.cjs$/i.test(p));
const tests = {
  currentCMakeMathTestSources:[...testRefs].sort(),
  javascriptContractReviewCandidates:jsOnlyContractCandidates,
  retirementPlan:[
    {category:'retire-js-only-contract-tests',rule:'Retire only contracts for withdrawn JS identities/prototypes/descriptors/call mutation/sparse holes/JS coercion, Context projection, and host/object-heap mechanics. The candidate filename list is a review queue, not a claim every file has been read; decide per script and record rationale.'},
    {category:'rewrite-mathematical-coverage',rule:'Preserve math assertions from mixed scripts in typed C++ tests and Source-locked outputs: parser/package behavior, MML structure/attributes, CommonWrapper/BBox/placement, fonts, SVG bytes, errors, and persistent semantic state. The CMake-linked tests list is the current compiled baseline, not the future acceptance suite.'},
    {category:'retain-development-oracles',rule:'CJS scripts may remain as offline Source oracles/data generators; they are not linked into or executed by native rendering.'},
    {category:'single-target',rule:'Root replaces the current folia_math source graph once with the reviewed graph. Retire old target inputs and tests as batches; add no adapter, alternate target, native2 option, or legacy fallback.'}
  ]
};
let revision = null;
try { revision = cp.execFileSync('git',['rev-parse','HEAD'],{cwd:root,encoding:'utf8'}).trim(); } catch {}
const report = {
  schema:'folia.math.native-raii.inventory.v1',generatedAt:new Date().toISOString(),root:'.',baseRevision:revision,
  scope:{allSrcMathFiles:textFiles.length,codeSources:sourceFiles.length,moduleUnits:sourceFiles.filter(p=>p.endsWith('.ixx')).length,
    implementationUnits:sourceFiles.filter(p=>p.endsWith('.cpp')).length,otherSourceFiles:sourceFiles.filter(p=>! /\.(ixx|cpp)$/.test(p)).length,
    currentCMakeDirectFoliaMathReferences:mathRefs.size,existingCMakeDirectFoliaMathFiles:existingMathRefs.length,
    unresolvedCMakeDirectFoliaMathFiles:unresolvedMathRefs.length,moduleImportClosureFiles:active.size,
    filesOutsideFoliaMathModuleClosure:textFiles.filter(p=>!mathRefs.has(p)&&!active.has(p)).length,
    allTargetCMakeMentions:cmakeRefs.size,generatedFoliaMathFontSources:generatedFontSources.length,
    lockedButInactiveFontShardCandidates:generatedFontSourceCandidates.length-generatedFontSources.length},
  graph:{cmakeFiles,cmakeMathSources:[...existingMathRefs].sort().map(p=>fileIds.get(p)),unresolvedCMakeMathSources:unresolvedMathRefs,
    cmakeSourcesByTarget:Object.fromEntries(Object.entries(targetSources).map(([k,v])=>[k,[...v].sort().map(p=>fileIds.get(p)).filter(Boolean)])),
    unresolvedSourcesByTarget:Object.fromEntries(Object.entries(targetSources).map(([k,v])=>[k,[...v].filter(p=>!fileIds.has(p))]).filter(([,v])=>v.length)),
    generatedSources:generatedFontSources,
    priorFontSourceLockCandidatesWhenInactive:activeFontShardGeneration?[]:generatedFontSourceCandidates.map(({id,path,sha256,bytes,lock})=>({id,path,sha256,bytes,lock})),
    dynamicCMakeSourceExpressions:activeFontShardGeneration?['FOLIA_MATH_FONT_SHARD_SOURCES is populated from docs/math-port/evidence/font-data-source-lock.json outputs and appended to folia_math; exact generated output paths and lock hashes are in generatedSources.']:[],
    edgeTupleColumns:['fromFileId','writtenImport','resolvedFileIdOrNull','kind','exported'],
    activeModuleInterfaces:[...activeModuleInterfaces].sort().map(p=>fileIds.has(p)?fileIds.get(p):p),
    activeInterfaceProviders:Object.fromEntries([...interfaceProviders].map(([name,ps])=>[name,ps.map(p=>fileIds.get(p))])),
    duplicateActiveInterfaceProviders, candidateInterfaceProviders:Object.fromEntries([...candidateInterfaceProviders].map(([name,ps])=>[name,ps.map(p=>fileIds.get(p))])),
    implicitPrimaryInterfaceEdges:sourceFiles.filter(p=>existingMathRefs.includes(p)&&moduleFacts(p).isImplementationUnit)
      .flatMap(p=>(edges[p]??[]).filter(e=>e.kind==='implicit-primary-interface').map(e=>[fileIds.get(p),fileIds.get(e.file)])),
    unresolvedActiveImports,cmakeModuleMapPath,cmakeModuleMapEvidence:cmakeModuleMap,interfaceMapCheck,
    moduleImportEdges:sourceFiles.flatMap(p=>moduleFacts(p).imports.map(i=>{
      const resolved=edges[p].find(e=>e.module===i.module);
      return [fileIds.get(p),i.module,resolved?fileIds.get(resolved.file):null,i.kind,i.exported];
    })),
    forwardDeclarationTupleColumns:['fileId','symbol','importedModule','importedFileId'],
    forwardDeclarationRisks:forwardDeclarationRisks.map(r=>[fileIds.get(r.file),r.symbol,r.importedModule,fileIds.get(r.importedFile)]),
    fileIdNote:'Each tuple references the id field in files; unresolved imports have null target ids.'},tests:{...tests,observedCMakeTargets:[...new Set(targets)]},files,
  limitations:['CMake inventory starts at the working-tree root CMakeLists.txt and follows literal add_subdirectory paths only; it never scans ignored candidate or draft CMake overlays. Conditions, generator expressions, and variable-defined target sources are not evaluated.',
    'The FoliaTests recursive source glob is expanded against present working-tree files, excluding tests/platform, tests/encoding and tests/math as the root CMake does. Historical mathematical test migration remains incomplete.',
    'Active module providers are limited to registered folia_math CXX_MODULES FILE_SET interfaces. Implementation units are not import providers; they receive an implicit edge to their registered primary interface. Missing active providers are reported as configuration errors. Candidate interfaces from inactive files are separate and never satisfy active closure imports. Textual includes and other generated dependencies are not resolved.',
    'Forward-declaration flags are conservative textual name checks against imported module units, not compiler visibility diagnostics.',
    'Source mappings are harvested only from tracked Markdown lines that explicitly mention a native path/module and a pinned TS path/hash. Missing mappings remain unresolved; existing notes may be partial.',
    `Disposition and owner fields are proposals. The ${jsOnlyContractCandidates.length} JavaScript filename candidates are a review queue, not a blanket claim that every candidate is JS-only; mixed mathematical assertions must be rewritten and retained.`],
  sourceProvenanceGate:{status:mappedSourceFiles.length===sourceFiles.length?'pass':'fail-incomplete',mappedFiles:mappedSourceFiles.length,totalSourceFiles:sourceFiles.length,unmappedSourceFiles:files.filter(f=>sourceFiles.includes(f.path)&&!f.pinnedSource.startsWith('MathJax 4.1.3 ')).map(f=>f.path)}
};
const compact={schema:'folia.math.native-raii.decision-manifest.v1',snapshotRevision:revision,
  scope:report.scope,limitations:[...report.limitations,'Generated module map was read only; no build or CMake generation was invoked.'],sourceProvenanceGate:{...report.sourceProvenanceGate,unmappedSourceFiles:undefined},
  cmakeModuleMapEvidence:cmakeModuleMap,interfaceMapCheck,files:files.map(f=>({path:f.path,disposition:f.disposition,nativeResponsibility:f.nativeResponsibility,
    namedOwner:f.namedOwner,availableSourceProvenance:f.sourceAnnotations.map(a=>({document:a.document,line:a.line,sourcePaths:a.sourcePaths,sourceSha256:a.sourceSha256,status:a.status}))}))};
fs.mkdirSync(path.dirname(out),{recursive:true});
fs.mkdirSync(path.dirname(manifestOut),{recursive:true});
fs.writeFileSync(out,JSON.stringify(report,null,2)+'\n');
fs.writeFileSync(manifestOut,JSON.stringify(compact,null,2)+'\n');
