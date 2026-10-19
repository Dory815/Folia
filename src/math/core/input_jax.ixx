// MathJax 4.1.3 core/InputJax.ts; Apache-2.0.
export module folia.math.core.input_jax;
export import folia.math.core.math_item;
export import folia.math.core.mml_tree.mml_factory;
export import folia.math.core.jax_filter_record_context;
export import folia.math.util.option_functions;
import std;
export namespace folia::math::core {
using MathItemReference=std::variant<Missing,PropertyNull,MathItem*>;
using InputAdaptorReference=std::variant<Missing,PropertyNull,options::ObjectRef,NativeObjectRef>;
// Borrowed fixtures retain the typed legacy service; application sessions use
// the owning arm so nodes' factory_ back-edges and product arena stay alive.
using InputMmlFactoryReference=std::variant<Missing,PropertyNull,mml::MmlFactory*,std::shared_ptr<mml::MmlFactory>>;
using InputFilterMathReference=std::variant<Missing,PropertyNull,MathItem*,options::Value>;
using InputFilterDocumentReference=std::variant<Missing,PropertyNull,MathDocumentService*,options::Value>;
struct InputJaxFilterData;
using InputJaxFilterDataRef=std::shared_ptr<InputJaxFilterData>;
struct InputJaxFilterData:NativeObject {
private:
    InputFilterMathReference math_=Missing{};
    InputFilterDocumentReference document_=Missing{};
    std::any data_;
    friend class AbstractInputJax;
public:
    std::shared_ptr<JaxFilterRecordContext> sourceRecord;
    std::function<options::Value(const std::any&)> encodeData;
    std::function<std::any(options::Value)> decodeData;
    options::Value rawMathValue()const;
    options::Value rawDocumentValue()const;
    options::Value rawDataValue()const;
    options::Value field(std::u16string_view name)const;
    options::Value setField(std::u16string_view name,options::Value value);
    InputFilterMathReference mathReference()const;
    InputFilterDocumentReference documentReference()const;
    std::any currentData()const;
    options::ObjectRef sourceIdentity()const;
    void initialize(MathItemReference,MathDocumentReference,std::any);
    void bindSourceRecord(options::Context&,const InputJaxFilterDataRef&,
                          std::weak_ptr<std::atomic_bool> lease={});
};
// MathItem's compile-only back-edge is the same service, not another jax state.
class InputJax:public NativeObject,public MathInputJaxService {
public:
    options::Value options=options::Undefined{};
    std::shared_ptr<FunctionList> preFilters,postFilters;
    InputAdaptorReference adaptor=PropertyNull{};
    InputMmlFactoryReference mmlFactory=PropertyNull{};
    virtual options::Value name()=0;
    virtual bool processStrings()const=0;
    virtual void setAdaptor(InputAdaptorReference)=0;
    virtual void setMmlFactory(InputMmlFactoryReference)=0;
    virtual void initialize()=0;
    virtual void reset(FunctionArgs args={})=0;
    virtual ProtoMathItems findMath(std::any which={},options::Value options=options::Undefined{})=0;
};
class AbstractInputJax:public InputJax {
    OptionFunctionBindings& bindings_;
    options::ObjectRef identity_;
    std::function<options::Value(const std::any&)> filterDataEncoder_;
    std::function<std::any(options::Value)> filterDataDecoder_;
    std::weak_ptr<std::atomic_bool> filterExecutionLease_;
    std::shared_ptr<JaxFilterRecordOwnerScope> filterRecordOwner_;
protected:
    AbstractInputJax(OptionFunctionBindings& bindings,options::ObjectRef identity)
        :bindings_(bindings),identity_(identity){}
    std::any executeFilters(FunctionList&,MathItemReference,MathDocumentReference,std::any);
public:
    AbstractInputJax(const AbstractInputJax&)=delete;
    AbstractInputJax& operator=(const AbstractInputJax&)=delete;
    // Source base phase after full allocation, before derived Source fields.
    void initializeInputJax(options::Value supplied=options::Undefined{});
    options::Context& optionsContext()const{return bindings_.context();}
    options::ObjectRef instanceIdentity()const{return identity_;}
    void bindFilterRecordValueCodec(std::function<options::Value(const std::any&)>,
        std::function<std::any(options::Value)>,std::weak_ptr<std::atomic_bool>,
        std::shared_ptr<JaxFilterRecordOwnerScope> owner={});
    std::shared_ptr<JaxFilterRecordOwnerScope> filterRecordOwner()const noexcept{return filterRecordOwner_;}
    options::Value name()override;
    bool processStrings()const override;
    void setAdaptor(InputAdaptorReference)override;
    void setMmlFactory(InputMmlFactoryReference)override;
    void initialize()override;
    void reset(FunctionArgs args={})override;
    ProtoMathItems findMath(std::any which={},options::Value options=options::Undefined{})override;
};
}
