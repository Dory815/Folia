'use strict';
// Focused Source oracle for core/MathDocument.convert. The output action list
// is intentionally compile-only so the trace isolates document conversion,
// dynamic InputJax.reduce dispatch, and the temporary MathItem lifecycle.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');

const root=path.resolve(__dirname,'../..');
const packages=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax');
const src=path.join(packages,'src/cjs');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const sourceFile=path.join(packages,'src/ts/core/MathDocument.ts');
const sourceHash=digest(sourceFile);

const {mathjax}=require(path.join(src,'mathjax.js'));
const {TeX}=require(path.join(src,'input/tex.js'));
const {SVG}=require(path.join(src,'output/svg.js'));
const {liteAdaptor}=require(path.join(src,'adaptors/liteAdaptor.js'));
const {RegisterHTMLHandler}=require(path.join(src,'handlers/html.js'));
const {RenderList}=require(path.join(src,'core/MathDocument.js'));
const {MathJaxNewcmFont}=require(path.join(packages,'mathjax-newcm-font/cjs/svg.js'));

const adaptor=liteAdaptor({fontSize:16});
RegisterHTMLHandler(adaptor);
const input=new TeX({packages:['base']});
const output=new SVG({fontData:MathJaxNewcmFont,fontCache:'none'});
const document=mathjax.document('',{InputJax:input,OutputJax:output});
document.renderActions=RenderList.create({compile:[20]});

const trace=[];
const originalClear=document.clearPromises;
document.clearPromises=function(){trace.push({event:'clearPromises',receiver:this===document});return originalClear.apply(this,arguments);};

const inputProxy=new Proxy(input,{
  get(target,key,receiver){
    if(key==='name')trace.push({event:'inputJax.name'});
    return Reflect.get(target,key,receiver);
  }
});
const inputProxyLast=new Proxy(input,{
  get(target,key,receiver){
    if(key==='name')trace.push({event:'inputJax.last.name'});
    return Reflect.get(target,key,receiver);
  }
});
const inputArray=new Proxy([inputProxy,inputProxyLast],{
  get(target,key,receiver){
    if(key==='0'||key==='reduce')trace.push({event:`inputJax.${String(key)}`});
    if(key==='reduce'){
      const reduce=Reflect.get(target,key,receiver);
      return function(callback,initial){
        trace.push({event:'inputJax.reduce.call',receiver:this===inputArray,hasInitial:arguments.length>1});
        return Reflect.apply(reduce,this,[function(accumulator,current,index,array){
          trace.push({event:'inputJax.reduce.callback',index});
          return callback(accumulator,current,index,array);
        },initial]);
      };
    }
    return Reflect.get(target,key,receiver);
  }
});
document.inputJax=inputArray;

const OriginalMathItem=document.options.MathItem;
const originalSetMetrics=OriginalMathItem.prototype.setMetrics;
OriginalMathItem.prototype.setMetrics=function(em,ex,containerWidth,scale){
  trace.push({event:'setMetrics',em,ex,containerWidth,scale,
    startNode:this.start.node===adaptor.body(document.document)});
  return originalSetMetrics.apply(this,arguments);
};
const originalCompile=OriginalMathItem.prototype.compile;
OriginalMathItem.prototype.compile=function(){
  trace.push({event:'compile.enter',state:this.state(),startNode:this.start.node===adaptor.body(document.document)});
  try{
    const result=originalCompile.apply(this,arguments);
    trace.push({event:'compile.return',rootKind:this.root&&this.root.kind,
      state:this.state(),argumentCount:arguments.length});
    return result;
  }catch(error){
    trace.push({event:'compile.throw',name:error&&error.name,message:String(error),
      rootKind:this.root&&this.root.kind,state:this.state(),argumentCount:arguments.length});
    throw error;
  }
};
function TracedMathItem(...args){
  trace.push({event:'MathItem.construct',argumentCount:args.length,
    math:args[0],selectedInputIsLast:args[1]===inputProxyLast,display:args[2]});
  return Reflect.construct(OriginalMathItem,args,OriginalMathItem);
}
TracedMathItem.prototype=OriginalMathItem.prototype;
Object.setPrototypeOf(TracedMathItem,OriginalMathItem);
document.options.MathItem=TracedMathItem;

const optionReads=[];
const supplied={};
for(const [name,value] of Object.entries({
  format:'TeX',display:false,end:999,ex:7,em:13,containerWidth:null,scale:1.25,family:''
}))Object.defineProperty(supplied,name,{enumerable:true,get(){optionReads.push(name);return value;}});
const result=document.convert('x+y',supplied);
const nullWidthTrace=trace.slice();
const nullWidthReads=optionReads.slice();
assert.deepEqual(optionReads,['format','display','end','ex','em','containerWidth','scale','family']);
assert.equal(nullWidthTrace.find(item=>item.event==='MathItem.construct')?.argumentCount,3);
assert.equal(nullWidthTrace.find(item=>item.event==='MathItem.construct')?.selectedInputIsLast,true);
assert.ok(trace.some(item=>item.event==='inputJax.reduce.call'&&item.hasInitial));
assert.ok(trace.findIndex(item=>item.event==='clearPromises')>
  trace.findIndex(item=>item.event==='setMetrics'));
assert.equal(nullWidthTrace.find(item=>item.event==='setMetrics')?.containerWidth,560);

trace.length=0;
optionReads.length=0;
const undefinedWidthResult=document.convert('x+y',{containerWidth:undefined});
const undefinedWidthTrace=trace.slice();
assert.equal(undefinedWidthTrace.find(item=>item.event==='setMetrics')?.containerWidth,undefined);
assert.equal(undefinedWidthTrace.find(item=>item.event==='MathItem.construct')?.argumentCount,3);

const outputRecord={source:'core/MathDocument.ts',sourceSha256:sourceHash,
  nullWidth:{optionGetterReads:nullWidthReads,trace:nullWidthTrace,
    resultKind:result&&adaptor.kind(result)},
  undefinedWidth:{trace:undefinedWidthTrace,
    resultKind:undefinedWidthResult&&adaptor.kind(undefinedWidthResult)}};
process.stdout.write(JSON.stringify(outputRecord,null,2)+'\n');
