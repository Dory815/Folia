//! One Source-capacity stage list and one persistent context per builder.
use arrayvec::ArrayVec;
use tiny_skia_path::Transform;
use super::{pipeline_context_types::PipelineContext,pipeline_control::compile_pipeline,pipeline_control_types::{PipelineStage,CompiledPipeline}};
#[derive(Debug)]
pub struct PipelineBuilder {
    stages:ArrayVec<PipelineStage,32>,
    force_hq_pipeline:bool,
    pub context:PipelineContext,
}
impl PipelineBuilder {
    pub fn new()->Self{Self{stages:ArrayVec::new(),force_hq_pipeline:false,context:PipelineContext::default()}}
    pub fn set_force_hq_pipeline(&mut self,value:bool){self.force_hq_pipeline=value;}
    pub fn force_hq_pipeline(&self)->bool{self.force_hq_pipeline}
    pub fn stages(&self)->&[PipelineStage]{self.stages.as_slice()}
    pub fn push(&mut self,stage:PipelineStage){self.stages.push(stage);}
    pub fn push_transform(&mut self,transform:Transform){
        if transform.is_finite() && !transform.is_identity() {
            self.stages.push(PipelineStage::Transform);
            self.context.transform=transform;
        }
    }
    /// Consume the sole owner through the already accepted compile control.
    pub fn compile(self)->CompiledPipeline<PipelineContext>{compile_pipeline(self.stages.as_slice(),self.force_hq_pipeline,self.context)}
}
impl Default for PipelineBuilder{fn default()->Self{Self::new()}}
