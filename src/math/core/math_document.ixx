/*************************************************************
 * MathJax 4.1.3 core/MathDocument.ts generic owner.
 * HTML handlers provide the concrete MathItem allocator and DOM edits.
 *************************************************************/
export module folia.math.core.math_document;
export import folia.math.core.math_render_actions;
export import folia.math.core.math_list;
export import folia.math.core.input_jax;
export import folia.math.core.output_jax;
export import folia.math.core.dom_adaptor;
export import folia.math.core.mml_tree.mml_factory;
export import folia.math.util.runtime_modules;
export import folia.math.util.context_property_bindings;
import std;

export namespace folia::math::core {
struct InputJaxBinding {
    options::ObjectRef identity;
    std::shared_ptr<InputJax> owner;
};

class MathDocument : public NativeObject, public MathDocumentService,
    public MathLocationService,
    public std::enable_shared_from_this<MathDocument> {
protected:
    options::Context* context_;
    ContextPropertyBindings* properties_;
    util::RuntimeModules* modules_;
    std::shared_ptr<void> serviceOwner_;
    std::vector<InputJaxBinding> inputJax_;
    std::shared_ptr<OutputJax> output_;
    options::ObjectRef outputIdentity_;
    std::shared_ptr<AbstractDOMAdaptor> adaptor_;
    options::ObjectRef adaptorIdentity_;
    std::shared_ptr<mml::MmlFactory> mmlFactory_;
    std::function<options::Value(const mml::NodeValue&)> mmlProject_;
    std::function<mml::MmlNode&(options::ObjectRef)> mmlResolve_;
    DOMNodeRef documentNode_;
    options::ObjectRef identity_;
    options::ObjectRef renderActionsIdentity_;
    options::Value options_=options::Undefined{};
    std::unique_ptr<AbstractMathList> math_;
    std::unique_ptr<RenderActions> renderActions_;
    options::ObjectRef itemConstructor_;
    std::vector<options::ObjectRef> privateRequestReducerRecords_;
    std::size_t privateRequestReducerRetireIndex_=0;
    bool privateRequestOwnerScopeActive_=false;

    MathDocument(options::Context&,ContextPropertyBindings&,util::RuntimeModules&,
        std::vector<InputJaxBinding>,std::shared_ptr<OutputJax>,options::ObjectRef,
        std::shared_ptr<AbstractDOMAdaptor>,options::ObjectRef,
        std::shared_ptr<mml::MmlFactory>,DOMNodeRef,
        std::function<options::Value(const mml::NodeValue&)>,
        std::function<mml::MmlNode&(options::ObjectRef)>,std::shared_ptr<void>);
    void initializeCore(options::Value supplied,options::ObjectRef mathItemConstructor,
                        options::Value classOptions=options::Undefined{});
    void setItemConstructor(options::ObjectRef value){itemConstructor_=value;}
    virtual MathItem& resolveMathItem(options::ObjectRef)=0;
    virtual options::Value sourceResult(MathItem&)=0;
    void installCoreDocumentMethods();
    options::Value convertCore(options::Value math,options::Value supplied);
    void beginPrivateRequestOwnerScope();
    void retirePrivateRequestOwnerScope();
public:
    MathDocument(const MathDocument&)=delete;
    MathDocument& operator=(const MathDocument&)=delete;
    options::Value convert(options::Value math,
                           options::Value options=options::Undefined{});
    options::Context& sourceContext()const override;
    options::ObjectRef sourceIdentity()const override;
    MathRenderActionsReference renderActionsReference()const override;
    MathOutputJaxReference outputJaxReference()const override;
    std::shared_ptr<AbstractDOMAdaptor> domAdaptorOwner()const override;
    DOMNodeValue documentNode()const override;
    MathListService& mathList()const override;
    MathLocationService& locationService()const override;
    Property get(const NativeObjectRef&,std::string_view)override;
    void set(const NativeObjectRef&,std::string_view,Property)override;
};
}
