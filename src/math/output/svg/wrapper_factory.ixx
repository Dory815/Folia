/*************************************************************
 * Native SVG composition over the shared Common wrapper factory.
 *************************************************************/
export module folia.math.output.svg.wrapper_factory;
export import folia.math.output.common;
import std;

export namespace folia::math::output::svg {
class SvgWrapperFactory final : public common::CommonWrapperFactory {
public:
  explicit SvgWrapperFactory(common::CommonOutputJax &,
                             common::CommonWrapperCatalog actual = defaultNodes());
  static common::CommonWrapperCatalog defaultNodes();
};
} // namespace folia::math::output::svg
