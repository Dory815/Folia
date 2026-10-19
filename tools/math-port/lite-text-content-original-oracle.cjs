'use strict';

const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const pins = {
  'ts/adaptors/liteAdaptor.ts': '6aded576671ae8cf4b1146cc82c793628fa593abdc00d05e3e8880c53fe6a214',
  'cjs/adaptors/liteAdaptor.js': 'e9879de720f356cf8f54c0960102b1e40b4eafc1e34a14481aa896e1e99bd76f',
  'cjs/adaptors/lite/Text.js': 'aab6de43d689d449401990932909c895bc29b76e28bcf22158d6f543a2059ffd'
};
const digest = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
for (const [file, expected] of Object.entries(pins)) {
  const actual = digest(path.join(file.startsWith('ts/') ? sourceRoot : packageRoot, file));
  if (actual !== expected) throw new Error(`Pinned Source changed: ${file}`);
}
const {liteAdaptor} = require(path.join(packageRoot, 'cjs/adaptors/liteAdaptor.js'));
const records=[];
function observe(name,run) {
  try {records.push({name,actual:{value:run()}});}
  catch(error) {records.push({name,actual:{error:{name:error?.name,message:error?.message}}});}
}

observe('nested_element_text_and_comment',()=>{
  const adaptor=liteAdaptor();
  const root=adaptor.node('span',{},[
    adaptor.text('alpha'),adaptor.comment('ignored'),
    adaptor.node('b',{},[adaptor.text('beta')])
  ]);
  return adaptor.textContent(root);
});
observe('text_receiver_with_assigned_children',()=>{
  const adaptor=liteAdaptor();
  const text=adaptor.text('original');
  text.children=[adaptor.text('custom'),adaptor.comment('hidden')];
  return adaptor.textContent(text);
});
observe('children_reduce_getter_and_callback_order',()=>{
  const adaptor=liteAdaptor();
  const trace=[];
  const child={};
  Object.defineProperty(child,'kind',{get(){trace.push('kind');return '#text';}});
  Object.defineProperty(child,'value',{get(){trace.push('value');return 'x';}});
  const children=[child];
  Object.defineProperty(children,'reduce',{get(){
    trace.push('reduce-get');
    return function(callback,initial){
      trace.push(`reduce-call:${this===children}:${arguments.length}`);
      return Array.prototype.reduce.call(this,function(acc,value,index,array){
        trace.push(`callback:${index}:${array===children}`);
        return callback(acc,value,index,array);
      },initial);
    };
  }});
  const node={children};
  return {value:adaptor.textContent(node),trace};
});
observe('recursive_call_uses_live_adaptor_method',()=>{
  const adaptor=liteAdaptor();
  const trace=[];
  const child=adaptor.node('b',{},[adaptor.text('ignored')]);
  const parent=adaptor.node('p',{},[child]);
  const original=adaptor.textContent;
  adaptor.textContent=function(node){
    trace.push(node===child?'child':'parent');
    return original.call(this,node);
  };
  return {value:adaptor.textContent(parent),trace};
});
observe('text_receiver_without_children_throws_on_reduce',()=>{
  const adaptor=liteAdaptor();
  return adaptor.textContent(adaptor.text('x'));
});
observe('null_children_throws_on_reduce',()=>{
  const adaptor=liteAdaptor();
  return adaptor.textContent({children:null});
});
observe('text_content_plus_preserves_number_addition_and_primitive_order',()=>{
  const adaptor=liteAdaptor();
  const trace=[];
  const left={valueOf(){trace.push('left');return 2;}};
  const right={valueOf(){trace.push('right');return 3;}};
  const child={kind:'#text',value:right};
  const children=[child];
  children.reduce=function(callback,initial){
    trace.push(`reduce:${this===children}:${arguments.length}`);
    return callback(left,child,0,children);
  };
  return {value:adaptor.textContent({children}),trace};
});
observe('text_content_plus_uses_symbol_to_primitive_default_hint',()=>{
  const adaptor=liteAdaptor();
  const trace=[];
  const child={kind:'#text',value:{[Symbol.toPrimitive](hint){trace.push(hint);return 'x';}}};
  const children=[child];
  children.reduce=function(callback){return callback(2,child,0,children);};
  return {value:adaptor.textContent({children}),trace};
});
observe('text_content_plus_rejects_symbol_primitive',()=>{
  const adaptor=liteAdaptor();
  const child={kind:'#text',value:{[Symbol.toPrimitive](){return Symbol('bad');}}};
  const children=[child];
  children.reduce=function(callback){return callback(2,child,0,children);};
  return adaptor.textContent({children});
});

process.stdout.write(JSON.stringify({runtime:process.version,records},null,2)+'\n');
