// Root fingerprint of the directories fontdb 0.23 scans on Windows.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const hash=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');
function fingerprint(){
 const dirs=[['system',path.join(process.env.SYSTEMROOT||'C:/Windows','Fonts')]];
 if(process.env.USERPROFILE)for(const kind of ['Local','Roaming'])dirs.push([kind.toLowerCase(),path.join(process.env.USERPROFILE,'AppData',kind,'Microsoft/Windows/Fonts')]);
 const files=[];function walk(label,base,dir){if(!fs.existsSync(dir))return;for(const e of fs.readdirSync(dir,{withFileTypes:true})){const file=path.join(dir,e.name);if(e.isDirectory())walk(label,base,file);else if(e.isFile()&&/\.(ttf|otf|ttc)$/i.test(e.name)){const bytes=fs.readFileSync(file);files.push({path:label+'/'+path.relative(base,file).replaceAll('\\','/'),bytes:bytes.length,sha256:hash(bytes)});}}}
 for(const[label,dir]of dirs)walk(label,dir,dir);files.sort((a,b)=>a.path.localeCompare(b.path,'en'));return files;
}
module.exports={fingerprint,hash};
