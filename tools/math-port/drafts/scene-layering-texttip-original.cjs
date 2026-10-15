'use strict';
// Read the actual successful Source text-tooltip path with the pinned profile.
const {createReference}=require('../reference-profile.cjs');
const ref=createReference();
const {SvgWrapper}=ref.require('@mathjax/src/js/output/svg/Wrapper.js');
const trace=[];
const originals=Object.fromEntries(['firstChild','kind','getAttribute','next','insert']
  .map(name=>[name,ref.adaptor[name]]));
const kind=node=>node?originals.kind.call(ref.adaptor,node):null;
for(const name of Object.keys(originals)){
  ref.adaptor[name]=function(...args){
    const input=args.map(value=>typeof value==='object'?kind(value):value);
    const result=originals[name].apply(this,args);
    trace.push({method:name,input,result:typeof result==='object'?kind(result):result});
    return result;
  };
}
const firstChild=SvgWrapper.prototype.firstChild;
SvgWrapper.prototype.firstChild=function(...args){
  trace.push({method:'SvgWrapper.firstChild',input:args.map(kind)});
  const result=firstChild.apply(this,args);
  trace.push({method:'SvgWrapper.firstChild.return',result:kind(result)});
  return result;
};
const tex='\\texttip{x}{tip}';
const node=ref.document.convert(tex,{display:false,em:16,ex:8,containerWidth:1280});
console.log(JSON.stringify({provenance:ref.provenance,tex,trace,
  svg:ref.adaptor.serializeXML(node)},null,2));
