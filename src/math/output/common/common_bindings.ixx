// Context/native identity bindings for the actual common output-Jax graph.
export module folia.math.output.common:bindings;
export import :output_jax;
export import folia.math.output.common.font_data_bindings;
export import folia.math.output.common.bbox_context_bindings;
export import folia.math.util.runtime_modules;
export import folia.math.util.context_property_bindings;
export import folia.math.util.option_functions;
export import folia.math.util.function_list_bindings;
export import folia.math.util.map_bindings;
import std;

export namespace folia::math::output::common {
struct CommonRuntimeBindings {
    util::RuntimeModules& modules;
    FontDataContextBindings& fonts;
    class CommonFactoryContextBindings& factories;
    ContextPropertyBindings& properties;
    BBoxContextBindings bboxes;
private:
    // Source methods retain only a weak token to guard their non-owning
    // runtime pointer. This token belongs to this exact runtime value.
    std::shared_ptr<void> lifetime_=std::make_shared<int>(0);
public:

    explicit CommonRuntimeBindings(util::RuntimeModules& modules,
                                   FontDataContextBindings& fonts,
                                   CommonFactoryContextBindings& factories,
                                   ContextPropertyBindings& properties);
    options::Context& context() const;
    std::weak_ptr<void> lifetimeToken() const noexcept{return lifetime_;}
};

// Canonical identity bridge for CommonWrapperFactory instances and their
// constructor. The factory's mutable Source `jax` field remains its tagged
// `jaxValue`; typed wrapper calls resolve that current value on demand.
class CommonFactoryContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    struct OwnerCounts { std::size_t wrapperIdentities=0; std::size_t retainedLayoutJobs=0; };
    using FactoryAllocator = std::function<std::shared_ptr<CommonWrapperFactory>(
        options::Context&, options::ObjectRef actualConstructor,
        std::span<const options::Value> constructorArgs)>;
    using WrapperClassResolver = std::function<CommonWrapperClassRef(options::ObjectRef)>;
    using FactoryResolver = std::function<std::shared_ptr<CommonWrapperFactory>(options::ObjectRef)>;
    using MmlNodeResolver = std::function<mml::MmlNode&(options::ObjectRef)>;
    using MmlNodeIdentityResolver = std::function<options::ObjectRef(mml::MmlNode&)>;
    explicit CommonFactoryContextBindings(options::Context&);
    ~CommonFactoryContextBindings();
    CommonFactoryContextBindings(const CommonFactoryContextBindings&)=delete;
    CommonFactoryContextBindings& operator=(const CommonFactoryContextBindings&)=delete;

    options::Context& context() const;
    std::shared_ptr<void> lifetimeToken() const;
    WrapperClassResolver wrapperClassResolver() const;
    FactoryResolver factoryResolver() const;
    options::ObjectRef registerConstructor(std::u16string name=u"CommonWrapperFactory",
        std::optional<options::ObjectRef> parentConstructor={},
        FactoryAllocator allocator={});
    options::ObjectRef constructor() const;
    // Connects the inherited AbstractFactory/AbstractWrapperFactory methods
    // to the one Context's canonical MML node identities before factories
    // are registered for an output runtime.
    void bindFactoryMethods(util::RuntimeModules&,ContextPropertyBindings&,options::MapContextBindings&,
                            MmlNodeResolver,MmlNodeIdentityResolver,
                            std::weak_ptr<std::atomic_bool> executionLifetime);
    options::ObjectRef bind(const std::shared_ptr<CommonWrapperFactory>&);
    options::ObjectRef bind(const std::shared_ptr<CommonWrapperFactory>&,
                            options::ObjectRef actualConstructor);
    // Install the one canonical CommonWrapper instance bridge on the actual
    // factory. Products are Context-bound before initializeSource runs.
    void bindWrapperFactory(CommonWrapperFactory&);
    CommonWrapper& resolveWrapper(options::ObjectRef) const;
    std::shared_ptr<CommonWrapperFactory> resolve(options::ObjectRef) const;
    std::shared_ptr<CommonWrapperFactory> construct(options::ObjectRef constructor,
        std::span<const options::Value> args={});
    void bindWrapperClass(CommonWrapperClassRef, options::ObjectRef actualConstructor);
    CommonWrapperClassRef resolveWrapperClass(options::ObjectRef actualConstructor) const;
    CommonOutputJax& resolveJax(const options::Value&) const;
    // Private Renderer only: erase canonical wrapper leases for its closed
    // request job after SVG/Scene copying or failure unwinding.
    std::size_t retireRendererLayoutJob(const std::shared_ptr<LayoutJob>&);
    OwnerCounts ownerCountsForDiagnostics()const;
};

// Per-Context constructor/instance identity for the real Source LinebreakVisitor.
// Native layout code and the Context `linebreaks` field share the same owner.
class LinebreakVisitorContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    LinebreakVisitorContextBindings(options::Context&,
                                    CommonFactoryContextBindings&);
    ~LinebreakVisitorContextBindings();
    LinebreakVisitorContextBindings(const LinebreakVisitorContextBindings&)=delete;
    LinebreakVisitorContextBindings& operator=(const LinebreakVisitorContextBindings&)=delete;
    options::Context& context()const;
    options::ObjectRef registerConstructor();
    options::ObjectRef constructor()const;
    options::ObjectRef bind(const std::shared_ptr<Linebreaks>&,
                            options::ObjectRef constructor);
    std::shared_ptr<Linebreaks> resolve(options::ObjectRef)const;
    std::optional<options::ObjectRef> objectFor(const Linebreaks*)const;
    std::pair<std::shared_ptr<Linebreaks>,options::ObjectRef> construct(
        options::ObjectRef constructor,std::span<const options::Value> args={});
};

// Canonical Context identity for the real StyleJsonSheet owner used by the
// Common constructor. It exposes the original writable `styles` cell and the
// Source prototype methods over the same StyleJsonSheet instance.
class StyleJsonContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    StyleJsonContextBindings(options::Context&,ContextPropertyBindings&);
    ~StyleJsonContextBindings();
    StyleJsonContextBindings(const StyleJsonContextBindings&)=delete;
    StyleJsonContextBindings& operator=(const StyleJsonContextBindings&)=delete;
    options::ObjectRef constructor();
    options::ObjectRef bind(const std::shared_ptr<style_json::StyleJsonSheet>&);
    std::shared_ptr<style_json::StyleJsonSheet> resolve(options::ObjectRef)const;
    std::shared_ptr<style_json::StyleJsonSheet> construct(std::span<const options::Value> args={});
};

// The binding owns no duplicate class/object graph. Its class argument is the
// real Source constructor and every produced instance is the actual native
// output Jax retained in that Context object's payload.
class CommonJaxClassBindings {
    options::Context* context_;
    OptionFunctionBindings* functions_;
    FunctionListContextBindings* filters_;
    CommonRuntimeBindings* runtime_;
public:
    CommonJaxClassBindings(OptionFunctionBindings&,FunctionListContextBindings&,
                           CommonRuntimeBindings&);
    options::ObjectRef bindClassDefaults(options::ObjectRef actualClass);
    template<class T>
    std::shared_ptr<T> construct(options::ObjectRef actualClass,
                                 options::Value suppliedOptions,
                                 options::ObjectRef defaultFactory,
                                 options::ObjectRef defaultFont);
};

template<class T>
std::shared_ptr<T> CommonJaxClassBindings::construct(
    options::ObjectRef actualClass,options::Value suppliedOptions,
    options::ObjectRef defaultFactory,options::ObjectRef defaultFont) {
    static_assert(std::is_base_of_v<CommonOutputJax,T>);
    auto& context=*context_;
    const auto identity=context.instance(actualClass);
    auto owner=std::make_shared<T>(*functions_,identity,*filters_);
    context.payload(identity)=std::static_pointer_cast<NativeObject>(owner);
    owner->bindOutputJaxFields(std::static_pointer_cast<void>(owner));
    owner->initializeCommon(std::move(suppliedOptions),defaultFactory,defaultFont,
                            *runtime_,std::static_pointer_cast<void>(owner));
    return owner;
}
}
