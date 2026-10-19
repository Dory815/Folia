/*************************************************************
 * SvgWrapperFactory uses the exact Source SvgWrappers class map as the
 * defaults for its inherited live creator object.
 *************************************************************/
module folia.math.output.svg.wrapper_factory;
import std;

namespace folia::math::output::svg {
common::CommonWrapperClassMap SvgWrapperFactory::defaultNodes(
    common::CommonWrapperClassMap actualSvgWrappers){
    return actualSvgWrappers;
}

SvgWrapperFactory::SvgWrapperFactory(common::CommonWrapperClassMap actualSvgWrappers)
    :common::CommonWrapperFactory(defaultNodes(std::move(actualSvgWrappers))){}
}
