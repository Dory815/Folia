// MathJax 4.1.3 core/InputJax.ts; Apache-2.0.
export module folia.math.core.input_jax;
export import folia.math.core.math_item;
export import folia.math.core.mml_tree.mml_factory;
export import folia.math.util.option_functions;
import std;
export namespace folia::math::core {
using MathItemReference=std::variant<Missing,PropertyNull,MathItem*>;
using InputAdaptorReference=std::variant<Missing,PropertyNull,NativeObjectRef>;
using InputMmlFactoryReference=std::variant<Missing,PropertyNull,mml::MmlFactory*>;
struct InputJaxFilterData:NativeObject {
    MathItemReference math=Missing{};
    MathDocumentReference document=Missing{};
    std::any data;
};
using InputJaxFilterDataRef=std::shared_ptr<InputJaxFilterData>;
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
    options::Value name()override;
    bool processStrings()const override;
    void setAdaptor(InputAdaptorReference)override;
    void setMmlFactory(InputMmlFactoryReference)override;
    void initialize()override;
    void reset(FunctionArgs args={})override;
    ProtoMathItems findMath(std::any which={},options::Value options=options::Undefined{})override;
};
}
