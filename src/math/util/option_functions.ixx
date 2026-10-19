// Native callable identity/array access binding for Source option filter arrays.
// This host owns no FunctionList algorithm and no option dictionary copy.
export module folia.math.util.option_functions;
export import folia.math.util.options;
export import folia.math.util.function_list;
import folia.math.util.property_host;
import std;
export namespace folia::math {
// Converters carry no callback algorithm. The binding always invokes the one
// retained FunctionRef body between argument conversion and result conversion.
struct OptionCallBinding {
    std::function<FunctionReceiver(options::Context&,options::Value)> receiver;
    std::function<std::vector<std::any>(options::Context&,std::span<const options::Value>)> arguments;
    std::function<options::Value(options::Context&,const std::any&)> result;
};
class OptionFunctionBindings {
    options::Context& context_;
    std::vector<std::pair<FunctionRef,options::ObjectRef>> bindings_;
public:
    explicit OptionFunctionBindings(options::Context& context):context_(context){}
    OptionFunctionBindings(const OptionFunctionBindings&)=delete;
    OptionFunctionBindings& operator=(const OptionFunctionBindings&)=delete;
    options::Context& context()const{return context_;}
    std::size_t canonicalBindingCountForDiagnostics()const noexcept{return bindings_.size();}
    std::optional<options::ObjectRef> find(FunctionRef function)const{
        for(const auto& [known,object]:bindings_)if(known==function)return object;
        return {};
    }
    std::optional<FunctionRef> find(options::ObjectRef object)const{
        for(const auto& [function,known]:bindings_)if(known==object)return function;
        return {};
    }
    options::Value bind(FunctionRef function,OptionCallBinding binding,std::u16string label=u"native"){
        if(!function||!function->call||!binding.receiver||!binding.arguments||!binding.result)
            throw tree::FactoryError("Unimplemented",u"Option function requires its canonical body and typed call binding");
        for(const auto&[existing,object]:bindings_)if(existing==function)return object;
        const auto metadata=function->sourceMetadata;
        const auto object=context_.function(metadata?metadata->name:std::move(label),
            [function,binding=std::move(binding)](options::Context& context,options::Value receiver,std::span<const options::Value> arguments){
                const auto nativeReceiver=binding.receiver(context,receiver);
                const auto nativeArguments=binding.arguments(context,arguments);
                const auto result=function->call(nativeReceiver,nativeArguments);
                return binding.result(context,result);
            },function);
        if(metadata)context_.define(object,options::Key(u"length"),
            options::Descriptor::data(options::Value(static_cast<double>(metadata->length)),false,false,true));
        bindings_.emplace_back(std::move(function),object);
        return object;
    }
    FunctionValue functionValue(options::Value value){
        if(auto* object=std::get_if<options::ObjectRef>(&value.data)){
            for(const auto&[function,known]:bindings_)if(known==*object)return function;
            // Do not manufacture a second body or silently replace an object.
            throw tree::FactoryError("Unimplemented",u"Option callable/object has no canonical native identity binding");
        }
        if(std::holds_alternative<options::Undefined>(value.data))return Property(Missing{});
        if(std::holds_alternative<options::Null>(value.data))return Property(PropertyNull{});
        if(auto* p=std::get_if<bool>(&value.data))return Property(*p);
        if(auto* p=std::get_if<double>(&value.data))return Property(*p);
        if(auto* p=std::get_if<std::u16string>(&value.data))return Property(property_host::debugText(*p));
        throw tree::FactoryError("Unimplemented",u"Option Symbol callback slot requires a native symbol binding");
    }
};
// Captures one actual array handle; Get and length remain live, including holes
// and inherited indexed accessors. The tuple object is captured before 0/1 reads.
class OptionFunctionDefinitions final:public FunctionDefinitionsSource {
    OptionFunctionBindings& bindings_;
    options::ObjectRef array_;
public:
    OptionFunctionDefinitions(OptionFunctionBindings& bindings,options::Value array):bindings_(bindings){
        auto* object=std::get_if<options::ObjectRef>(&array.data);
        if(!object||bindings.context().kind(*object)!=options::ObjectKind::array)
            throw tree::FactoryError("Unimplemented",u"Option filters require the native ordinary-array iterator binding");
        array_=*object;
    }
    std::size_t length()override{return bindings_.context().arrayLength(array_);}
    FunctionDefinition read(std::size_t index)override{
        auto& context=bindings_.context();const auto ascii=std::to_string(index);
        const auto value=context.get(array_,options::Key(std::u16string(ascii.begin(),ascii.end())));
        const auto* tuple=std::get_if<options::ObjectRef>(&value.data);
        if(!tuple||context.kind(*tuple)!=options::ObjectKind::array)return bindings_.functionValue(value);
        const auto item=bindings_.functionValue(context.get(*tuple,options::Key(u"0")));
        const auto priority=context.get(*tuple,options::Key(u"1"));
        auto result=std::make_shared<FunctionTuple>();result->item=item;
        if(auto* number=std::get_if<double>(&priority.data))result->priority=*number;
        else if(!std::holds_alternative<options::Undefined>(priority.data))
            throw tree::FactoryError("Unimplemented",u"Option filter tuple priority requires the native number/Undefined binding");
        return result;
    }
};
inline std::shared_ptr<FunctionList> makeOptionFunctionList(OptionFunctionBindings& bindings,options::Value array){
    auto result=makeFunctionList();
    if(options::truthy(array)){OptionFunctionDefinitions source(bindings,array);result->addList(source);}
    return result;
}
}
