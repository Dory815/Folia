const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const {MathJaxBboldxFontExtension} = require('@mathjax/mathjax-bboldx-font-extension/js/svg.js');
const {MathJaxBbmFontExtension} = require('@mathjax/mathjax-bbm-font-extension/js/svg.js');
const {MathJaxDsfontFontExtension} = require('@mathjax/mathjax-dsfont-font-extension/js/svg.js');

// Exercise the shipped bundle, including its offline dynamic font loader.
const assetDirectory = path.resolve(__dirname, '../../src/app-winui/Assets/mathjax');
const bundlePath = process.argv[2] || path.join(assetDirectory, 'mathjax-quickjs.js');
const context = vm.createContext({
  console: {
    warn(...messages) { throw new Error(messages.join(' ')); },
    log(...messages) { throw new Error(messages.join(' ')); }
  },
  FoliaLoadMathJaxModule(name) {
    const filename = path.posix.basename(name);
    assert.match(filename, /^[A-Za-z0-9.-]+\.js$/);
    vm.runInContext(fs.readFileSync(path.join(assetDirectory, 'font', filename), 'utf8'), context);
  }
});
vm.runInContext(fs.readFileSync(bundlePath, 'utf8'), context);

const cases = [
  [String.raw`\mathbb{R}`, MathJaxBboldxFontExtension, '-bboldx', 0x211D],
  [String.raw`\mathbfbb{R}`, MathJaxBboldxFontExtension, '-bboldx-bold', 0x211D],
  [String.raw`\setOptions[bboldx]{light=true}\mathbb{R}\setOptions[bboldx]{light=false}`, MathJaxBboldxFontExtension, '-bboldx-light', 0x211D],
  [String.raw`\bbGamma`, MathJaxBboldxFontExtension, '-bboldx', 0x393],
  [String.raw`\mathversion{normal}\mathbbm{R}`, MathJaxBbmFontExtension, '-bbm-normal', 0x211D],
  [String.raw`\mathversion{bold}\mathbbm{R}\mathversion{normal}`, MathJaxBbmFontExtension, '-bbm-bold', 0x211D],
  [String.raw`\mathbbmss{R}`, MathJaxBbmFontExtension, '-bbm-sans-serif', 0x211D],
  [String.raw`\mathversion{bold}\mathbbmss{R}\mathversion{normal}`, MathJaxBbmFontExtension, '-bbm-sans-serif-bold', 0x211D],
  [String.raw`\mathbbmtt{R}`, MathJaxBbmFontExtension, '-bbm-monospace', 0x211D],
  [String.raw`\mathds{R}`, MathJaxDsfontFontExtension, '-ds-rm', 0x211D],
  [String.raw`\setOptions[dsfont]{sans=true}\mathds{R}\setOptions[dsfont]{sans=false}`, MathJaxDsfontFontExtension, '-ds-ss', 0x211D]
];
for (const [tex, extension, variant, character] of cases) {
  for (const display of [false, true]) {
    const svg = context.FoliaMathJax.render(tex, display, 18, 900);
    assert.doesNotMatch(svg, /data-mjx-error=|<merror|<text\b/, tex);
    const paths = Array.from(svg.matchAll(/<path\b[^>]*\bd="([^"]*)"/g), match => match[1]);
    // Compare the outline to the official font data; a valid SVG with a plain
    // roman fallback must fail this test too.
    const outline = extension.chars[variant][character][3].p;
    assert.deepEqual(paths, [`M${outline}Z`], `${tex} (${display ? 'display' : 'inline'})`);
  }
}
console.log(`ok: ${cases.length * 2} font outline checks`);
