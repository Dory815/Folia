'use strict';
// Empirical Number conversion witnesses for the shared native Context edge.
// These do not stand in for a TeX/layout/render corpus.
const records=[];
function observe(name,make) {
  const events=[];
  let value,error=null;
  try {value=Number(make(events));}
  catch(caught) {error={name:caught.name,message:caught.message};}
  const result=error?null:Number.isNaN(value)?{$:'NaN'}:
    Object.is(value,-0)?{$:'-0'}:value;
  records.push({name,events,result,error});
}
observe('number-hint-exotic',events=>{
  const object={};
  Object.defineProperty(object,Symbol.toPrimitive,{get(){
    events.push('get exotic');
    return function(hint){events.push('exotic '+hint+' '+(this===object));return '12';};
  }});
  Object.defineProperty(object,'valueOf',{get(){throw Error('must not get valueOf');}});
  return object;
});
observe('ordinary-number-order',events=>{
  const object={};
  Object.defineProperty(object,'valueOf',{get(){
    events.push('get valueOf');
    return function(){events.push('valueOf '+(this===object));return object;};
  }});
  Object.defineProperty(object,'toString',{get(){
    events.push('get toString');
    return function(){events.push('toString '+(this===object));return '17';};
  }});
  return object;
});
observe('abrupt-exotic',events=>({[Symbol.toPrimitive](hint){
  events.push('exotic '+hint);throw new RangeError('number-probe');
}}));
observe('symbol-exotic-result',events=>({[Symbol.toPrimitive](hint){
  events.push('exotic '+hint);return Symbol('probe');
}}));
observe('noncallable-valueOf',events=>({
  valueOf:4,toString(){events.push('toString');return '-0';}
}));
observe('object-exotic-result',events=>({[Symbol.toPrimitive](hint){
  events.push('exotic '+hint);return {};
}}));
process.stdout.write(JSON.stringify({node:process.version,v8:process.versions.v8,records},null,2)+'\n');
