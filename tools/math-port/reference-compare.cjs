// Root-owned comparator. Implementation packets cannot weaken this contract.
const fs = require('node:fs');
const zlib = require('node:zlib');

function geometry(path) {
  // Codepoints, script levels, classes, indices, priorities and font records
  // are discrete/exact, even though JavaScript represents them as numbers.
  return /^\$\.layout_runs\[\d+\]\.wrappers\[\d+\]\.(bbox|outer_bbox|line_bboxes\[\d+\])\.(w|h|d|scale|rscale|L|R|ic|oc|sk|dx)$/.test(path)
    || /^\$\.placements\[\d+\]\.(x|y|dx|rscale)$/.test(path);
}

function compare(expected, actual, {limit = 20} = {}) {
  const differences=[];
  function fail(path,reason) {if (differences.length<limit) differences.push({path,reason});}
  function walk(a,b,path) {
    if (differences.length>=limit) return;
    if (typeof a==='number' && typeof b==='number' && geometry(path)) {
      if (!Number.isFinite(a) || !Number.isFinite(b) || Math.abs(a-b)>1e-10+1e-10*Math.max(Math.abs(a),Math.abs(b)))
        fail(path,'geometry differs beyond absolute 1e-10 + relative 1e-10');
      return;
    }
    if (a===null || b===null || typeof a!=='object' || typeof b!=='object') {
      if (!Object.is(a,b)) fail(path,'exact value differs');
      return;
    }
    if (Array.isArray(a)!==Array.isArray(b)) {fail(path,'array/object kind differs'); return;}
    if (Array.isArray(a)) {
      if (a.length!==b.length) fail(path,'array length differs');
      for (let i=0;i<Math.min(a.length,b.length);i++) walk(a[i],b[i],`${path}[${i}]`);
    } else {
      const ak=Object.keys(a).sort(),bk=Object.keys(b).sort();
      if (JSON.stringify(ak)!==JSON.stringify(bk)) fail(path,'own property names differ');
      for (const key of ak) if (Object.hasOwn(b,key)) walk(a[key],b[key],path+'.'+key);
    }
  }
  walk(expected,actual,'$');
  return differences;
}

function read(file) {
  const data=fs.readFileSync(file);
  return JSON.parse(file.endsWith('.gz')?zlib.gunzipSync(data):data.toString('utf8'));
}
module.exports={compare,geometry,read};
if (require.main===module) {
  const [expected,actual]=process.argv.slice(2);
  if (!expected || !actual) throw Error('Usage: node reference-compare.cjs expected.json[.gz] actual.json[.gz]');
  const differences=compare(read(expected),read(actual));
  console.log(JSON.stringify({equal:!differences.length,differences},null,2));
  process.exitCode=differences.length?1:0;
}
