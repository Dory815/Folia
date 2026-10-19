// MathJax 4.1.3 core/OutputJax.ts; Apache-2.0.
module folia.math.core.output_jax;
import folia.math.core.math_render_actions;
import folia.math.util.property_host;
import std;
namespace folia::math::core {
namespace {
std::optional<std::size_t> methodLength(std::u16string_view name){
    if(name==u"setAdaptor")return 1;
    if(name==u"getMetrics"||name==u"styleSheet"||name==u"pageElements")return 1;
    if(name==u"executeFilters")return 4;
    if(name==u"initialize"||name==u"reset")return 0;
    return {};
}
options::Value argument(std::span<const options::Value> args,std::size_t index){
    return index<args.size()?args[index]:options::Value(options::Undefined{});
}
void installMethod(options::Context& context,options::ObjectRef prototype,
                   std::u16string name,options::NativeCall body){
    const auto function=context.function(name,std::move(body));
    if(const auto length=methodLength(name))
        context.define(function,options::Key(u"length"),options::Descriptor::data(options::Value(static_cast<double>(*length)),false,false,true));
    // The pinned CJS output assigns methods directly to the prototype, so
    // these data properties are enumerable as well as writable/configurable.
    context.define(prototype,options::Key(std::move(name)),options::Descriptor::data(options::Value(function),true,true,true));
}
options::Value outputValue(const std::any& value){
    if(!value.has_value())return options::Undefined{};
    if(const auto* v=std::any_cast<options::Value>(&value))return *v;
    if(const auto* v=std::any_cast<bool>(&value))return *v;
    if(const auto* v=std::any_cast<double>(&value))return *v;
    if(const auto* v=std::any_cast<std::u16string>(&value))return *v;
    if(const auto* v=std::any_cast<std::string>(&value))return property_host::text(*v);
    throw options::Exception("Unimplemented",u"OutputJax filter data has no canonical Context identity binding");
}
void bindFilterRecord(options::Context& context,
                      std::weak_ptr<std::atomic_bool> executionLease,
                      const OutputJaxFilterDataRef& record,options::ObjectRef object){
    const std::array<JaxFilterRecordField,3> fields{{
        {options::Key(u"math"),[record](){return record->rawMathValue();},[record](options::Value value){record->assignMath(std::move(value));},[record](){record->removeMath();}},
        {options::Key(u"document"),[record](){return record->rawDocumentValue();},[record](options::Value value){record->assignDocument(std::move(value));},[record](){record->removeDocument();}},
        {options::Key(u"data"),[record](){return record->rawDataValue();},[record](options::Value value){record->assignData(std::move(value));},[record](){record->removeData();}}
    }};
    try{
        record->sourceRecord=bindJaxFilterRecord(context,std::static_pointer_cast<NativeObject>(record),std::move(executionLease),fields,object);
        context.payload(object)=record;
    }catch(...){context.retireObjectRecord(object);throw;}
}
}
options::Value OutputJaxFilterData::rawMathValue()const{
    if(const auto* value=std::get_if<options::Value>(&math))return *value;
    if(std::holds_alternative<Missing>(math))return options::Undefined{};
    if(std::holds_alternative<PropertyNull>(math))return options::Null{};
    auto* item=std::get<MathItem*>(math);
    const auto* identity=item?dynamic_cast<const MathItemSourceIdentity*>(item):nullptr;
    if(!identity||!sourceRecord||&identity->sourceContext()!=&sourceRecord->context())
        throw options::Exception("Unimplemented",u"Output filter MathItem has no identity in its Context");
    return identity->sourceIdentity();
}
options::Value OutputJaxFilterData::rawDocumentValue()const{
    if(const auto* value=std::get_if<options::Value>(&document))return *value;
    if(std::holds_alternative<Missing>(document))return options::Undefined{};
    if(std::holds_alternative<PropertyNull>(document))return options::Null{};
    auto* owner=std::get<MathDocumentService*>(document);
    if(!owner||!sourceRecord||&owner->sourceContext()!=&sourceRecord->context())
        throw options::Exception("Unimplemented",u"Output filter MathDocument has no identity in its Context");
    return owner->sourceIdentity();
}
options::Value OutputJaxFilterData::rawDataValue()const{
    if(const auto* value=std::any_cast<options::Value>(&data))return *value;
    if(const auto* value=std::any_cast<Property>(&data)){
        if(!encodeProperty)throw options::Exception("Unimplemented",u"Output filter Property has no canonical Context encoder");
        return encodeProperty(*value);
    }
    return outputValue(data);
}
options::Value OutputJaxFilterData::field(std::u16string_view name)const{
    if(!sourceRecord)throw options::Exception("TypeError",u"Output filter record has no Context owner");
    return sourceRecord->get(options::Key(std::u16string(name)));
}
options::Value OutputJaxFilterData::setField(std::u16string_view name,options::Value value){
    if(!sourceRecord)throw options::Exception("TypeError",u"Output filter record has no Context owner");
    sourceRecord->set(options::Key(std::u16string(name)),value);return value;
}
OutputMathItemReference OutputJaxFilterData::mathReference()const{
    const auto value=field(u"math");
    if(std::holds_alternative<options::Undefined>(value.data))return Missing{};
    if(std::holds_alternative<options::Null>(value.data))return PropertyNull{};
    if(const auto* object=std::get_if<options::ObjectRef>(&value.data)){
        const auto* payload=std::any_cast<NativeObjectRef>(&sourceRecord->context().payload(*object));
        const auto item=payload?std::dynamic_pointer_cast<MathItem>(*payload):nullptr;
        if(!item)throw options::Exception("TypeError",u"Output filter math is not a MathItem");
        const auto* identity=dynamic_cast<const MathItemSourceIdentity*>(item.get());
        if(!identity||&identity->sourceContext()!=&sourceRecord->context()||identity->sourceIdentity()!=*object)
            throw options::Exception("TypeError",u"Output filter math belongs to a different Context");
        return item.get();
    }
    return value;
}
OutputJaxDocumentReference OutputJaxFilterData::documentReference()const{
    const auto value=field(u"document");
    if(std::holds_alternative<options::Undefined>(value.data))return Missing{};
    if(std::holds_alternative<options::Null>(value.data))return PropertyNull{};
    if(const auto* object=std::get_if<options::ObjectRef>(&value.data)){
        const auto* payload=std::any_cast<NativeObjectRef>(&sourceRecord->context().payload(*object));
        const auto owner=payload?std::dynamic_pointer_cast<MathDocumentService>(*payload):nullptr;
        if(!owner)throw options::Exception("TypeError",u"Output filter document is not a MathDocument");
        if(&owner->sourceContext()!=&sourceRecord->context()||owner->sourceIdentity()!=*object)
            throw options::Exception("TypeError",u"Output filter document belongs to a different Context");
        return owner.get();
    }
    return value;
}
options::Value OutputJaxFilterData::dataValue()const{return field(u"data");}
options::ObjectRef OutputJaxFilterData::sourceIdentity()const{
    if(!sourceRecord)throw options::Exception("TypeError",u"Output filter record has no Context identity");
    sourceRecord->ensureActive();
    return sourceRecord->identity();
}
void OutputJaxFilterData::initialize(OutputMathItemReference mathValue,
    OutputJaxDocumentReference documentValue,std::any dataValue,
    std::function<options::Value(const Property&)> encoder){
    math=std::move(mathValue);document=std::move(documentValue);data=std::move(dataValue);
    encodeProperty=std::move(encoder);
}
OutputJaxModule::OutputJaxModule(options::Context& context,OptionFunctionBindings& bindings)
    :abstractClass_(context.constructor(u"AbstractOutputJax")),filterBindings_(context,bindings),
     filterRecordOwner_(std::make_shared<JaxFilterRecordOwnerScope>(context)){
    filterBindings_.addNativeArgumentEncoder([](const std::any& argument)->std::optional<options::Value>{
        if(const auto* record=std::any_cast<OutputJaxFilterDataRef>(&argument);record&&*record)
            return options::Value((*record)->sourceIdentity());
        return std::nullopt;
    });
    // The pinned CJS build emits this class as an ordinary ES5 function with
    // one formal parameter, so its function length is one and its prototype
    // property remains writable.
    context.define(abstractClass_,options::Key(u"length"),
        options::Descriptor::data(options::Value(1),false,false,true));
    const auto defaults=context.object();
    context.set(defaults,options::Key(u"preFilters"),context.array());
    context.set(defaults,options::Key(u"postFilters"),context.array());
    context.set(abstractClass_,options::Key(u"NAME"),u"generic");
    context.set(abstractClass_,options::Key(u"OPTIONS"),defaults);
    const auto prototype=std::get<options::ObjectRef>(context.get(options::Value(abstractClass_),options::Key(u"prototype")).data);
    context.define(abstractClass_,options::Key(u"prototype"),options::Descriptor::data(options::Value(prototype),true,false,false));
    const auto getter=context.function(u"get name",[](options::Context& cx,options::Value self,std::span<const options::Value>)->options::Value{
        const auto constructor=cx.get(std::move(self),options::Key(u"constructor"));
        return cx.get(constructor,options::Key(u"NAME"));
    });
    context.define(prototype,options::Key(u"name"),options::Descriptor::access(getter,{},false,true));
    installMethod(context,prototype,u"setAdaptor",[](options::Context& cx,options::Value self,std::span<const options::Value> args){
        cx.set(std::move(self),options::Key(u"adaptor"),argument(args,0));return options::Value(options::Undefined{});
    });
    installMethod(context,prototype,u"initialize",[](options::Context&,options::Value,std::span<const options::Value>){
        return options::Value(options::Undefined{});
    });
    installMethod(context,prototype,u"reset",[](options::Context&,options::Value,std::span<const options::Value>){
        return options::Value(options::Undefined{});
    });
    installMethod(context,prototype,u"getMetrics",[](options::Context&,options::Value,std::span<const options::Value>){
        return options::Value(options::Undefined{});
    });
    installMethod(context,prototype,u"styleSheet",[](options::Context&,options::Value,std::span<const options::Value>){
        return options::Value(options::Null{});
    });
    installMethod(context,prototype,u"pageElements",[](options::Context&,options::Value,std::span<const options::Value>){
        return options::Value(options::Null{});
    });
    auto toPropertyValue=filterBindings_.toValueConverter();
    auto executionLease=filterBindings_.executionLifetime();
    installMethod(context,prototype,u"executeFilters",[toPropertyValue=std::move(toPropertyValue),executionLease,
        tracker=filterRecordOwner_](options::Context& cx,options::Value,std::span<const options::Value> args){
        const auto documentArgument=argument(args,2);
        auto owner=std::make_shared<OutputJaxFilterData>();
        owner->initialize(argument(args,1),documentArgument,std::any(argument(args,3)),toPropertyValue);
        const auto record=cx.object();
        bindFilterRecord(cx,executionLease,owner,record);
        if(const auto* document=std::get_if<options::ObjectRef>(&documentArgument.data)){
            try{tracker->capture(*document,record);}
            catch(...){cx.retireObjectRecord(record);throw;}
        }
        const auto filters=argument(args,0);
        const auto execute=cx.get(filters,options::Key(u"execute"));
        const std::array<options::Value,1> callArguments{options::Value(record)};
        (void)cx.callMember(execute,filters,callArguments,u"filters.execute");
        return cx.get(options::Value(record),options::Key(u"data"));
    });
}
void AbstractOutputJax::bindOutputJaxFields(std::shared_ptr<void> owner){
    auto& context=optionsContext();
    auto* self=this;
    auto adaptorField=std::make_shared<options::NativeDataField>(std::move(owner),
        [self]()->options::Value{
            if(std::holds_alternative<Missing>(self->adaptor))return options::Undefined{};
            if(std::holds_alternative<PropertyNull>(self->adaptor))return options::Null{};
            if(const auto* value=std::get_if<options::Value>(&self->adaptor))return *value;
            throw options::Exception("Unimplemented",u"Native adaptor has no Context identity");
        },
        [self](options::Value value){
            if(std::holds_alternative<options::Undefined>(value.data))self->adaptor=Missing{};
            else if(std::holds_alternative<options::Null>(value.data))self->adaptor=PropertyNull{};
            else self->adaptor=std::move(value);
        });
    context.bindData(identity_,options::Key(u"adaptor"),std::move(adaptorField));
}
void AbstractOutputJax::initializeOutputJax(options::Value supplied,std::shared_ptr<void> owner){
    auto& context=bindings_.context();
    if(std::holds_alternative<options::Undefined>(supplied.data))
        supplied=options::Value(context.object());
    const options::Value klass=context.get(options::Value(identity_),options::Key(u"constructor"));
    const options::Value destination(context.object());
    const options::Value classOptions=context.get(klass,options::Key(u"OPTIONS"));
    const std::array<options::Value,1> defaults{classOptions};
    const options::Value defaulted=options::defaultOptions(context,destination,defaults);
    const std::array<options::Value,1> userDefinitions{supplied};
    const options::Value initialized=options::userOptions(context,defaulted,userDefinitions);
    options=initialized;
    auto* self=this;
    auto bindList=filterBindings_.bindConverter();
    auto resolveList=filterBindings_.resolveConverter();
    auto optionsField=std::make_shared<options::NativeDataField>(owner,
        [self]()->options::Value{return self->options;},
        [self](options::Value value){self->options=std::move(value);});
    context.bindData(identity_,options::Key(u"options"),std::move(optionsField));
    const options::Value pre=context.get(options,options::Key(u"preFilters"));
    preFilters=makeOptionFunctionList(bindings_,pre);
    auto preField=std::make_shared<options::NativeDataField>(owner,
        [self,bindList]()->options::Value{return options::Value(bindList(self->preFilters));},
        [self,resolveList](options::Value value){
            const auto* object=std::get_if<options::ObjectRef>(&value.data);
            if(!object)throw options::Exception("Unimplemented",u"Replacing preFilters requires a registered FunctionList instance");
            auto list=resolveList(*object);
            if(!list)throw options::Exception("Unimplemented",u"Foreign preFilters value has no canonical FunctionList owner");
            self->preFilters=std::move(list);
        });
    context.bindData(identity_,options::Key(u"preFilters"),std::move(preField));
    const options::Value post=context.get(options,options::Key(u"postFilters"));
    postFilters=makeOptionFunctionList(bindings_,post);
    auto postField=std::make_shared<options::NativeDataField>(owner,
        [self,bindList]()->options::Value{return options::Value(bindList(self->postFilters));},
        [self,resolveList](options::Value value){
            const auto* object=std::get_if<options::ObjectRef>(&value.data);
            if(!object)throw options::Exception("Unimplemented",u"Replacing postFilters requires a registered FunctionList instance");
            auto list=resolveList(*object);
            if(!list)throw options::Exception("Unimplemented",u"Foreign postFilters value has no canonical FunctionList owner");
            self->postFilters=std::move(list);
        });
    context.bindData(identity_,options::Key(u"postFilters"),std::move(postField));
}
options::Value AbstractOutputJax::name(){
    auto& context=optionsContext();
    const auto constructor=context.get(options::Value(instanceIdentity()),options::Key(u"constructor"));
    return context.get(constructor,options::Key(u"NAME"));
}
void AbstractOutputJax::setAdaptor(OutputAdaptorReference value){
    // A native adaptor is retained as its original owner reference.  Until a
    // canonical DOMAdaptor identity registry exists, do not manufacture a
    // plain Context object as a substitute identity.
    adaptor=std::move(value);
}
void AbstractOutputJax::initialize(){}
void AbstractOutputJax::reset(FunctionArgs){}
void AbstractOutputJax::getMetrics(MathDocumentReference){}
MathOutput AbstractOutputJax::styleSheet(MathDocumentReference){return PropertyNull{};}
MathOutput AbstractOutputJax::pageElements(MathDocumentReference){return PropertyNull{};}
std::any AbstractOutputJax::executeFilters(FunctionList& filters,OutputMathItemReference math,
                                          MathDocumentReference document,std::any data){
    std::optional<options::ObjectRef> captureDocument;
    if(filterRecordOwner_){
        if(const auto* actual=std::get_if<MathDocumentService*>(&document);
           actual && *actual && &(*actual)->sourceContext()==&optionsContext())
            captureDocument=(*actual)->sourceIdentity();
    }
    auto record=std::make_shared<OutputJaxFilterData>();
    OutputJaxDocumentReference filterDocument=Missing{};
    if(std::holds_alternative<Missing>(document))filterDocument=Missing{};
    else if(std::holds_alternative<PropertyNull>(document))filterDocument=PropertyNull{};
    else if(const auto* native=std::get_if<MathDocumentService*>(&document))filterDocument=*native;
    record->initialize(std::move(math),std::move(filterDocument),std::move(data),filterBindings_.toValueConverter());
    bindFilterRecord(optionsContext(),filterBindings_.executionLifetime(),record,optionsContext().object());
    if(captureDocument){
        try{filterRecordOwner_->capture(*captureDocument,record->sourceIdentity());}
        catch(...){optionsContext().retireObjectRecord(record->sourceIdentity());throw;}
    }
    const std::array<std::any,1> arguments{OutputJaxFilterDataRef(record)};
    filters.execute(arguments);
    return std::any(record->dataValue());
}
}
