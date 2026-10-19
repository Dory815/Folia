#!/usr/bin/env node
'use strict';

const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'../..');
const original=JSON.parse(fs.readFileSync(path.join(root,'build/math-port/newcm-font-module-provider-original.json'),'utf8'));
const candidatePath=process.argv[2]||path.join(root,'build/math-port/newcm-font-module-provider-native.json');
const candidate=JSON.parse(fs.readFileSync(candidatePath,'utf8'));
function firstDiff(a,b,p='$'){
  if(Object.is(a,b))return null;
  if(a===null||b===null||typeof a!=='object'||typeof b!=='object')return {path:p,expected:a,actual:b};
  if(Array.isArray(a)!==Array.isArray(b))return {path:p,expected:Array.isArray(a)?'array':typeof a,actual:Array.isArray(b)?'array':typeof b};
  const ak=Object.keys(a),bk=Object.keys(b);
  if(ak.length!==bk.length)return {path:p,expectedKeys:ak,actualKeys:bk};
  for(const key of ak){if(!Object.hasOwn(b,key))return {path:`${p}.${key}`,expected:a[key],actual:'<missing>'};
    const found=firstDiff(a[key],b[key],`${p}.${key}`);if(found)return found;}
  return null;
}
const diff=firstDiff(original,candidate);
if(diff){process.stderr.write(`${JSON.stringify(diff,null,2)}\n`);process.exitCode=1;}
else process.stdout.write('NewCM bundled module provider matches the pinned Source trace.\n');
