// MathJax 4.1.3 core/InputJax.ts methods; Apache-2.0.
module folia.math.core.input_jax;
import std;
namespace folia::math::core {
options::Value AbstractInputJax::name(){
    auto& context=optionsContext();
    const options::Value constructor=context.get(options::Value(instanceIdentity()),options::Key(u"constructor"));
    return context.get(constructor,options::Key(u"NAME"));
}
bool AbstractInputJax::processStrings()const{return true;}
void AbstractInputJax::setAdaptor(InputAdaptorReference value){adaptor=std::move(value);}
void AbstractInputJax::setMmlFactory(InputMmlFactoryReference value){mmlFactory=std::move(value);}
void AbstractInputJax::initialize(){}
void AbstractInputJax::reset(FunctionArgs){}
ProtoMathItems AbstractInputJax::findMath(std::any,options::Value){return std::make_shared<std::vector<ProtoMathItem>>();}
std::any AbstractInputJax::executeFilters(FunctionList& filters,MathItemReference math,MathDocumentReference document,std::any value){
    auto record=std::make_shared<InputJaxFilterData>();
    record->math=std::move(math);
    record->document=std::move(document);
    record->data=std::move(value);
    std::array<std::any,1> arguments{InputJaxFilterDataRef(record)};
    filters.execute(arguments);
    return record->data;
}
}
