#!/usr/bin/env node
'use strict';
// Full field-by-field comparator for the pinned Source snapshot and native
// dump. It does not treat profile constants as native coverage evidence.
const fs=require('node:fs');
const {expectedEffectiveRemaps}=require('./font-data-remap-expectations.cjs');
const [sourcePath,nativePath]=process.argv.slice(2);
if(!sourcePath||!nativePath) throw new Error('usage: node compare-font-dump.cjs font-source-expected.json native-font-dump.json');
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const native=JSON.parse(fs.readFileSync(nativePath,'utf8'));
const failures=[]; let compared=0, mismatchCount=0;
function eq(actual,expected,path){
  ++compared;
  if(typeof expected==='number') {
    if(typeof actual!=='number'||!Number.isFinite(actual)||actual!==expected) fail(path,actual,expected);
  } else if(expected===null||typeof expected!=='object') {
    if(actual!==expected) fail(path,actual,expected);
  } else if(Array.isArray(expected)) {
    if(!Array.isArray(actual)||actual.length!==expected.length){fail(path,actual,expected);return;}
    expected.forEach((v,i)=>eq(actual[i],v,`${path}[${i}]`));
  } else {
    if(!actual||typeof actual!=='object'||Array.isArray(actual)){fail(path,actual,expected);return;}
    const ak=Object.keys(actual).sort(), ek=Object.keys(expected).sort();
    if(JSON.stringify(ak)!==JSON.stringify(ek))fail(`${path} keys`,ak,ek);
    for(const k of ek)if(Object.hasOwn(actual,k))eq(actual[k],expected[k],`${path}.${k}`);
  }
}
function fail(path,a,e){++mismatchCount;if(failures.length<100)failures.push({path,actual:a,expected:e});}
function sourceGlyph(t){
  const o=t[3]||{}; return {metrics:[t[0]??0,t[1]??0,t[2]??0,o.ic??0,o.sk??0,o.dx??0,o.oc??0],path_defined:Object.hasOwn(o,'p'),path_data:typeof o.p==='string'?o.p:'',content_data:sourceContentString(o.c),composition:sourceComposition(o.c)};
}
function sourceContentString(c){if(c===undefined)return null;if(typeof c==='string')return c;if(Array.isArray(c))return c.map(x=>x==null?'':String(x)).join(',');throw Error(`unsupported Source composition recipe: ${JSON.stringify(c)}`);}
function sourceComposition(c){if(c===undefined)return[];const result=[];const add=x=>{if(typeof x==='string'){for(let i=0;i<x.length;){const cp=x.codePointAt(i);result.push(cp);i+=cp>0xFFFF?2:1;}}else if(Number.isInteger(x)&&x>=0&&x<=0x10FFFF)result.push(x);else throw Error(`unsupported Source composition element: ${JSON.stringify(x)}`);};if(Array.isArray(c))c.forEach(add);else add(c);return result;}
const utf16Units=text=>{const units=[];for(let i=0;i<text.length;++i)units.push(text.charCodeAt(i));return units;};
function point(x,y){return [x/1000,y/1000];}
function outlineFromFragment(fragment){
  const input=fragment?`M${fragment}Z`:'';
  const out=[];let i=0,active='',x=0,y=0,sx=0,sy=0;
  const skip=()=>{while(i<input.length&&/[\t\n\r ,]/.test(input[i]))++i;};
  const failure=(reason,offset,command)=>({segments:out,error:{byte_offset:offset,command:command||null,reason}});
  const number=command=>{
    skip();const offset=i;
    if(i===input.length||/[A-Za-z]/.test(input[i]))return {error:failure('incomplete_segment',i,command)};
    const begin=i;if(input[i]==='+'||input[i]==='-')++i;let digits=0;
    while(i<input.length&&/[0-9]/.test(input[i])){++i;++digits;}
    if(input[i]==='.'){++i;while(i<input.length&&/[0-9]/.test(input[i])){++i;++digits;}}
    if(!digits)return {error:failure('invalid_number',offset,command)};
    if(input[i]==='e'||input[i]==='E'){++i;if(input[i]==='+'||input[i]==='-')++i;const exp=i;while(i<input.length&&/[0-9]/.test(input[i]))++i;if(i===exp)return {error:failure('invalid_number',offset,command)};}
    const value=Number(input.slice(begin,i));if(!Number.isFinite(value))return {error:failure('invalid_number',offset,command)};
    return {value};
  };
  const readPoint=(command,relative,origin)=>{
    const a=number(command);if(a.error)return a;
    const b=number(command);if(b.error)return b;
    return {value:[relative?origin[0]+a.value:a.value,relative?origin[1]+b.value:b.value]};
  };
  while(true){
    skip();if(i===input.length)break;
    if(/[A-Za-z]/.test(input[i]))active=input[i++];
    if(!active)return failure('missing_command',i,null);
    const command=active.toUpperCase(),relative=active!==command;
    if(command==='Z'){out.push({type:'Z'});x=sx;y=sy;active='';continue;}
    if(!['M','L','C','Q'].includes(command))return failure('unsupported_command',i-1,active);
    const origin=[x,y];
    if(command==='M'||command==='L'){
      const to=readPoint(active,relative,origin);if(to.error)return to.error;
      [x,y]=to.value;out.push({type:command==='M'?'M':'L',to:point(x,y)});
      if(command==='M'){sx=x;sy=y;active=relative?'l':'L';}
    }else if(command==='Q'){
      const control=readPoint(active,relative,origin);if(control.error)return control.error;
      const to=readPoint(active,relative,origin);if(to.error)return to.error;
      [x,y]=to.value;out.push({type:'Q',control:point(...control.value),to:point(x,y)});
    }else{
      const c1=readPoint(active,relative,origin);if(c1.error)return c1.error;
      const c2=readPoint(active,relative,origin);if(c2.error)return c2.error;
      const to=readPoint(active,relative,origin);if(to.error)return to.error;
      [x,y]=to.value;out.push({type:'C',control1:point(...c1.value),control2:point(...c2.value),to:point(x,y)});
    }
  }
  return {segments:out,error:null};
}
const nativeAliasTargets=new Map((native.svg_aliases||[]).map(([from,to])=>[JSON.stringify(from),to]));
function nativeSvgLookup(key){
  const k=JSON.stringify(key), direct=native.svg_resources[k];
  if(direct)return direct;
  const target=nativeAliasTargets.get(k);return target?native.svg_resources[JSON.stringify(target)]||null:null;
}
if(native.face!=='MathJaxNewcm')fail('face',native.face,'MathJaxNewcm');
eq(native.source_profile.source_profile_glyph_count,source.sourceProfile.newcm_variant_glyph_keys,'source_profile.source_profile_glyph_count');
eq(native.source_profile.source_dynamic_module_count,source.sourceProfile.dynamic_modules,'source_profile.source_dynamic_module_count');
eq(native.source_profile.source_provider_trace_events,source.sourceProfile.provider_trace_events,'source_profile.source_provider_trace_events');
const n=source.newcm;
eq(native.parameters,n.effectiveParameters,'parameters');
eq(native.size_variants,n.effectiveSizeVariants,'size_variants');
eq(native.stretch_variants,n.effectiveStretchVariants,'stretch_variants');
const sourceMaps={operator_remap:n.effectiveRemaps.mo||{},accent_remap:n.effectiveRemaps.accent||{},number_remap:n.effectiveRemaps.mn||{}};
for(const [key,table] of Object.entries(sourceMaps)){const encoded={};for(const [cp,text] of Object.entries(table))encoded[cp]=utf16Units(text);eq(native[key],encoded,key);}
const sourceDelimiters=n.effectiveDelimiters;
for(const [cp,d] of Object.entries(sourceDelimiters)){
  const a=native.delimiters[cp];if(!a){fail(`delimiters.${cp}`,null,d);continue;}
  const expected={direction:d.dir==='v'?2:d.dir==='h'?1:0,sizes:d.sizes||[],variants:d.variants||[],schar:d.schar||[],stretch:d.stretch||[],stretchv:d.stretchv||[],ext:d.ext??null,min:d.min??null,c:d.c==null?null:Number(d.c),hd:d.hd??null,HDW:d.HDW??null,fullExt:d.fullExt??null};eq(a,expected,`delimiters.${cp}`);
}
if(Object.keys(native.delimiters).length!==Object.keys(sourceDelimiters).length)fail('delimiters count',Object.keys(native.delimiters).length,Object.keys(sourceDelimiters).length);
const sourceVariants=n.loadedVariants, actualVariants=native.variants;
const sourceAliasTargets=new Map((source.svgAliases||[]).map(([from,to])=>[JSON.stringify(from),to]));
const inheritance=new Map(n.defaultVariants.map(([name,parent,link])=>[name,{parent:parent??null,link:link??null}]));
for(const extension of source.extensions)for(const [name,parent] of extension.data.variants?.['[+]']||[]){
  const prior=inheritance.get(name)||{parent:null,link:null}; inheritance.set(name,{...prior,parent});
}
for(const [name,record] of inheritance){
  const actual=actualVariants[name];if(actual){eq(actual.inherit,record.parent,`variants.${name}.inherit`);eq(actual.link_overlay,record.link,`variants.${name}.link_overlay`);}
}
const extensionCounts={};
for(const extension of source.extensions)extensionCounts[extension.name]=Object.values(extension.data.chars||{}).reduce((sum,chars)=>sum+Object.keys(chars).length,0);
eq(native.extension_glyph_counts,extensionCounts,'extension_glyph_counts');
for(const [name,sv] of Object.entries(sourceVariants)){
  const av=actualVariants[name];if(!av){fail(`variants.${name}`,null,'present');continue;}
  eq(av.cache_id,sv.cacheID??'',`variants.${name}.cache_id`);
  const expectedGlyphs={};for(const [cp,t] of Object.entries(sv.ownChars))if(!Number.isInteger(t?.[3]?.smp)){
    const g=sourceGlyph(t);expectedGlyphs[cp]={metrics:g.metrics,path_defined:g.path_defined,composition:g.composition};
  }
  if(Object.keys(av.glyphs).length!==Object.keys(expectedGlyphs).length)fail(`variants.${name}.glyph count`,Object.keys(av.glyphs).length,Object.keys(expectedGlyphs).length);
  for(const [cp,g] of Object.entries(expectedGlyphs)){
    const a=av.glyphs[cp];if(!a){fail(`variants.${name}.glyphs.${cp}`,null,g);continue;}
    eq(a.metrics,g.metrics,`variants.${name}.glyphs.${cp}.metrics`);eq(a.path_defined,g.path_defined,`variants.${name}.glyphs.${cp}.path_defined`);eq(a.composition,g.composition,`variants.${name}.glyphs.${cp}.composition`);
    const t=sv.ownChars[cp], sg=sourceGlyph(t), parsed=sg.path_defined?outlineFromFragment(sg.path_data):null;
    if(parsed===null){if(a.outline!==null)fail(`variants.${name}.glyphs.${cp}.outline`,a.outline,null);eq(a.outline_error,null,`variants.${name}.glyphs.${cp}.outline_error`);}
    // Native resource identity preserves the semantic variant. SVG cacheID is
    // checked separately above; extensions may share/omit that cache namespace.
    else {if(!a.outline)fail(`variants.${name}.glyphs.${cp}.outline`,null,'resource');else {eq(a.outline.font_id,`MathJaxNewcm/${name}`,`variants.${name}.glyphs.${cp}.outline.font_id`);eq(a.outline.segments,parsed.segments,`variants.${name}.glyphs.${cp}.outline.segments`);}eq(a.outline_error,parsed.error,`variants.${name}.glyphs.${cp}.outline_error`);}
  }
  // The generated native map contains all Source-visible SMP rows (the
  // generator uses `for..in` over materialized chars), including inherited
  // properties. `ownChars` JSON keys are only the own-property subset.
  const remaps=expectedEffectiveRemaps(name,sv.visibleChars);
  eq(av.remap,remaps,`variants.${name}.remap`);
  // Compare every effective key after exact generated aliases. This checks
  // Source link/inherit semantics without allowing arbitrary fallback.
  for(const [cp,t] of Object.entries(sv.visibleChars)){
    const mapped=t?.[3]?.smp, resolvedCp=Number.isInteger(mapped)?mapped:Number(cp);
    const resolved=source.sourceResolvedVisible?.[name]?.[cp];
    if(!resolved){fail(`Source getChar ${name}/${cp}`,null,'captured result');continue;}
    eq(resolved.requested_codepoint,Number(cp),`Source getChar ${name}/${cp}.requested_codepoint`);
    eq(resolved.target_codepoint,resolvedCp,`Source getChar ${name}/${cp}.target_codepoint`);
    const g=sourceGlyph(resolved.tuple), row=nativeSvgLookup([name,resolvedCp]);
    if(g.path_defined||g.content_data!==null){
      if(!row){fail(`svg ${name}/${cp}->${resolvedCp}`,null,'present Source resource');continue;}
      eq(row.path_defined,g.path_defined,`svg ${name}/${cp}.path_defined`);
      eq(row.path_data,g.path_defined&&g.path_data?`M${g.path_data}Z`:'',`svg ${name}/${cp}.path_data`);
      eq(row.content_data,g.content_data,`svg ${name}/${cp}.content_data`);
    }
  }
  const actualVisible=native.effective_glyphs?.[name]||{};
  if(Object.keys(actualVisible).length!==Object.keys(sv.visibleChars).length)
    fail(`effective_glyphs.${name} count`,Object.keys(actualVisible).length,Object.keys(sv.visibleChars).length);
  for(const [cp,t] of Object.entries(sv.visibleChars)){
    const mapped=t?.[3]?.smp, resolvedCp=Number.isInteger(mapped)?mapped:Number(cp);
    const resolved=source.sourceResolvedVisible?.[name]?.[cp];
    if(!resolved){fail(`Source getChar ${name}/${cp}`,null,'captured result');continue;}
    eq(resolved.requested_codepoint,Number(cp),`Source getChar ${name}/${cp}.requested_codepoint`);
    eq(resolved.target_codepoint,resolvedCp,`Source getChar ${name}/${cp}.target_codepoint`);
    const g=sourceGlyph(resolved.tuple), a=actualVisible[cp];
    if(!a){fail(`effective_glyphs.${name}.${cp}`,null,'present');continue;}
    eq(a.metrics,g.metrics,`effective_glyphs.${name}.${cp}.metrics`);
    eq(a.path_defined,g.path_defined,`effective_glyphs.${name}.${cp}.path_defined`);
    eq(a.composition,g.composition,`effective_glyphs.${name}.${cp}.composition`);
    eq(a.glyph_key,[name,sv.cacheID??'',resolvedCp],`effective_glyphs.${name}.${cp}.glyph_key`);
    const parsed=g.path_defined?outlineFromFragment(g.path_data):null;
    eq(a.outline_error,parsed?.error??null,`effective_glyphs.${name}.${cp}.outline_error`);
    const alias=sourceAliasTargets.get(JSON.stringify([name,resolvedCp]));
    const expectedOutlineFont=`MathJaxNewcm/${alias?alias[0]:name}`;
    if(g.path_defined)eq(a.outline_font_id,expectedOutlineFont,`effective_glyphs.${name}.${cp}.outline_font_id`);
    if(g.path_defined||g.content_data!==null){
      const expectedSvg={path_defined:g.path_defined,path_data:g.path_defined&&g.path_data?`M${g.path_data}Z`:'',content_data:g.content_data};
      eq(a.svg_resource,expectedSvg,`effective_glyphs.${name}.${cp}.svg_resource`);
    } else eq(a.svg_resource,null,`effective_glyphs.${name}.${cp}.svg_resource`);
  }
}
for(const name of Object.keys(actualVariants))if(!Object.hasOwn(sourceVariants,name))fail(`variants.${name}`,actualVariants[name],null);
const actualGlyphCount=Object.values(actualVariants).reduce((s,v)=>s+Object.keys(v.glyphs).length,0);
const sourceGlyphCount=Object.values(sourceVariants).reduce((s,v)=>s+Object.values(v.ownChars).filter(t=>!Number.isInteger(t?.[3]?.smp)).length,0);
const sourceSemanticKeys=Object.values(sourceVariants).reduce((s,v)=>s+Object.keys(v.ownChars).length,0);
if(actualGlyphCount!==sourceGlyphCount)fail('actual native direct glyph count',actualGlyphCount,sourceGlyphCount);
const actualResourceCount=Object.keys(native.svg_resources).length;
const actualAliasCount=(native.svg_aliases||[]).length;
// The immutable catalog is a map; its iteration order is not SVG emission
// order. Compare the complete mapping in a common order, retaining duplicates.
const aliasOrder=rows=>[...(rows||[])].sort((a,b)=>JSON.stringify(a).localeCompare(JSON.stringify(b)));
eq(aliasOrder(native.svg_aliases),aliasOrder(source.svgAliases),'svg_aliases');
const remapCount=Object.values(actualVariants).reduce((s,v)=>s+Object.keys(v.remap).length,0);
const outlineResourceRows=Object.values(actualVariants).flatMap(v=>Object.values(v.glyphs)).filter(g=>g.outline!==null);
const outlineSegmentCount=outlineResourceRows.reduce((s,g)=>s+g.outline.segments.length,0);
eq(native.native_actual_counts.variant_records,Object.keys(actualVariants).length,'native_actual_counts.variant_records');
eq(native.native_actual_counts.direct_glyph_records,actualGlyphCount,'native_actual_counts.direct_glyph_records');
eq(native.native_actual_counts.remap_records,remapCount,'native_actual_counts.remap_records');
eq(native.native_actual_counts.distinct_glyph_resource_objects,outlineResourceRows.length,'native_actual_counts.distinct_glyph_resource_objects');
eq(native.native_actual_counts.outline_segment_records,outlineSegmentCount,'native_actual_counts.outline_segment_records');
console.log(JSON.stringify({passed:mismatchCount===0,compared_values:compared,mismatch_count:mismatchCount,mismatch_sample_limit:100,mismatches:failures.length?failures:null,
  source_semantic_variant_keys:sourceSemanticKeys,source_direct_glyph_rows:sourceGlyphCount,native_actual_direct_glyph_records:actualGlyphCount,
  native_actual_remap_records:remapCount,native_actual_glyph_resource_objects:outlineResourceRows.length,
  native_actual_outline_segment_records:outlineSegmentCount,native_actual_svg_resources:actualResourceCount,
  native_actual_svg_aliases:actualAliasCount},null,2));
if(mismatchCount)process.exitCode=1;
