/*************************************************************
 * MathJax 4.1.3 handlers/html/HTMLMathItem.ts DOM replacement owner.
 *************************************************************/
export module folia.math.handlers.html.html_math_item;
export import folia.math.core.math_document;
import std;

export namespace folia::math::handlers::html {
class HTMLMathItemHost {
public:
    virtual ~HTMLMathItemHost()=default;
    virtual AbstractDOMAdaptor& domAdaptor()const=0;
    virtual bool processStrings(core::MathInputJaxReference)const=0;
    virtual ContextPropertyBindings& propertyBindings()const=0;
    virtual core::MathLocationService& locationService()const=0;
};
class HTMLMathItem final : public core::AbstractMathItem,
                           public core::MathItemSourceIdentity {
    options::Context* context_;
    options::ObjectRef identity_;
    HTMLMathItemHost* document_;
public:
    HTMLMathItem(options::Context&,options::ObjectRef,HTMLMathItemHost&,
        Property,core::MathInputJaxReference,Property,core::MathLocation,
        core::MathLocation);
    bool isEscaped()const override;
    options::Context& sourceContext()const override;
    options::ObjectRef sourceIdentity()const override;
    void updateDocument(core::MathDocumentReference)override;
    void removeFromDocument(Property restore=Missing{})override;
};
}
