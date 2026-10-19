// Focused observations for the generic Array.prototype.unshift Context
// intrinsic, including holes, inherited indices, partial effects and a live
// NativeIndexedView. No copied storage is used for the indexed-view case.
import std;
import folia.math.util.option_types;

namespace {
using namespace folia::math::options;

struct IndexedOwner final:NativeIndexedView{
    std::vector<std::optional<Value>> values;
    bool lengthWritable=true;
    explicit IndexedOwner(std::vector<Value> initial){
        for(auto& value:initial)values.emplace_back(std::move(value));
    }
    std::uint32_t length()const override{return static_cast<std::uint32_t>(values.size());}
    Descriptor lengthDescriptor()const override{
        return Descriptor::data(Value(static_cast<double>(values.size())),lengthWritable,false,false);
    }
    std::optional<Descriptor> indexDescriptor(std::uint32_t index)const override{
        if(index>=values.size()||!values[index])return {};
        return Descriptor::data(*values[index]);
    }
    std::vector<std::uint32_t> ownIndexKeys()const override{
        std::vector<std::uint32_t> result;
        for(std::uint32_t i=0;i<values.size();++i)if(values[i])result.push_back(i);
        return result;
    }
    void defineIndex(std::uint32_t index,Descriptor descriptor)override{
        if(descriptor.accessor||!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
            throw Exception("TypeError",u"Unexpected indexed descriptor");
        if(index>=values.size()){
            if(!lengthWritable)throw Exception("TypeError",u"Array length is not writable");
            values.resize(static_cast<std::size_t>(index)+1);
        }
        values[index]=std::move(descriptor.value);
    }
    void defineLength(Descriptor descriptor)override{
        const auto* number=std::get_if<double>(&descriptor.value.data);
        if(descriptor.accessor||descriptor.enumerable||descriptor.configurable||!number||
           *number<0||*number>4294967295.0||std::trunc(*number)!=*number)
            throw Exception("RangeError",u"Invalid native array length");
        if(!lengthWritable&&descriptor.writable)
            throw Exception("TypeError",u"Cannot make array length writable");
        values.resize(static_cast<std::size_t>(*number));
        lengthWritable=descriptor.writable;
    }
    void deleteIndex(std::uint32_t index)override{
        if(index<values.size())values[index].reset();
    }
};

void expectText(Context& context,ObjectRef object,std::u16string_view key,
                std::u16string_view expected){
    const auto value=context.get(Value(object),Key(std::u16string(key)));
    const auto* text=std::get_if<std::u16string>(&value.data);
    if(!text||*text!=expected)throw Exception("AssertionError",u"Unexpected indexed value");
}
bool own(Context& context,ObjectRef object,std::u16string_view key){
    return context.hasOwn(Value(object),Key(std::u16string(key)));
}
std::string jsonString(std::u16string_view text){
    std::string result="\"";
    for(const char16_t ch:text){
        if(ch==u'"'||ch==u'\\')result.push_back('\\');
        if(ch<0x20)throw Exception("AssertionError",u"Unexpected control character");
        result.push_back(static_cast<char>(ch));
    }
    result.push_back('"');
    return result;
}
std::u16string textValue(Context& context,ObjectRef object,std::u16string_view key){
    const auto value=context.get(Value(object),Key(std::u16string(key)));
    const auto* text=std::get_if<std::u16string>(&value.data);
    if(!text)throw Exception("AssertionError",u"Expected string observation");
    return *text;
}
std::string traceJson(const std::vector<std::u16string>& trace){
    std::ostringstream output;
    output<<'[';
    for(std::size_t i=0;i<trace.size();++i){
        if(i)output<<',';
        output<<jsonString(trace[i]);
    }
    output<<']';
    return output.str();
}
}

int main(){
 try{
    using namespace folia::math::options;
    Context context;
    const ObjectRef prototype=context.intrinsics().arrayPrototype;
    const auto method=std::get<ObjectRef>(context.get(Value(prototype),Key(u"unshift")).data);
    const auto descriptor=context.descriptor(Value(prototype),Key(u"unshift"));
    if(!descriptor||!descriptor->writable||descriptor->enumerable||!descriptor->configurable||
       context.get(method,Key(u"name"))!=Value(u"unshift")||
       context.get(method,Key(u"length"))!=Value(1))
        throw Exception("AssertionError",u"Array.prototype.unshift descriptor differs from Source");

    const auto ordinary=context.array(2);
    context.set(Value(ordinary),Key(u"0"),Value(u"two"));
    context.set(Value(ordinary),Key(u"1"),Value(u"three"));
    const std::array<Value,1> first{Value(u"one")};
    if(context.call(method,Value(ordinary),first)!=Value(3))
        throw Exception("AssertionError",u"unshift returned the wrong length");
    expectText(context,ordinary,u"0",u"one");
    expectText(context,ordinary,u"1",u"two");
    expectText(context,ordinary,u"2",u"three");

    const auto inheritedPrototype=context.object();
    context.set(Value(inheritedPrototype),Key(u"0"),Value(u"prototype-zero"));
    const auto sparse=context.array(2);
    context.set(Value(sparse),Key(u"1"),Value(u"tail"));
    context.setPrototype(sparse,inheritedPrototype);
    const std::array<Value,1> headArg{Value(u"head")};
    if(context.call(method,Value(sparse),headArg)!=Value(3))
        throw Exception("AssertionError",u"inherited unshift returned the wrong length");
    expectText(context,sparse,u"0",u"head");
    expectText(context,sparse,u"1",u"prototype-zero");
    expectText(context,sparse,u"2",u"tail");

    const auto holey=context.array(2);
    context.set(Value(holey),Key(u"1"),Value(u"tail"));
    if(context.call(method,Value(holey),headArg)!=Value(3)||
       !own(context,holey,u"0")||own(context,holey,u"1")||!own(context,holey,u"2"))
        throw Exception("AssertionError",u"unshift did not preserve a sparse hole");

    const auto genericPrototype=context.object();
    context.set(Value(genericPrototype),Key(u"1"),Value(u"inherited-tail"));
    const auto generic=context.objectWithPrototype(genericPrototype);
    context.set(Value(generic),Key(u"length"),Value(2));
    context.set(Value(generic),Key(u"0"),Value(u"head"));
    if(context.call(method,Value(generic),first)!=Value(3))
        throw Exception("AssertionError",u"generic unshift returned the wrong length");
    expectText(context,generic,u"0",u"one");
    expectText(context,generic,u"1",u"head");
    expectText(context,generic,u"2",u"inherited-tail");

    std::vector<std::u16string> order;
    const auto observed=context.object();
    context.set(Value(observed),Key(u"length"),Value(2));
    const auto getOne=context.function(u"get one",
        [&](Context&,Value,std::span<const Value>)->Value{
            order.push_back(u"get:1");return Value(u"one");
        });
    const auto setOne=context.function(u"set one",
        [&](Context&,Value,std::span<const Value> args)->Value{
            order.push_back(u"set:1:"+std::get<std::u16string>(args[0].data));return Undefined{};
        });
    const auto setTwo=context.function(u"set two",
        [&](Context&,Value,std::span<const Value> args)->Value{
            order.push_back(u"set:2:"+std::get<std::u16string>(args[0].data));return Undefined{};
        });
    const auto getZero=context.function(u"get zero",
        [&](Context&,Value,std::span<const Value>)->Value{
            order.push_back(u"get:0");return Value(u"zero");
        });
    const auto setZero=context.function(u"set zero",
        [&](Context&,Value,std::span<const Value> args)->Value{
            order.push_back(u"set:0:"+std::get<std::u16string>(args[0].data));return Undefined{};
        });
    context.define(observed,Key(u"1"),Descriptor::access(getOne,setOne,true,true));
    context.define(observed,Key(u"2"),Descriptor::access({},setTwo,true,true));
    context.define(observed,Key(u"0"),Descriptor::access(getZero,setZero,true,true));
    const std::array<Value,1> accessorArgument{Value(u"first")};
    if(context.call(method,Value(observed),accessorArgument)!=Value(3)||
       order!=std::vector<std::u16string>{u"get:1",u"set:2:one",u"get:0",
                                         u"set:1:zero",u"set:0:first"}){
        throw Exception("AssertionError",u"unshift operation order differs from Source");
    }

    std::vector<std::u16string> zeroArgOrder;
    const auto zeroPrototype=context.object();
    const auto inheritedIndex=context.function(u"get inherited index",
        [&](Context&,Value,std::span<const Value>)->Value{
            zeroArgOrder.push_back(u"get:1");return Value(u"inherited");
        });
    context.define(zeroPrototype,Key(u"1"),Descriptor::access(inheritedIndex,{},true,true));
    const auto zeroArg=context.objectWithPrototype(zeroPrototype);
    const auto lengthGetter=context.function(u"get length",
        [&](Context&,Value,std::span<const Value>)->Value{
            zeroArgOrder.push_back(u"get:length");return Value(2);
        });
    const auto lengthSetter=context.function(u"set length",
        [&](Context&,Value,std::span<const Value>)->Value{
            zeroArgOrder.push_back(u"set:length");return Undefined{};
        });
    context.define(zeroArg,Key(u"length"),Descriptor::access(lengthGetter,lengthSetter,true,true));
    const auto zeroArgResult=context.call(method,Value(zeroArg));
    if(zeroArgResult!=Value(2)||
       zeroArgOrder!=std::vector<std::u16string>{u"get:length",u"set:length"})
        throw Exception("AssertionError",u"zero-argument unshift touched indexed properties");

    const auto readOnly=context.object();
    context.define(readOnly,Key(u"length"),Descriptor::data(Value(2),false,true,false));
    bool readOnlyLengthThrows=false;
    try{
        (void)context.call(method,Value(readOnly));
        throw Exception("AssertionError",u"zero-argument unshift should set array-like length");
    }catch(const Exception& error){
        readOnlyLengthThrows=error.name=="TypeError";
        if(!readOnlyLengthThrows)throw;
    }

    const auto partial=context.array(2);
    context.set(Value(partial),Key(u"0"),Value(u"zero"));
    context.define(partial,Key(u"1"),Descriptor::data(Value(u"fixed"),false,true,true));
    bool partialWriteThrows=false;
    try{
        (void)context.call(method,Value(partial),first);
        throw Exception("AssertionError",u"unshift should fail on a non-writable target");
    }catch(const Exception& error){
        partialWriteThrows=error.name=="TypeError";
        if(!partialWriteThrows)throw;
    }
    if(context.arrayLength(partial)!=3)
        throw Exception("AssertionError",u"unshift did not preserve the earlier index write");
    expectText(context,partial,u"2",u"fixed");

    auto owner=std::make_shared<IndexedOwner>(std::vector<Value>{Value(u"left"),Value(u"right")});
    const auto live=context.nativeArray(owner);
    const std::array<Value,1> prefix{Value(u"start")};
    if(context.call(method,Value(live),prefix)!=Value(3)||owner->length()!=3||
       !owner->values[0]||*owner->values[0]!=Value(u"start")||
       !owner->values[1]||*owner->values[1]!=Value(u"left")||
       !owner->values[2]||*owner->values[2]!=Value(u"right"))
        throw Exception("AssertionError",u"unshift bypassed or copied native indexed storage");

    bool nullReceiverThrows=false;
    try{
        (void)context.call(method,Value(Null{}),first);
        throw Exception("AssertionError",u"unshift should reject a null receiver");
    }catch(const Exception& error){
        nullReceiverThrows=error.name=="TypeError";
        if(!nullReceiverThrows)throw;
    }

    const auto methodName=textValue(context,method,u"name");
    const auto methodLength=context.get(method,Key(u"length"));
    const auto* lengthNumber=std::get_if<double>(&methodLength.data);
    if(!lengthNumber)throw Exception("AssertionError",u"Method length was not numeric");
    const auto number=[](const Value& value)->double{
        const auto* result=std::get_if<double>(&value.data);
        if(!result)throw Exception("AssertionError",u"Expected numeric observation");
        return *result;
    };
    const auto boolean=[](bool value){return value?"true":"false";};
    std::cout<<R"({"source":"ts/util/Options.ts","sourceSha256":"b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3","method":{"name":)"<<jsonString(methodName)
             <<R"(,"length":)"<<*lengthNumber<<R"(,"writable":)"<<boolean(descriptor->writable)
             <<R"(,"enumerable":)"<<boolean(descriptor->enumerable)
             <<R"(,"configurable":)"<<boolean(descriptor->configurable)
             <<R"(},"inherited":[)"<<jsonString(textValue(context,sparse,u"0"))<<','
             <<jsonString(textValue(context,sparse,u"1"))<<','
             <<jsonString(textValue(context,sparse,u"2"))<<R"(],"sparsePresence":[)"
             <<boolean(own(context,holey,u"0"))<<','<<boolean(own(context,holey,u"1"))<<','
             <<boolean(own(context,holey,u"2"))<<R"(],"order":)"<<traceJson(order)
             <<R"(,"zeroArgTrace":)"<<traceJson(zeroArgOrder)
             <<R"(,"partial":{"length":)"<<context.arrayLength(partial)
             <<R"(,"values":[)"
             <<jsonString(textValue(context,partial,u"0"))<<','
             <<jsonString(textValue(context,partial,u"1"))<<','
             <<jsonString(textValue(context,partial,u"2"))<<R"(]},"nullReceiverThrows":)"
             <<boolean(nullReceiverThrows)<<R"(,"nativeChecks":{"ordinary":[)"
             <<jsonString(textValue(context,ordinary,u"0"))<<','
             <<jsonString(textValue(context,ordinary,u"1"))<<','
             <<jsonString(textValue(context,ordinary,u"2"))<<R"(],"generic":[)"
             <<jsonString(textValue(context,generic,u"0"))<<','
             <<jsonString(textValue(context,generic,u"1"))<<','
             <<jsonString(textValue(context,generic,u"2"))<<R"(],"nativeView":{"length":)"
             <<owner->length()<<R"(,"values":[)"
             <<jsonString(std::get<std::u16string>(owner->values[0]->data))<<','
             <<jsonString(std::get<std::u16string>(owner->values[1]->data))<<','
             <<jsonString(std::get<std::u16string>(owner->values[2]->data))
             <<R"(]},"zeroArgResult":)"<<number(zeroArgResult)
             <<R"(,"readOnlyLengthThrows":)"<<boolean(readOnlyLengthThrows)
             <<R"(,"partialWriteThrows":)"<<boolean(partialWriteThrows)<<"}}\n";
    return 0;
 }catch(const folia::math::options::Exception& error){
    std::cerr<<"unshift candidate "<<error.name<<": ";
    for(const char16_t ch:error.message)std::cerr<<(ch<0x80?static_cast<char>(ch):'?');
    std::cerr<<'\n';
    return 2;
 }catch(const std::exception& error){
    std::cerr<<"unshift candidate NativeException: "<<error.what()<<'\n';
    return 2;
 }catch(...){
    std::cerr<<"unshift candidate unclassified exception\n";
    return 2;
 }
}
