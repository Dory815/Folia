// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Original constructors and borrowed surface projections; pipeline ownership is separate.
use tiny_skia_path::{IntSize,IntRect};
use super::surface_types::{BorrowedPixmapMut,OwnedMask,LogicalTarget,LogicalMaskRef};
use super::surface_size::data_len_for_size;
impl<'a> BorrowedPixmapMut<'a>{
 pub fn from_bytes(data:&'a mut [u8],width:u32,height:u32)->Option<Self>{
  let size=IntSize::from_wh(width,height)?;
  let data_len=data_len_for_size(size)?;
  if data.len()<data_len{return None;}
  Some(Self{data,size})
 }
 pub fn as_subpixmap(&mut self)->LogicalTarget<'_>{
  LogicalTarget{size:self.size,real_width:self.size.width() as usize,data:&mut *self.data}
 }
 pub fn subpixmap(&mut self,rect:IntRect)->Option<LogicalTarget<'_>>{
  let rect=self.size.to_int_rect(0,0).intersect(&rect)?;
  let row_bytes=self.size.width() as usize*4;
  let offset=rect.top() as usize*row_bytes+rect.left() as usize*4;
  Some(LogicalTarget{size:rect.size(),real_width:self.size.width() as usize,data:&mut self.data[offset..]})
 }
}
impl OwnedMask{
 pub fn from_vec(data:Vec<u8>,size:IntSize)->Option<Self>{
  let data_len=size.width() as usize*size.height() as usize;
  if data.len()!=data_len{return None;}
  Some(Self{data,size})
 }
 pub fn as_submask(&self)->LogicalMaskRef<'_>{
  LogicalMaskRef{size:self.size,real_width:self.size.width(),data:&self.data}
 }
 pub fn submask(&self,rect:IntRect)->Option<LogicalMaskRef<'_>>{
  let rect=self.size.to_int_rect(0,0).intersect(&rect)?;
  let row_bytes=self.size.width() as usize;
  let offset=rect.top() as usize*row_bytes+rect.left() as usize;
  Some(LogicalMaskRef{size:rect.size(),real_width:self.size.width(),data:&self.data[offset..]})
 }
 pub fn as_subpixmap(&mut self)->LogicalTarget<'_>{
  LogicalTarget{size:self.size,real_width:self.size.width() as usize,data:&mut self.data}
 }
 pub fn subpixmap(&mut self,rect:IntRect)->Option<LogicalTarget<'_>>{
  let rect=self.size.to_int_rect(0,0).intersect(&rect)?;
  let row_bytes=self.size.width() as usize;
  let offset=rect.top() as usize*row_bytes+rect.left() as usize;
  Some(LogicalTarget{size:rect.size(),real_width:self.size.width() as usize,data:&mut self.data[offset..]})
 }
}
