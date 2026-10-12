// Candidate native observations for parser-array-host-source.json.
// Kept outside tests/ auto-globs until Root adds this packet to a build.
#include "../../tests/support/folia_test.hpp"
import folia.math.util.options;
import folia.math.util.js_number;
import folia.math.util.property_host;
namespace {
using namespace boost::ut;
namespace opts=folia::math::options;

struct ArrayFixture {
    opts::Context context;
    opts::ObjectRef array = context.array();
    std::vector<std::string> trace;

    opts::Key key(std::uint64_t index) const {
        const auto ascii=std::to_string(index);
        return opts::Key(std::u16string(ascii.begin(),ascii.end()));
    }
    void set(std::uint64_t index, opts::Value value) { context.set(array,key(index),std::move(value)); }
    std::string text(const opts::Value& value) const {
        if (std::holds_alternative<opts::Undefined>(value.data)) return "U";
        if (std::holds_alternative<opts::Null>(value.data)) return "N";
        if (const auto* b=std::get_if<bool>(&value.data)) return *b?"true":"false";
        if (const auto* n=std::get_if<double>(&value.data)) return folia::math::js_number::toString(*n);
        if (const auto* s=std::get_if<std::u16string>(&value.data)) return folia::math::property_host::debugText(*s);
        return "Object";
    }
    std::string yes(bool value) const { return value?"true":"false"; }
    std::string summary(opts::Value value) {
        const auto* ref=std::get_if<opts::ObjectRef>(&value.data);
        if (!ref || context.kind(*ref)!=opts::ObjectKind::array) return text(value);
        std::string result="len:"+std::to_string(context.arrayLength(*ref));
        for (std::uint64_t i=0;i<context.arrayLength(*ref);++i) {
            result+=";"+std::to_string(i)+":";
            const auto descriptor=context.descriptor(*ref,key(i));
            if (!descriptor) result+="H";
            else if (descriptor->accessor) result+="A";
            else result+="O:"+text(descriptor->value);
        }
        return result;
    }
    opts::Value method(std::u16string name,std::span<const opts::Value> args={}) {
        const opts::Value value=context.get(opts::Value(array),opts::Key(std::move(name)));
        const auto* function=std::get_if<opts::ObjectRef>(&value.data);
        if (!function || context.kind(*function)!=opts::ObjectKind::function)
            throw opts::Exception("TypeError",u"Array method is not callable");
        return context.call(*function,opts::Value(array),args);
    }
    opts::ObjectRef getter(opts::ObjectRef owner,std::u16string key,
                           std::function<opts::Value(opts::Context&,opts::Value)> body) {
        auto fn=context.function(u"get",[body](auto& ctx,auto receiver,auto)->opts::Value {
            return body(ctx,receiver);
        });
        context.define(owner,opts::Key(std::move(key)),opts::Descriptor::access(fn,{},true,true));
        return fn;
    }
    std::string traceText() const {
        std::string result;
        for (const auto& part:trace) { if(!result.empty()) result+="/"; result+=part; }
        return result;
    }
    std::string guarded(std::function<std::string()> body) {
        try {
            const auto value=body();
            const auto trace=traceText();
            return "ok|"+value+"|"+trace;
        } catch(const opts::Exception& error) {
            const auto trace=traceText();
            return error.name+"|THREW|"+trace;
        }
    }
    std::string metadata(std::u16string name) {
        const opts::Value methodValue=context.get(opts::Value(array),opts::Key(name));
        const auto* method=std::get_if<opts::ObjectRef>(&methodValue.data);
        if(!method)return "missing";
        const auto descriptor=context.descriptor(context.intrinsics().arrayPrototype,opts::Key(name));
        if(!descriptor)return "missing";
        return text(context.get(opts::Value(*method),opts::Key(u"length")))+":"+
            yes(descriptor->enumerable)+":"+yes(descriptor->writable)+":"+yes(descriptor->configurable);
    }
};

suite array_host=[] {
"source_rows_0_to_7_method_metadata"_test=[] {
    ArrayFixture e;
    const std::array<std::pair<std::u16string,std::string>,8> expected{{
        {u"slice","2:false:true:true"},{u"reverse","0:false:true:true"},
        {u"includes","1:false:true:true"},{u"push","1:false:true:true"},
        {u"pop","0:false:true:true"},{u"join","1:false:true:true"},
        {u"fill","1:false:true:true"},{u"find","1:false:true:true"}
    }};
    std::string actual;
    for(const auto& [name,value]:expected){(void)value;if(!actual.empty())actual+="/";actual+=e.metadata(name);}
    expect(eq(actual,std::string("2:false:true:true/0:false:true:true/1:false:true:true/1:false:true:true/0:false:true:true/1:false:true:true/1:false:true:true/1:false:true:true")));
};
"slice_holes_and_length"_test=[] {
    ArrayFixture e;
    e.context.set(e.array,u"length",4); e.set(1,u"x");
    const std::array<opts::Value,1> args{0};
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice",args));}),
              std::string("ok|len:4;0:H;1:O:x;2:H;3:H|")));
};
"slice_species_nullish_defaults_and_order"_test=[] {
    ArrayFixture e; e.set(0,5);
    auto ctor=e.context.function(u"C");
    e.context.set(e.array,u"constructor",ctor);
    auto species_get=e.context.function(u"species-get",[&](auto&,auto receiver,auto)->opts::Value {
        e.trace.push_back("species:"+e.yes(receiver==opts::Value(ctor))); return opts::Value(opts::Null{});
    });
    e.context.define(ctor,opts::Key(e.context.intrinsics().symbolSpecies),
                     opts::Descriptor::access(species_get,{},true,true));
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice"));}),
              std::string("ok|len:1;0:O:5|species:true")));
};
"slice_custom_species_is_explicit_boundary"_test=[] {
    ArrayFixture e; e.set(0,1);
    auto ctor=e.context.function(u"C");
    auto species=e.context.constructor(u"Species");
    e.context.set(ctor,opts::Key(e.context.intrinsics().symbolSpecies),species);
    e.context.set(e.array,u"constructor",ctor);
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice"));}),
              std::string("Unimplemented|THREW|")));
};
"slice_primitive_constructor_throws"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"constructor",opts::Null{});
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice"));}),
              std::string("TypeError|THREW|")));
};
"slice_false_and_zero_constructors_throw"_test=[] {
    ArrayFixture e;
    e.context.set(e.array,u"constructor",false);
    const auto booleanResult=e.guarded([&]{return e.summary(e.method(u"slice"));});
    e.context.set(e.array,u"constructor",0);
    const auto numberResult=e.guarded([&]{return e.summary(e.method(u"slice"));});
    expect(eq(booleanResult+":"+numberResult,std::string("TypeError|THREW|:TypeError|THREW|")));
};
"slice_constructor_undefined_defaults"_test=[] {
    ArrayFixture e; e.set(0,1); e.context.set(e.array,u"constructor",opts::Undefined{});
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice"));}),
              std::string("ok|len:1;0:O:1|")));
};
"slice_object_constructor_nullish_species_defaults"_test=[] {
    ArrayFixture e; e.set(0,1);
    auto ctor=e.context.object(); e.context.set(ctor,opts::Key(e.context.intrinsics().symbolSpecies),opts::Null{});
    e.context.set(e.array,u"constructor",ctor);
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice"));}),
              std::string("ok|len:1;0:O:1|")));
};
"slice_function_constructor_missing_species_defaults"_test=[] {
    ArrayFixture e; e.set(0,1); e.context.set(e.array,u"constructor",e.context.function(u"C"));
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice"));}),
              std::string("ok|len:1;0:O:1|")));
};
"slice_constructor_get_precedes_index_get"_test=[] {
    ArrayFixture e; e.set(0,1);
    auto constructor_get=e.context.function(u"constructor-get",[&](auto&,auto,auto)->opts::Value {
        e.trace.push_back("constructor"); return opts::Value(e.context.intrinsics().arrayConstructor);
    });
    e.context.define(e.array,u"constructor",opts::Descriptor::access(constructor_get,{},true,true));
    auto index_get=e.context.function(u"index-get",[&](auto&,auto,auto)->opts::Value {
        e.trace.push_back("index"); return 1;
    });
    e.context.define(e.array,u"0",opts::Descriptor::access(index_get,{},true,true));
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice"));}),
              std::string("ok|len:1;0:O:1|constructor/index")));
};
"slice_numeric_bounds_constructor_species_index_order"_test=[] {
    ArrayFixture e; e.set(0,1);
    auto ctor=e.context.function(u"C");
    auto constructor_get=e.context.function(u"constructor-get",[&](auto&,auto,auto)->opts::Value {
        e.trace.push_back("constructor"); return opts::Value(ctor);
    });
    e.context.define(e.array,u"constructor",opts::Descriptor::access(constructor_get,{},true,true));
    auto species_get=e.context.function(u"species-get",[&](auto&,opts::Value receiver,auto)->opts::Value {
        e.trace.push_back("species:"+e.yes(receiver==opts::Value(ctor))); return opts::Value(opts::Null{});
    });
    e.context.define(ctor,opts::Key(e.context.intrinsics().symbolSpecies),opts::Descriptor::access(species_get,{},true,true));
    auto index_get=e.context.function(u"index-get",[&](auto&,auto,auto)->opts::Value {
        e.trace.push_back("get0"); return 1;
    });
    e.context.define(e.array,u"0",opts::Descriptor::access(index_get,{},true,true));
    const std::array<opts::Value,2> args{0,1};
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice",args));}),
              std::string("ok|len:1;0:O:1|constructor/species:true/get0")));
};
"slice_negative_and_clamped_bounds"_test=[] {
    ArrayFixture e; e.set(0,0);e.set(1,1);e.set(2,2);e.set(3,3);
    const std::array<opts::Value,2> args{-9,8};
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice",args));}),
              std::string("ok|len:4;0:O:0;1:O:1;2:O:2;3:O:3|")));
};
"slice_object_bound_is_explicit_conversion_boundary"_test=[] {
    ArrayFixture e; auto object=e.context.object(); const std::array<opts::Value,1> args{object};
    expect(eq(e.guarded([&]{return e.summary(e.method(u"slice",args));}),
              std::string("TypeError|THREW|")));
};
"slice_result_owns_data_and_bypasses_prototype_setter"_test=[] {
    ArrayFixture e; e.set(0,4);
    auto setter=e.context.function(u"set",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("setter");return {};});
    e.context.define(e.context.intrinsics().arrayPrototype,u"0",opts::Descriptor::access({},setter,true,true));
    auto result=e.method(u"slice");
    const auto* ref=std::get_if<opts::ObjectRef>(&result.data);
    const auto descriptor=e.context.descriptor(*ref,u"0");
    expect(eq(e.yes(descriptor&& !descriptor->accessor && descriptor->writable &&
                    descriptor->enumerable && descriptor->configurable)+":"+e.traceText(),
              std::string("true:")));
};
"reverse_live_pair_order_and_holes"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",2);
    e.set(0,1); e.set(1,2);
    auto low=e.context.function(u"get-low",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("get0");return 1;});
    auto high=e.context.function(u"get-high",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("get1");return 2;});
    auto setlow=e.context.function(u"set-low",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("set0");return {};});
    auto sethigh=e.context.function(u"set-high",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("set1");return {};});
    e.context.define(e.array,u"0",opts::Descriptor::access(low,setlow,true,true));
    e.context.define(e.array,u"1",opts::Descriptor::access(high,sethigh,true,true));
    expect(eq(e.guarded([&]{return e.summary(e.method(u"reverse"));}),
              std::string("ok|len:2;0:A;1:A|get0/get1/set0/set1")));
};
"reverse_two_sided_holes"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",4);e.set(1,u"b");
    expect(eq(e.guarded([&]{return e.summary(e.method(u"reverse"));}),
              std::string("ok|len:4;0:H;1:H;2:O:b;3:H|")));
};
"reverse_lower_get_mutates_upper_before_test"_test=[] {
    ArrayFixture e; e.set(0,1); e.set(1,2);
    auto getter=e.context.function(u"get",[&](auto&,auto,auto)->opts::Value {
        e.trace.push_back("get0"); e.context.erase(e.array,u"1"); return 1;
    });
    e.context.define(e.array,u"0",opts::Descriptor::access(getter,{},true,true));
    expect(eq(e.guarded([&]{return e.summary(e.method(u"reverse"));}),
              std::string("ok|len:2;0:H;1:O:1|get0")));
};
"reverse_moves_one_sided_slots"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",2); e.set(1,8);
    expect(eq(e.guarded([&]{return e.summary(e.method(u"reverse"));}),
              std::string("ok|len:2;0:O:8;1:H|")));
};
"reverse_moves_lower_only_slot"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",2); e.set(0,u"a");
    expect(eq(e.guarded([&]{return e.summary(e.method(u"reverse"));}),
              std::string("ok|len:2;0:H;1:O:a|")));
};
"reverse_lower_set_throw_stops_before_upper_set"_test=[] {
    ArrayFixture e; e.set(0,1);e.set(1,2);
    auto setter=e.context.function(u"set",[&](auto&,auto,auto)->opts::Value {
        e.trace.push_back("set0");throw opts::Exception("Error",u"setter");
    });
    e.context.define(e.array,u"0",opts::Descriptor::access({},setter,true,true));
    expect(eq(e.guarded([&]{return e.summary(e.method(u"reverse"));}),
              std::string("Error|THREW|set0")));
};
"includes_hole_samevaluezero_and_index_conversion"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",2); e.set(1,std::numeric_limits<double>::quiet_NaN());
    const std::array<opts::Value,2> args{opts::Value(std::numeric_limits<double>::quiet_NaN()),opts::Value(-1.5)};
    expect(eq(e.guarded([&]{return e.text(e.method(u"includes",args));}),std::string("ok|true|")));
};
"includes_hole_matches_undefined"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",2);
    const std::array<opts::Value,1> args{opts::Undefined{}};
    expect(eq(e.guarded([&]{return e.text(e.method(u"includes",args));}),std::string("ok|true|")));
};
"includes_zero_length_skips_symbol_conversion"_test=[] {
    ArrayFixture e;
    const std::array<opts::Value,2> args{u"missing",e.context.symbol(u"offset")};
    expect(eq(e.guarded([&]{return e.text(e.method(u"includes",args));}),std::string("ok|false|")));
};
"includes_nan_uses_samevaluezero"_test=[] {
    ArrayFixture e; e.set(0,std::numeric_limits<double>::quiet_NaN());
    const std::array<opts::Value,1> args{std::numeric_limits<double>::quiet_NaN()};
    expect(eq(e.guarded([&]{return e.text(e.method(u"includes",args));}),std::string("ok|true|")));
};
"includes_infinite_and_nan_offsets"_test=[] {
    ArrayFixture e; e.set(0,1);e.set(1,2);e.set(2,3);
    auto call=[&](opts::Value offset) {const std::array<opts::Value,2> args{1,offset};return e.text(e.method(u"includes",args));};
    expect(eq(call(std::numeric_limits<double>::infinity())+":"+
                 call(-std::numeric_limits<double>::infinity())+":"+
                 call(std::numeric_limits<double>::quiet_NaN()),
              std::string("false:true:true")));
};
"includes_fractional_negative_offset_truncates_toward_zero"_test=[] {
    ArrayFixture e; e.set(0,1);e.set(1,2);e.set(2,3);
    const std::array<opts::Value,2> args{1,-1.5};
    expect(eq(e.guarded([&]{return e.text(e.method(u"includes",args));}),
              std::string("ok|false|")));
};
"includes_object_offset_is_explicit_conversion_boundary"_test=[] {
    ArrayFixture e; e.set(0,2);auto object=e.context.object();
    const std::array<opts::Value,2> args{2,object};
    expect(eq(e.guarded([&]{return e.text(e.method(u"includes",args));}),
              std::string("TypeError|THREW|")));
};
"includes_from_index_string_to_number"_test=[] {
    ArrayFixture e; e.set(0,0); e.set(1,1); e.set(2,2);
    const std::array<opts::Value,2> args{0,u"1"};
    expect(eq(e.guarded([&]{return e.text(e.method(u"includes",args));}),std::string("ok|false|")));
};
"includes_reads_inherited_hole"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",1);
    auto proto=e.context.objectWithPrototype(e.context.intrinsics().arrayPrototype);
    e.context.setPrototype(e.array,proto);
    auto getter=e.context.function(u"get",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("get0");return 7;});
    e.context.define(proto,u"0",opts::Descriptor::access(getter,{},true,true));
    const std::array<opts::Value,1> args{7};
    expect(eq(e.guarded([&]{return e.text(e.method(u"includes",args));}),std::string("ok|true|get0")));
};
"push_variadic_identity_and_metadata"_test=[] {
    ArrayFixture e; e.set(0,1); auto identity=e.context.object();
    const std::array<opts::Value,2> args{2,identity};
    auto result=e.method(u"push",args);
    expect(eq(e.text(result)+":"+e.yes(e.context.get(e.array,u"2")==opts::Value(identity)),
              std::string("3:true")));
};
"push_empty_on_nonwritable_length_throws"_test=[] {
    ArrayFixture e;
    e.context.define(e.array,u"length",opts::Descriptor::data(0,false,false,false));
    expect(eq(e.guarded([&]{return e.text(e.method(u"push"));}),std::string("TypeError|THREW|")));
};
"push_partial_write_before_inherited_setter_throw"_test=[] {
    ArrayFixture e;
    auto proto=e.context.objectWithPrototype(e.context.intrinsics().arrayPrototype);
    auto setter=e.context.function(u"set",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("set1");throw opts::Exception("Error",u"setter");});
    e.context.define(proto,u"1",opts::Descriptor::access({},setter,true,true));
    e.context.setPrototype(e.array,proto);
    const std::array<opts::Value,3> args{3,4,5};
    expect(eq(e.guarded([&]{return e.text(e.method(u"push",args));}),
              std::string("Error|THREW|set1")));
    expect(eq(e.text(e.context.get(e.array,u"0")),std::string("3")));
};
"push_max_length_writes_tail_then_range_errors"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",4294967295.0);
    const std::array<opts::Value,1> args{7};
    expect(eq(e.guarded([&]{return e.text(e.method(u"push",args));}),
              std::string("RangeError|THREW|")));
    expect(eq(e.text(e.context.get(e.array,u"4294967295")),std::string("7")));
};
"pop_get_delete_then_length"_test=[] {
    ArrayFixture e; e.set(0,4);
    auto getter=e.context.function(u"get",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("get0");return 4;});
    e.context.define(e.array,u"0",opts::Descriptor::access(getter,{},true,true));
    expect(eq(e.guarded([&]{return e.text(e.method(u"pop"));}),std::string("ok|4|get0")));
    expect(eq(std::to_string(e.context.arrayLength(e.array)),std::string("0")));
};
"pop_nonconfigurable_delete_aborts_length_write"_test=[] {
    ArrayFixture e; e.set(0,4);
    e.context.define(e.array,u"0",opts::Descriptor::data(4,true,true,false));
    expect(eq(e.guarded([&]{return e.text(e.method(u"pop"));}),std::string("TypeError|THREW|")));
    expect(eq(std::to_string(e.context.arrayLength(e.array)),std::string("1")));
};
"pop_deletes_tail_before_readonly_length_failure"_test=[] {
    ArrayFixture e; e.set(0,4);
    e.context.define(e.array,u"0",opts::Descriptor::data(4,true,true,true));
    e.context.define(e.array,u"length",opts::Descriptor::data(1,false,false,false));
    std::string error="ok";
    try { (void)e.method(u"pop"); }
    catch(const opts::Exception& caught) { error=caught.name; }
    expect(eq(error+":"+std::to_string(e.context.arrayLength(e.array))+":"+
              e.yes(e.context.hasOwn(e.array,opts::Key(u"0"))),
              std::string("TypeError:1:false")));
};
"pop_hole_gets_inherited_value"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",1);
    auto proto=e.context.objectWithPrototype(e.context.intrinsics().arrayPrototype);
    e.context.setPrototype(e.array,proto); e.context.set(proto,u"0",9);
    expect(eq(e.guarded([&]{return e.text(e.method(u"pop"));}),std::string("ok|9|")));
    expect(eq(std::to_string(e.context.arrayLength(e.array)),std::string("0")));
};
"pop_zero_length_nonwritable_length_throws"_test=[] {
    ArrayFixture e; e.context.define(e.array,u"length",opts::Descriptor::data(0,false,false,false));
    expect(eq(e.guarded([&]{
        try { (void)e.method(u"pop"); }
        catch(const opts::Exception& error) { e.trace.push_back(error.name); }
        return e.text(e.context.get(e.array,u"length"));
    }),std::string("ok|0|TypeError")));
};
"join_nullish_separator_and_primitives"_test=[] {
    ArrayFixture e; e.set(0,opts::Null{}); e.set(1,opts::Undefined{}); e.set(2,7); e.set(3,false);
    expect(eq(e.guarded([&]{return e.text(e.method(u"join"));}),std::string("ok|,,7,false|")));
};
"join_hole_gets_and_captured_length"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",3); e.set(1,1);
    auto proto=e.context.objectWithPrototype(e.context.intrinsics().arrayPrototype);
    e.context.setPrototype(e.array,proto);
    auto get0=e.context.function(u"get0",[&](auto&,auto,auto)->opts::Value {
        e.trace.push_back("get0"); return u"p";
    });
    e.context.define(proto,u"0",opts::Descriptor::access(get0,{},true,true));
    auto get1=e.context.function(u"get1",[&](auto& context,auto,auto)->opts::Value {
        e.trace.push_back("get1"); context.set(e.array,u"3",2); return 1;
    });
    e.context.define(e.array,u"1",opts::Descriptor::access(get1,{},true,true));
    const std::array<opts::Value,1> args{u"|"};
    expect(eq(e.guarded([&]{return e.text(e.method(u"join",args));}),std::string("ok|p|1||get0/get1")));
};
"join_symbol_throws_and_object_conversion_boundary"_test=[] {
    ArrayFixture e; e.set(0,e.context.symbol(u"x"));
    expect(eq(e.guarded([&]{return e.text(e.method(u"join"));}),
              std::string("TypeError|THREW|")));
};
"join_symbol_separator_throws"_test=[] {
    ArrayFixture e; e.set(0,1);
    const std::array<opts::Value,1> args{e.context.symbol(u"x")};
    expect(eq(e.guarded([&]{return e.text(e.method(u"join",args));}),
              std::string("TypeError|THREW|")));
};
"join_object_separator_conversion_is_explicit_boundary"_test=[] {
    ArrayFixture e; e.set(0,opts::Null{});e.set(1,opts::Undefined{});
    const std::array<opts::Value,1> args{e.context.object()};
    expect(eq(e.guarded([&]{return e.text(e.method(u"join",args));}),
              std::string("Unimplemented|THREW|")));
};
"join_object_to_primitive_is_explicit_boundary"_test=[] {
    ArrayFixture e; e.set(0,e.context.object());
    expect(eq(e.guarded([&]{return e.text(e.method(u"join"));}),
              std::string("Unimplemented|THREW|")));
};
"fill_converts_bounds_then_writes"_test=[] {
    ArrayFixture e; e.set(0,0); e.set(1,0); e.set(2,0);
    const std::array<opts::Value,3> args{7,-1.5,opts::Undefined{}};
    expect(eq(e.guarded([&]{return e.summary(e.method(u"fill",args));}),
              std::string("ok|len:3;0:O:0;1:O:0;2:O:7|")));
};
"fill_partial_set_via_inherited_setter"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",3);
    auto proto=e.context.objectWithPrototype(e.context.intrinsics().arrayPrototype);
    auto setter=e.context.function(u"set",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("set1");throw opts::Exception("Error",u"setter");});
    e.context.define(proto,u"1",opts::Descriptor::access({},setter,true,true));
    e.context.setPrototype(e.array,proto);
    const std::array<opts::Value,1> args{7};
    expect(eq(e.guarded([&]{return e.summary(e.method(u"fill",args));}),
              std::string("Error|THREW|set1")));
    expect(eq(e.text(e.context.get(e.array,u"0")),std::string("7")));
};
"fill_partial_set_via_own_accessor"_test=[] {
    ArrayFixture e; e.set(0,0);e.set(1,0);e.set(2,0);
    auto setter=e.context.function(u"set",[&](auto&,auto,auto)->opts::Value {
        e.trace.push_back("set1");throw opts::Exception("Error",u"setter");
    });
    e.context.define(e.array,u"1",opts::Descriptor::access({},setter,true,true));
    const std::array<opts::Value,1> args{7};
    expect(eq(e.guarded([&]{return e.summary(e.method(u"fill",args));}),
              std::string("Error|THREW|set1")));
    expect(eq(e.text(e.context.get(e.array,u"0")),std::string("7")));
};
"fill_nan_bounds_cover_full_range"_test=[] {
    ArrayFixture e; e.set(0,0); e.set(1,0);
    const std::array<opts::Value,3> args{7,std::numeric_limits<double>::quiet_NaN(),opts::Undefined{}};
    expect(eq(e.guarded([&]{return e.summary(e.method(u"fill",args));}),
              std::string("ok|len:2;0:O:7;1:O:7|")));
};
"fill_fractional_and_infinite_bounds"_test=[] {
    ArrayFixture e; e.set(0,0);e.set(1,0);e.set(2,0);
    auto run=[&](opts::Value start,opts::Value end) {
        e.set(0,0);e.set(1,0);e.set(2,0);
        const std::array<opts::Value,3> args{7,start,end};
        return e.summary(e.method(u"fill",args));
    };
    expect(eq(run(-1.5,opts::Undefined{})+"|"+
                 run(std::numeric_limits<double>::infinity(),opts::Undefined{})+"|"+
                 run(-std::numeric_limits<double>::infinity(),2),
              std::string("len:3;0:O:0;1:O:0;2:O:7|len:3;0:O:0;1:O:0;2:O:0|len:3;0:O:7;1:O:7;2:O:0")));
};
"fill_object_bound_is_explicit_conversion_boundary"_test=[] {
    ArrayFixture e; auto object=e.context.object();
    const std::array<opts::Value,2> args{7,object};
    expect(eq(e.guarded([&]{return e.summary(e.method(u"fill",args));}),
              std::string("TypeError|THREW|")));
};
"find_reads_holes_and_passes_exact_callback_arguments"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",2); auto sentinel=e.context.object();
    auto callback=e.context.function(u"find",[&](auto&,auto receiver,auto args)->opts::Value {
        e.trace.push_back(std::to_string(args.size())+":"+e.yes(receiver==opts::Value(sentinel))+":"+e.text(args[0])+":"+e.text(args[1])+":"+e.yes(args[2]==opts::Value(e.array)));
        return opts::Value(true);
    });
    const std::array<opts::Value,2> args{callback,sentinel};
    expect(eq(e.guarded([&]{return e.text(e.method(u"find",args));}),
              std::string("ok|U|3:true:U:0:true")));
};
"find_reads_future_values_live_captured_length"_test=[] {
    ArrayFixture e; e.set(0,1); e.set(1,2);
    auto callback=e.context.function(u"find",[&](auto& context,auto,auto args)->opts::Value {
        const auto index=static_cast<int>(std::get<double>(args[1].data));
        e.trace.push_back(std::to_string(index)+":"+e.text(args[0]));
        if(index==0) context.set(e.array,u"1",9);
        return index==1;
    });
    const std::array<opts::Value,1> args{callback};
    expect(eq(e.guarded([&]{return e.text(e.method(u"find",args));}),
              std::string("ok|9|0:1/1:9")));
};
"find_shrink_still_gets_captured_slots"_test=[] {
    ArrayFixture e; e.set(0,1); e.set(1,2); e.set(2,3);
    auto callback=e.context.function(u"find",[&](auto& context,auto,auto args)->opts::Value {
        const auto index=static_cast<int>(std::get<double>(args[1].data));
        e.trace.push_back(std::to_string(index)+":"+e.text(args[0]));
        if(index==0)context.set(e.array,u"length",1);
        return false;
    });
    const std::array<opts::Value,1> args{callback};
    expect(eq(e.guarded([&]{return e.text(e.method(u"find",args));}),
              std::string("ok|U|0:1/1:U/2:U")));
};
"find_hole_callback_returns_undefined_value"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",2);e.set(1,5);
    auto callback=e.context.function(u"find",[&](auto&,auto receiver,auto args)->opts::Value {
        e.trace.push_back(std::to_string(args.size())+":"+e.text(args[0])+":"+
                          e.yes(args[2]==opts::Value(e.array))+":"+e.yes(receiver==opts::Value(opts::Undefined{})));
        return std::get<double>(args[1].data)==0;
    });
    const std::array<opts::Value,1> args{callback};
    expect(eq(e.guarded([&]{return e.text(e.method(u"find",args));}),
              std::string("ok|U|3:U:true:true")));
};
"find_inherited_hole_getter"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",1);
    auto proto=e.context.objectWithPrototype(e.context.intrinsics().arrayPrototype);e.context.setPrototype(e.array,proto);
    auto getter=e.context.function(u"get",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("get0");return 4;});
    e.context.define(proto,u"0",opts::Descriptor::access(getter,{},true,true));
    auto callback=e.context.function(u"find",[](auto&,auto,auto args)->opts::Value{return std::get<double>(args[0].data)==4;});
    const std::array<opts::Value,1> args{callback};
    expect(eq(e.guarded([&]{return e.text(e.method(u"find",args));}),std::string("ok|4|get0")));
};
"find_invalid_callback_before_get"_test=[] {
    ArrayFixture e; e.context.set(e.array,u"length",1);
    auto getter=e.context.function(u"get",[&](auto&,auto,auto)->opts::Value{e.trace.push_back("get");return 1;});
    e.context.define(e.array,u"0",opts::Descriptor::access(getter,{},true,true));
    const std::array<opts::Value,1> args{opts::Null{}};
    expect(eq(e.guarded([&]{return e.text(e.method(u"find",args));}),
              std::string("TypeError|THREW|")));
};
"current_method_override_is_called"_test=[] {
    ArrayFixture e;
    auto override=e.context.function(u"override",[&](auto&,auto receiver,auto args)->opts::Value {
        e.trace.push_back("override:"+e.yes(receiver==opts::Value(e.array))+":"+std::to_string(args.size()));
        return opts::Value(u"result");
    });
    e.context.set(e.array,u"find",override);
    const std::array<opts::Value,2> args{opts::Null{},7};
    expect(eq(e.guarded([&]{return e.text(e.method(u"find",args));}),
              std::string("ok|result|override:true:2")));
};
};
}
