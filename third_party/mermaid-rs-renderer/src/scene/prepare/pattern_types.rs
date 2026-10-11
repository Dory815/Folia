//! Borrowed prepared Pattern state; no persistent pipeline context is retained here.
use tiny_skia_path::{NormalizedF32,Transform};
use super::sampling::pattern::SourcePixmap;
use crate::scene::paint::SpreadMode;
#[repr(u8)]
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub enum FilterQuality{Nearest=0,Bilinear=1,Bicubic=2}
#[derive(Debug,Clone)]
pub struct PreparedPattern<'source>{
    pub source:SourcePixmap<'source>,pub spread:SpreadMode,pub quality:FilterQuality,
    pub opacity:NormalizedF32,pub transform:Transform,
}
