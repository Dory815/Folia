// Root append-only calls to original constructors and borrowed subregion APIs.
'use strict';
module.exports={
'src/pixmap.rs':`
#[allow(missing_docs)]
pub type FoliaBorrowedSurface=(alloc::string::String,Option<(u32,u32,usize,usize,usize,Vec<u8>,Vec<u8>)>,Vec<u8>);
#[allow(missing_docs)]
pub fn folia_borrowed_pixmap(mut data:Vec<u8>,width:u32,height:u32,rect:Option<[i32;4]>,write:Option<(usize,u8)>)->FoliaBorrowedSurface{
 let base=data.as_ptr() as usize;
 let result=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||{
  let Some(mut p)=PixmapMut::from_bytes(&mut data,width,height)else{return(alloc::string::String::from("ctor-none"),None)};
  let mut view=if let Some(r)=rect{let Some(r)=IntRect::from_xywh(r[0],r[1],r[2] as u32,r[3] as u32)else{return(alloc::string::String::from("rect-none"),None)};let Some(v)=p.subpixmap(r)else{return(alloc::string::String::from("rect-none"),None)};v}else{p.as_subpixmap()};
  let offset=view.data.as_ptr() as usize-base;let before=view.data.to_vec();
  if let Some((index,value))=write{if let Some(byte)=view.data.get_mut(index){*byte=value;}}
  (alloc::string::String::from("some"),Some((view.size.width(),view.size.height(),view.real_width,offset,view.data.len(),before,view.data.to_vec())))
 }));
 let(status,view)=result.unwrap_or_else(|_|(alloc::string::String::from("panic"),None));(status,view,data)
}
`,
'src/mask.rs':`
#[allow(missing_docs)]
pub fn folia_borrowed_mask(data:Vec<u8>,width:u32,height:u32,rect:Option<[i32;4]>,target:bool,write:Option<(usize,u8)>)->crate::pixmap::FoliaBorrowedSurface{
 let Some(size)=IntSize::from_wh(width,height)else{return(alloc::string::String::from("ctor-none"),None,data)};
 let original=data.clone();let Some(mut mask)=Mask::from_vec(data,size)else{return(alloc::string::String::from("ctor-none"),None,original)};let base=mask.data.as_ptr() as usize;
 let result=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||{
  let r=if let Some(r)=rect{let Some(r)=IntRect::from_xywh(r[0],r[1],r[2] as u32,r[3] as u32)else{return(alloc::string::String::from("rect-none"),None)};Some(r)}else{None};
  let record=if target{let mut v=if let Some(r)=r{let Some(v)=mask.subpixmap(r)else{return(alloc::string::String::from("rect-none"),None)};v}else{mask.as_subpixmap()};let offset=v.data.as_ptr() as usize-base;let before=v.data.to_vec();if let Some((i,b))=write{if let Some(byte)=v.data.get_mut(i){*byte=b;}}(v.size.width(),v.size.height(),v.real_width,offset,v.data.len(),before,v.data.to_vec())}
  else{let v=if let Some(r)=r{let Some(v)=mask.submask(r)else{return(alloc::string::String::from("rect-none"),None)};v}else{mask.as_submask()};(v.size.width(),v.size.height(),v.real_width as usize,v.data.as_ptr() as usize-base,v.data.len(),v.data.to_vec(),v.data.to_vec())};
  (alloc::string::String::from("some"),Some(record))
 }));let(status,view)=result.unwrap_or_else(|_|(alloc::string::String::from("panic"),None));(status,view,mask.data)
}
`,
'src/lib.rs':'\n#[allow(missing_docs)] pub use pixmap::{FoliaBorrowedSurface,folia_borrowed_pixmap};\n#[allow(missing_docs)] pub use mask::folia_borrowed_mask;\n',
};
