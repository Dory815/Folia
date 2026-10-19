// Source-visible SvgFontData class registration over the common FontData
// identity service and actual FontData owners.
export module folia.math.output.svg.font_data_surface;
export import folia.math.output.svg.font_data;
export import folia.math.output.common.font_data_surface;
import std;

export namespace folia::math::output::svg {
class SvgFontDataSourceSurface {
    options::Context* context_;
    common::FontDataContextBindings* identities_;
    common::FontDataSourceSurface* commonSurface_;
public:
    SvgFontDataSourceSurface(options::Context&,
        common::FontDataContextBindings&,common::FontDataSourceSurface&);
    options::ObjectRef registerSvg(std::shared_ptr<SvgFontDataClass>,
        common::FontLoadHost&,std::shared_ptr<void> hostAnchor,
        common::FontDataContextBindings::Allocator,
        options::ObjectRef parentConstructor,
        common::FontDataContextBindings::InstanceInitializer={});
    void installSvg(options::ObjectRef constructor);
};
}
