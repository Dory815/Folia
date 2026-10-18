#include "support/folia_test.hpp"
import folia.math.util.native_data_fields;
import folia.math.util.options;

namespace {
using namespace boost::ut;
namespace o=folia::math::options;
struct Owner {o::Value value=7;};
suite native_data_fields=[] {
    "native_data_cell_reads_and_writes_the_same_actual_owner"_test=[] {
        o::Context context;
        auto owner=std::make_shared<Owner>();
        const auto object=context.object();
        o::bindNativeValue(context,object,o::Key(u"value"),owner,&Owner::value);
        owner->value=19;
        expect(context.get(object,o::Key(u"value"))==o::Value(19));
        context.set(object,o::Key(u"value"),o::Value(23));
        expect(owner->value==o::Value(23));
        const auto alias=context.object();
        context.set(alias,o::Key(u"same"),object);
        context.set(context.get(alias,o::Key(u"same")),o::Key(u"value"),o::Value(31));
        expect(owner->value==o::Value(31));
        const auto descriptor=context.descriptor(object,o::Key(u"value"));
        expect(descriptor&&!descriptor->accessor&&descriptor->writable&&descriptor->enumerable&&descriptor->configurable);
        expect(descriptor&&descriptor->value==o::Value(31));
        const auto reflection=context.descriptorObject(*descriptor);
        expect(context.hasOwn(reflection,o::Key(u"value"))&&!context.hasOwn(reflection,o::Key(u"get")));
    };
    "native_data_cell_metadata_queries_do_not_load_the_field"_test=[] {
        o::Context context;
        auto owner=std::make_shared<Owner>();
        const auto object=context.object();
        int reads=0;
        context.set(object,o::Key(u"first"),true);
        context.bindData(object,o::Key(u"value"),std::make_shared<o::NativeDataField>(owner,
            [&]{++reads;return owner->value;},[&](o::Value value){owner->value=std::move(value);}));
        context.set(object,o::Key(u"last"),true);
        expect(context.hasOwn(object,o::Key(u"value"))&&context.has(object,o::Key(u"value")));
        expect(static_cast<bool>(context.enumerableStringKeys(object)==std::vector<std::u16string>{u"first",u"value",u"last"}));
        expect(context.ownKeys(object).size()==3);
        expect(eq(reads,0));
        expect(context.get(object,o::Key(u"value"))==o::Value(7));
        expect(eq(reads,1));
    };
    "native_data_cell_redefinition_keeps_the_owner_as_value_authority"_test=[] {
        o::Context context;
        auto owner=std::make_shared<Owner>();
        const auto object=context.object();
        o::bindNativeValue(context,object,o::Key(u"value"),owner,&Owner::value);
        context.define(object,o::Key(u"value"),o::Descriptor::data(41,false,false,false));
        expect(owner->value==o::Value(41));
        bool rejected=false;
        try{context.set(object,o::Key(u"value"),43);}catch(const o::Exception& e){rejected=e.name=="TypeError";}
        expect(rejected&&owner->value==o::Value(41));
        context.define(object,o::Key(u"value"),o::Descriptor::data(41,false,false,false));
        expect(context.descriptor(object,o::Key(u"value"))->value==o::Value(41));
        rejected=false;
        try{context.define(object,o::Key(u"value"),o::Descriptor::data(47,false,false,false));}
        catch(const o::Exception& e){rejected=e.name=="TypeError";}
        expect(rejected&&owner->value==o::Value(41));
        rejected=false;
        try{context.erase(object,o::Key(u"value"));}catch(const o::Exception& e){rejected=e.name=="TypeError";}
        expect(rejected&&context.hasOwn(object,o::Key(u"value")));
    };
    "native_data_cell_prototype_assignment_creates_a_receiver_shadow"_test=[] {
        o::Context context;
        auto owner=std::make_shared<Owner>();
        const auto prototype=context.object();
        o::bindNativeValue(context,prototype,o::Key(u"value"),owner,&Owner::value);
        const auto child=context.objectWithPrototype(prototype);
        expect(context.get(child,o::Key(u"value"))==o::Value(7));
        context.set(child,o::Key(u"value"),53);
        expect(owner->value==o::Value(7));
        expect(context.get(child,o::Key(u"value"))==o::Value(53));
        context.erase(child,o::Key(u"value"));
        owner->value=59;
        expect(context.get(child,o::Key(u"value"))==o::Value(59));
    };
    "native_data_cell_deletion_reaches_the_owner_before_removing_metadata"_test=[] {
        o::Context context;
        auto owner=std::make_shared<Owner>();
        const auto object=context.object();
        auto field=std::make_shared<o::NativeDataField>(owner,
            [owner]{return owner->value;},[owner](o::Value value){owner->value=std::move(value);},
            [owner]{owner->value=o::Undefined{};});
        context.bindData(object,o::Key(u"value"),field);
        context.erase(object,o::Key(u"value"));
        expect(std::holds_alternative<o::Undefined>(owner->value.data));
        expect(!context.hasOwn(object,o::Key(u"value")));
        context.assignNativeData(object,o::Key(u"value"),field,o::Value(61));
        expect(context.get(object,o::Key(u"value"))==o::Value(61));
        expect(owner->value==o::Value(61));
        const auto descriptor=context.descriptor(object,o::Key(u"value"));
        expect(descriptor&&descriptor->writable&&descriptor->enumerable&&descriptor->configurable);
    };
    "native_data_constructor_assignment_binds_new_cell_without_shadow_value"_test=[] {
        o::Context context;
        const auto object=context.object();
        context.set(object,o::Key(u"before"),o::Value(1));
        auto owner=std::make_shared<Owner>();
        auto field=std::make_shared<o::NativeDataField>(owner,
            [owner]{return owner->value;},[owner](o::Value value){owner->value=std::move(value);});
        context.assignNativeData(object,o::Key(u"value"),field,o::Value(17));
        expect(owner->value==o::Value(17)&&context.get(object,o::Key(u"value"))==o::Value(17));
        const bool keysMatch=context.enumerableStringKeys(object)==std::vector<std::u16string>{u"before",u"value"};
        expect(keysMatch);
        const auto descriptor=context.descriptor(object,o::Key(u"value"));
        expect(descriptor&&descriptor->writable&&descriptor->enumerable&&descriptor->configurable);
        context.set(object,o::Key(u"value"),o::Value(19));
        expect(owner->value==o::Value(19));
    };
    "native_data_constructor_assignment_respects_inherited_descriptors"_test=[] {
        o::Context context;
        auto prototype=context.object();
        const auto setter=context.function(u"set value",[&](auto& cx,auto receiver,std::span<const o::Value> args){
            cx.set(receiver,o::Key(u"seen"),args[0]);return o::Value(o::Undefined{});
        });
        context.define(prototype,o::Key(u"value"),o::Descriptor::access({},setter));
        const auto child=context.objectWithPrototype(prototype);
        auto owner=std::make_shared<Owner>();
        auto field=std::make_shared<o::NativeDataField>(owner,
            [owner]{return owner->value;},[owner](o::Value value){owner->value=std::move(value);});
        context.assignNativeData(child,o::Key(u"value"),field,o::Value(73));
        expect(context.get(child,o::Key(u"seen"))==o::Value(73));
        expect(!context.hasOwn(child,o::Key(u"value"))&&owner->value==o::Value(7));

        context.define(prototype,o::Key(u"blocked"),o::Descriptor::data(o::Value(1),false,true,true));
        bool rejected=false;
        try{context.assignNativeData(child,o::Key(u"blocked"),field,o::Value(79));}
        catch(const o::Exception& e){rejected=e.name=="TypeError";}
        expect(rejected&&owner->value==o::Value(7)&&!context.hasOwn(child,o::Key(u"blocked")));

        const auto ownSetter=context.function(u"set own",[&](auto& cx,auto receiver,std::span<const o::Value> args){
            cx.set(receiver,o::Key(u"ownSeen"),args[0]);return o::Value(o::Undefined{});
        });
        context.define(child,o::Key(u"own"),o::Descriptor::access({},ownSetter));
        context.assignNativeData(child,o::Key(u"own"),field,o::Value(83));
        expect(context.get(child,o::Key(u"ownSeen"))==o::Value(83)&&owner->value==o::Value(7));

        context.define(child,o::Key(u"ownBlocked"),o::Descriptor::data(o::Value(2),false,true,true));
        rejected=false;
        try{context.assignNativeData(child,o::Key(u"ownBlocked"),field,o::Value(89));}
        catch(const o::Exception& e){rejected=e.name=="TypeError";}
        expect(rejected&&owner->value==o::Value(7));
    };
    "declared_native_data_stays_absent_until_an_own_assignment_including_undefined_and_null"_test=[] {
        o::Context context;
        const auto object=context.object();context.set(object,o::Key(u"before"),o::Value(1));
        auto owner=std::make_shared<Owner>();
        const std::weak_ptr<Owner> lifetime=owner;
        auto field=std::make_shared<o::NativeDataField>(owner,
            [owner]{return owner->value;},[owner](o::Value value){owner->value=std::move(value);},
            [owner]{owner->value=o::Undefined{};});
        context.declareNativeData(object,o::Key(u"configuration"),field);
        owner.reset();
        const bool initiallyAbsent=!context.hasOwn(object,o::Key(u"configuration"));
        const bool initialKeysMatch=context.enumerableStringKeys(object)==std::vector<std::u16string>{u"before"};
        const bool initialValueIsUndefined=std::holds_alternative<o::Undefined>(context.get(object,o::Key(u"configuration")).data);
        const bool ownerIsRetained=!lifetime.expired();
        expect(initiallyAbsent);
        expect(initialKeysMatch);
        expect(initialValueIsUndefined);
        expect(ownerIsRetained);
        context.set(object,o::Key(u"configuration"),o::Value(o::Undefined{}));
        const bool assignedOwn=context.hasOwn(object,o::Key(u"configuration"));
        const bool assignedValueIsUndefined=std::holds_alternative<o::Undefined>(context.get(object,o::Key(u"configuration")).data);
        const bool assignedKeysMatch=context.enumerableStringKeys(object)==std::vector<std::u16string>{u"before",u"configuration"};
        expect(assignedOwn);
        expect(assignedValueIsUndefined);
        expect(assignedKeysMatch);
        context.set(object,o::Key(u"configuration"),o::Value(o::Null{}));
        expect(std::holds_alternative<o::Null>(context.get(object,o::Key(u"configuration")).data));
        context.erase(object,o::Key(u"configuration"));
        expect(!context.hasOwn(object,o::Key(u"configuration"))&&!lifetime.expired());
        context.set(object,o::Key(u"configuration"),o::Value(17));
        const bool reassignedValueMatches=context.get(object,o::Key(u"configuration"))==o::Value(17);
        const bool reassignedKeysMatch=context.enumerableStringKeys(object)==std::vector<std::u16string>{u"before",u"configuration"};
        expect(reassignedValueMatches);
        expect(reassignedKeysMatch);
    };
    "declared_native_data_obeys_inherited_setters_and_nonwritable_data"_test=[] {
        o::Context context;
        const auto prototype=context.object();
        auto setterOwner=std::make_shared<Owner>();
        auto setterField=std::make_shared<o::NativeDataField>(setterOwner,
            [setterOwner]{return setterOwner->value;},[setterOwner](o::Value value){setterOwner->value=std::move(value);});
        const auto setter=context.function(u"set inherited",[](auto& cx,auto receiver,std::span<const o::Value> args){
            cx.set(receiver,o::Key(u"seen"),args[0]);return o::Value(o::Undefined{});
        });
        context.define(prototype,o::Key(u"value"),o::Descriptor::access({},setter));
        const auto child=context.objectWithPrototype(prototype);
        context.declareNativeData(child,o::Key(u"value"),setterField);
        context.set(child,o::Key(u"value"),o::Value(29));
        expect(context.get(child,o::Key(u"seen"))==o::Value(29)&&!context.hasOwn(child,o::Key(u"value"))&&
            setterOwner->value==o::Value(7));

        context.define(prototype,o::Key(u"blocked"),o::Descriptor::data(o::Value(3),false,true,true));
        auto blockedOwner=std::make_shared<Owner>();
        auto blockedField=std::make_shared<o::NativeDataField>(blockedOwner,
            [blockedOwner]{return blockedOwner->value;},[blockedOwner](o::Value value){blockedOwner->value=std::move(value);});
        context.declareNativeData(child,o::Key(u"blocked"),blockedField);
        bool rejected=false;
        try{context.set(child,o::Key(u"blocked"),o::Value(31));}catch(const o::Exception& error){rejected=error.name=="TypeError";}
        expect(rejected&&!context.hasOwn(child,o::Key(u"blocked"))&&blockedOwner->value==o::Value(7));
    };
    "declared_native_data_activates_only_after_inherited_writable_data_resolution"_test=[] {
        o::Context context;
        const auto prototype=context.object();
        context.define(prototype,o::Key(u"value"),o::Descriptor::data(o::Value(11),true,true,true));
        const auto child=context.objectWithPrototype(prototype);
        auto owner=std::make_shared<Owner>();
        auto field=std::make_shared<o::NativeDataField>(owner,
            [owner]{return owner->value;},[owner](o::Value value){owner->value=std::move(value);},
            [owner]{owner->value=o::Undefined{};});
        context.set(child,o::Key(u"before"),o::Value(1));
        context.declareNativeData(child,o::Key(u"value"),field);
        context.set(child,o::Key(u"value"),o::Value(67));
        const bool ownerValueMatches=owner->value==o::Value(67);
        const bool childValueMatches=context.get(child,o::Key(u"value"))==o::Value(67);
        const bool prototypeValueUnchanged=context.get(prototype,o::Key(u"value"))==o::Value(11);
        const bool childKeysMatch=context.enumerableStringKeys(child)==std::vector<std::u16string>{u"before",u"value"};
        expect(ownerValueMatches);
        expect(childValueMatches);
        expect(prototypeValueUnchanged);
        expect(childKeysMatch);
    };
    "declared_native_data_define_data_activates_and_accessor_cancels"_test=[] {
        o::Context context;
        const auto dataObject=context.object();auto dataOwner=std::make_shared<Owner>();
        auto dataField=std::make_shared<o::NativeDataField>(dataOwner,
            [dataOwner]{return dataOwner->value;},[dataOwner](o::Value value){dataOwner->value=std::move(value);});
        context.declareNativeData(dataObject,o::Key(u"value"),dataField);
        context.define(dataObject,o::Key(u"value"),o::Descriptor::data(o::Value(43),false,false,true));
        const auto descriptor=context.descriptor(dataObject,o::Key(u"value"));
        expect(dataOwner->value==o::Value(43)&&descriptor&&!descriptor->writable&&!descriptor->enumerable&&
            descriptor->configurable&&descriptor->value==o::Value(43));

        const auto accessorObject=context.object();auto accessorOwner=std::make_shared<Owner>();
        auto accessorField=std::make_shared<o::NativeDataField>(accessorOwner,
            [accessorOwner]{return accessorOwner->value;},[accessorOwner](o::Value value){accessorOwner->value=std::move(value);});
        const auto getter=context.function(u"get value",[](auto&,auto,std::span<const o::Value>){return o::Value(47);});
        context.declareNativeData(accessorObject,o::Key(u"value"),accessorField);
        context.define(accessorObject,o::Key(u"value"),o::Descriptor::access(getter,{}));
        expect(context.get(accessorObject,o::Key(u"value"))==o::Value(47)&&accessorOwner->value==o::Value(7));
        context.define(accessorObject,o::Key(u"value"),o::Descriptor::data(o::Value(53)));
        expect(context.get(accessorObject,o::Key(u"value"))==o::Value(53)&&accessorOwner->value==o::Value(7));
    };
    "declared_native_data_cells_keep_distinct_owners_across_contexts"_test=[] {
        o::Context first,second;
        const auto firstObject=first.object(),secondObject=second.object();
        auto firstOwner=std::make_shared<Owner>(),secondOwner=std::make_shared<Owner>();
        auto firstField=std::make_shared<o::NativeDataField>(firstOwner,
            [firstOwner]{return firstOwner->value;},[firstOwner](o::Value value){firstOwner->value=std::move(value);});
        auto secondField=std::make_shared<o::NativeDataField>(secondOwner,
            [secondOwner]{return secondOwner->value;},[secondOwner](o::Value value){secondOwner->value=std::move(value);});
        first.declareNativeData(firstObject,o::Key(u"value"),firstField);
        second.declareNativeData(secondObject,o::Key(u"value"),secondField);
        first.set(firstObject,o::Key(u"value"),o::Value(59));
        second.set(secondObject,o::Key(u"value"),o::Value(61));
        expect(first.get(firstObject,o::Key(u"value"))==o::Value(59)&&
            second.get(secondObject,o::Key(u"value"))==o::Value(61)&&
            firstOwner->value==o::Value(59)&&secondOwner->value==o::Value(61));
    };
    "native_data_cell_invalid_conversion_does_not_change_value_or_metadata"_test=[] {
        o::Context context,foreign;
        auto owner=std::make_shared<Owner>();
        const auto object=context.object();
        o::bindNativeValue(context,object,o::Key(u"value"),owner,&Owner::value);
        bool rejected=false;
        try{context.define(object,o::Key(u"value"),o::Descriptor::data(foreign.object(),false,false,false));}
        catch(const o::Exception&){rejected=true;}
        expect(rejected&&owner->value==o::Value(7));
        const auto descriptor=context.descriptor(object,o::Key(u"value"));
        expect(descriptor&&descriptor->writable&&descriptor->enumerable&&descriptor->configurable);
    };
    "native_data_cell_configurable_accessor_replacement_has_one_active_property"_test=[] {
        o::Context context;
        auto owner=std::make_shared<Owner>();
        const auto object=context.object();
        o::bindNativeValue(context,object,o::Key(u"value"),owner,&Owner::value);
        const auto getter=context.function(u"get",[](auto&,auto,auto){return o::Value(67);});
        context.define(object,o::Key(u"value"),o::Descriptor::access(getter,{}));
        const auto accessor=context.descriptor(object,o::Key(u"value"));
        expect(accessor&&accessor->accessor&&accessor->getter==getter);
        expect(context.get(object,o::Key(u"value"))==o::Value(67));
        owner->value=o::Value(91);
        expect(context.get(object,o::Key(u"value"))==o::Value(67));
        context.define(object,o::Key(u"value"),o::Descriptor::data(o::Value(34),true,true,true));
        const auto restored=context.descriptor(object,o::Key(u"value"));
        expect(restored&&!restored->accessor&&restored->writable&&restored->enumerable&&restored->configurable);
        expect(context.get(object,o::Key(u"value"))==o::Value(34));
    };
    "native_data_cell_retains_the_real_owner_lifetime"_test=[] {
        o::Context context;
        auto owner=std::make_shared<Owner>();
        const std::weak_ptr<Owner> weak=owner;
        const auto object=context.object();
        o::bindNativeValue(context,object,o::Key(u"value"),owner,&Owner::value);
        owner.reset();
        expect(!weak.expired());
        expect(context.get(object,o::Key(u"value"))==o::Value(7));
        context.erase(object,o::Key(u"value"));
        expect(!weak.expired());
    };
};
}
