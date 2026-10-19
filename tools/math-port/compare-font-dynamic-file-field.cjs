#!/usr/bin/env node
'use strict';

const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'../..');
const source=JSON.parse(fs.readFileSync(process.argv[2]||path.join(root,'build/math-port/font-dynamic-file-field-original.json'),'utf8'));
const native=JSON.parse(fs.readFileSync(process.argv[3]||path.join(root,'build/math-port/font-dynamic-file-field-native.json'),'utf8'));
function first(a,b,at='$'){
  if(Object.is(a,b))return null;
  if(a===null||b===null||typeof a!=='object'||typeof b!=='object')return {path:at,source:a,native:b};
  const ak=Object.keys(a).sort(),bk=Object.keys(b).sort();
  if(ak.length!==bk.length||ak.some((k,i)=>k!==bk[i]))return {path:`${at}.$keys`,source:ak,native:bk};
  for(const key of ak){const difference=first(a[key],b[key],`${at}.${key}`);if(difference)return difference;}
  return null;
}
const diff=first(source.ordinaryFields,native.ordinaryFields);
if(diff){process.stdout.write(`${JSON.stringify(diff)}\n`);process.exitCode=1;}
else process.stdout.write('DynamicFile ordinary field replacement matched Source\n');
