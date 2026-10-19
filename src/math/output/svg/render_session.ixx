// Concrete, per-render Source service graph. The Context owns constructor
// records; this graph owns the actual providers and only borrows that Context.
export module folia.math.output.svg.render_session;
export import folia.math.output.svg.output_jax_bindings;
export import folia.math.output.common.font_data_surface;
export import folia.math.output.svg.font_data_surface;
export import folia.math.input.tex.parser;
export import folia.math.util.runtime_modules;
export import folia.math.util.regexp_binding;
export import folia.math.util.unicode_normalization;
export import folia.math.adaptors.lite.lite_module;
export import folia.math.fonts.newcm_font_data_surface;
export import folia.math.fonts.newcm_font_module_provider;
export import folia.math.core.output_jax;
export import folia.math.core.mml_tree.mml_factory;
export import folia.math.output.svg.wrapper_constructors;
export import folia.math.output.svg.wrapper_source_methods;
export import folia.math.output.svg.menclose_surface;
export import folia.math.output.svg.maction_surface;
export import folia.math.output.svg.scene_context_nodes;
import std;

export namespace folia::math::output::svg {
enum class NewcmFontExtension : std::uint8_t { bboldx, bbm, dsfont };
using NewcmFontProfile = std::vector<NewcmFontExtension>;
// The host is the real environment-specific font loader. The session takes
// ownership of its callback table and replaces only its queue with the one
// owned by ParserRuntime; missing load operations are rejected at bootstrap.
class SvgRenderServiceGraph final
    : public std::enable_shared_from_this<SvgRenderServiceGraph> {
    options::Context& context_;
    std::shared_ptr<util::RuntimeModules> modules_;
    std::shared_ptr<OptionFunctionBindings> functions_;
    std::shared_ptr<regexp_host::RegExpBindings> regexp_;
    std::shared_ptr<Unicode16Normalization> unicode_;
    std::shared_ptr<core::InputJaxModule> inputJax_;
    std::shared_ptr<core::FindMathModule> findMath_;
    std::shared_ptr<folia::math::tex::FindTeXModule> findTeX_;
    std::shared_ptr<folia::math::tex::ParserRuntime> parser_;
    std::shared_ptr<core::OutputJaxModule> outputJax_;
    std::shared_ptr<folia::math::mml::MmlFactory> mmlFactory_;
    std::shared_ptr<options::MapContextBindings> maps_;
    std::shared_ptr<common::FontLoadHost> fontHost_;
    std::shared_ptr<common::FontDataContextBindings> fonts_;
    std::shared_ptr<common::FontDataSourceSurface> fontSurface_;
    std::shared_ptr<common::FontDataClass> baseFontClass_;
    std::shared_ptr<svg::SvgFontDataClass> svgFontClass_;
    std::shared_ptr<folia::math::fonts::NewcmFontDataClass> newcmFontClass_;
    std::shared_ptr<folia::math::fonts::NewcmFontModuleProvider> newcmModules_;
    NewcmFontProfile fontProfile_;
    std::shared_ptr<common::CommonFactoryContextBindings> commonFactories_;
    std::shared_ptr<common::CommonRuntimeBindings> commonRuntime_;
    std::shared_ptr<common::CommonJaxClassBindings> commonClasses_;
    std::shared_ptr<svg::SvgWrapperFactoryContextBindings> svgFactories_;
    std::shared_ptr<svg::SvgOutputJaxContextBindings> svgJax_;
    std::shared_ptr<svg::SvgWrapperSourceMethods> wrapperMethods_;
    std::shared_ptr<SvgMencloseSourceSurface> mencloseSurface_;
    std::shared_ptr<SceneContextNodes> sceneContextNodes_;
    std::shared_ptr<TimerHost> mactionTimers_;
    std::shared_ptr<SvgMactionSourceSurface> mactionSurface_;
    std::shared_ptr<folia::math::adaptors::lite::LiteModule> lite_;
    std::vector<options::ObjectRef> retainedConstructors_;
    options::ObjectRef commonConstructor_{};
    options::ObjectRef fontConstructor_{};
    options::ObjectRef svgFontConstructor_{};
    options::ObjectRef newcmFontConstructor_{};
    options::ObjectRef svgFactoryConstructor_{};
    std::vector<std::pair<std::u16string,options::ObjectRef>> wrapperConstructors_;
    SvgWrapperConstructorSet wrapperSet_{};

    explicit SvgRenderServiceGraph(options::Context&,common::FontLoadHost,NewcmFontProfile);
    void initialize();
public:
    static std::shared_ptr<SvgRenderServiceGraph> create(
        options::Context&,common::FontLoadHost,NewcmFontProfile={});
    SvgRenderServiceGraph(const SvgRenderServiceGraph&)=delete;
    SvgRenderServiceGraph& operator=(const SvgRenderServiceGraph&)=delete;

    options::Context& context()const noexcept{return context_;}
    util::RuntimeModules& modules()const noexcept{return *modules_;}
    OptionFunctionBindings& functions()const noexcept{return *functions_;}
    folia::math::tex::ParserRuntime& parser()const noexcept{return *parser_;}
    folia::math::mml::MmlFactory& mmlFactory()const noexcept{return *mmlFactory_;}
    options::MapContextBindings& mapBindings()const noexcept{return *maps_;}
    std::shared_ptr<folia::math::mml::MmlFactory> mmlFactoryOwner()const noexcept{return mmlFactory_;}
    common::FontDataContextBindings& fonts()const noexcept{return *fonts_;}
    common::CommonFactoryContextBindings& commonFactories()const noexcept{return *commonFactories_;}
    common::CommonRuntimeBindings& commonRuntime()const noexcept{return *commonRuntime_;}
    common::CommonJaxClassBindings& commonClasses()const noexcept{return *commonClasses_;}
    SvgWrapperFactoryContextBindings& svgFactories()const noexcept{return *svgFactories_;}
    SvgOutputJaxContextBindings& svgJax()const noexcept{return *svgJax_;}
    folia::math::adaptors::lite::LiteModule& lite()const noexcept{return *lite_;}
    SvgMencloseSourceSurface& menclose()const noexcept{return *mencloseSurface_;}
    SceneContextNodes& sceneContextNodes()const noexcept{return *sceneContextNodes_;}
    SvgMactionSourceSurface& maction()const noexcept{return *mactionSurface_;}
    TimerHost& mactionTimers()const noexcept{return *mactionTimers_;}
    options::ObjectRef commonConstructor()const noexcept{return commonConstructor_;}
    options::ObjectRef fontConstructor()const noexcept{return fontConstructor_;}
    options::ObjectRef svgFontConstructor()const noexcept{return svgFontConstructor_;}
    options::ObjectRef newcmFontConstructor()const noexcept{return newcmFontConstructor_;}
    options::ObjectRef extensionObject(NewcmFontExtension)const;
    void applyFontProfile(options::ObjectRef jaxInstance);
    options::ObjectRef svgFactoryConstructor()const noexcept{return svgFactoryConstructor_;}
    const SvgWrapperConstructorSet& wrapperSet()const noexcept{return wrapperSet_;}

    void retainForConstructor(options::ObjectRef);
    void retainRegisteredConstructors();
};

// Owns one actual Context and the complete services needed to construct and
// typeset through the canonical SVG output Jax. Font loading remains an
// explicit host capability, rather than an empty fallback implementation.
class NativeRenderSession final {
    std::shared_ptr<options::Context> context_;
    std::shared_ptr<SvgRenderServiceGraph> services_;
    std::shared_ptr<SvgOutputJax> jax_;
    options::ObjectRef adaptorObject_{};
public:
    explicit NativeRenderSession(common::FontLoadHost fontHost,NewcmFontProfile={});
    NativeRenderSession(const NativeRenderSession&)=delete;
    NativeRenderSession& operator=(const NativeRenderSession&)=delete;
    ~NativeRenderSession()=default;

    options::Context& context()const noexcept{return *context_;}
    // Shared lifetime token for public handles that must keep this actual
    // Context alive. Context-owned callbacks must never capture this token.
    std::shared_ptr<options::Context> contextOwner()const noexcept{return context_;}
    SvgRenderServiceGraph& services()const noexcept{return *services_;}
    std::shared_ptr<SvgOutputJax> outputJax()const noexcept{return jax_;}
    std::size_t dispatchMactionTimers();
    std::shared_ptr<adaptors::lite::LiteAdaptor> adaptor()const;
    options::ObjectRef adaptorObject()const noexcept{return adaptorObject_;}
    std::shared_ptr<folia::math::mml::MmlFactory> mathFactory()const;
    std::shared_ptr<SvgOutputJax> construct(options::Value options=options::Undefined{});
};
}
