// Root oracle: exact upstream C4 PNG bytes and independent PNG RGB decoding.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),zlib=require('node:zlib'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),source=fs.readFileSync(path.join(root,'.reference/mermaid-registry/mermaid-rs-renderer-0.3.1/src/render.rs'));
const sha=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');
assert.equal(sha(source),'026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4');
const out=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures'),evidence=[];
function emit(name,bytes){const file=path.join(out,name);if(process.argv.includes('--check'))assert.ok(fs.readFileSync(file).equals(bytes),name+' differs from pristine source');else fs.writeFileSync(file,bytes);}
for(const name of ['C4_PERSON_ICON','C4_EXTERNAL_PERSON_ICON']){
 const match=source.toString().match(new RegExp('const '+name+': &str = "data:image/png;base64,([^"]+)";'));assert.ok(match,name);
 const png=Buffer.from(match[1],'base64');assert.equal(png.subarray(0,8).toString('hex'),'89504e470d0a1a0a');let offset=8,header,idat=[];
 while(offset<png.length){const size=png.readUInt32BE(offset),kind=png.toString('ascii',offset+4,offset+8),bytes=png.subarray(offset+8,offset+8+size);if(kind==='IHDR')header=bytes;if(kind==='IDAT')idat.push(bytes);offset+=size+12;}
 const width=header.readUInt32BE(0),height=header.readUInt32BE(4);assert.equal(width,48);assert.equal(height,48);assert.equal(header.subarray(8).toString('hex'),'0802000000','RGB8 noninterlaced PNG');
 const inflated=zlib.inflateSync(Buffer.concat(idat)),stride=width*3,rgb=Buffer.alloc(stride*height);assert.equal(inflated.length,(stride+1)*height);
 for(let y=0;y<height;y++){const filter=inflated[y*(stride+1)];assert.ok(filter<=4);for(let x=0;x<stride;x++){const a=x>=3?rgb[y*stride+x-3]:0,b=y?rgb[(y-1)*stride+x]:0,c=x>=3&&y?rgb[(y-1)*stride+x-3]:0;let predictor=0;if(filter===1)predictor=a;if(filter===2)predictor=b;if(filter===3)predictor=Math.floor((a+b)/2);if(filter===4){const p=a+b-c,pa=Math.abs(p-a),pb=Math.abs(p-b),pc=Math.abs(p-c);predictor=pa<=pb&&pa<=pc?a:pb<=pc?b:c;}rgb[y*stride+x]=(inflated[y*(stride+1)+1+x]+predictor)&255;}}
 const stem=name.toLowerCase();emit(stem+'.png',png);emit(stem+'.rgb',rgb);evidence.push({source_constant:name,width,height,png_sha256:sha(png),rgb_sha256:sha(rgb)});
}
emit('c4-image-reference.json',Buffer.from(JSON.stringify({source_sha256:sha(source),images:evidence},null,2)+'\n'));console.log('Both pristine C4 PNGs and RGB references verified.');
