'use strict';
const {createReference}=require('../../reference-profile.cjs');
const ref=createReference();
const render=tex=>ref.adaptor.serializeXML(ref.document.convert(tex,{display:false,em:16,ex:8,containerWidth:1280}));
const dataChars=svg=>[...svg.matchAll(/data-c="([^"]+)"/g)].map(match=>match[1]);
const cases=[
  ['macro-definition',String.raw`\newcommand{\FoliaLifeMacro}{x}\FoliaLifeMacro`],
  ['macro-use',String.raw`\FoliaLifeMacro`],
  ['plain-x','x'],
  ['untagged-first',String.raw`\begin{equation}\label{folia:lifetime:first}x\end{equation}`],
  ['untagged-second-ref',String.raw`\begin{equation}\label{folia:lifetime:second}\ref{folia:lifetime:first}\end{equation}`],
  ['explicit-tag-first',String.raw`\begin{equation}\tag{A}\label{folia:lifetime:explicit}x\end{equation}`],
  ['explicit-tag-ref',String.raw`\begin{equation}\tag{B}\label{folia:lifetime:explicit-second}\ref{folia:lifetime:explicit}\end{equation}`],
];
const observations=cases.map(([name,tex])=>({name,tex,dataChars:dataChars(render(tex))}));
process.stdout.write(JSON.stringify({source:'pinned MathJax 4.1.3 profile via reference-profile.cjs',
  tagConfiguration:{parseOptionsTags:ref.input.parseOptions.options.tags,
    activeTagsClass:ref.input.parseOptions.tags.constructor.name,
    configurationTags:Object.keys(ref.input.configuration.tags)},
  macroUseMatchesPlainGlyphs:JSON.stringify(observations[1].dataChars)===JSON.stringify(observations[2].dataChars),
  observations},null,2)+'\n');
