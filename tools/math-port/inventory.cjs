// Root-owned scope inventory. This generates no native implementation.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const cp = require('node:child_process');

const root = path.resolve(__dirname, '../..');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const expectedCommit = 'fb987178c2d279a99b0db5ea02e751691703955e';
const commit = cp.execFileSync('git', ['rev-parse', 'HEAD'], {cwd: sourceRoot, encoding: 'utf8'}).trim();
if (commit !== expectedCommit) throw new Error(`Unexpected MathJax source commit: ${commit}`);
const sha256 = value => crypto.createHash('sha256').update(value).digest('hex');
const slash = value => value.replaceAll('\\', '/');
const snake = value => value === 'BBox' ? 'bbox' : value.replace(/([a-z0-9])([A-Z])/g, '$1_$2').replace(/([A-Z])([A-Z][a-z])/g, '$1_$2').toLowerCase();
const profilePath = path.join(root, 'tools/mathjax-bundle/entry.cjs');
const profile = fs.readFileSync(profilePath, 'utf8');
const packageArray = profile.match(/packages:\s*\[([\s\S]*?)\]/);
if (!packageArray) throw new Error('No Folia TeX package profile');
const packages = [...packageArray[1].matchAll(/'([^']+)'/g)].map(match => match[1]);
const packageRoots = [...profile.matchAll(/require\('@mathjax\/src\/js\/(input\/tex\/[^']+)\.js'\)/g)]
  .map(match => `ts/${match[1]}.ts`);
const seeds = ['ts/input/tex.ts', 'ts/output/common.ts', 'ts/output/svg.ts',
  'ts/util/BBox.ts', 'ts/core/MmlTree/MmlFactory.ts', ...packageRoots];
const pending = [...seeds];
const files = new Map();
const unresolved = [];

function owner(source) {
  if (source.startsWith('ts/input/')) return 'tex';
  if (source === 'ts/output/common/FontData.ts' || source.includes('/output/svg/Font')) return 'fonts';
  if (source.startsWith('ts/output/common')) return 'layout';
  if (source.startsWith('ts/output/svg')) return 'scene_svg';
  if (source.startsWith('ts/core/MmlTree') || source.startsWith('ts/core/Tree') || source.startsWith('ts/util/')) return 'semantics_util';
  return 'root_host_adapter';
}
while (pending.length) {
  const relative = pending.pop();
  if (files.has(relative)) continue;
  const absolute = path.join(sourceRoot, relative);
  if (!fs.existsSync(absolute)) throw new Error(`Missing source: ${relative}`);
  const bytes = fs.readFileSync(absolute);
  const text = bytes.toString('utf8');
  const dependencies = [];
  const externalImports = [];
  // Only static import/re-export declarations, not prose such as "from '@'"
  // in comments. The resulting closure remains subject to root's G0 review.
  const imports = /^\s*import\s+(?:[^;'"\u0060]*?\bfrom\s*)?['"]([^'"]+)['"]/gm;
  const exports = /^\s*export\s+(?:type\s+)?(?:\{[^;]*?\}|\*)\s+from\s*['"]([^'"]+)['"]/gm;
  for (const match of [...text.matchAll(imports), ...text.matchAll(exports)]) {
    const specifier = match[1];
    if (!specifier.startsWith('.')) { externalImports.push(specifier); continue; }
    const target = slash(path.relative(sourceRoot, path.resolve(path.dirname(absolute), specifier.replace(/\.js$/, '.ts'))));
    if (!fs.existsSync(path.join(sourceRoot, target))) {
      unresolved.push({source: relative, specifier, candidate: target});
      continue;
    }
    dependencies.push(target);
    pending.push(target);
  }
  const parts = relative.replace(/^ts\//, '').replace(/\.ts$/, '').split('/').map(snake);
  files.set(relative, {source: relative, source_sha256: sha256(bytes),
    proposed_target: `src/math/${parts.join('/')}.ixx`,
    proposed_module: `folia.math.${parts.join('.')}`,
    owner: owner(relative), disposition: 'unreviewed', adaptation_reason: '',
    port_status: 'not_started', acceptance: 'not_verified',
    dependencies: [...new Set(dependencies)].sort(), external_imports: [...new Set(externalImports)].sort()});
}

const lockPath = path.join(root, 'tools/mathjax-bundle/package-lock.json');
const lockBytes = fs.readFileSync(lockPath);
const lock = JSON.parse(lockBytes);
const fontPackages = ['mathjax-newcm-font', 'mathjax-bbm-font-extension',
  'mathjax-bboldx-font-extension', 'mathjax-dsfont-font-extension'].map(name => {
  const entry = lock.packages[`node_modules/@mathjax/${name}`];
  if (!entry || entry.version !== '4.1.3') throw new Error(`Unpinned font ${name}`);
  return {package: `@mathjax/${name}`, version: entry.version, integrity: entry.integrity,
    disposition: 'generate_data_and_port_resolver', acceptance: 'not_verified'};
});
const manifestPath = path.join(root, 'docs/math-port/scope.json');
const prior = fs.existsSync(manifestPath) ? JSON.parse(fs.readFileSync(manifestPath, 'utf8')) : null;
for (const item of files.values()) {
  const previous = prior?.files.find(old => old.source === item.source);
  if (previous?.source_sha256 === item.source_sha256) {
    for (const key of ['disposition', 'adaptation_reason', 'port_status', 'acceptance']) item[key] = previous[key];
  }
}
const reviewed = [...files.values()].filter(item => item.disposition !== 'unreviewed').length;
const status = reviewed === files.size ? 'scope_reviewed_not_release_ready' :
  reviewed ? 'scope_partially_reviewed_not_release_ready' : 'inventory_unreviewed_not_release_ready';
const manifest = {schema: 1, status,
  upstream: {repository: 'https://github.com/mathjax/MathJax-src', version: '4.1.3', commit: expectedCommit},
  profile: {file: 'tools/mathjax-bundle/entry.cjs', sha256: sha256(profile), tex_packages: packages,
    svg_options: {fontCache: 'none', exFactor: 0.5, displayOverflow: 'linebreak', linebreaks: {inline: true}}},
  dependency_lock_sha256: sha256(lockBytes), font_packages: fontPackages,
  source_roots: [...new Set(seeds)].sort(), unresolved_imports: unresolved,
  files: [...files.values()].sort((a, b) => a.source < b.source ? -1 : a.source > b.source ? 1 : 0)};
const serialized = JSON.stringify(manifest, null, 2) + '\n';
if (process.argv.includes('--check')) {
  if (!prior || JSON.stringify(prior) !== JSON.stringify(manifest)) throw new Error('Scope inventory differs from pinned sources/profile');
} else {
  fs.mkdirSync(path.dirname(manifestPath), {recursive: true});
  fs.writeFileSync(manifestPath, serialized);
}
console.log(`${files.size} upstream files, ${packages.length} TeX packages, ${unresolved.length} unresolved imports`);
if (unresolved.length) process.exitCode = 1;
