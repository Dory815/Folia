/*************************************************************
 * MathJax 4.1.3 core/MathDocument.ts and handlers/html/HTMLMathItem.ts.
 * This module owns the generic HTML document/item algorithms. Backend
 * sessions only assemble the actual providers passed to create().
 *************************************************************/
export module folia.math.handlers.html.html_math_document;
export import folia.math.core.math_document;
export import folia.math.handlers.html.html_math_item;
import folia.math.util.regexp_binding;
import std;

export namespace folia::math::handlers::html {
class HTMLMathDocument final : public core::MathDocument, public HTMLMathItemHost {
    options::ObjectRef itemPrototype_;
    std::optional<options::ObjectRef> lastRequestItem_;
    regexp_host::RegExpBindings* regexp_;

    HTMLMathDocument(options::Context&,ContextPropertyBindings&,util::RuntimeModules&,
        regexp_host::RegExpBindings&,
        std::vector<core::InputJaxBinding>,std::shared_ptr<core::OutputJax>,
        options::ObjectRef,std::shared_ptr<AbstractDOMAdaptor>,options::ObjectRef,
        std::shared_ptr<mml::MmlFactory>,DOMNodeRef,
        std::function<options::Value(const mml::NodeValue&)>,
        std::function<mml::MmlNode&(options::ObjectRef)>,std::shared_ptr<void>);
    void initialize(options::Value supplied,options::ObjectRef itemConstructor,
                    options::Value classOptions);
    void installItemSurface();
    options::ObjectRef constructItem(options::Value,options::Value,
        options::Value,options::Value,options::Value);
    void retireLastRequestItem();
    void beginRequestOwnerScopeImpl();
    static HTMLMathItem& requireItem(options::Context&,options::Value);
    core::MathItem& resolveMathItem(options::ObjectRef) override;
    options::Value sourceResult(core::MathItem&) override;
public:
    static std::shared_ptr<HTMLMathDocument> create(options::Context&,
        ContextPropertyBindings&,util::RuntimeModules&,regexp_host::RegExpBindings&,
        std::vector<core::InputJaxBinding>,std::shared_ptr<core::OutputJax>,
        options::ObjectRef outputIdentity,
        std::shared_ptr<AbstractDOMAdaptor>,options::ObjectRef adaptorIdentity,
        std::shared_ptr<mml::MmlFactory>,
        DOMNodeRef,std::function<options::Value(const mml::NodeValue&)>,
        std::function<mml::MmlNode&(options::ObjectRef)>,
        options::Value options=options::Undefined{},std::shared_ptr<void> serviceOwner={});
    HTMLMathDocument(const HTMLMathDocument&)=delete;
    HTMLMathDocument& operator=(const HTMLMathDocument&)=delete;
    AbstractDOMAdaptor& domAdaptor()const override{return *adaptor_;}
    bool processStrings(core::MathInputJaxReference)const override;
    core::OutputJax& outputJax()const{return *output_;}
    ContextPropertyBindings& propertyBindings()const override{return *properties_;}
    core::MathLocationService& locationService()const override{return core::MathDocument::locationService();}
    // Called only by the private Renderer after its copied result is complete.
    void retireCompletedRequestItem(){retireLastRequestItem();}
    void beginRequestOwnerScope(){beginRequestOwnerScopeImpl();}
};
}
