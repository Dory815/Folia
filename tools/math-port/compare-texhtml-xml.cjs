'use strict';

// Strict comparison for the TexHtml, HTMLDomStrings, and XML wrapper ports.
// Source records are regenerated from the pinned MathJax package in isolated
// Node processes; native JSON remains an input produced by the native runner.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const cp = require('node:child_process');

const root = path.resolve(__dirname, '../..');
const lockPath = path.join(root, 'docs/math-port/evidence/texhtml-xml-source-lock.json');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const [texNativeArg, xmlNativeArg, reportArg] = process.argv.slice(2);
if (!texNativeArg || !xmlNativeArg) {
  throw new Error('Usage: node tools/math-port/compare-texhtml-xml.cjs <texhtml-native.json> <xml-native.json> [report.json]');
}
const nativeTexPath = path.resolve(root, texNativeArg);
const nativeXmlPath = path.resolve(root, xmlNativeArg);
const reportPath = path.resolve(root, reportArg || 'build/math-port/texhtml-xml-comparison.json');
const digestBytes = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const digestFile = file => digestBytes(fs.readFileSync(file));
const sameJson = (a, b) => JSON.stringify(a) === JSON.stringify(b);
const failures = [];
const provenance = { lock: { path: path.relative(root, lockPath), sha256: digestFile(lockPath) },
  nativeInputs: [], originalOracles: [], lockedDependencies: { typescript: [], cjs: [] } };
const fail = (record, difference) => failures.push({ record, difference });

function orderedDiff(expected, actual, at, out = []) {
  if (Object.is(expected, actual)) return out;
  if (expected === null || actual === null || typeof expected !== 'object' || typeof actual !== 'object') {
    out.push({ path: at, source: expected, native: actual });
    return out;
  }
  if (Array.isArray(expected) || Array.isArray(actual)) {
    if (!Array.isArray(expected) || !Array.isArray(actual)) {
      out.push({ path: at, sourceType: Array.isArray(expected) ? 'array' : typeof expected,
        nativeType: Array.isArray(actual) ? 'array' : typeof actual });
      return out;
    }
    if (expected.length !== actual.length)
      out.push({ path: `${at}.length`, source: expected.length, native: actual.length });
    for (let i = 0; i < Math.min(expected.length, actual.length); i++)
      orderedDiff(expected[i], actual[i], `${at}[${i}]`, out);
    return out;
  }
  const expectedKeys = Object.keys(expected), actualKeys = Object.keys(actual);
  if (!sameJson(expectedKeys, actualKeys))
    out.push({ path: `${at}.[key-order]`, source: expectedKeys, native: actualKeys });
  const actualKeySet = new Set(actualKeys);
  for (const key of expectedKeys) {
    if (!actualKeySet.has(key)) continue;
    orderedDiff(expected[key], actual[key], `${at}.${key}`, out);
  }
  for (const key of actualKeys) {
    if (!Object.hasOwn(expected, key)) out.push({ path: `${at}.${key}`, source: { $: 'missing' }, native: actual[key] });
  }
  return out;
}

function validateLock(lock) {
  const expectedMeta = { schema: 1, reference: 'MathJax 4.1.3' };
  for (const [key, value] of Object.entries(expectedMeta)) {
    if (lock[key] !== value) fail(`lock.${key}`, { expected: value, actual: lock[key] });
  }
  const scopePath = path.join(root, 'docs/math-port/scope.json');
  if (!fs.existsSync(scopePath)) fail('lock.scope.json', { missing: scopePath });
  else if (digestFile(scopePath) !== lock.scopeSha256)
    fail('lock.scopeSha256', { expected: lock.scopeSha256, actual: digestFile(scopePath) });

  if (!Array.isArray(lock.sources)) fail('lock.sources', { expected: 'array', actual: typeof lock.sources });
  else for (const item of lock.sources) {
    const upstream = path.join(sourceRoot, item.file);
    const installed = path.join(packageRoot, item.file);
    if (!fs.existsSync(upstream)) { fail(`locked TS ${item.file}`, { missing: upstream }); continue; }
    const actual = digestFile(upstream);
    if (actual !== item.sha256) fail(`locked TS ${item.file}`, { expected: item.sha256, actual });
    provenance.lockedDependencies.typescript.push({ file: item.file, sha256: actual });
    if (!fs.existsSync(installed)) { fail(`installed TS ${item.file}`, { missing: installed }); continue; }
    const sourceText = fs.readFileSync(upstream, 'utf8').replaceAll('\r\n', '\n');
    const installedText = fs.readFileSync(installed, 'utf8').replaceAll('\r\n', '\n');
    if (installedText !== sourceText) fail(`installed TS ${item.file}`, { differsFrom: path.relative(root, upstream) });
  }
  if (!Array.isArray(lock.cjsDependencies)) fail('lock.cjsDependencies', { expected: 'array', actual: typeof lock.cjsDependencies });
  else for (const item of lock.cjsDependencies) {
    const file = path.join(packageRoot, item.file.replace(/^cjs\//, 'cjs/'));
    if (!fs.existsSync(file)) { fail(`locked CJS ${item.file}`, { missing: file }); continue; }
    const actual = digestFile(file);
    if (actual !== item.sha256) fail(`locked CJS ${item.file}`, { expected: item.sha256, actual });
    provenance.lockedDependencies.cjs.push({ file: item.file, sha256: actual });
  }
}

function generateOracle(script, outputName) {
  const scriptPath = path.join(root, script);
  const outputPath = path.join(root, 'build/math-port/texhtml-xml-comparer', outputName);
  fs.mkdirSync(path.dirname(outputPath), { recursive: true });
  try {
    const stdout = cp.execFileSync(process.execPath, [scriptPath, path.relative(root, outputPath)],
      { cwd: root, encoding: 'utf8', stdio: ['ignore', 'pipe', 'pipe'] });
    const record = { script, scriptSha256: digestFile(scriptPath), output: path.relative(root, outputPath),
      outputSha256: digestFile(outputPath), stdout: stdout.trim() };
    provenance.originalOracles.push(record);
    return JSON.parse(fs.readFileSync(outputPath, 'utf8'));
  } catch (error) {
    fail(`oracle ${script}`, { message: error.message, stderr: error.stderr?.toString() || '' });
    return null;
  }
}

function validateOriginal(label, source, expectedKeys, lock, loadedFiles=true) {
  if (!source) return;
  if (!sameJson(Object.keys(source), expectedKeys))
    fail(`${label} envelope keys`, { expected: expectedKeys, actual: Object.keys(source) });
  for (const [key, value] of Object.entries({ schema: lock.schema, reference: lock.reference,
    commit: lock.commit, scopeSha256: lock.scopeSha256, sourceHashes: lock.sources })) {
    const differences = orderedDiff(value, source[key], `${label}.${key}`);
    for (const difference of differences) fail(`${label} metadata`, difference);
  }
  if (loadedFiles) {
    if (!Array.isArray(source.loadedFiles)) { fail(`${label}.loadedFiles`, { expected: 'array' }); return; }
    const byFile = new Map(lock.cjsDependencies.map(item => [item.file, item.sha256]));
    const seen = new Set();
    for (const item of source.loadedFiles) {
      if (seen.has(item.file)) fail(`${label}.loadedFiles`, { duplicate: item.file });
      seen.add(item.file);
      if (byFile.get(item.file) !== item.sha256)
        fail(`${label}.loadedFiles.${item.file}`, { expected: byFile.get(item.file), actual: item.sha256 });
      const actualPath = path.join(packageRoot, item.file);
      if (!fs.existsSync(actualPath)) fail(`${label}.loadedFiles.${item.file}`, { missing: actualPath });
      else if (digestFile(actualPath) !== item.sha256)
        fail(`${label}.loadedFiles.${item.file}`, { expected: item.sha256, actual: digestFile(actualPath) });
    }
    const sorted = [...source.loadedFiles].sort((a, b) => a.file.localeCompare(b.file));
    if (!sameJson(sorted, source.loadedFiles)) fail(`${label}.loadedFiles order`, { expected: sorted.map(x => x.file), actual: source.loadedFiles.map(x => x.file) });
  }
}

function readNative(file, label) {
  try {
    const bytes = fs.readFileSync(file);
    provenance.nativeInputs.push({ label, path: path.relative(root, file), sha256: digestBytes(bytes) });
    return JSON.parse(bytes.toString('utf8'));
  } catch (error) {
    fail(`${label} input`, { path: file, message: error.message });
    return null;
  }
}

const lock = JSON.parse(fs.readFileSync(lockPath, 'utf8'));
validateLock(lock);
const texSource = generateOracle('tools/math-port/texhtml-original-oracle.cjs', 'texhtml-original.json');
const domSource = generateOracle('tools/math-port/html-dom-strings-original-oracle.cjs', 'html-dom-strings-original.json');
const xmlSource = generateOracle('tools/math-port/texhtml-xml-original-oracle.cjs', 'texhtml-xml-original.json');
validateOriginal('texhtml source', texSource,
  ['schema', 'reference', 'commit', 'scopeSha256', 'sourceHashes', 'loadedFiles', 'cases'], lock);
validateOriginal('HTMLDomStrings source', domSource,
  ['schema', 'reference', 'commit', 'scopeSha256', 'sourceHashes', 'defaultsBefore', 'configReplacement',
    'registeredAfterConfig', 'instanceOptions', 'nestedMarkers', 'loadedFiles'], lock);
validateOriginal('XML source', xmlSource,
  ['schema', 'reference', 'commit', 'scopeSha256', 'sourceHashes', 'loadedFiles', 'cases'], lock);

const nativeTex = readNative(nativeTexPath, 'TexHtml/HTMLDomStrings native');
const nativeXml = readNative(nativeXmlPath, 'XML native');
const texCandidate = path.join(root, 'tools/math-port/drafts/texhtml-native-candidate.cpp');
const xmlCandidate = path.join(root, 'tools/math-port/drafts/texhtml-xml-native-candidate.cpp');
for (const [label, file] of [['TexHtml native candidate source', texCandidate], ['XML native candidate source', xmlCandidate]]) {
  if (fs.existsSync(file)) provenance.nativeInputs.push({ label, path: path.relative(root, file), sha256: digestFile(file) });
  else fail(label, { missing: file });
}

function compareCaseSet(label, expected, actual) {
  if (!Array.isArray(expected) || !Array.isArray(actual)) {
    fail(`${label}.cases`, { sourceIsArray: Array.isArray(expected), nativeIsArray: Array.isArray(actual) });
    return;
  }
  if (expected.length !== actual.length) fail(`${label}.cases.length`, { source: expected.length, native: actual.length });
  for (let i = 0; i < Math.max(expected.length, actual.length); i++) {
    if (i >= expected.length) { fail(`${label}.cases[${i}]`, { unexpectedNativeRecord: actual[i] }); continue; }
    if (i >= actual.length) { fail(`${label}.cases[${i}]`, { missingNativeRecord: expected[i] }); continue; }
    const recordName = expected[i]?.name || `record-${i}`;
    const differences = orderedDiff(expected[i], actual[i], `${label}.cases[${i}](${recordName})`);
    for (const difference of differences) fail(`${label} record ${recordName}`, difference);
  }
}

if (texSource && nativeTex) {
  const meta = { schema: texSource.schema, reference: texSource.reference, commit: texSource.commit,
    scopeSha256: texSource.scopeSha256, sourceHashes: texSource.sourceHashes };
  const expectedKeys = [...Object.keys(meta), 'texhtml', 'domStrings'];
  for (const difference of orderedDiff(expectedKeys, Object.keys(nativeTex), 'nativeTex.[key-order]'))
    fail('TexHtml native envelope', difference);
  for (const [key, value] of Object.entries(meta))
    for (const difference of orderedDiff(value, nativeTex[key], `nativeTex.${key}`)) fail('TexHtml native metadata', difference);
  const texCases = nativeTex.texhtml?.cases;
  compareCaseSet('TexHtml', texSource.cases, texCases);
  const nativeDomCases = nativeTex.domStrings?.cases;
  if (!Array.isArray(nativeDomCases) || nativeDomCases.length !== 2) {
    fail('HTMLDomStrings native cases', { expected: 2, actual: Array.isArray(nativeDomCases) ? nativeDomCases.length : typeof nativeDomCases });
  } else {
    const combined = {};
    for (const key of ['defaultsBefore', 'configReplacement', 'registeredAfterConfig', 'instanceOptions']) {
      if (Object.hasOwn(nativeDomCases[0], key)) combined[key] = nativeDomCases[0][key];
    }
    if (Object.hasOwn(nativeDomCases[1], 'nestedMarkers')) combined.nestedMarkers = nativeDomCases[1].nestedMarkers;
    const expectedDom = {};
    for (const key of ['defaultsBefore', 'configReplacement', 'registeredAfterConfig', 'instanceOptions', 'nestedMarkers'])
      if (Object.hasOwn(domSource || {}, key)) expectedDom[key] = domSource[key];
    for (const difference of orderedDiff(expectedDom, combined, 'HTMLDomStrings'))
      fail('HTMLDomStrings native records', difference);
    for (const [index, keys] of [[0, ['defaultsBefore', 'configReplacement', 'registeredAfterConfig', 'instanceOptions']],
      [1, ['nestedMarkers']]]) {
      const actualKeys = Object.keys(nativeDomCases[index]);
      if (!sameJson(actualKeys, keys)) fail(`HTMLDomStrings native case ${index} keys`, { expected: keys, actual: actualKeys });
    }
  }
  if (!nativeTex.texhtml || !sameJson(Object.keys(nativeTex.texhtml), ['cases']))
    fail('TexHtml native group keys', { expected: ['cases'], actual: nativeTex.texhtml && Object.keys(nativeTex.texhtml) });
  if (!nativeTex.domStrings || !sameJson(Object.keys(nativeTex.domStrings), ['cases']))
    fail('HTMLDomStrings native group keys', { expected: ['cases'], actual: nativeTex.domStrings && Object.keys(nativeTex.domStrings) });
}

if (xmlSource && nativeXml) {
  if (!sameJson(Object.keys(nativeXml), ['cases']))
    fail('XML native envelope keys', { expected: ['cases'], actual: Object.keys(nativeXml) });
  compareCaseSet('XML', xmlSource.cases, nativeXml.cases);
}

const report = {
  accepted: failures.length === 0,
  recordTotals: {
    texhtml: texSource?.cases?.length || 0,
    domStrings: 2,
    xml: xmlSource?.cases?.length || 0,
    failures: failures.length,
  },
  provenance,
  failures,
};
fs.mkdirSync(path.dirname(reportPath), { recursive: true });
fs.writeFileSync(reportPath, JSON.stringify(report, null, 2) + '\n');
console.log(JSON.stringify({ accepted: report.accepted, recordTotals: report.recordTotals,
  failures: failures.length, firstFailure: failures[0] || null, report: path.relative(root, reportPath) }));
process.exitCode = report.accepted ? 0 : 1;
