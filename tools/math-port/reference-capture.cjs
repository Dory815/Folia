// Full-pipeline reference snapshots, owned by root, not implementation agents.
const assert = require('node:assert/strict');
const {createReference} = require('./reference-profile.cjs');

function value(v, seen = new Set()) {
  if (v === undefined) return {$undefined:true};
  if (typeof v === 'number') {
    if (!Number.isFinite(v)) return {$number:String(v)};
    if (Object.is(v,-0)) return {$number:'-0'};
    return v;
  }
  if (v === null || typeof v === 'string' || typeof v === 'boolean') return v;
  // These tags are for debug state, not proof of function-body equivalence.
  if (typeof v === 'function') return {$function:v.name || '<anonymous>'};
  if (typeof v === 'symbol') return {$symbol:String(v)};
  assert.equal(typeof v,'object');
  assert.equal(seen.has(v),false,'Unmapped circular reference in snapshot state');
  seen.add(v);
  let result;
  if (Array.isArray(v)) result=Array.from(v,x=>value(x,seen));
  else {
    result={};
    for (const key of Object.keys(v).sort()) result[key]=value(v[key],seen);
  }
  seen.delete(v);
  return result;
}

function own(object) {
  if (object == null) return null;
  return {names:Object.keys(object), values:value(Object.fromEntries(Object.keys(object).map(k=>[k,object[k]])))};
}

function mmlSnapshot(root, {ids = new Map(), extras = []} = {}) {
  if (!root) return null;
  const nodes=[];
  function add(node,id) {
    if (!node || ids.has(node)) return;
    ids.set(node,id); nodes.push(node);
    (node.childNodes||[]).forEach((child,i)=>add(child,id+'/'+i));
  }
  add(root,'m');
  for (const [node,id] of extras) add(node,id);
  const ref=node=>{
    if (!node) return null;
    if (!ids.has(node)) add(node,'external/'+ids.size);
    return ids.get(node);
  };
  const result=[];
  // Parent/core references can expose nodes outside the child-tree root. Keep
  // those objects, not a shared placeholder that erases distinct identities.
  for (const node of nodes) {
    const attributes=node.attributes;
    const layers=attributes ? {
      explicit:own(attributes.getAllAttributes()), inherited:own(attributes.getAllInherited()),
      defaults:own(attributes.getAllDefaults()), global:own(attributes.getAllGlobals())
    } : null;
    const names=layers ? [...new Set(Object.values(layers).flatMap(layer=>layer.names))].sort() : [];
    result.push({id:ref(node),kind:node.kind,parent:ref(node.parent),logical_parent:ref(node.Parent),
      children:(node.childNodes||[]).map(ref),properties:own(node.getAllProperties()), attributes:layers,
      resolved:attributes ? Object.fromEntries(names.map(name=>[name,value(attributes.get(name))])) : null,
      state:value({texClass:node.texClass,prevClass:node.prevClass,prevLevel:node.prevLevel,
        arity:node.arity,isToken:node.isToken,isEmpty:node.isEmpty,isEmbellished:node.isEmbellished,
        isSpacelike:node.isSpacelike,isInferred:node.isInferred,notParent:node.notParent,
        linebreakContainer:node.linebreakContainer,linebreakAlign:node.linebreakAlign}),
      core:ref(node.core()),core_mo:ref(node.coreMO()),core_index:value(node.coreIndex()),
      child_position:value(node.childPosition()),
      text:typeof node.getText==='function' ? node.getText() : null,
      xml:typeof node.getSerializedXML==='function' ? node.getSerializedXML() : null});
  }
  return result;
}

const bboxFields=['w','h','d','scale','rscale','L','R','ic','oc','sk','dx','pwidth'];
function bbox(box) {
  if (!box) return null;
  return Object.fromEntries(bboxFields.map(key=>[key,value(box[key])]));
}
function lineBBox(box) {
  return box ? {...bbox(box),state:value({indentData:box.indentData,lineLeading:box.lineLeading,
    originalL:box.originalL,isFirst:box.isFirst,start:box.start,end:box.end})} : null;
}

function domSnapshot(adaptor,node) {
  const kind=adaptor.kind(node);
  if (kind==='#text' || kind==='#comment') return {kind,text:adaptor.value(node)};
  return {kind,attributes:Object.fromEntries(adaptor.allAttributes(node).map(({name,value})=>[name,String(value)]).sort((a,b)=>a[0]<b[0]?-1:a[0]>b[0]?1:0)),
    children:adaptor.childNodes(node).map(child=>domSnapshot(adaptor,child))};
}

function createCapture() {
  const reference=createReference();
  const {input,output,document,adaptor}=reference;
  const {SvgWrapper}=reference.require('@mathjax/src/js/output/svg/Wrapper.js');
  let active=null;
  let wrappers=null;
  function wrapperRef(wrapper) {
    if (!wrappers.has(wrapper)) wrappers.set(wrapper,'w'+wrappers.size);
    return wrappers.get(wrapper);
  }
  function addWrappers(wrapper) {
    if (!wrapper || wrappers.has(wrapper)) return;
    wrapperRef(wrapper);
    for (const child of wrapper.childNodes||[]) addWrappers(child);
    // These synthetic wrappers are absent from the semantic input tree.
    for (const key of ['surd','slash']) if (wrapper[key]) addWrappers(wrapper[key]);
  }
  function hook(object,name,observe) {
    const original=object[name];
    assert.equal(typeof original,'function',`Missing upstream method ${name}`);
    object[name]=function(...args) {
      if (!active) return original.apply(this,args);
      let sourceError=null;
      const receiver=this;
      function invoke(...forwarded) {
        try {return original.apply(receiver,forwarded);}
        catch (error) {sourceError=error; throw error;}
      }
      try {return observe.call(this,invoke,args);}
      catch (error) {
        if (error!==sourceError) active.observer_error=error;
        throw error;
      }
    };
  }
  hook(input,'formatError',function(original,args){
    if (active) active.diagnostics.push(value({kind:'formula_error',id:args[0].id,message:args[0].message}));
    return original.apply(this,args);
  });
  hook(input,'compile',function(original,args){
    const result=original.apply(this,args);
    if (active) active.parsed_mml=mmlSnapshot(result);
    return result;
  });
  for (const [i,entry] of [...input.postFilters].entries()) {
    const original=entry.item;
    entry.item=function(...args) {
      const result=original.apply(this,args);
      if (active) {
        try {active.postfilters.push({index:i,priority:entry.priority,name:original.name||'<anonymous>',
          tree:mmlSnapshot(args[0].data.root)});}
        catch (error) {active.observer_error=error; throw error;}
      }
      return result;
    };
  }
  hook(output,'processMath',function(original,args){
    const [wrapper,parent]=args;
    addWrappers(wrapper);
    const result=original.apply(this,args);
    if (active) {
      // Run after layout/emission so lazy/stretched/linebroken boxes are final.
      // Include newly constructed wrappers registered during emission as well.
      if (this.nodeMap) for (const w of this.nodeMap.values()) addWrappers(w);
      addWrappers(wrapper);
      const ids=new Map();
      const tree=mmlSnapshot(wrapper.node,{ids,extras:[...wrappers].map(([w,id])=>[w.node,'synthetic/'+id])});
      active.layout_runs.push({root:wrapperRef(wrapper),
        mml:tree,container:domSnapshot(adaptor,parent),
        wrappers:[...wrappers].map(([w,id])=>{
          const lines=[];
          for (let i=0;i<=w.breakCount;i++) lines.push(lineBBox(w.getLineBBox(i)));
          const state={};
          for (const key of ['variant','size','stretch','delimChar','surdH','pad','dx','bboxComputed','isStack','pwidth','breakStyle','multChar'])
            if (key in w) state[key]=value(w[key]);
          return {id,node_id:ids.get(w.node),kind:w.kind,parent:w.parent?wrapperRef(w.parent):null,
            children:(w.childNodes||[]).map(wrapperRef),bbox:bbox(w.getBBox()),outer_bbox:bbox(w.getOuterBBox()),
            break_count:w.breakCount,line_bboxes:lines,state,query_state:'after_bbox_and_line_probes',
            removed_styles:value(w.removedStyles),style_data:value(w.styleData),
            synthetic:Object.fromEntries(['surd','slash'].filter(k=>w[k]).map(k=>[k,wrapperRef(w[k])]))};
        })});
    }
    return result;
  });
  hook(SvgWrapper.prototype,'place',function(original,args){
    if (active) active.placements.push({wrapper:wrapperRef(this),method:'place',x:value(args[0]),y:value(args[1]),
      dx:value(this.dx),rscale:value(this.bbox.rscale),explicit_element:!!args[2]});
    return original.apply(this,args);
  });
  hook(SvgWrapper.prototype,'placeChar',function(original,args){
    if (active) active.placements.push({wrapper:wrapperRef(this),method:'placeChar',codepoint:args[0],
      x:value(args[1]),y:value(args[2]),variant:args[4]===undefined||args[4]===null?this.variant:args[4],buffer:!!args[5]});
    return original.apply(this,args);
  });
  hook(output.font,'getChar',function(original,args){
    const result=original.apply(this,args);
    if (active) active.glyph_queries.push({variant:args[0],codepoint:args[1],data:value(result)});
    return result;
  });

  function capture(request) {
    assert.equal(active,null,'Capture is synchronous and non-reentrant');
    assert.equal(typeof request.tex,'string');
    const render={tex:request.tex,display:!!request.display,em:Number(request.em)||16,width:Number(request.width)||1280};
    active={schema:1,request:render,provenance:reference.provenance,parsed_mml:null,
      postfilters:[],diagnostics:[],layout_runs:[],placements:[],glyph_queries:[],result:null};
    wrappers=new Map();
    const oldWarn=console.warn;
    console.warn=(...messages)=>active.diagnostics.push({kind:'warning',message:messages.map(String).join(' ')});
    try {
      const node=document.convert(render.tex,{display:render.display,em:render.em,ex:render.em*0.5,containerWidth:render.width});
      if (active.observer_error) throw active.observer_error;
      const children=adaptor.childNodes(node);
      const fragments=[];
      const separators=[];
      for (const child of children) {
        if (adaptor.kind(child)==='svg') fragments.push({svg:adaptor.serializeXML(child),tree:domSnapshot(adaptor,child)});
        else separators.push({before_fragment:fragments.length,tree:domSnapshot(adaptor,child)});
      }
      active.result={kind:active.diagnostics.some(d=>d.kind==='formula_error')?'formula_error':'rendered',
        serialized:adaptor.serializeXML(node),container:domSnapshot(adaptor,node),fragments,separators};
      assert.ok(fragments.length,'Reference render has no SVG fragments');
    } catch (error) {
      // Instrumentation failures are never reclassified as valid reference errors.
      if (active.observer_error || error.name==='AssertionError') {
        const observerError=active.observer_error||error;
        active=null; wrappers=null;
        throw observerError;
      }
      active.result={kind:'exception',name:error.name,message:error.message};
    } finally {console.warn=oldWarn;}
    const snapshot=active;
    active=null;
    wrappers=null;
    return snapshot;
  }
  return {capture,reference};
}
module.exports={createCapture,value,mmlSnapshot,bbox,domSnapshot};
if (require.main===module) {
  const fs=require('node:fs');
  const requests=JSON.parse(fs.readFileSync(0,'utf8'));
  if (process.argv.includes('--render-only')) {
    const {document,adaptor}=createReference();
    console.log(JSON.stringify(requests.map(request=>{
      try {
        const em=Number(request.em)||16;
        const node=document.convert(request.tex,{display:!!request.display,em,ex:em*0.5,containerWidth:Number(request.width)||1280});
        return {serialized:adaptor.serializeXML(node)};
      } catch (error) {return {exception:{name:error.name,message:error.message}};}
    })));
  } else {
    const {capture}=createCapture();
    console.log(JSON.stringify(requests.map(capture),null,2));
  }
}
