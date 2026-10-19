/*************************************************************
 * MathJax 4.1.3 output/svg/WrapperFactory.ts.
 * SvgWrapperFactory differs from CommonWrapperFactory only in the actual
 * class map supplied to the same creator-table construction path.
 *************************************************************/
export module folia.math.output.svg.wrapper_factory;
export import folia.math.output.common;
import std;

export namespace folia::math::output::svg {
class SvgWrapperFactory final : public common::CommonWrapperFactory {
public:
    explicit SvgWrapperFactory(common::CommonWrapperClassMap actualSvgWrappers);
    static common::CommonWrapperClassMap defaultNodes(
        common::CommonWrapperClassMap actualSvgWrappers);
};
}
