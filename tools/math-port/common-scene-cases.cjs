'use strict';

// The pinned canonical 75 inputs are the original fixed 18 followed by the
// 57 locked profile cases. Keep this helper limited to those inputs and their
// existing defaults so strict and hosted Source witnesses share one corpus.
function canonicalSceneCases(profileCases) {
  const baseline = [
    ['plain', 'x+y', false, 1280], ['mathbb', '\\mathbb{R}^n', false, 1280],
    ['fraction-inline', '\\frac{a+b}{c}', false, 1280],
    ['fraction-display', '\\frac{a+b}{c}', true, 1280],
    ['atop', '{a+b\\atop c}', true, 1280], ['root', '\\sqrt{x^2+1}', true, 1280],
    ['indexed-root', '\\sqrt[3]{x}', false, 1280], ['scripts', 'x_i^{j^2}', false, 1280],
    ['sum-limits', '\\sum_{i=1}^{n} i', true, 1280],
    ['stretch-fences', '\\left(\\frac{1}{x}\\right)', true, 1280],
    ['accent', '\\widehat{abc}+\\overline{x}', false, 1280],
    ['matrix', '\\begin{pmatrix}a&b\\\\c&d\\end{pmatrix}', true, 1280],
    ['arrow', '\\xrightarrow[a]{b}', true, 1280],
    ['text', '\\text{hello }x', false, 1280],
    ['color', '\\color{red}{x+1}', false, 1280],
    ['bbox', '\\bbox[2px,border:1px solid blue]{x}', false, 1280],
    ['phantom', 'a\\phantom{x}+b', false, 1280],
    ['linebreak', 'a+b+c+d+e+f+g+h+i+j', true, 100]
  ].map(([name, tex, display, width]) => ({name, tex, display, width, features: ['base']}));

  return baseline.concat(profileCases.cases.map(item => ({
    name: item.name,
    tex: item.tex,
    display: item.display ?? false,
    width: item.width ?? 1280,
    features: item.features
  })));
}

module.exports = {canonicalSceneCases};
