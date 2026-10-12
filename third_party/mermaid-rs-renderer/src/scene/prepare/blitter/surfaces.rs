//! Root-frozen eight original constructors/projections; no renderer ownership here.
use tiny_skia_path::{IntSize,IntRect};
use super::surface_types::{BorrowedPixmapMut,OwnedMask,LogicalTarget,LogicalMaskRef};
use super::surface_size::data_len_for_size;
impl<'a> BorrowedPixmapMut<'a>{
 pub fn from_bytes(data:&'a mut [u8],width:u32,height:u32)->Option<Self>{unimplemented!("Unimplemented Root-frozen PixmapMut::from_bytes")}
 pub fn as_subpixmap(&mut self)->LogicalTarget<'_>{unimplemented!("Unimplemented Root-frozen PixmapMut::as_subpixmap")}
 pub fn subpixmap(&mut self,rect:IntRect)->Option<LogicalTarget<'_>>{unimplemented!("Unimplemented Root-frozen PixmapMut::subpixmap")}
}
impl OwnedMask{
 pub fn from_vec(data:Vec<u8>,size:IntSize)->Option<Self>{unimplemented!("Unimplemented Root-frozen Mask::from_vec")}
 pub fn as_submask(&self)->LogicalMaskRef<'_>{unimplemented!("Unimplemented Root-frozen Mask::as_submask")}
 pub fn submask(&self,rect:IntRect)->Option<LogicalMaskRef<'_>>{unimplemented!("Unimplemented Root-frozen Mask::submask")}
 pub fn as_subpixmap(&mut self)->LogicalTarget<'_>{unimplemented!("Unimplemented Root-frozen Mask::as_subpixmap")}
 pub fn subpixmap(&mut self,rect:IntRect)->Option<LogicalTarget<'_>>{unimplemented!("Unimplemented Root-frozen Mask::subpixmap")}
}
