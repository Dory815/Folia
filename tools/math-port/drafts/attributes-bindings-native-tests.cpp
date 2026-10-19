// Draft focused tests for the canonical Source Attributes surface.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.attributes_bindings;
import folia.math.util.option_functions;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::options;

struct Fixture {
    std::shared_ptr<std::atomic_bool> lease=std::make_shared<std::atomic_bool>(true);
    Context context;
    OptionFunctionBindings functions{context};
    ContextPropertyBindings properties{context,functions};
    AttributesContextBindings attributes{context,functions,properties,lease};
};

suite attributes_bindings = [] {
"attributes_binding_preserves_the_live_four_layer_owner"_test = [] {
    Fixture f;
    PropertyList defaults{{"mathcolor",Property("blue")},{"mathsize",Property(12)}};
    PropertyList globals{{"mathcolor",Property("red")},{"dir",Property("ltr")}};
    auto owner=std::make_shared<Attributes>(defaults,globals);
    const auto object=f.attributes.bind(owner);
    expect(f.attributes.bind(owner)==object);
    expect(f.attributes.resolve(object)==owner);
    const auto ownKeys=f.context.ownKeys(Value(object));
    expect(ownKeys.size()==4);
    expect(std::get<std::u16string>(ownKeys[0].data)==u"global");
    expect(std::get<std::u16string>(ownKeys[1].data)==u"defaults");
    expect(std::get<std::u16string>(ownKeys[2].data)==u"inherited");
    expect(std::get<std::u16string>(ownKeys[3].data)==u"attributes");
    const auto all=f.context.get(Value(object),Key(u"attributes"));
    const auto listed=f.context.callMember(f.context.get(Value(f.attributes.prototype()),Key(u"getAllAttributes")),
        Value(object));
    expect(all==listed);
    const auto defaultsObject=std::get<ObjectRef>(f.context.get(Value(object),Key(u"defaults")).data);
    const auto globalObject=std::get<ObjectRef>(f.context.get(Value(object),Key(u"global")).data);
    expect(f.context.prototype(defaultsObject)==globalObject);
    const auto inheritedObject=std::get<ObjectRef>(f.context.get(Value(object),Key(u"inherited")).data);
    expect(f.context.prototype(inheritedObject)==defaultsObject);
    const auto explicitObject=std::get<ObjectRef>(all.data);
    expect(f.context.prototype(explicitObject)==inheritedObject);
    expect(property_host::string(owner->get("mathcolor"))==u"blue");
};

"attributes_binding_methods_dispatch_to_the_actual_owner"_test = [] {
    Fixture f;
    PropertyList defaults{{"mathcolor",Property("blue")}};
    PropertyList globals{{"mathcolor",Property("red")}};
    auto owner=std::make_shared<Attributes>(defaults,globals);
    const auto object=f.attributes.bind(owner);
    const auto set=f.context.get(Value(f.attributes.prototype()),Key(u"set"));
    const std::array<Value,2> args{Value(u"actiontype"),Value(u"toggle")};
    f.context.callMember(set,Value(object),args);
    expect(owner->hasExplicit("actiontype"));
    expect(owner->get("actiontype")==Property("toggle"));
    const auto names=f.context.callMember(f.context.get(Value(f.attributes.prototype()),Key(u"getExplicitNames")),Value(object));
    expect(f.context.arrayLength(std::get<ObjectRef>(names.data))==1);
    const auto get=f.context.get(Value(f.attributes.prototype()),Key(u"get"));
    const std::array<Value,1> getArgs{Value(u"mathcolor")};
    expect(f.context.callMember(get,Value(object),getArgs)==Value(u"blue"));
};

"attributes_binding_source_constructor_keeps_caller_layer_identities"_test = [] {
    Fixture f;
    const auto defaults=f.context.object();
    const auto globals=f.context.object();
    f.context.set(defaults,Key(u"mathsize"),Value(12));
    f.context.set(globals,Key(u"mathcolor"),Value(u"red"));
    const std::array<Value,2> args{Value(defaults),Value(globals)};
    const auto object=f.context.construct(f.attributes.constructor(),args);
    const auto actualDefaults=std::get<ObjectRef>(f.context.get(Value(object),Key(u"defaults")).data);
    const auto actualGlobals=std::get<ObjectRef>(f.context.get(Value(object),Key(u"global")).data);
    expect(actualDefaults!=defaults);
    expect(actualGlobals==globals);
    expect(f.context.prototype(actualDefaults)==globals);
    const auto get=f.context.get(Value(f.attributes.prototype()),Key(u"get"));
    const std::array<Value,1> color{Value(u"mathcolor")};
    const std::array<Value,1> size{Value(u"mathsize")};
    expect(f.context.callMember(get,Value(object),color)==Value(u"red"));
    expect(f.context.callMember(get,Value(object),size)==Value(12));
};

"attributes_binding_constructor_matches_null_global_and_objectAssign_defaults"_test = [] {
    Fixture f;
    const std::array<Value,2> args{Value(Undefined{}),Value(Null{})};
    const auto object=f.context.construct(f.attributes.constructor(),args);
    expect(f.context.get(Value(object),Key(u"global"))==Value(Null{}));
    const auto defaults=std::get<ObjectRef>(f.context.get(Value(object),Key(u"defaults")).data);
    expect(!f.context.prototype(defaults).has_value());
    const auto globalNames=f.context.get(Value(f.attributes.prototype()),Key(u"getGlobalNames"));
    bool threw=false;
    try{(void)f.context.callMember(globalNames,Value(object));}
    catch(const Exception& error){threw=error.name=="TypeError";}
    expect(threw);

    const auto globals=f.context.object();
    const std::array<Value,2> stringDefaults{Value(u"ab"),Value(globals)};
    const auto stringObject=f.context.construct(f.attributes.constructor(),stringDefaults);
    const auto get=f.context.get(Value(f.attributes.prototype()),Key(u"getDefault"));
    const std::array<Value,1> firstIndex{Value(u"0")};
    expect(f.context.callMember(get,Value(stringObject),firstIndex)==Value(u"a"));
};

"attributes_binding_replacement_changes_only_the_assigned_source_layer"_test = [] {
    Fixture f;
    auto owner=std::make_shared<Attributes>(PropertyList{},PropertyList{{"x",Property(1)}});
    const auto object=f.attributes.bind(owner);
    const auto oldDefaults=std::get<ObjectRef>(f.context.get(Value(object),Key(u"defaults")).data);
    const auto newDefaults=f.context.object();
    f.context.set(newDefaults,Key(u"y"),Value(2));
    f.context.set(object,Key(u"defaults"),Value(newDefaults));
    expect(std::get<ObjectRef>(f.context.get(Value(object),Key(u"defaults")).data)==newDefaults);
    expect(owner->getDefault("y")==Property(2));
    expect(f.context.prototype(std::get<ObjectRef>(f.context.get(Value(object),Key(u"inherited")).data))==oldDefaults);
};

"attributes_binding_setList_keeps_source_getter_order_and_partial_writes"_test = [] {
    Fixture f;
    auto owner=std::make_shared<Attributes>(PropertyList{},PropertyList{});
    const auto object=f.attributes.bind(owner);
    const auto source=f.context.object();
    f.context.set(source,Key(u"first"),Value(1));
    bool laterGetterSawEarlierWrite=false;
    const auto getter=f.context.function(u"get later",[&](Context& context,Value,std::span<const Value>){
        laterGetterSawEarlierWrite=owner->get("first")==Property(1);
        throw Exception("TypeError",u"later getter failed");
        return Value(Undefined{});
    });
    f.context.define(source,Key(u"later"),Descriptor::access(getter,{}));
    bool threw=false;
    try{
        const std::array<Value,1> args{Value(source)};
        f.context.callMember(f.context.get(Value(f.attributes.prototype()),Key(u"setList")),
            Value(object),args);
    }catch(const Exception& error){threw=error.name=="TypeError";}
    expect(threw);
    expect(laterGetterSawEarlierWrite);
    expect(owner->get("first")==Property(1));
    expect(!owner->hasExplicit("later"));
};

"attributes_binding_invalid_method_receiver_uses_context_type_error"_test = [] {
    Fixture f;
    bool threw=false;
    try{
        const std::array<Value,1> args{Value(u"x")};
        f.context.callMember(f.context.get(Value(f.attributes.prototype()),Key(u"get")),
            Value(1),args);
    }catch(const Exception& error){threw=error.name=="TypeError";}
    expect(threw);
};

"attributes_binding_callbacks_expire_with_the_parser_execution_lease"_test = [] {
    Fixture f;
    auto owner=std::make_shared<Attributes>(PropertyList{},PropertyList{});
    const auto object=f.attributes.bind(owner);
    f.lease->store(false);
    bool threw=false;
    try{(void)f.context.get(Value(object),Key(u"attributes"));}
    catch(const Exception& error){threw=error.name=="TypeError";}
    expect(threw);
};
};
}
