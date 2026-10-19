#include "support/folia_test.hpp"
import folia.math.util.option_types;
import folia.math.util.js_number;

namespace {
using namespace boost::ut;
namespace opts=folia::math::options;
struct Fixture {
    opts::Context context;
    opts::ObjectRef array=context.array();
    opts::ObjectRef method=std::get<opts::ObjectRef>(context.get(array,opts::Key(u"concat")).data);
    std::vector<std::string> trace;
    static opts::Key index(std::uint64_t value){const auto text=std::to_string(value);return opts::Key(std::u16string(text.begin(),text.end()));}
    void set(opts::ObjectRef target,std::uint64_t i,opts::Value value){context.set(target,index(i),std::move(value));}
    std::string text(const opts::Value& value)const{
        if(std::holds_alternative<opts::Undefined>(value.data))return "U";
        if(std::holds_alternative<opts::Null>(value.data))return "N";
        if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?"true":"false";
        if(const auto* number=std::get_if<double>(&value.data))return folia::math::js_number::toString(*number);
        if(const auto* string=std::get_if<std::u16string>(&value.data))return std::string(string->begin(),string->end());
        if(const auto* object=std::get_if<opts::ObjectRef>(&value.data))return context.kind(*object)==opts::ObjectKind::array?"A":"O";
        return "Symbol";
    }
    std::string shape(opts::ObjectRef target){
        std::string result;
        const auto length=context.arrayLength(target);
        for(std::uint64_t i=0;i<length;++i){if(i)result+=',';result+=context.hasOwn(target,index(i))?text(context.get(target,index(i))):"H";}
        return result;
    }
    opts::Value concat(opts::Value receiver,std::span<const opts::Value> args={}){return context.call(method,std::move(receiver),args);}
    opts::ObjectRef arrayOf(std::initializer_list<opts::Value> values){auto result=context.array();std::uint64_t i=0;for(const auto& value:values)set(result,i++,value);return result;}
};
suite concat_suite=[] {
    "array_concat_has_canonical_function_metadata"_test=[] {
        Fixture f;const auto descriptor=f.context.descriptor(f.context.intrinsics().arrayPrototype,opts::Key(u"concat"));
        expect(descriptor.has_value());expect(descriptor&&!descriptor->accessor&&descriptor->writable&&!descriptor->enumerable&&descriptor->configurable);
        expect(f.context.get(f.method,opts::Key(u"name"))==opts::Value(u"concat"));
        expect(f.context.get(f.method,opts::Key(u"length"))==opts::Value(1));
        const auto symbolDescriptor=f.context.descriptor(f.context.intrinsics().symbolConstructor,opts::Key(u"isConcatSpreadable"));
        expect(f.context.get(f.context.intrinsics().symbolConstructor,opts::Key(u"isConcatSpreadable"))==opts::Value(f.context.intrinsics().symbolIsConcatSpreadable));
        expect(symbolDescriptor&&!symbolDescriptor->writable&&!symbolDescriptor->enumerable&&!symbolDescriptor->configurable);
    };
    "array_concat_spreads_only_one_level_and_preserves_identity"_test=[] {
        Fixture f;auto child=f.arrayOf({3});auto nested=f.arrayOf({2,child});auto other=f.context.object();const std::array<opts::Value,2> args{opts::Value(f.arrayOf({4})),opts::Value(other)};
        f.set(f.array,0,1);auto result=std::get<opts::ObjectRef>(f.concat(f.array,args).data);
        expect(eq(f.shape(result),std::string("1,4,O")));expect(f.context.get(result,Fixture::index(2))==opts::Value(other));
        const std::array<opts::Value,1> nestedArgs{opts::Value(nested)};auto nestedResult=std::get<opts::ObjectRef>(f.concat(f.array,nestedArgs).data);
        expect(eq(f.context.arrayLength(nestedResult),3u));expect(f.context.get(nestedResult,Fixture::index(2))==opts::Value(child));
    };
    "array_concat_preserves_holes_but_materializes_inherited_values"_test=[] {
        Fixture f;f.context.set(f.array,opts::Key(u"length"),3);f.set(f.array,1,u"b");
        auto proto=f.context.objectWithPrototype(f.context.intrinsics().arrayPrototype);
        auto source=f.context.array();f.context.set(source,opts::Key(u"length"),1);f.context.setPrototype(source,proto);
        auto getter=f.context.function(u"get0",[&](auto&,auto,auto)->opts::Value{f.trace.push_back("get0");return u"p";});
        f.context.define(proto,Fixture::index(0),opts::Descriptor::access(getter,{},true,true));
        const std::array<opts::Value,1> args{opts::Value(source)};auto result=std::get<opts::ObjectRef>(f.concat(f.array,args).data);
        expect(eq(f.shape(result),std::string("H,b,H,p")));expect(static_cast<bool>(f.trace==std::vector<std::string>{"get0"}));
    };
    "array_concat_observes_is_concat_spreadable_and_live_length_slots"_test=[] {
        Fixture f;auto object=f.context.object();f.context.set(object,opts::Key(u"length"),2);f.context.set(object,opts::Key(f.context.intrinsics().symbolIsConcatSpreadable),true);
        f.set(object,0,u"a");auto getter=f.context.function(u"get1",[&](auto&,auto receiver,auto)->opts::Value{f.trace.push_back("get1:"+std::string(receiver==opts::Value(object)?"same":"other"));return u"b";});
        f.context.define(object,Fixture::index(1),opts::Descriptor::access(getter,{},true,true));
        const std::array<opts::Value,1> args{opts::Value(object)};auto result=std::get<opts::ObjectRef>(f.concat(f.array,args).data);
        expect(eq(f.shape(result),std::string("a,b")));expect(static_cast<bool>(f.trace==std::vector<std::string>{"get1:same"}));
    };
    "array_concat_false_spreadable_keeps_array_identity"_test=[] {
        Fixture f;auto source=f.arrayOf({4,5});f.context.set(source,opts::Key(f.context.intrinsics().symbolIsConcatSpreadable),false);
        const std::array<opts::Value,1> args{opts::Value(source)};auto result=std::get<opts::ObjectRef>(f.concat(f.array,args).data);
        expect(eq(f.context.arrayLength(result),1u));expect(f.context.get(result,Fixture::index(0))==opts::Value(source));
    };
    "array_concat_gets_species_before_spreadability"_test=[] {
        Fixture f;auto ctor=f.context.constructor(u"C");
        auto speciesGetter=f.context.function(u"species-get",[&](auto&,auto,auto)->opts::Value{f.trace.push_back("species");return opts::Null{};});
        f.context.define(ctor,opts::Key(f.context.intrinsics().symbolSpecies),opts::Descriptor::access(speciesGetter,{},false,true));
        auto constructorGetter=f.context.function(u"constructor-get",[&](auto&,auto,auto)->opts::Value{f.trace.push_back("constructor");return ctor;});
        f.context.define(f.array,opts::Key(u"constructor"),opts::Descriptor::access(constructorGetter,{},true,true));
        auto spreadGetter=f.context.function(u"spread-get",[&](auto&,auto,auto)->opts::Value{f.trace.push_back("spreadable");return true;});
        f.context.define(f.array,opts::Key(f.context.intrinsics().symbolIsConcatSpreadable),opts::Descriptor::access(spreadGetter,{},true,true));
        const auto result=std::get<opts::ObjectRef>(f.concat(f.array).data);
        expect(eq(f.context.arrayLength(result),0u));
        expect(static_cast<bool>(f.trace==std::vector<std::string>{"constructor","species","spreadable"}));
    };
    "array_concat_generic_object_receiver_spreads_by_symbol"_test=[] {
        Fixture f;auto receiver=f.context.object();f.context.set(receiver,opts::Key(u"length"),1);f.set(receiver,0,u"x");
        f.context.set(receiver,opts::Key(f.context.intrinsics().symbolIsConcatSpreadable),true);
        const std::array<opts::Value,1> args{opts::Value(2)};auto result=std::get<opts::ObjectRef>(f.concat(receiver,args).data);
        expect(eq(f.shape(result),std::string("x,2")));
    };
    "array_concat_boxes_primitive_receiver_with_actual_internal_value"_test=[] {
        Fixture f;auto result=std::get<opts::ObjectRef>(f.concat(opts::Value(u"abc")).data);
        expect(eq(f.context.arrayLength(result),1u));const auto boxed=std::get<opts::ObjectRef>(f.context.get(result,Fixture::index(0)).data);
        expect(f.context.prototype(boxed)==f.context.intrinsics().stringPrototype);
        const auto* value=std::any_cast<opts::Value>(&f.context.payload(boxed));expect(value&&*value==opts::Value(u"abc"));
        expect(f.context.get(boxed,opts::Key(u"length"))==opts::Value(3));expect(f.context.get(boxed,Fixture::index(1))==opts::Value(u"b"));
    };
    "array_concat_boxes_number_boolean_and_symbol_receivers"_test=[] {
        Fixture f;const auto number=std::get<opts::ObjectRef>(f.concat(opts::Value(42)).data);
        const auto boolean=std::get<opts::ObjectRef>(f.concat(opts::Value(false)).data);
        const auto symbolValue=f.context.symbol(u"x");const auto symbol=std::get<opts::ObjectRef>(f.concat(opts::Value(symbolValue)).data);
        const auto numberBox=std::get<opts::ObjectRef>(f.context.get(number,Fixture::index(0)).data);
        const auto booleanBox=std::get<opts::ObjectRef>(f.context.get(boolean,Fixture::index(0)).data);
        const auto symbolBox=std::get<opts::ObjectRef>(f.context.get(symbol,Fixture::index(0)).data);
        expect(f.context.prototype(numberBox)==f.context.intrinsics().numberPrototype);
        expect(f.context.prototype(booleanBox)==f.context.intrinsics().booleanPrototype);
        expect(f.context.prototype(symbolBox)==f.context.intrinsics().symbolPrototype);
        const auto* numberValue=std::any_cast<opts::Value>(&f.context.payload(numberBox));
        const auto* booleanValue=std::any_cast<opts::Value>(&f.context.payload(booleanBox));
        const auto* symbolValueBox=std::any_cast<opts::Value>(&f.context.payload(symbolBox));
        expect(numberValue&&*numberValue==opts::Value(42));expect(booleanValue&&*booleanValue==opts::Value(false));
        expect(symbolValueBox&&*symbolValueBox==opts::Value(symbolValue));
    };
    "array_concat_primitive_arguments_ignore_concat_spreadable_prototype"_test=[] {
        Fixture f;const auto prototype=f.context.intrinsics().numberPrototype;
        f.context.set(prototype,opts::Key(f.context.intrinsics().symbolIsConcatSpreadable),true);
        const std::array<opts::Value,1> args{opts::Value(5)};auto result=std::get<opts::ObjectRef>(f.concat(f.array,args).data);
        expect(eq(f.context.arrayLength(result),1u));expect(f.context.get(result,Fixture::index(0))==opts::Value(5));
    };
    "array_concat_null_receiver_throws_type_error"_test=[] {
        Fixture f;std::string error;try{(void)f.concat(opts::Null{});}catch(const opts::Exception& caught){error=caught.name;}
        expect(eq(error,std::string("TypeError")));
    };
    "array_concat_spreadability_getter_error_propagates"_test=[] {
        Fixture f;auto source=f.context.array();auto getter=f.context.function(u"spread",[&](auto&,auto,auto)->opts::Value{f.trace.push_back("spreadable");throw opts::Exception("Error",u"spread");});
        f.context.define(source,opts::Key(f.context.intrinsics().symbolIsConcatSpreadable),opts::Descriptor::access(getter,{},true,true));
        const std::array<opts::Value,1> args{opts::Value(source)};std::string error;try{(void)f.concat(f.array,args);}catch(const opts::Exception& caught){error=caught.name;}
        expect(eq(error,std::string("Error")));expect(static_cast<bool>(f.trace==std::vector<std::string>{"spreadable"}));
    };
    "array_concat_defines_own_data_without_inherited_setter"_test=[] {
        Fixture f;auto source=f.arrayOf({7});int setterCalls=0;auto setter=f.context.function(u"set0",[&](auto&,auto,auto)->opts::Value{++setterCalls;return opts::Undefined{};});
        f.context.define(f.context.intrinsics().arrayPrototype,Fixture::index(0),opts::Descriptor::access({},setter,true,true));
        auto result=std::get<opts::ObjectRef>(f.concat(f.context.array(),std::array<opts::Value,1>{opts::Value(source)}).data);
        const auto descriptor=f.context.descriptor(result,Fixture::index(0));
        expect(f.context.hasOwn(result,Fixture::index(0)));expect(descriptor&&!descriptor->accessor&&descriptor->writable&&descriptor->enumerable&&descriptor->configurable);
        expect(eq(setterCalls,0));
    };
    "array_concat_method_lookup_remains_dynamic"_test=[] {
        Fixture f;auto override=f.context.function(u"override",[](auto&,auto,auto)->opts::Value{return 29;});
        f.context.set(f.array,opts::Key(u"concat"),override);const auto method=f.context.get(f.array,opts::Key(u"concat"));
        expect(f.context.call(std::get<opts::ObjectRef>(method.data),f.array)==opts::Value(29));
    };
    "array_concat_custom_species_has_explicit_native_boundary"_test=[] {
        Fixture f;auto ctor=f.context.constructor(u"C");auto custom=f.context.constructor(u"Species");
        f.context.define(ctor,opts::Key(f.context.intrinsics().symbolSpecies),opts::Descriptor::data(opts::Value(custom)));
        f.context.set(f.array,opts::Key(u"constructor"),opts::Value(ctor));std::string error;
        try{(void)f.concat(f.array);}catch(const opts::Exception& caught){error=caught.name;}
        expect(eq(error,std::string("Unimplemented")));
    };
    "array_concat_object_length_to_primitive_has_explicit_boundary"_test=[] {
        Fixture f;auto source=f.context.object();f.context.set(source,opts::Key(f.context.intrinsics().symbolIsConcatSpreadable),true);
        auto lengthObject=f.context.object();auto lengthGetter=f.context.function(u"length",[&](auto&,auto,auto)->opts::Value{f.trace.push_back("length");return lengthObject;});
        f.context.define(source,opts::Key(u"length"),opts::Descriptor::access(lengthGetter,{},true,true));std::string error;
        try{(void)f.concat(f.array,std::array<opts::Value,1>{opts::Value(source)});}catch(const opts::Exception& caught){error=caught.name;}
        expect(eq(error,std::string("")));expect(static_cast<bool>(f.trace==std::vector<std::string>{"length"}));
    };
};
}
