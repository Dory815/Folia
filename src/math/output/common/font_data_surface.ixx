// Context-visible Source class surface for the canonical FontData owners.
// This module installs methods on the real constructor/prototype objects made
// by FontDataContextBindings; it does not own a second class registry.
export module folia.math.output.common.font_data_surface;
export import folia.math.output.common.font_data_bindings;
export import folia.math.util.context_property_bindings;
export import folia.math.util.map_bindings;
import std;

export namespace folia::math::output::common {
class FontDataSourceSurface {
    options::Context* context_;
    regexp_host::RegExpBindings* regexp_;
    FontDataContextBindings* identities_;
    ContextPropertyBindings* properties_;
    options::MapContextBindings* maps_;
    options::ObjectRef method(options::ObjectRef target,std::u16string_view name,
                              std::uint32_t length,options::NativeCall body);
public:
    FontDataSourceSurface(options::Context&,FontDataContextBindings&,
                          ContextPropertyBindings&,options::MapContextBindings&,
                          regexp_host::RegExpBindings* regexp=nullptr);
    void bindHost(FontLoadHost& host)const noexcept{host.regexp=regexp_;}
    options::MapContextBindings& mapBindings()const noexcept{return *maps_;}
    options::ObjectRef registerBase(std::shared_ptr<FontDataClass>,FontLoadHost&,
        std::shared_ptr<void> hostAnchor,FontDataContextBindings::Allocator,
        FontDataContextBindings::InstanceInitializer={});
    options::ObjectRef registerSvg(std::shared_ptr<FontDataClass>,FontLoadHost&,
        std::shared_ptr<void> hostAnchor,FontDataContextBindings::Allocator,
        options::ObjectRef parentConstructor,
        FontDataContextBindings::InstanceInitializer={});
    options::ObjectRef registerNewcm(std::shared_ptr<FontDataClass>,FontLoadHost&,
        std::shared_ptr<void> hostAnchor,FontDataContextBindings::Allocator,
        options::ObjectRef parentConstructor,
        FontDataContextBindings::InstanceInitializer={});
    // Installs the Source own data fields that share storage with the actual
    // FontData instance. Derived registrars call this in their constructor
    // initializer after FontDataContextBindings has bound the real identity.
    void initializeInstanceFields(const std::shared_ptr<FontData>&,
                                  options::ObjectRef instance);
    // Bind the three canonical Source array fields on this actual class
    // constructor. Derived class registrars use the same owner-backed path.
    void bindClassArrayFields(const std::shared_ptr<FontDataClass>&,
                              options::ObjectRef constructor);
    // Create one Context object carrying the generated extension record as an
    // owned payload and the Source-visible `name` field. The same object is
    // passed through the actual Jax/FontData Context methods.
    options::ObjectRef extensionObject(const FontExtensionData&);
    void installBase(options::ObjectRef constructor);
    void installSvgOverrides(options::ObjectRef constructor);
};
}
