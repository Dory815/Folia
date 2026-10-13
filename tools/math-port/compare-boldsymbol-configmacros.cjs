'use strict';
// Compare every emitted native observation; Source input metadata is not output.
const fs=require('node:fs');
const path=require('node:path');
const assert=require('node:assert/strict');
const crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..');
const sourcePath=path.resolve(process.argv[2]||path.join(root,'tools/math-port/boldsymbol-configmacros-reference/original.json'));
const nativePath=path.resolve(process.argv[3]||path.join(root,'build/math-port/boldsymbol-configmacros-native.json'));
const sourceBytes=fs.readFileSync(sourcePath);
assert.equal(crypto.createHash('sha256').update(sourceBytes).digest('hex'),
  '7a1683ebd2385cda44d273082658e54aaa437a6a78a187579aac60be120e969c',
  'Source corpus differs from independently repeated pinned CJS output');
const source=JSON.parse(sourceBytes),native=JSON.parse(fs.readFileSync(nativePath));
assert.deepStrictEqual(Object.keys(native),['cases'],'unexpected native top-level schema');
assert.equal(source.upstream.version,'4.1.3');
assert.equal(source.upstream.commit,'fb987178c2d279a99b0db5ea02e751691703955e');
assert.deepStrictEqual(native.cases.map(row=>row.name),source.cases.map(row=>row.name),
  'native case coverage/order differs from Source');
const fields=['name','result','error','maps','packageData'];
const failures=[];
for(let i=0;i<source.cases.length;i++){
  const original=source.cases[i],actual=native.cases[i];
  try{
    assert.deepStrictEqual(Object.keys(actual).sort(),[...fields].sort(),
      `${original.name}: missing or unexpected output fields`);
    const expected=Object.fromEntries(fields.map(key=>[key,original[key]]));
    assert.deepStrictEqual(actual,expected,`${original.name}: observed output differs`);
  }catch(error){
    failures.push(original.name);
    process.stderr.write(error.message.split('\n').slice(0,22).join('\n')+'\n');
  }
}
process.stdout.write(JSON.stringify({compared:source.cases.length,
  matched:source.cases.length-failures.length,failures,
  observedFields:fields,inputMetadata:['input','packages','options']})+'\n');
if(failures.length)process.exitCode=1;
