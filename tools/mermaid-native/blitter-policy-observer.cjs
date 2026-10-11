// Root append-only original policy/quantization calls. No algorithm substitution.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const source=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0'),text=fs.readFileSync(path.join(source,'src/blend_mode.rs'),'utf8');const variants=text.match(/pub enum BlendMode \{([\s\S]*?)\n\}/)[1].split('\n').map(s=>s.trim()).filter(s=>/^[A-Za-z]+,$/.test(s)).map(s=>s.slice(0,-1));assert.equal(variants.length,29);
module.exports={'src/blend_mode.rs':`
#[allow(missing_docs)]
pub fn folia_blitter_policy(blend:u8,color_space:u8,color:[u32;4])->(alloc::string::String,bool,Option<alloc::string::String>,Option<alloc::string::String>,Option<alloc::string::String>,Option<([u32;4],[u8;4])>){
 let blend=match blend{${variants.map((v,i)=>`${i}=>BlendMode::${v}`).join(',')},_=>panic!("invalid policy transport")};
 let cs=match color_space{0=>crate::ColorSpace::Linear,1=>crate::ColorSpace::Gamma2,2=>crate::ColorSpace::SimpleSRGB,3=>crate::ColorSpace::FullSRGBGamma,_=>panic!("invalid color-space transport")};
 let components=color.map(f32::from_bits);
 let color=crate::Color::from_rgba(components[0],components[1],components[2],components[3]).map(|c|{
  let c=c.premultiply();let bytes=c.to_color_u8();([c.red(),c.green(),c.blue(),c.alpha()].map(f32::to_bits),[bytes.red(),bytes.green(),bytes.blue(),bytes.alpha()])
 });
 (alloc::format!("{:?}",blend),blend.should_pre_scale_coverage(),blend.to_stage().map(|s|alloc::format!("{:?}",s)),cs.expand_dest_stage().map(|s|alloc::format!("{:?}",s)),cs.compress_stage().map(|s|alloc::format!("{:?}",s)),color)
}
`,'src/lib.rs':'\n#[allow(missing_docs)] pub use blend_mode::folia_blitter_policy;\n'};
