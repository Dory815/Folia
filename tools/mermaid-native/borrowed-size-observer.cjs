// Original private Pixmap storage helpers, exported only by an appended observer.
'use strict';module.exports={
'src/pixmap.rs':`
#[allow(missing_docs)]
pub fn folia_borrowed_size(width:u32,height:u32,row_bytes:usize)->(bool,Option<usize>,Option<usize>,Option<usize>){let Some(size)=IntSize::from_wh(width,height)else{return(false,None,None,None)};(true,min_row_bytes(size).map(core::num::NonZeroUsize::get),compute_data_len(size,row_bytes),data_len_for_size(size))}
`,
'src/lib.rs':'\n#[allow(missing_docs)] pub use pixmap::folia_borrowed_size;\n',
};
