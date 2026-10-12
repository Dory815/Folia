'use strict';module.exports={
'src/mask.rs':`
#[allow(missing_docs)]
pub fn folia_mask_ownership(bytes:&[u8],capacity:usize,width:u32,height:u32)->(alloc::string::String,bool,usize,usize){let mut data=Vec::with_capacity(capacity);data.extend_from_slice(bytes);let ptr=data.as_ptr();let before=data.capacity();let Some(size)=IntSize::from_wh(width,height)else{return(alloc::string::String::from("none"),false,before,0)};match Mask::from_vec(data,size){Some(mask)=>(alloc::string::String::from("some"),mask.data.as_ptr()==ptr,before,mask.data.capacity()),None=>(alloc::string::String::from("none"),false,before,0)}}
`,
'src/lib.rs':'\n#[allow(missing_docs)] pub use mask::folia_mask_ownership;\n',
};
