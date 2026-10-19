// Per-Context class and object identities for the MathJax Lite DOM module.
export module folia.math.adaptors.lite.lite_module;
export import folia.math.adaptors.lite_adaptor;
export import folia.math.util.context_property_bindings;
export import folia.math.util.regexp_binding;
export import folia.math.util.styles;
import std;

export namespace folia::math::adaptors::lite {
using LiteContext = options::Context;
using LiteObject = options::ObjectRef;

class StylesContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    StylesContextBindings(LiteContext&, OptionFunctionBindings&, ContextPropertyBindings&);
    ~StylesContextBindings();
    StylesContextBindings(const StylesContextBindings&) = delete;
    StylesContextBindings& operator=(const StylesContextBindings&) = delete;
    LiteObject constructor() const;
    LiteObject prototype() const;
    LiteObject bind(const std::shared_ptr<styles::Styles>&);
    std::shared_ptr<styles::Styles> resolve(LiteObject) const;
    std::shared_ptr<styles::Styles> construct(std::span<const options::Value> args = {});
};

// One MathJax Lite module instance per native Context/session. It owns the
// canonical constructors and the Context-object <-> Lite-node identity map;
// every adaptor supplied with this module shares its node store.
class LiteModule {
    struct State;
    std::shared_ptr<State> state_;
public:
    // One mark covers both request-created Lite nodes and host registrations.
    // The sequence boundary lets nested request owners retire only callbacks
    // they introduced, even when they registered on an older node.
    struct RequestMark {
    private:
        LiteNodeStore::RequestMark nodes;
        std::uint64_t firstEvent=0;
        const void* owner=nullptr;
        RequestMark(LiteNodeStore::RequestMark nodeMark,std::uint64_t event,
                    const void* moduleOwner)
            :nodes(nodeMark),firstEvent(event),owner(moduleOwner){}
        friend class LiteModule;
    public:
        RequestMark(const RequestMark&)=default;
        RequestMark(RequestMark&&)noexcept=default;
        RequestMark& operator=(const RequestMark&)=default;
        RequestMark& operator=(RequestMark&&)noexcept=default;
    };
    using EventMetadataTarget=std::function<void(
        LiteContext&,LiteObject,std::u16string_view)>;
    using AppendTarget=std::function<std::optional<options::Value>(
        LiteContext&,options::Value,std::span<const options::Value>)>;
    using AttributeTarget=std::function<std::optional<options::Value>(
        LiteContext&,options::Value,std::span<const options::Value>)>;
    using StyleTarget=std::function<std::optional<options::Value>(
        LiteContext&,options::Value,std::span<const options::Value>)>;
    // A short-lived target for the ordinary Lite DOM navigation and insertion
    // methods. Scene handles use the same methods and canonical node storage;
    // non-Scene values fall through to the Lite node store.
    using NodeOperationTarget=std::function<std::optional<options::Value>(
        LiteContext&,std::u16string_view,std::span<const options::Value>)>;
    LiteModule(LiteContext&, OptionFunctionBindings&, ContextPropertyBindings&,
               std::shared_ptr<regexp_host::RegExpBindings>,
               std::shared_ptr<LiteNodeStore> store = {});
    ~LiteModule();
    LiteModule(const LiteModule&) = delete;
    LiteModule& operator=(const LiteModule&) = delete;
    std::shared_ptr<LiteNodeStore> store() const;
    std::size_t nodeIdentityCountForDiagnostics()const noexcept;
    std::vector<std::pair<NodeId,NodeKind>> nodeIdentitySnapshotForDiagnostics()const;
    std::vector<std::tuple<NodeId,NodeKind,std::string>> nodeIdentityNamedSnapshotForDiagnostics()const;
    RequestMark requestMark()const noexcept;
    void retireRequestNodesSince(RequestMark);
    // Associates a canonical downstream owner with an existing Lite identity.
    // The Scene/session owns the view strongly; the Lite store retains only a
    // weak reference and reports expiration rather than falling back to copies.
    void bindOwnerView(LiteNodeRef, std::weak_ptr<LiteNodeOwnerView>);
    LiteContext& context() const;
    std::shared_ptr<entities::EntityModule> entityModule() const;
    LiteObject constructor(NodeKind) const;
    LiteObject parserConstructor() const;
    LiteObject adaptorConstructor() const;
    LiteObject windowObject();
    LiteObject objectFor(LiteNodeRef);
    LiteNodeRef nodeFor(LiteObject) const;
    LiteObject bindAdaptor(std::shared_ptr<LiteAdaptor>);
    std::shared_ptr<LiteAdaptor> resolveAdaptor(LiteObject) const;
    std::shared_ptr<LiteAdaptor> constructAdaptor(std::span<const options::Value> args = {});
    // Temporarily intercepts only the canonical append function's native
    // target dispatch. The Context function and its descriptor are unchanged;
    // an empty result falls through to the ordinary Lite DOM operation.
    AppendTarget exchangeAppendTarget(AppendTarget);
    AttributeTarget exchangeAttributeTarget(AttributeTarget);
    StyleTarget exchangeStyleTarget(StyleTarget);
    NodeOperationTarget exchangeNodeOperationTarget(NodeOperationTarget);
    // Scoped observer of registrations already retained by this Lite session.
    // It carries metadata outward; it does not own a second listener registry.
    EventMetadataTarget exchangeEventMetadataTarget(EventMetadataTarget);
    void registerHostEvent(options::ObjectRef,options::Value eventType,
                           options::Value callback);
    StylesContextBindings& stylesBindings();
    LiteObject stylesConstructor() const;
    std::shared_ptr<styles::Styles> styleFor(LiteObject) const;
};
}
