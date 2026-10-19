// MathJax 4.1.3 core/InputJax.ts methods; Apache-2.0.
module folia.math.core.input_jax;
import folia.math.core.math_render_actions;
import std;
namespace folia::math::core {
namespace {
options::Key filterKey(std::u16string_view name){return options::Key(std::u16string(name));}
[[noreturn]] void filterTypeError(std::u16string message){throw options::Exception("TypeError",std::move(message));}
}
options::Value InputJaxFilterData::rawMathValue()const{
    if(const auto* value=std::get_if<options::Value>(&math_))return *value;
    if(std::holds_alternative<Missing>(math_))return options::Undefined{};
    if(std::holds_alternative<PropertyNull>(math_))return options::Null{};
    auto* item=std::get<MathItem*>(math_);
    auto* identity=item?dynamic_cast<MathItemSourceIdentity*>(item):nullptr;
    if(!identity||!sourceRecord||&identity->sourceContext()!=&sourceRecord->context())
        throw options::Exception("Unimplemented",u"Input filter MathItem has no identity in its Context");
    return identity->sourceIdentity();
}
options::Value InputJaxFilterData::rawDocumentValue()const{
    if(const auto* value=std::get_if<options::Value>(&document_))return *value;
    if(std::holds_alternative<Missing>(document_))return options::Undefined{};
    if(std::holds_alternative<PropertyNull>(document_))return options::Null{};
    auto* owner=std::get<MathDocumentService*>(document_);
    if(!owner||!sourceRecord||&owner->sourceContext()!=&sourceRecord->context())
        throw options::Exception("Unimplemented",u"Input filter MathDocument has no identity in its Context");
    return owner->sourceIdentity();
}
options::Value InputJaxFilterData::rawDataValue()const{
    if(const auto* value=std::any_cast<options::Value>(&data_))return *value;
    if(!data_.has_value())return options::Undefined{};
    if(!encodeData)throw options::Exception("Unimplemented",u"Input filter data has no canonical Context encoder");
    return encodeData(data_);
}
options::Value InputJaxFilterData::field(std::u16string_view name)const{
    if(!sourceRecord)throw options::Exception("TypeError",u"Input filter record has no Context owner");
    return sourceRecord->get(filterKey(name));
}
options::Value InputJaxFilterData::setField(std::u16string_view name,options::Value value){
    if(!sourceRecord)throw options::Exception("TypeError",u"Input filter record has no Context owner");
    sourceRecord->set(filterKey(name),value);return value;
}
InputFilterMathReference InputJaxFilterData::mathReference()const{
    const auto value=field(u"math");
    if(std::holds_alternative<options::Undefined>(value.data))return Missing{};
    if(std::holds_alternative<options::Null>(value.data))return PropertyNull{};
    if(const auto* object=std::get_if<options::ObjectRef>(&value.data)){
        const auto* payload=std::any_cast<NativeObjectRef>(&sourceRecord->context().payload(*object));
        const auto item=payload?std::dynamic_pointer_cast<MathItem>(*payload):nullptr;
        if(!item)filterTypeError(u"Filter math is not a MathItem");
        const auto* identity=dynamic_cast<const MathItemSourceIdentity*>(item.get());
        if(!identity||&identity->sourceContext()!=&sourceRecord->context()||identity->sourceIdentity()!=*object)
            filterTypeError(u"Filter math belongs to a different Context");
        return item.get();
    }
    return value;
}
InputFilterDocumentReference InputJaxFilterData::documentReference()const{
    const auto value=field(u"document");
    if(std::holds_alternative<options::Undefined>(value.data))return Missing{};
    if(std::holds_alternative<options::Null>(value.data))return PropertyNull{};
    if(const auto* object=std::get_if<options::ObjectRef>(&value.data)){
        const auto* payload=std::any_cast<NativeObjectRef>(&sourceRecord->context().payload(*object));
        const auto owner=payload?std::dynamic_pointer_cast<MathDocumentService>(*payload):nullptr;
        if(!owner)filterTypeError(u"Filter document is not a MathDocument");
        if(&owner->sourceContext()!=&sourceRecord->context()||owner->sourceIdentity()!=*object)
            filterTypeError(u"Filter document belongs to a different Context");
        return owner.get();
    }
    return value;
}
std::any InputJaxFilterData::currentData()const{
    const auto value=field(u"data");
    if(!decodeData)throw options::Exception("Unimplemented",u"Input filter data has no typed owner resolver");
    return decodeData(value);
}
options::ObjectRef InputJaxFilterData::sourceIdentity()const{
    if(!sourceRecord)throw options::Exception("TypeError",u"Input filter record has no Context identity");
    sourceRecord->ensureActive();
    return sourceRecord->identity();
}
void InputJaxFilterData::bindSourceRecord(options::Context& context,const InputJaxFilterDataRef& owner,
                                          std::weak_ptr<std::atomic_bool> lease){
    if(sourceRecord)throw options::Exception("TypeError",u"Input filter record is already bound");
    if(!owner||owner.get()!=this)throw options::Exception("TypeError",u"Input filter record owner mismatch");
    const std::array<JaxFilterRecordField,3> fields{{
        {filterKey(u"math"),[owner](){return owner->rawMathValue();},[owner](options::Value value){owner->math_=std::move(value);},[owner](){owner->math_=Missing{};}},
        {filterKey(u"document"),[owner](){return owner->rawDocumentValue();},[owner](options::Value value){owner->document_=std::move(value);},[owner](){owner->document_=Missing{};}},
        {filterKey(u"data"),[owner](){return owner->rawDataValue();},[owner](options::Value value){owner->data_=std::move(value);},[owner](){owner->data_=std::any(options::Value(options::Undefined{}));}}
    }};
    sourceRecord=bindJaxFilterRecord(context,std::static_pointer_cast<NativeObject>(owner),std::move(lease),fields);
    context.payload(sourceRecord->identity())=owner;
}
void InputJaxFilterData::initialize(MathItemReference math,MathDocumentReference document,std::any data){
    if(std::holds_alternative<Missing>(math))math_=Missing{};
    else if(std::holds_alternative<PropertyNull>(math))math_=PropertyNull{};
    else math_=std::get<MathItem*>(math);
    if(std::holds_alternative<Missing>(document))document_=Missing{};
    else if(std::holds_alternative<PropertyNull>(document))document_=PropertyNull{};
    else document_=std::get<MathDocumentService*>(document);
    data_=std::move(data);
}
options::Value AbstractInputJax::name(){
    auto& context=optionsContext();
    const options::Value constructor=context.get(options::Value(instanceIdentity()),options::Key(u"constructor"));
    return context.get(constructor,options::Key(u"NAME"));
}
bool AbstractInputJax::processStrings()const{return true;}
void AbstractInputJax::setAdaptor(InputAdaptorReference value){adaptor=std::move(value);}
void AbstractInputJax::setMmlFactory(InputMmlFactoryReference value){mmlFactory=std::move(value);}
void AbstractInputJax::bindFilterRecordValueCodec(
    std::function<options::Value(const std::any&)> encode,
    std::function<std::any(options::Value)> decode,
    std::weak_ptr<std::atomic_bool> lease){
    if(!encode||!decode)throw options::Exception("TypeError",u"Input filter record requires its data codec");
    filterDataEncoder_=std::move(encode);filterDataDecoder_=std::move(decode);
    filterExecutionLease_=std::move(lease);
}
void AbstractInputJax::initialize(){}
void AbstractInputJax::reset(FunctionArgs){}
ProtoMathItems AbstractInputJax::findMath(std::any,options::Value){return std::make_shared<std::vector<ProtoMathItem>>();}
std::any AbstractInputJax::executeFilters(FunctionList& filters,MathItemReference math,MathDocumentReference document,std::any value){
    auto record=std::make_shared<InputJaxFilterData>();
    record->initialize(std::move(math),std::move(document),std::move(value));
    record->encodeData=filterDataEncoder_;record->decodeData=filterDataDecoder_;
    record->bindSourceRecord(optionsContext(),record,filterExecutionLease_);
    std::array<std::any,1> arguments{InputJaxFilterDataRef(record)};
    filters.execute(arguments);
    // Source returns data.data. Re-enter the same live Context descriptor, then
    // decode that value back into the native data owner when one is registered.
    return record->currentData();
}
}
