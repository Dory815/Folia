//! Root-frozen radial pre/color/post ordering. Consumers share this plan.
#[repr(u8)]
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub enum RadialStage {
    XYToRadius=1, ApplyConcentricScaleBias=2,
    XYTo2PtConicalStrip=3, Mask2PtConicalNan=4,
    XYTo2PtConicalFocalOnCircle=5, XYTo2PtConicalWellBehaved=6,
    XYTo2PtConicalSmaller=7, XYTo2PtConicalGreater=8,
    Mask2PtConicalDegenerates=9, NegateX=10,
    Alter2PtConicalCompensateFocal=11, Alter2PtConicalUnswap=12,
    ApplyVectorMask=13,
}

#[derive(Debug,Clone,PartialEq)]
pub struct RadialStagePlan {
    pub p0:f32,
    pub p1:f32,
    pub before_color:Vec<RadialStage>,
    pub after_color:Vec<RadialStage>,
    /// Eligibility of these stages only; final pipeline also checks all other stages and force-HQ.
    pub geometry_lowp_compatible:bool,
}
