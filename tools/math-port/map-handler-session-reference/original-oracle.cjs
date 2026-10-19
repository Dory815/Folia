#!/usr/bin/env node
// Source witness for MapHandler's module-owned registry lifetime.
// The two fresh MapHandler module instances stand for separate loaded Source
// module environments. Multiple owners using one instance share its map.
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

const root = path.resolve(__dirname, '../../..');
const lockPath = path.join(__dirname, 'source-lock.json');
const lock = JSON.parse(fs.readFileSync(lockPath, 'utf8'));
const digest = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
for (const source of lock.sources) {
  const tsPath = path.join(root, '.reference/MathJax-src', source.file);
  const cjsPath = path.join(root, source.cjsFile);
  if (digest(tsPath) !== source.sha256) throw new Error(`TS source hash mismatch: ${source.file}`);
  if (digest(cjsPath) !== source.cjsSha256) throw new Error(`CJS source hash mismatch: ${source.cjsFile}`);
}

const cjsPath = path.join(root, lock.sources[0].cjsFile);
function loadFreshMapHandler() {
  const resolved = require.resolve(cjsPath);
  delete require.cache[resolved];
  return require(resolved).MapHandler;
}

const handlerA = loadFreshMapHandler();
const mapA = { name: 'session-map', callback: () => 'A' };
handlerA.register(mapA);
const sameRuntimeA = handlerA.getMap('session-map');

const handlerB = loadFreshMapHandler();
const mapB = { name: 'session-map', callback: () => 'B' };
handlerB.register(mapB);

const sequence = [
  handlerA.getMap('session-map').callback(),
  handlerB.getMap('session-map').callback(),
  handlerA.getMap('session-map').callback(),
  handlerB.getMap('session-map').callback()
];
const result = {
  sourceLockSha256: digest(lockPath),
  cases: [{
    name: 'map-handler-two-module-instances-interleaved',
    values: {
      sameModuleOwnersShareMap: sameRuntimeA === mapA,
      moduleAStillOwnsOriginalMapAfterModuleBRegistersSameName: handlerA.getMap('session-map') === mapA,
      moduleBOwnsItsMap: handlerB.getMap('session-map') === mapB,
      distinctModuleMaps: mapA !== mapB,
      interleavedCallbacks: sequence
    }
  }]
};
process.stdout.write(JSON.stringify(result) + '\n');
