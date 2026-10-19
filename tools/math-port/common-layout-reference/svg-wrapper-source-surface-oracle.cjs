const path = require('node:path');
const fs = require('node:fs');
const crypto = require('node:crypto');
const project = path.resolve(__dirname, '../../..');
const bundle = path.join(project, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const lock = JSON.parse(fs.readFileSync(path.join(__dirname, 'profile-source-lock.json'), 'utf8'));
for (const file of ['src/cjs/output/common/Wrapper.js',
                    'src/cjs/output/svg/Wrappers.js',
                    'src/cjs/output/svg/Wrappers/TextNode.js']) {
  const entry = lock.dependencies.find((item) => item.file === file);
  if (!entry) throw new Error(`pinned lock is missing ${file}`);
  const actual = crypto.createHash('sha256')
    .update(fs.readFileSync(path.join(bundle, file.slice('src/'.length)))).digest('hex');
  if (actual !== entry.sha256) throw new Error(`pinned source hash mismatch: ${file}`);
}
const {SvgWrappers} = require(path.join(bundle, 'cjs/output/svg/Wrappers.js'));
const SvgWrapper = SvgWrappers.unknown;
const CommonWrapper = Object.getPrototypeOf(SvgWrapper);
const SvgTextNode = SvgWrappers.text;

function ownSurface(value) {
  return Reflect.ownKeys(value).map((key) => {
    const descriptor = Object.getOwnPropertyDescriptor(value, key);
    return {
      key: typeof key === 'symbol' ? `Symbol(${key.description})` : key,
      enumerable: descriptor.enumerable,
      configurable: descriptor.configurable,
      writable: Object.prototype.hasOwnProperty.call(descriptor, 'writable') ? descriptor.writable : null,
      kind: Object.prototype.hasOwnProperty.call(descriptor, 'value')
        ? typeof descriptor.value : 'accessor',
      functionName: typeof descriptor.value === 'function' ? descriptor.value.name : null,
      functionLength: typeof descriptor.value === 'function' ? descriptor.value.length : null,
    };
  });
}

const calls = [];
const target = {addStyles(value) { calls.push(value); }};
CommonWrapper.addStyles.call(CommonWrapper, target, {});
SvgTextNode.addStyles.call(SvgTextNode, target, {options: {blacker: 3.5}});

process.stdout.write(JSON.stringify({
  commonStatics: ownSurface(CommonWrapper),
  commonPrototype: ownSurface(CommonWrapper.prototype),
  textNodeStatics: ownSurface(SvgTextNode),
  textNodeAddStyles: calls,
}, null, 2) + '\n');
