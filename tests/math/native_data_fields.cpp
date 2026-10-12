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
        o::bindNativeValue(context,object,o::Key(u"value"),owner,&Owner::value);
        context.erase(object,o::Key(u"value"));
        expect(std::holds_alternative<o::Undefined>(owner->value.data));
        expect(!context.hasOwn(object,o::Key(u"value")));
        context.define(object,o::Key(u"value"),o::Descriptor::data(61));
        expect(context.get(object,o::Key(u"value"))==o::Value(61));
        expect(owner->value==o::Value(61));
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
        const auto getter=context.function(u"get",[](auto&,auto,auto){return o::Value(67);});
        rejected=false;
        try{context.define(object,o::Key(u"value"),o::Descriptor::access(getter,{}));}
        catch(const o::Exception& e){rejected=e.name=="Unimplemented";}
        expect(rejected&&owner->value==o::Value(7));
        expect(context.get(object,o::Key(u"value"))==o::Value(7));
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
