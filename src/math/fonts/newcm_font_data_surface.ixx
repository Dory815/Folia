// Actual Source constructor registration for the NewCM SVG font class.
export module folia.math.fonts.newcm_font_data_surface;
export import folia.math.fonts.newcm_font_data;
export import folia.math.output.svg.font_data_surface;
import std;

export namespace folia::math::fonts {
class NewcmFontDataSourceSurface {
    options::Context* context_;
    output::common::FontDataContextBindings* identities_;
    output::common::FontDataSourceSurface* commonSurface_;
public:
    NewcmFontDataSourceSurface(options::Context&,
        output::common::FontDataContextBindings&,
        output::common::FontDataSourceSurface&);
    options::ObjectRef registerNewcm(std::shared_ptr<NewcmFontDataClass>,
        output::common::FontLoadHost&,std::shared_ptr<void> hostAnchor,
        output::common::FontDataContextBindings::Allocator,
        options::ObjectRef parentConstructor,
        output::common::FontDataContextBindings::InstanceInitializer={});
};
}
