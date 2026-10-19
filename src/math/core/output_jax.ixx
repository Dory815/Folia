// MathJax 4.1.3 core/OutputJax.ts; Apache-2.0.
export module folia.math.core.output_jax;
export import folia.math.core.math_item;
export import folia.math.core.jax_filter_record_context;
export import folia.math.util.function_list_bindings;
import std;
export namespace folia::math::core {
using OutputMathItemReference=std::variant<Missing,PropertyNull,MathItem*,options::Value>;
using OutputJaxDocumentReference=std::variant<Missing,PropertyNull,MathDocumentService*,options::Value>;
// Keep the Context identity when an adaptor arrived through the Source object
// graph; native hosts may also pass an already-owned native object directly.
using OutputAdaptorReference=std::variant<Missing,PropertyNull,options::Value,NativeObjectRef>;
struct OutputJaxFilterData:NativeObject {
private:
    OutputMathItemReference math=Missing{};
    OutputJaxDocumentReference document=Missing{};
    std::any data;
    std::function<options::Value(const Property&)> encodeProperty;
public:
    std::shared_ptr<JaxFilterRecordContext> sourceRecord;
    options::Value rawMathValue()const;
    options::Value rawDocumentValue()const;
    options::Value rawDataValue()const;
    options::Value field(std::u16string_view name)const;
    options::Value setField(std::u16string_view name,options::Value value);
    OutputMathItemReference mathReference()const;
    OutputJaxDocumentReference documentReference()const;
    options::Value dataValue()const;
    options::ObjectRef sourceIdentity()const;
    void initialize(OutputMathItemReference,OutputJaxDocumentReference,std::any,
        std::function<options::Value(const Property&)>);
    void assignMath(options::Value value){math=std::move(value);}
    void assignDocument(options::Value value){document=std::move(value);}
    void assignData(options::Value value){data=std::move(value);}
    void removeMath(){math=Missing{};}
    void removeDocument(){document=Missing{};}
    void removeData(){data=std::any(options::Value(options::Undefined{}));}
};
using OutputJaxFilterDataRef=std::shared_ptr<OutputJaxFilterData>;
class OutputJax:public NativeObject,public MathOutputJaxService {
public:
    options::Value options=options::Undefined{};
    std::shared_ptr<FunctionList> preFilters,postFilters;
    OutputAdaptorReference adaptor=PropertyNull{};
    virtual options::Value name()=0;
    virtual void setAdaptor(OutputAdaptorReference)=0;
    virtual void initialize()=0;
    virtual void reset(FunctionArgs args={})=0;
    virtual void getMetrics(MathDocumentReference)=0;
    virtual MathOutput styleSheet(MathDocumentReference)=0;
    virtual MathOutput pageElements(MathDocumentReference)=0;
};
class AbstractOutputJax:public OutputJax {
    friend class OutputJaxModule;
    OptionFunctionBindings& bindings_;
    options::ObjectRef identity_;
    FunctionListContextBindings& filterBindings_;
    std::shared_ptr<JaxFilterRecordOwnerScope> filterRecordOwner_;
protected:
    AbstractOutputJax(OptionFunctionBindings& bindings,options::ObjectRef identity,
                      FunctionListContextBindings& filterBindings,
                      std::shared_ptr<JaxFilterRecordOwnerScope> owner={})
        :bindings_(bindings),identity_(identity),filterBindings_(filterBindings),
         filterRecordOwner_(std::move(owner)){
        if(filterRecordOwner_ && &filterRecordOwner_->context()!=&bindings_.context())
            throw options::Exception("TypeError",u"Output filter owner belongs to another Context");
    }
    std::any executeFilters(FunctionList&,OutputMathItemReference,
                            MathDocumentReference,std::any);
public:
    AbstractOutputJax(const AbstractOutputJax&)=delete;
    AbstractOutputJax& operator=(const AbstractOutputJax&)=delete;
    void bindOutputJaxFields(std::shared_ptr<void> owner);
    void initializeOutputJax(options::Value supplied,std::shared_ptr<void> owner);
    options::Context& optionsContext()const{return bindings_.context();}
    options::ObjectRef instanceIdentity()const{return identity_;}
    std::shared_ptr<JaxFilterRecordOwnerScope> filterRecordOwner()const noexcept{return filterRecordOwner_;}
    options::Value name()override;
    void setAdaptor(OutputAdaptorReference)override;
    void initialize()override;
    void reset(FunctionArgs args={})override;
    MathOutput typeset(MathItem&,MathDocumentReference)override=0;
    MathOutput escaped(MathItem&,MathDocumentReference)override=0;
    void getMetrics(MathDocumentReference)override;
    MathOutput styleSheet(MathDocumentReference)override;
    MathOutput pageElements(MathDocumentReference)override;
};
class OutputJaxModule {
    options::ObjectRef abstractClass_;
    FunctionListContextBindings filterBindings_;
    std::shared_ptr<JaxFilterRecordOwnerScope> filterRecordOwner_;
public:
    OutputJaxModule(options::Context& context,OptionFunctionBindings& bindings);
    OutputJaxModule(const OutputJaxModule&)=delete;
    OutputJaxModule& operator=(const OutputJaxModule&)=delete;
    options::ObjectRef abstractClass()const{return abstractClass_;}
    FunctionListContextBindings& filterBindings(){return filterBindings_;}
    std::shared_ptr<JaxFilterRecordOwnerScope> filterRecordOwner()const noexcept{return filterRecordOwner_;}
};
template<class T>std::shared_ptr<T> makeOutputJax(OptionFunctionBindings& bindings,
    FunctionListContextBindings& filterBindings,options::ObjectRef actualConstructor,
    options::Value supplied=options::Undefined{}){
    static_assert(std::is_base_of_v<AbstractOutputJax,T>);
    auto& context=bindings.context();
    const auto identity=context.instance(actualConstructor);
    auto result=std::make_shared<T>(bindings,identity,filterBindings);
    context.payload(identity)=std::static_pointer_cast<NativeObject>(result);
    result->bindOutputJaxFields(std::static_pointer_cast<void>(result));
    result->initializeOutputJax(std::move(supplied),std::static_pointer_cast<void>(result));
    return result;
}
}
