'use strict';
// Pinned MathJax CJS witness for user definitions and group map lifetime.
// Real TeX/MML compilation; fixed expected state and output; no native implementation is claimed.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');

function findRoot(start) {
  for (let current = path.resolve(start); ; current = path.dirname(current)) {
    if (fs.existsSync(path.join(current, '.reference', 'MathJax-src')) &&
        fs.existsSync(path.join(current, 'tools', 'mathjax-bundle', 'node_modules', '@mathjax', 'src'))) return current;
    const parent = path.dirname(current);
    if (parent === current) throw new Error('Cannot locate md-cpp repo root; pass --repo-root <path>');
  }
}

const explicitRoot = process.argv.indexOf('--repo-root');
const root = explicitRoot >= 0 ? path.resolve(process.argv[explicitRoot + 1]) : findRoot(__dirname);
const packageRoot = path.join(root, 'tools', 'mathjax-bundle', 'node_modules', '@mathjax', 'src');
const pin = JSON.parse(fs.readFileSync(path.join(root, 'tests/math/fixtures/tex-source/session-definitions-source-lock.json'), 'utf8'));
assert.equal(JSON.parse(fs.readFileSync(path.join(packageRoot, 'package.json'), 'utf8')).version, '4.1.3');
assert.equal(pin.upstream.commit, 'fb987178c2d279a99b0db5ea02e751691703955e');
for (const source of pin.sources) {
  const ts = fs.readFileSync(path.join(root, '.reference', 'MathJax-src', source.path));
  const cjs = fs.readFileSync(path.join(packageRoot, 'cjs', source.cjs));
  assert.equal(crypto.createHash('sha256').update(ts).digest('hex'), source.tsSha256, `Source changed: ${source.path}`);
  assert.equal(crypto.createHash('sha256').update(cjs).digest('hex'), source.cjsSha256, `CJS changed: ${source.cjs}`);
}

const load = name => require(path.join(packageRoot, 'cjs', name));
const TeX = load('input/tex.js').TeX;
const { MmlFactory } = load('core/MmlTree/MmlFactory.js');
const { AbstractMathItem } = load('core/MathItem.js');
const { HandlerType } = load('input/tex/HandlerTypes.js');
const { NewcommandConfiguration } = load('input/tex/newcommand/NewcommandConfiguration.js');
const { BegingroupConfiguration } = load('input/tex/begingroup/BegingroupConfiguration.js');
const { BegingroupStack } = load('input/tex/begingroup/BegingroupStack.js');
const { MapHandler } = load('input/tex/MapHandler.js');
const { NewcommandTables } = load('input/tex/newcommand/NewcommandUtil.js');

const configuredPackages = ['base', 'begingroup', 'newcommand'];
function makeJax() {
  const jax = new TeX({ packages: configuredPackages });
  jax.setMmlFactory(new MmlFactory());
  return jax;
}
function compile(jax, source) {
  try {
    const result = jax.compile(new AbstractMathItem(source, jax, true), {});
    let hasError = false;
    result.walkTree(node => { if (node.kind === 'merror') hasError = true; });
    // TeX catches TexError and returns an merror root. A normal return alone
    // cannot prove that a definition actually expanded successfully.
    return { ok: !hasError, result };
  } catch (error) {
    return { ok: false, id: error.id ?? null, name: error.name || error.constructor.name, message: String(error.message), stack: error.stack };
  }
}
function lookup(jax, handlerType, token) {
  const handler = jax.parseOptions.handlers.get(handlerType);
  const map = handler.applicable(token);
  return map ? { map: map.name, value: map.lookup(token) } : null;
}

const jax = makeJax();
const stack = jax.parseOptions.packageData.get('begingroup').stack;
assert.ok(stack instanceof BegingroupStack);
const local = compile(jax, String.raw`\begingroup\def\raiiLocal{x}\endgroup`);
assert.equal(local.ok, true, 'a balanced group with a definition compiles');
assert.equal(lookup(jax, HandlerType.MACRO, 'raiiLocal'), null,
  'group-local command disappears after endgroup');

const global = compile(jax, String.raw`\global\def\raiiGlobal{x}`);
assert.equal(global.ok, true, `global definition compiles: ${global.ok ? 'ok' : JSON.stringify(global)}`);
const globalDefinition = lookup(jax, HandlerType.MACRO, 'raiiGlobal');
assert.ok(globalDefinition?.value, 'global command remains in the session-backed map');
assert.equal(compile(jax, String.raw`\raiiGlobal`).ok, true,
  'global command is available to a later expression on the same jax');

const optionalMacro = compile(jax, String.raw`\newcommand{\raiiOptional}[2][d]{#1/#2}\raiiOptional{x}`);
assert.equal(optionalMacro.ok, true, 'newcommand stores optional first-argument default and replacement');
const delimitedMacro = compile(jax, String.raw`\def\raiiDelimited#1,#2;{#2/#1}\raiiDelimited a,b;`);
assert.equal(delimitedMacro.ok, true, 'def stores a nontrivial delimiter template and expands');
const letMacro = compile(jax, String.raw`\newcommand{\raiiOrigin}{x}\let\raiiCopy=\raiiOrigin\raiiCopy`);
assert.equal(letMacro.ok, true, 'let aliases an existing user-defined replacement');
const letDelimiter = compile(jax, String.raw`\let\raiiParen=(\raiiParen`);
assert.equal(letDelimiter.ok, true, 'let copies delimiter character and attributes into a new delimiter');

const environment = compile(jax, String.raw`\newenvironment{raiiEnv}[1][d]{(#1)}{!}\begin{raiiEnv}q\end{raiiEnv}`);
assert.equal(environment.ok, true, 'optional/default environment definition is usable');
const environmentDefinition = lookup(jax, HandlerType.ENVIRONMENT, 'raiiEnv');
assert.ok(environmentDefinition?.value, 'environment entry persists through its named map');

const builtinAlias = compile(jax,
  String.raw`\let\raiiSavedRoot=\sqrt\def\sqrt{z}\raiiSavedRoot{a}`);
assert.equal(builtinAlias.ok, true, 'let snapshots the builtin method before its name is replaced');
assert.equal(builtinAlias.result.toString(), 'math([msqrt([mi(a)])])');
const characterAlias = compile(jax,
  String.raw`\let\raiiSavedAlpha=\alpha\let\alpha=\beta\raiiSavedAlpha\alpha`);
assert.equal(characterAlias.ok, true, 'let also copies an actual CharacterMap token');
assert.equal(characterAlias.result.toString(), 'math([mi(α),mi(β)])');

const undefinition = compile(jax, String.raw`\let\frac=\raiiMissing`);
assert.equal(undefinition.ok, true, 'let to an undefined command executes undefinition');
const blockedFraction = lookup(jax, HandlerType.MACRO, 'frac');
assert.ok(blockedFraction?.value, 'undefinition installs a macro-map suppression entry for a later mapping');
assert.equal(typeof blockedFraction.value.func, 'function', 'Source fallback sentinel is carried by a macro entry');
const delimiterUndefinition = compile(jax, String.raw`\let\langle=\raiiMissingDelimiter`);
assert.equal(delimiterUndefinition.ok, true, 'let to an undefined command checks delimiter suppression too');
const blockedDelimiter = lookup(jax, HandlerType.DELIMITER, '\\langle');
assert.ok(blockedDelimiter?.value, 'undefinition installs the null-character delimiter blocker when needed');
assert.equal(blockedDelimiter.value.char, null, 'delimiter blocker keeps the Source null-character sentinel');

const handledError = compile(jax, String.raw`\begingroup\def\raiiBeforeError{x}\raiiUndefinedAfterDefinition`);
assert.equal(handledError.ok, false, 'the actual TeX input creates merror for the undefined command');
assert.ok(lookup(jax, HandlerType.MACRO, 'raiiBeforeError')?.value,
  'a handled TexError preserves definitions before the error');
assert.equal(stack.i, -101, 'postfilters commit the unclosed group after the handled error');
assert.equal(compile(jax, String.raw`\raiiBeforeError`).ok, true,
  'the preserved group definition expands in the next expression');
assert.equal(compile(jax, String.raw`\endgroup`).ok, true);
assert.equal(lookup(jax, HandlerType.MACRO, 'raiiBeforeError'), null);

const globalErase = compile(jax, String.raw`\def\raiiTarget{x}\let\raiiTargetDelimiter=(\newenvironment{raiiTargetEnv}{a}{b}\begingroup\def\raiiTarget{y}\let\raiiTargetDelimiter=[\newenvironment{raiiTargetEnv}{c}{d}`);
assert.equal(globalErase.ok, true);
const clearedTargets = stack.checkGlobal(
  ['raiiTarget', '\\raiiTargetDelimiter', 'raiiTargetEnv'],
  [NewcommandTables.NEW_COMMAND, NewcommandTables.NEW_DELIMITER, NewcommandTables.NEW_ENVIRONMENT]);
const globalEraseObserved = clearedTargets.map((map, i) =>
  map.lookup(['raiiTarget', '\\raiiTargetDelimiter', 'raiiTargetEnv'][i]) == null);
assert.deepEqual(globalEraseObserved, [true, true, true],
  'Source marker removes the terminal target entry as well as all inner definitions');
assert.equal(lookup(jax, HandlerType.MACRO, 'raiiTarget'), null);
assert.equal(lookup(jax, HandlerType.DELIMITER, '\\raiiTargetDelimiter'), null);
assert.equal(lookup(jax, HandlerType.ENVIRONMENT, 'raiiTargetEnv'), null);
assert.equal(compile(jax, String.raw`\endgroup`).ok, true);

stack.push();
const afterPush = stack.i;
stack.finish();
stack.push();
const afterSecondPush = stack.i;
stack.remove();
assert.equal(afterPush, -101);
assert.equal(afterSecondPush, -102);
assert.equal(stack.i, afterPush, 'remove pops only groups opened after finish checkpoint');
stack.reset();
assert.equal(stack.i, stack.base, 'reset removes scopes back to the active base');

const sandboxBefore = stack.base;
stack.sandbox();
const sandboxBase = stack.base;
assert.equal(sandboxBefore, -100);
assert.equal(sandboxBase, -101, 'sandbox installs a protected new base layer');
assert.equal(stack.i, sandboxBase);
stack.sandbox();
assert.equal(stack.i, stack.base, 'starting another sandbox resets the prior sandbox base');

const output = path.join(root, 'build', 'math-port', 'tex-session-definitions-source.json');
fs.mkdirSync(path.dirname(output), { recursive: true });
const witness = {
  upstream: pin.upstream,
  configuredPackages,
  packageConfigurations: [NewcommandConfiguration.name, BegingroupConfiguration.name],
  observed: {
    localDefinitionAfterEndgroup: lookup(jax, HandlerType.MACRO, 'raiiLocal'),
    globalDefinitionAfterLaterExpression: Boolean(globalDefinition?.value),
    environmentRecordPresent: Boolean(environmentDefinition?.value),
    optionalMacroCompiles: optionalMacro.ok,
    delimitedMacroCompiles: delimitedMacro.ok,
    letMacroCompiles: letMacro.ok,
    letDelimiterCompiles: letDelimiter.ok,
    handledErrorRetainsGroupUntilNextEndgroup: !handledError.ok,
    globalSelectionErasesExistingTargetEntries: globalEraseObserved,
    outputs: {
      optionalMacro: optionalMacro.result.toString(),
      delimitedMacro: delimitedMacro.result.toString(),
      letMacro: letMacro.result.toString(),
      letDelimiter: letDelimiter.result.toString(),
      environment: environment.result.toString(),
      builtinAliasAfterRedefinition: builtinAlias.result.toString(),
      characterAliasAfterRedefinition: characterAlias.result.toString(),
    },
    undefinedPackageMacroMap: blockedFraction?.map ?? null,
    undefinedDelimiterBlocker: blockedDelimiter ? { map: blockedDelimiter.map, char: blockedDelimiter.value.char } : null,
    priorities: { afterPush, afterSecondPush, afterRemove: afterPush, normalBase: sandboxBefore, sandboxBase },
  },
  cases: ['balanced-local-definition', 'global-definition-survives-expression',
    'newcommand-optional-default', 'def-delimited-template', 'let-user-macro-alias',
    'let-delimiter-alias',
    'newenvironment-optional-default', 'let-to-undefined-suppresses-package-macro',
    'let-builtin-method-snapshot', 'let-character-map-token-snapshot',
    'let-to-undefined-suppresses-delimiter',
    'handled-error-definition-and-group-persistence',
    'global-marker-erases-terminal-target-in-all-three-domains',
    'finish-remove-checkpoint', 'reset-to-base', 'sandbox-base-reset'],
};
assert.deepEqual(witness, JSON.parse(fs.readFileSync(path.join(root, 'tests/math/fixtures/tex-source/session-definitions-source-expected.json'), 'utf8')));
fs.writeFileSync(output, JSON.stringify(witness, null, 2) + '\n');
console.log(`PASS: ${witness.cases.length} definition/group source checks; ${path.relative(root, output)}`);
