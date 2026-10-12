//! Root-frozen original private Pixmap storage-size helper responsibilities.
use tiny_skia_path::IntSize;use std::num::NonZeroUsize;
pub fn min_row_bytes(size:IntSize)->Option<NonZeroUsize>{unimplemented!("Unimplemented Root-frozen min_row_bytes")}
pub fn compute_data_len(size:IntSize,row_bytes:usize)->Option<usize>{unimplemented!("Unimplemented Root-frozen compute_data_len")}
pub fn data_len_for_size(size:IntSize)->Option<usize>{unimplemented!("Unimplemented Root-frozen data_len_for_size")}
