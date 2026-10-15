'use strict';
// Pinned Source behavior witness for the two distinct SVG tooltip branches.
const {createReference}=require('../reference-profile.cjs');
const ref=createReference();
const settings={display:false,em:16,ex:8,containerWidth:1280};
const records=[];
for(const tex of ['\\mathtip{x+1}{y^2}','\\texttip{x}{tip}']){
  try{
    const node=ref.document.convert(tex,settings);
    const svg=ref.adaptor.serializeXML(node);
    records.push({tex,error:null,svgLength:svg.length,title:svg.includes('<title>tip</title>')});
  }catch(error){
    records.push({tex,error:{name:error.name,message:error.message}});
  }
}
console.log(JSON.stringify({version:ref.version,records}));
