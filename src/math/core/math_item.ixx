// MathJax 4.1.3 core/MathItem.ts. Apache-2.0.
// The actual expression/job state lives here, outside the TeX parser family.
export module folia.math.core.math_item;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.core.dom_node;
export import folia.math.core.dom_adaptor;
export import folia.math.core.math_location;
export import folia.math.util.context_property_bindings;
export import folia.math.util.object_types;
import std;
export namespace folia::math::core {
class MathItem;
class MathDocumentService;
class MathInputJaxService;
class MathOutputJaxService;
class MathRenderActionsService;
class MathListService;
using MathDocumentReference=std::variant<Missing,PropertyNull,MathDocumentService*>;
using MathInputJaxReference=std::variant<Missing,PropertyNull,MathInputJaxService*>;
using MathOutputJaxReference=std::variant<Missing,PropertyNull,MathOutputJaxService*>;
using MathRenderActionsReference=std::variant<Missing,PropertyNull,MathRenderActionsService*>;
// A common-output result is the same canonical DOM handle held by the
// adaptor. Other output backends may return their actual native owner.
using MathOutput=std::variant<Missing,PropertyNull,NativeObjectRef,DOMNodeRef>;
// Convert the actual output result to the Source-facing Context value. DOM
// results use the canonical node Property binding; no replacement object is
// allocated for a node handle.
options::Value sourceOutputValue(const MathOutput&,ContextPropertyBindings&);
// Generic Location may include a native N/T node. Keep the complete supplied
// location object opaque and identical; scalar location records use Property.
// Source ProtoItem belongs to core/MathItem.ts, shared by InputJax and FindMath.
// Scalar records use the existing lossless Property graph; native N/T locations
// may retain a complete opaque proto record.
using ProtoMathItem=std::variant<Property,NativeObjectRef>;
using ProtoMathItems=std::shared_ptr<std::vector<ProtoMathItem>>;
class MathInputJaxService {
public:
    virtual ~MathInputJaxService()=default;
    virtual mml::NodeValue compile(MathItem&,MathDocumentReference)=0;
};
class MathOutputJaxService {
public:
    virtual ~MathOutputJaxService()=default;
    virtual MathOutput typeset(MathItem&,MathDocumentReference)=0;
    virtual MathOutput escaped(MathItem&,MathDocumentReference)=0;
};
class MathRenderActionsService {
public:
    virtual ~MathRenderActionsService()=default;
    virtual void renderMath(MathItem&,MathDocumentReference,Property start)=0;
    virtual void renderConvert(MathItem&,MathDocumentReference,Property end)=0;
};
class MathDocumentService {
public:
    virtual ~MathDocumentService()=default;
    virtual MathRenderActionsReference renderActionsReference()const=0;
    virtual MathOutputJaxReference outputJaxReference()const=0;
    // Retain the actual document adaptor when an MML HTML node outlives
    // the filter invocation that created it.
    virtual std::shared_ptr<AbstractDOMAdaptor> domAdaptorOwner()const {
        throw options::Exception("Unimplemented",u"MathDocument has no retained DOM adaptor");
    }
    // These are live views of the actual MathDocument fields. The default
    // throws so legacy document adapters cannot silently invent empty/null
    // Source state while they are migrated to the canonical graph.
    virtual DOMNodeValue documentNode()const {
        throw options::Exception("Unimplemented",u"MathDocument.document has no canonical DOM node binding");
    }
    virtual MathListService& mathList()const {
        throw options::Exception("Unimplemented",u"MathDocument.math has no live MathList binding");
    }
    virtual MathLocationService& locationService()const {
        throw options::Exception("Unimplemented",u"MathDocument has no live MathLocation accessor");
    }
    virtual options::Context& sourceContext()const {
        throw options::Exception("Unimplemented",u"MathDocument has no canonical Source Context");
    }
    virtual options::ObjectRef sourceIdentity()const {
        throw options::Exception("Unimplemented",u"MathDocument has no canonical Source identity");
    }
};
class MathItem:public NativeObject {
public:
    Property math=Missing{},display=Missing{};
    MathLocation start=Property(Missing{}),end=Property(Missing{});
    MathInputJaxReference inputJax=Missing{};
    mml::NodeValue root=PropertyNull{};
    MathOutput typesetRoot=PropertyNull{};
    Property metrics=Missing{},inputData=Missing{},outputData=Missing{};
    virtual ~MathItem()=default;
    virtual bool isEscaped()const=0;
    virtual void render(MathDocumentReference)=0;
    virtual void rerender(MathDocumentReference,Property start=Missing{})=0;
    virtual void convert(MathDocumentReference,Property end=Missing{})=0;
    virtual void compile(MathDocumentReference)=0;
    virtual void typeset(MathDocumentReference)=0;
    virtual void updateDocument(MathDocumentReference)=0;
    virtual void removeFromDocument(Property restore=Missing{})=0;
    virtual void setMetrics(Property em,Property ex,Property cwidth,Property scale)=0;
    virtual Property state(Property state=Missing{},Property restore=Missing{})=0;
    virtual void reset(Property restore=Missing{})=0;
    virtual void clear()=0;
};
// The document and output layers depend on this exact live list owner. It is
// declared after MathItem so its callback always names the canonical type.
class MathListService {
public:
    virtual ~MathListService()=default;
    virtual void forEach(const std::function<void(MathItem&)>&)=0;
};
class AbstractMathItem:public MathItem {
protected:
    Property state_=Missing{};
    AbstractMathItem(Property math,MathInputJaxReference,Property display=Missing{},
                     MathLocation start=Property(Missing{}),MathLocation end=Property(Missing{}));
public:
    AbstractMathItem(const AbstractMathItem&)=delete;
    AbstractMathItem& operator=(const AbstractMathItem&)=delete;
    // Source exposes _state as an ordinary writable field. Context projection
    // must read/write the actual state owner without invoking state()'s
    // transition side effects or narrowing values to a number.
    Property sourceStateField()const{return state_;}
    void setSourceStateField(Property value){state_=std::move(value);}
    bool isEscaped()const override;
    void render(MathDocumentReference)override;
    void rerender(MathDocumentReference,Property start=Missing{})override;
    void convert(MathDocumentReference,Property end=Missing{})override;
    void compile(MathDocumentReference)override;
    void typeset(MathDocumentReference)override;
    void updateDocument(MathDocumentReference)override;
    void removeFromDocument(Property restore=Missing{})override;
    void setMetrics(Property em,Property ex,Property cwidth,Property scale)override;
    Property state(Property state=Missing{},Property restore=Missing{})override;
    void reset(Property restore=Missing{})override;
    void clear()override;
};
extern PropertyList STATE;
Property protoItem(Property open,Property math,Property close,Property n,
                   Property start,Property end,Property display=Missing{});
void newState(std::string_view name,Property state);
}
