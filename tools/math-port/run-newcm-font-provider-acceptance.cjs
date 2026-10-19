#!/usr/bin/env node
'use strict';

// Launch a freshly linked native provider dump and retain execution provenance
// beside its unmodified JSON observation. This is intentionally a separate
// runner so the Source oracle and the candidate dumper remain untouched.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const {spawnSync} = require('node:child_process');

const root = path.resolve(__dirname, '../..');
const executable = path.resolve(process.argv[2] || path.join(root, 'build/core/FoliaMathFontModuleDump.exe'));
const output = path.resolve(process.argv[3] || path.join(root, 'build/math-port/newcm-font-provider-acceptance-native.json'));
if (!fs.existsSync(executable)) throw new Error(`Native executable does not exist: ${executable}`);
const sha256 = crypto.createHash('sha256').update(fs.readFileSync(executable)).digest('hex');
const startedUtc = new Date().toISOString();
const result = spawnSync(executable, [], {encoding:'utf8', windowsHide:true, maxBuffer:128 * 1024 * 1024});
const finishedUtc = new Date().toISOString();
let observation;
let parseError = null;
if (result.stdout) {
  try { observation = JSON.parse(result.stdout); }
  catch (error) { parseError = String(error); }
}
const artifact = {
  executable,
  executableSha256: sha256,
  pid: result.pid ?? null,
  startedUtc,
  finishedUtc,
  exitCode: result.status,
  signal: result.signal,
  error: result.error ? String(result.error) : null,
  stdout: result.stdout || '',
  stderr: result.stderr || '',
  parseError,
  ...(observation === undefined ? {} : {observation}),
};
fs.mkdirSync(path.dirname(output), {recursive:true});
fs.writeFileSync(output, `${JSON.stringify(artifact, null, 2)}\n`);
process.stdout.write(`${JSON.stringify({output, executableSha256:sha256, pid:artifact.pid,
  startedUtc, finishedUtc, exitCode:artifact.exitCode, signal:artifact.signal, parseError})}\n`);
if (artifact.error || artifact.exitCode !== 0 || parseError || observation === undefined) {
  process.exitCode = 1;
}
