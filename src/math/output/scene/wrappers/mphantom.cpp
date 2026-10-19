/*************************************************************
 * Source-mapped output/svg/Wrappers/mphantom.js placement.
 *************************************************************/
module folia.math.output.scene;
import :mphantom;

namespace folia::math::output::scene {
void MphantomPlacement::placeContent(common::CommonWrapper&,Builder&,
                                     WrapperFactory&,
                                     std::span<const NodeRef>){
    // Source SvgMphantom.toSVG deliberately stops after standardSvgNodes.
    // Child wrappers still contribute to Common measurement but are not
    // recursively emitted into the SVG scene.
}

void registerMphantomPlacement(WrapperFactory& factory){
    factory.registerKind("mphantom",std::make_shared<MphantomPlacement>());
}
}
