// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Original logical surface and storage representations.
use tiny_skia_path::IntSize;
/// The original mutable contiguous RGBA owner borrows all supplied bytes.
#[derive(Debug)]
pub struct BorrowedPixmapMut<'a>{pub data:&'a mut [u8],pub size:IntSize}
/// Original Mask storage is owned and has an exact logical byte count.
#[derive(Debug)]
pub struct OwnedMask{pub data:Vec<u8>,pub size:IntSize}
/// A8 or RGBA is selected by the original consumer, not this shared raw view.
#[derive(Debug)]
pub struct LogicalTarget<'a>{pub data:&'a mut [u8],pub size:IntSize,pub real_width:usize}
#[derive(Debug,Clone,Copy)]
pub struct LogicalMaskRef<'a>{pub data:&'a [u8],pub size:IntSize,pub real_width:u32}
