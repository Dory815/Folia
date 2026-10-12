// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Original private Pixmap storage-size helper responsibilities.
use tiny_skia_path::IntSize;use std::num::NonZeroUsize;
pub fn min_row_bytes(size:IntSize)->Option<NonZeroUsize>{
 let w=i32::try_from(size.width()).ok()?;
 let w=w.checked_mul(4)?;
 NonZeroUsize::new(w as usize)
}
pub fn compute_data_len(size:IntSize,row_bytes:usize)->Option<usize>{
 let h=size.height().checked_sub(1)?;
 let h=(h as usize).checked_mul(row_bytes)?;
 let w=(size.width() as usize).checked_mul(4)?;
 h.checked_add(w)
}
pub fn data_len_for_size(size:IntSize)->Option<usize>{
 let row_bytes=min_row_bytes(size)?;
 compute_data_len(size,row_bytes.get())
}
