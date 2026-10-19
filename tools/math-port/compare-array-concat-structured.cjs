'use strict';
const fs=require('node:fs');
const [sourcePath,nativePath]=process.argv.slice(2);
if(!sourcePath||!nativePath){console.error('usage: node compare-array-concat-structured.cjs <source.json> <native.json>');process.exit(2);}
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const native=JSON.parse(fs.readFileSync(nativePath,'utf8'));
const mapping=JSON.parse(fs.readFileSync('tools/math-port/array-concat-row-map.json','utf8'));
const sourceRows=new Map(source.rows.map(row=>[row.name,row]));
const nativeRows=new Map(native.rows.map(row=>[row.name,row]));
const mismatches=[],boundaries=[],matched=[];
if(source.method!=='Array.prototype.concat'||native.method!==source.method)
  mismatches.push({reason:'method identity differs',source:source.method,native:native.method});
if(source.schema!=='tagged-slots-v1'||native.schema!==source.schema)
  mismatches.push({reason:'structured schema differs',source:source.schema,native:native.schema});
for(const row of mapping.rows){
  const expected=sourceRows.get(row.source),actual=nativeRows.get(row.source);
  if(!expected||!actual){mismatches.push({name:row.source,reason:!expected?'missing source row':'missing native row'});continue;}
  if(row.status==='unsupported'){
    boundaries.push({name:row.source,sourceError:expected.error,nativeError:actual.error,nativeResult:actual.result});
    if(expected.error!=='ok'||actual.error==='ok')mismatches.push({name:row.source,reason:'unsupported boundary did not execute both sides as expected'});
    continue;
  }
  if(JSON.stringify(expected)!==JSON.stringify(actual))mismatches.push({name:row.source,expected,actual});
  else matched.push(row.source);
}
if(sourceRows.size!==mapping.rows.length)mismatches.push({reason:'source row count differs from row map',source:sourceRows.size,mapped:mapping.rows.length});
if(nativeRows.size!==mapping.rows.length)mismatches.push({reason:'native row count differs from row map',native:nativeRows.size,mapped:mapping.rows.length});
const report={sourceNode:source.node,nativeSchema:native.schema,matched:matched.length,boundaries,mismatches};
console.log(JSON.stringify(report,null,2));
process.exitCode=mismatches.length?1:boundaries.length?2:0;
