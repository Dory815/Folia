#!/usr/bin/env node
'use strict';

// Strictly compare every observed provider field, including all module states,
// ordered events, and loaded glyph data. No summary-only or normalization path.
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
const sourcePath = path.resolve(process.argv[2] || path.join(root, 'build/math-port/newcm-font-module-provider-original.json'));
const runPath = path.resolve(process.argv[3] || path.join(root, 'build/math-port/newcm-font-provider-acceptance-native.json'));
const source = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
const run = JSON.parse(fs.readFileSync(runPath, 'utf8'));

function diff(a, b, p='$') {
  if (Object.is(a, b)) return null;
  if (a === null || b === null || typeof a !== 'object' || typeof b !== 'object')
    return {path:p, expected:a, actual:b};
  if (Array.isArray(a) !== Array.isArray(b))
    return {path:p, expected:Array.isArray(a)?'array':typeof a, actual:Array.isArray(b)?'array':typeof b};
  const ak = Object.keys(a), bk = Object.keys(b);
  for (const key of ak) {
    if (!Object.hasOwn(b, key)) return {path:`${p}.${key}`, expected:a[key], actual:'<missing>'};
    const d = diff(a[key], b[key], `${p}.${key}`);
    if (d) return d;
  }
  for (const key of bk) if (!Object.hasOwn(a, key))
    return {path:`${p}.${key}`, expected:'<missing>', actual:b[key]};
  return null;
}

const errors = [];
if (run.exitCode !== 0) errors.push({path:'$.execution.exitCode', expected:0, actual:run.exitCode});
if (run.signal !== null) errors.push({path:'$.execution.signal', expected:null, actual:run.signal});
if (run.error !== null) errors.push({path:'$.execution.error', expected:null, actual:run.error});
if (run.parseError !== null) errors.push({path:'$.execution.parseError', expected:null, actual:run.parseError});
if (!run.executableSha256 || !/^[0-9a-f]{64}$/.test(run.executableSha256))
  errors.push({path:'$.execution.executableSha256', expected:'SHA-256', actual:run.executableSha256});
if (!Number.isInteger(run.pid) || run.pid <= 0)
  errors.push({path:'$.execution.pid', expected:'positive process id', actual:run.pid});
if (!Number.isFinite(Date.parse(run.startedUtc)) || !Number.isFinite(Date.parse(run.finishedUtc)) ||
    !run.startedUtc?.endsWith('Z') || !run.finishedUtc?.endsWith('Z'))
  errors.push({path:'$.execution.timestamps', expected:'UTC timestamps', actual:[run.startedUtc,run.finishedUtc]});

const candidate = run.observation;
if (candidate === undefined) errors.push({path:'$.observation', expected:'parsed JSON', actual:'<missing>'});
else {
  const d = diff(source, candidate);
  if (d) errors.push(d);
  const modules = source.initialModuleCount;
  if (modules !== 40) errors.push({path:'$.source.initialModuleCount', expected:40, actual:modules});
  if (!Array.isArray(source.after) || source.after.length !== 40)
    errors.push({path:'$.source.after.length', expected:40, actual:source.after?.length});
  if (!Array.isArray(source.requests) || source.requests.length !== 80)
    errors.push({path:'$.source.requests.length', expected:80, actual:source.requests?.length});
  if (source.loadedGlyphCount !== 24646)
    errors.push({path:'$.source.loadedGlyphCount', expected:24646, actual:source.loadedGlyphCount});
}

if (errors.length) {
  process.stderr.write(`${JSON.stringify({errors, execution:{executable:run.executable,
    executableSha256:run.executableSha256,pid:run.pid,startedUtc:run.startedUtc,
    finishedUtc:run.finishedUtc,exitCode:run.exitCode}}, null, 2)}\n`);
  process.exitCode = 1;
} else {
  process.stdout.write(`Exact provider trace match: 40 modules, 80 events, 24646 glyphs.\n`);
  process.stdout.write(`Executable SHA-256 ${run.executableSha256}; PID ${run.pid}; ${run.startedUtc}–${run.finishedUtc}; exit ${run.exitCode}.\n`);
}
