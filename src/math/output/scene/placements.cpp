module folia.math.output.scene;
import :placements;

namespace folia::math::output::scene {
void registerScenePlacements(WrapperFactory& factory){
    registerInheritedSvgWrappers(factory);
    registerMoPlacement(factory);
    registerMphantomPlacement(factory);
    registerMpaddedPlacement(factory);
    registerMtablePlacement(factory);
    registerMtrPlacement(factory);
    registerMtdPlacement(factory);
    registerTextNodePlacement(factory);
    registerMathPlacement(factory);
    registerMfencedPlacement(factory);
    registerMactionPlacement(factory);
    registerSemanticsPlacement(factory);
    registerTeXAtomPlacement(factory);
    registerMglyphPlacement(factory);
    registerMrowSvgWrapper(factory);
    registerScriptbasePlacement(factory);
    registerMsubsupPlacement(factory);
    registerMunderoverPlacement(factory);
    registerMmultiscriptsPlacement(factory);
    registerXmlNodePlacement(factory);
}
}
