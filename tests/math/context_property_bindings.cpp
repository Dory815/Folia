#include "support/folia_test.hpp"
import folia.math.util.context_property_bindings;
import folia.math.core.tree.node_context_classes;
import folia.math.input.tex.node_util;
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts=folia::math::options;
bool copyContains(const std::vector<std::string>& values,std::string_view key){
    return std::find_if(values.begin(),values.end(),[key](const std::string& value){
        return std::string_view(value)==key;
    })!=values.end();
}
bool copyContains(const std::vector<opts::SymbolRef>& values,opts::SymbolRef key){
    return std::find(values.begin(),values.end(),key)!=values.end();
}
struct Graph {
    opts::Context context;
    OptionFunctionBindings functions{context};
    ContextPropertyBindings properties{context,functions};
};
struct NativeRecordFixture final:opts::NativeRecordView {
    std::vector<std::pair<opts::Key,opts::Descriptor>> fields;
    std::optional<opts::ObjectRef> parent;
    std::optional<opts::Descriptor> ownDescriptor(const opts::Key& key)const override{
        for(const auto& [name,value]:fields)if(name==key)return value;return {};
    }
    bool ownPropertyEnumerable(const opts::Key& key)const override{
        for(const auto& [name,value]:fields)if(name==key)return value.enumerable;return false;
    }
    std::vector<opts::Key> ownKeys()const override{std::vector<opts::Key> result;for(const auto& [key,value]:fields){(void)value;result.push_back(key);}return result;}
    void defineOwn(const opts::Key& key,opts::Descriptor value)override{
        for(auto& [name,old]:fields)if(name==key){old=std::move(value);return;}fields.emplace_back(key,std::move(value));
    }
    void deleteOwn(const opts::Key& key)override{std::erase_if(fields,[&](const auto& item){return item.first==key;});}
    std::optional<opts::ObjectRef> prototype()const override{return parent;}
    void setPrototype(std::optional<opts::ObjectRef> value)override{parent=value;}
};
struct BoundArrayFixture final:PropertyRecordBinding {
    PropertyArray values;
    PropertyList named;
    static std::optional<std::size_t> index(std::string_view name){
        if(name.empty()||(name.size()>1&&name.front()=='0'))return {};
        std::size_t value{};const auto [end,error]=std::from_chars(name.data(),name.data()+name.size(),value);
        if(error!=std::errc{}||end!=name.data()+name.size()||value>=std::numeric_limits<std::uint32_t>::max())return {};
        return value;
    }
    Property get(std::string_view name)override{return getOwn(name);}
    Property getOwn(std::string_view name)override{
        if(name=="length")return static_cast<double>(values.size());
        if(auto i=index(name))return values.has(*i)?values.get(*i):Property(Missing{});
        return named.getOwn(name);
    }
    bool has(std::string_view name)const override{return hasOwn(name);}
    bool hasOwn(std::string_view name)const override{
        if(name=="length")return true;
        if(auto i=index(name))return values.has(*i);
        return named.hasOwn(name);
    }
    void set(std::string_view name,Property value)override{setOwn(name,std::move(value));}
    void setOwn(std::string_view name,Property value)override{
        if(name=="length"){
            const auto* number=std::get_if<double>(&value.value);
            if(!number||*number<0||std::trunc(*number)!=*number)throw opts::Exception("RangeError",u"Invalid array length");
            values.resize(static_cast<std::size_t>(*number));return;
        }
        if(auto i=index(name)){values.set(*i,std::move(value));return;}
        named.setOwn(name,std::move(value));
    }
    void erase(std::string_view name)override{
        if(auto i=index(name)){values.erase(*i);return;}
        named.erase(name);
    }
    std::vector<std::string> keys()const override{
        std::vector<std::string> result;
        for(std::size_t i=0;i<values.size();++i)if(values.has(i))result.push_back(std::to_string(i));
        for(const auto& name:named.keys())result.push_back(name);
        return result;
    }
    Property prototype()const override{return Property(BuiltinProperty::arrayPrototype);}
    Property primitive(opts::PrimitiveHint)override{return Property("array");}
};
suite contextProperties=[] {
    "math_context_property_symbol_keeps_canonical_marker_identity"_test=[] {
        Graph graph;
        const auto marker=graph.context.symbol(u"marker");
        const auto peer=graph.context.symbol(u"marker");
        const auto value=graph.properties.from(marker);
        expect(value==graph.properties.from(marker));
        expect(value!=graph.properties.from(peer));
        expect(graph.properties.to(value)==opts::Value(marker));
        expect(property_host::truthy(value));
        tex::MapHandler registry;
        const auto map=tex::TokenMapHost::create<tex::CharacterMap>(registry,
            Property("native-symbol-marker"),tex::ParseMethod(Property(Missing{})),Property(PropertyList{}));
        map->add(Property("x"),tex::TokenLookup(value));
        expect(map->contains(Property("x")));
        const auto saved=map->lookup(Property("x"));
        expect(std::get<Property>(saved)==value);
        map->remove(Property("x"));
        expect(!map->contains(Property("x")));
    };
    "math_context_property_symbol_rejects_foreign_context_and_coercion"_test=[] {
        Graph graph,foreign;
        const auto value=graph.properties.from(graph.context.symbol(u"marker"));
        const auto rejected=[](auto operation,std::u16string_view message){
            try{operation();}catch(const opts::Exception& error){return error.name=="TypeError"&&error.message==message;}
            return false;
        };
        expect(rejected([&]{(void)property_host::string(value);},u"Cannot convert a Symbol value to a string"));
        expect(rejected([&]{(void)property_host::number(value);},u"Cannot convert a Symbol value to a number"));
        bool wrongFrom=false,wrongTo=false;
        try{(void)foreign.properties.from(graph.properties.to(value));}catch(const opts::Exception&){wrongFrom=true;}
        try{(void)foreign.properties.to(value);}catch(const opts::Exception&){wrongTo=true;}
        expect(wrongFrom);expect(wrongTo);
    };
    "math_context_property_live_alias_and_cycle"_test=[] {
        Graph graph;auto object=graph.context.object();auto child=graph.context.object();
        graph.context.set(object,u"self",object);graph.context.set(object,u"child",child);
        auto first=graph.properties.from(object),second=graph.properties.from(object);
        auto record=PropertyList::record(first).value();
        expect(first==second);expect(record.get("self")==first);
        expect(record.get("child")==graph.properties.from(child));
        graph.context.set(object,u"width",12);expect(record.get("width")==Property(12));
        record.set("width",14);expect(graph.context.get(object,u"width")==opts::Value(14));
        expect(graph.properties.to(first)==opts::Value(object));
        expect(graph.properties.to(record.get("child"))==opts::Value(child));
    };
    "math_context_property_plain_native_record_keeps_shared_storage_identity"_test=[] {
        Graph graph;PropertyList record{{"value",Property(3)}};const Property original(record);
        const auto object=std::get<opts::ObjectRef>(graph.properties.to(original).data);
        expect(graph.context.get(object,u"value")==opts::Value(3));
        graph.context.set(object,u"value",opts::Value(7));
        expect(record.getOwn("value")==Property(7));
        const auto roundTrip=graph.properties.from(object);
        const auto* before=std::get_if<std::shared_ptr<PropertyStorage>>(&original.value);
        const auto* after=std::get_if<std::shared_ptr<PropertyStorage>>(&roundTrip.value);
        expect(before&&after&&*before==*after);
    };
    "math_context_property_array_storage_round_trips_without_dense_copy"_test=[] {
        Graph graph;PropertyArray native;native.resize(3);native.set(1,Property(4));
        const Property original(native);const auto array=std::get<opts::ObjectRef>(graph.properties.to(original).data);
        expect(eq(graph.context.arrayLength(array),std::uint32_t(3)));
        expect(!graph.context.hasOwn(array,u"0"));expect(graph.context.hasOwn(array,u"1"));
        graph.context.set(array,u"2",opts::Value(9));expect(native.get(2)==Property(9));
        graph.context.erase(array,u"1");expect(!native.has(1));
        const auto roundTrip=graph.properties.from(array);
        const PropertyArray restored=PropertyArray::array(roundTrip).value();
        expect(restored.get(2)==Property(9));expect(!restored.has(1));
        const auto* before=std::get_if<std::shared_ptr<PropertyStorage>>(&original.value);
        const auto* after=std::get_if<std::shared_ptr<PropertyStorage>>(&roundTrip.value);
        expect(before&&after&&*before==*after);
    };
    "math_context_property_array_symbol_lookup_uses_inherited_iterator"_test=[] {
        Graph graph;PropertyArray native;native.push(Property(3));native.push(Property(5));
        const auto array=std::get<opts::ObjectRef>(graph.properties.to(Property(native)).data);
        const auto iteratorKey=opts::Key(graph.context.intrinsics().symbolIterator);
        const auto iterator=graph.context.get(opts::Value(array),iteratorKey);
        const auto* iteratorObject=std::get_if<opts::ObjectRef>(&iterator.data);
        expect(iteratorObject&&graph.context.kind(*iteratorObject)==opts::ObjectKind::function)
            << "PropertyArray inherits the canonical Symbol.iterator function";
        const auto ownSymbols=graph.context.ownSymbols(opts::Value(array));
        expect(ownSymbols.empty()) << "PropertyArray has no own Symbol keys";
        std::vector<double> seen;
        graph.context.forEachIterable(opts::Value(array),[&](opts::Value item){
            const auto* number=std::get_if<double>(&item.data);
            expect(number!=nullptr) << "inherited iterator yields numeric PropertyArray elements";
            if(number)seen.push_back(*number);
        });
        expect((seen==std::vector<double>{3,5})) << "iterator preserves the stored PropertyArray values";

        const auto record=graph.context.object();
        const auto marker=graph.context.symbol(u"marker");
        graph.context.set(record,opts::Key(marker),opts::Value(9));
        expect(graph.context.get(opts::Value(record),opts::Key(marker))==opts::Value(9))
            << "ordinary Context records still read their own Symbol property";
        const auto recordSymbols=graph.context.ownSymbols(opts::Value(record));
        expect(recordSymbols.size()==1&&recordSymbols.front()==marker)
            << "ordinary Context records still enumerate their own Symbol property";
    };
    "math_context_native_array_provider_delegates_to_bound_array_storage"_test=[] {
        Graph graph;auto binding=std::make_shared<BoundArrayFixture>();
        binding->values.resize(3);binding->values.set(1,Property(4));binding->named.setOwn("label",Property("live"));
        const auto native=PropertyArray::array(Property(PropertyList::live(binding,true))).value();
        const auto object=std::get<opts::ObjectRef>(graph.properties.to(Property(native)).data);
        expect(eq(graph.context.arrayLength(object),std::uint32_t(3)));
        expect(!graph.context.hasOwn(object,u"0"));expect(graph.context.get(object,u"1")==opts::Value(4));
        expect(graph.context.get(object,u"label")==opts::Value(u"live"));
        graph.context.set(object,u"0",opts::Value(8));expect(binding->values.get(0)==Property(8));
        graph.context.erase(object,u"1");expect(!binding->values.has(1));
        graph.context.set(object,u"label",opts::Value(u"changed"));expect(binding->named.getOwn("label")==Property("changed"));
        graph.context.set(object,u"2",opts::Value(9));expect(native.get(2)==Property(9));
    };
    "math_context_native_record_view_uses_context_prototype_and_descriptor_dispatch"_test=[] {
        Graph graph;const auto parent=graph.context.object();
        auto view=std::make_shared<NativeRecordFixture>();view->fields.emplace_back(opts::Key(u"fixed"),opts::Descriptor::data(1,false,true,false));
        const auto object=graph.context.nativeRecord(view,std::optional<opts::ObjectRef>{parent});
        int calls=0;bool receiver=false;
        const auto setter=graph.context.function(u"setter",[&](auto&,auto self,auto args)->opts::Value{
            ++calls;receiver=self==opts::Value(object)&&args.size()==1&&args[0]==opts::Value(9);return opts::Undefined{};
        });
        graph.context.define(parent,u"inherited",opts::Descriptor::access({},setter));
        graph.context.set(object,u"inherited",9);
        expect(eq(calls,1));expect(receiver);expect(!graph.context.hasOwn(object,u"inherited"));
        expect(graph.context.get(object,u"fixed")==opts::Value(1));
        bool readonly=false;try{graph.context.set(object,u"fixed",2);}catch(const opts::Exception& error){readonly=error.name=="TypeError";}
        expect(readonly);expect(graph.context.prototype(object)==parent);
        graph.context.define(object,u"fresh",opts::Descriptor::data(3));
        expect(view->ownDescriptor(opts::Key(u"fresh"))->value==opts::Value(3));
        graph.context.erase(object,u"fresh");expect(!graph.context.hasOwn(object,u"fresh"));
    };
    "math_context_property_getter_setter_receiver_and_keys"_test=[] {
        Graph graph;auto parent=graph.context.object(),object=graph.context.objectWithPrototype(parent);
        int gets=0,sets=0;bool getterReceiver=false,setterReceiver=false;
        auto getter=graph.context.function(u"get",[&](auto&,auto receiver,auto)->opts::Value{
            ++gets;getterReceiver=receiver==opts::Value(object);return 3;
        });
        auto setter=graph.context.function(u"set",[&](auto&,auto receiver,auto args)->opts::Value{
            ++sets;setterReceiver=receiver==opts::Value(object)&&args.size()==1&&args[0]==opts::Value(9);return opts::Undefined{};
        });
        graph.context.define(parent,u"inherited",opts::Descriptor::access(getter,setter,true,true));
        graph.context.define(object,u"hidden",opts::Descriptor::data(5,true,false,true));
        graph.context.set(object,u"2",2);graph.context.set(object,u"1",1);graph.context.set(object,u"name",u"x");
        auto record=PropertyList::record(graph.properties.from(object)).value();
        expect(record.keys()==std::vector<std::string>{"1","2","name"});expect(eq(gets,0));
        expect(record.has("inherited"));expect(!record.hasOwn("inherited"));
        expect(record.getOwn("inherited").missing());expect(eq(gets,0));
        expect(record.get("inherited")==Property(3));expect(getterReceiver);expect(eq(gets,1));
        record.set("inherited",9);expect(setterReceiver);expect(eq(sets,1));
        record.erase("name");expect(!graph.context.hasOwn(object,u"name"));
    };
    "math_context_property_array_holes_and_live_length"_test=[] {
        Graph graph;auto object=graph.context.array(3);graph.context.set(object,u"1",opts::Undefined{});
        auto array=PropertyArray::array(graph.properties.from(object)).value();
        expect(eq(array.size(),std::size_t(3)));expect(!array.has(0));expect(array.has(1));
        array.set(4,7);expect(eq(graph.context.arrayLength(object),std::uint32_t(5)));
        expect(graph.context.get(object,u"4")==opts::Value(7));
        graph.context.set(object,u"2",8);expect(array.get(2)==Property(8));
        array.erase(1);expect(!graph.context.hasOwn(object,u"1"));
        array.resize(1);expect(eq(graph.context.arrayLength(object),std::uint32_t(1)));
        expect(graph.properties.to(Property(array))==opts::Value(object));
    };
    "math_context_property_mml_definition_remains_live"_test=[] {
        Graph graph;mml::MmlFactory factory;tex::NodeUtil utility;
        auto definition=graph.context.object(),nested=graph.context.object();
        std::vector<std::string> trace;
        auto getter=graph.context.function(u"get",[&](auto&,auto receiver,auto)->opts::Value{
            trace.push_back(receiver==opts::Value(definition)?"same":"wrong");
            graph.context.set(definition,u"mathcolor",u"red");return nested;
        });
        graph.context.define(definition,u"data-payload",opts::Descriptor::access(getter,{},true,true));
        graph.context.set(definition,u"mathcolor",u"blue");
        auto node=factory.create("mi");auto value=graph.properties.from(definition);
        utility.setProperties(node,value);
        expect(trace==std::vector<std::string>{"same"});
        expect(utility.getAttribute(node,"mathcolor")==Property("red"));
        auto payload=utility.getAttribute(node,"data-payload");expect(payload==graph.properties.from(nested));
        graph.context.set(nested,u"mark",4);expect(PropertyList::record(payload)->get("mark")==Property(4));
        expect(graph.properties.to(value)==opts::Value(definition));
    };
    "math_context_property_canonical_function_identity"_test=[] {
        Graph graph;auto native=std::make_shared<Function>();
        native->call=[](const FunctionReceiver&,FunctionArgs)->std::any{return Property(6);};
        OptionCallBinding binding;
        binding.receiver=[](auto&,auto receiver)->FunctionReceiver{return receiver;};
        binding.arguments=[](auto&,auto){return std::vector<std::any>{};};
        binding.result=[](auto&,const std::any& result)->opts::Value{return std::get<double>(std::any_cast<Property>(result).value);};
        auto object=graph.functions.bind(native,std::move(binding));auto property=graph.properties.from(object);
        expect(std::get<FunctionRef>(property.value)==native);expect(graph.properties.to(property)==object);
    };
    "math_context_property_assign_copy_uses_own_keys_and_live_gets"_test=[] {
        Graph graph;auto parent=graph.context.object();auto source=graph.context.objectWithPrototype(parent);
        auto nested=graph.context.object();int reads=0;bool receiver=false;
        graph.context.set(parent,u"inherited",7);
        graph.context.define(source,u"hidden",opts::Descriptor::data(8,true,false,true));
        graph.context.set(source,u"2",2);graph.context.set(source,u"1",1);
        graph.context.set(source,u"child",nested);graph.context.set(source,u"later",3);
        auto getter=graph.context.function(u"get-later",[&](auto&,auto recv,auto)->opts::Value{
            ++reads;receiver=recv==opts::Value(source);return 9;
        });
        graph.context.define(source,u"live",opts::Descriptor::access(getter,{},true,true));
        const auto original=graph.properties.from(source);
        const auto copied=graph.properties.copyOwnEnumerableProperties(original);
        const auto copiedObject=std::get<opts::ObjectRef>(graph.properties.to(copied).data);
        expect(copiedObject!=source);expect(eq(reads,1));expect(receiver);
        const auto copiedKeys=graph.context.enumerableStringKeys(copiedObject);
        const bool copiedKeyOrder=copiedKeys.size()==5&&copiedKeys[0].compare(u"1")==0&&
            copiedKeys[1].compare(u"2")==0&&copiedKeys[2].compare(u"child")==0&&
            copiedKeys[3].compare(u"later")==0&&copiedKeys[4].compare(u"live")==0;
        expect(copiedKeyOrder);
        expect(!graph.context.hasOwn(copiedObject,u"inherited"));expect(!graph.context.hasOwn(copiedObject,u"hidden"));
        expect(graph.context.get(copiedObject,u"live")==opts::Value(9));
        expect(graph.context.get(copiedObject,u"child")==opts::Value(nested));
        graph.context.set(nested,u"mark",11);
        expect(graph.context.get(graph.properties.to(graph.properties.from(nested)),u"mark")==opts::Value(11));
    };
    "math_context_property_assign_checks_metadata_without_reading_native_value"_test=[] {
        Graph graph;
        auto source=graph.context.object();
        auto owner=std::make_shared<int>(0);
        int reads=0;
        graph.context.define(source,u"value",opts::Descriptor::data(opts::Value(0)));
        graph.context.assignNativeData(source,opts::Key(u"value"),
            std::make_shared<opts::NativeDataField>(owner,
                [&]()->opts::Value{++reads;return reads;},
                [](opts::Value){}),opts::Value(0));
        const auto copied=graph.properties.copyOwnEnumerableProperties(graph.properties.from(opts::Value(source)));
        const auto target=std::get<opts::ObjectRef>(graph.properties.to(copied).data);
        expect(eq(reads,1));
        expect(graph.context.get(opts::Value(target),opts::Key(u"value"))==opts::Value(1));
    };
    "math_tree_context_copy_checks_native_field_metadata_without_reading_value"_test=[] {
        Graph graph;
        auto source=graph.context.object();
        auto owner=std::make_shared<int>(0);
        int reads=0;
        graph.context.define(source,u"value",opts::Descriptor::data(opts::Value(0)));
        graph.context.assignNativeData(source,opts::Key(u"value"),
            std::make_shared<opts::NativeDataField>(owner,
                [&]()->opts::Value{++reads;return reads;},
                [](opts::Value){}),opts::Value(0));
        const auto copied=folia::math::tree::treeContextCopyOwnEnumerable(
            graph.context,graph.properties,opts::Value(source));
        const auto target=std::get<opts::ObjectRef>(graph.properties.to(copied).data);
        expect(eq(reads,1));
        expect(graph.context.get(opts::Value(target),opts::Key(u"value"))==opts::Value(1));
    };
    "math_context_property_assign_native_record_metadata_does_not_call_value_descriptor"_test=[] {
        struct CountingView final:opts::NativeRecordView {
            mutable int descriptorReads=0;
            mutable int metadataReads=0;
            opts::Value value=17;
            std::optional<opts::Descriptor> ownDescriptor(const opts::Key& key)const override{
                if(key!=opts::Key(u"value"))return {};
                ++descriptorReads;return opts::Descriptor::data(value);
            }
            bool ownPropertyEnumerable(const opts::Key& key)const override{
                ++metadataReads;return key==opts::Key(u"value");
            }
            std::vector<opts::Key> ownKeys()const override{return {opts::Key(u"value")};}
            void defineOwn(const opts::Key& key,opts::Descriptor descriptor)override{
                if(key==opts::Key(u"value"))value=descriptor.value;
            }
            void deleteOwn(const opts::Key&)override{}
            std::optional<opts::ObjectRef> prototype()const override{return {};}
            void setPrototype(std::optional<opts::ObjectRef>)override{}
        };
        Graph graph;
        auto view=std::make_shared<CountingView>();
        const auto source=graph.context.nativeRecord(view);
        const auto copied=graph.properties.copyOwnEnumerableProperties(graph.properties.from(opts::Value(source)));
        const auto target=std::get<opts::ObjectRef>(graph.properties.to(copied).data);
        expect(eq(view->metadataReads,1));
        expect(eq(view->descriptorReads,1));
        expect(graph.context.get(opts::Value(target),opts::Key(u"value"))==opts::Value(17));
    };
    "math_context_property_assign_reads_bound_property_owner_once"_test=[] {
        struct CountingBinding final:PropertyRecordBinding {
            mutable int reads=0;
            Property value=17;
            Property get(std::string_view name)override{return getOwn(name);}
            Property getOwn(std::string_view name)override{
                if(name!="value")return Missing{};
                ++reads;return value;
            }
            bool has(std::string_view name)const override{return hasOwn(name);}
            bool hasOwn(std::string_view name)const override{return name=="value";}
            void set(std::string_view name,Property next)override{setOwn(name,std::move(next));}
            void setOwn(std::string_view name,Property next)override{if(name=="value")value=std::move(next);}
            void erase(std::string_view name)override{if(name=="value")value=Missing{};}
            std::vector<std::string> keys()const override{return {"value"};}
            Property prototype()const override{return Missing{};}
            Property primitive(opts::PrimitiveHint)override{return BuiltinProperty::toString;}
        };
        Graph graph;
        auto binding=std::make_shared<CountingBinding>();
        const PropertyList source=PropertyList::live(binding);
        const auto copied=graph.properties.copyOwnEnumerableProperties(Property(source));
        const auto target=std::get<opts::ObjectRef>(graph.properties.to(copied).data);
        expect(eq(binding->reads,1));
        expect(graph.context.get(opts::Value(target),opts::Key(u"value"))==opts::Value(17));
    };
    "math_context_bound_property_list_copy_and_symbol_keys_are_metadata_only"_test=[] {
        Graph graph;
        const auto sourceObject=graph.context.object();
        auto nativeOwner=std::make_shared<int>(0);
        int nativeReads=0;
        graph.context.define(sourceObject,opts::Key(u"native"),opts::Descriptor::data(opts::Value(0)));
        graph.context.assignNativeData(sourceObject,opts::Key(u"native"),
            std::make_shared<opts::NativeDataField>(nativeOwner,
                [&]()->opts::Value{++nativeReads;return opts::Value(nativeReads);},
                [](opts::Value){}),opts::Value(0));

        const auto symbol=graph.context.symbol(u"copy-symbol-getter");
        int symbolGetterReads=0;
        const auto getter=graph.context.function(u"getSymbol",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>){++symbolGetterReads;return opts::Value(73.0);});
        graph.context.define(sourceObject,opts::Key(symbol),
            opts::Descriptor::access(getter,std::nullopt,true,true));

        const auto source=graph.properties.from(opts::Value(sourceObject));
        auto sourceRecord=folia::math::PropertyList::record(source);
        expect(sourceRecord.has_value());
        if(!sourceRecord)return;

        const auto symbols=sourceRecord->symbolKeys();
        expect(std::find(symbols.begin(),symbols.end(),symbol)!=symbols.end());
        expect(symbolGetterReads==0);
        expect(nativeReads==0);

        const auto copied=sourceRecord->copyOwnEnumerableProperties();
        expect(nativeReads==1);
        expect(symbolGetterReads==1);
        expect(copied.get("native")==Property(1));
        expect(copied.getKey(Property(symbol))==Property(73));
    };
    "math_context_reprojection_preserves_bound_record_enumerability_without_value_reads"_test=[] {
        opts::Context sourceContext;
        opts::Context targetContext;
        folia::math::OptionFunctionBindings sourceFunctions{sourceContext};
        folia::math::OptionFunctionBindings targetFunctions{targetContext};
        folia::math::ContextPropertyBindings sourceProperties{sourceContext,sourceFunctions};
        folia::math::ContextPropertyBindings targetProperties{targetContext,targetFunctions};
        const auto sourceObject=sourceContext.object();
        int getterReads=0;
        const auto getter=sourceContext.function(u"get",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>){++getterReads;return opts::Value(41.0);});
        sourceContext.define(sourceObject,opts::Key(u"visible"),
            opts::Descriptor::access(getter,std::nullopt,true,true));
        sourceContext.define(sourceObject,opts::Key(u"hidden"),
            opts::Descriptor::access(getter,std::nullopt,false,true));

        const auto projectedValue=targetProperties.to(sourceProperties.from(opts::Value(sourceObject)));
        const auto* projected=std::get_if<opts::ObjectRef>(&projectedValue.data);
        expect(projected!=nullptr);
        if(!projected)return;

        expect(targetContext.ownPropertyEnumerable(projectedValue,opts::Key(u"visible")));
        expect(!targetContext.ownPropertyEnumerable(projectedValue,opts::Key(u"hidden")));
        expect(getterReads==0);
        expect(targetContext.get(*projected,opts::Key(u"visible"))==opts::Value(41.0));
        expect(getterReads==1);
    };
    "math_context_property_assign_copy_snapshots_keys_but_rechecks_descriptors"_test=[] {
        Graph graph;auto source=graph.context.object();int reads=0;bool receiver=false;
        auto getter=graph.context.function(u"mutate",[&](auto&,auto recv,auto)->opts::Value{
            ++reads;receiver=recv==opts::Value(source);graph.context.erase(source,u"gone");
            graph.context.set(source,u"added",4);graph.context.set(source,u"updated",5);return 1;
        });
        graph.context.define(source,u"first",opts::Descriptor::access(getter,{},true,true));
        graph.context.set(source,u"gone",2);graph.context.set(source,u"updated",3);
        const auto copied=graph.properties.copyOwnEnumerableProperties(graph.properties.from(source));
        const auto target=std::get<opts::ObjectRef>(graph.properties.to(copied).data);
        expect(eq(reads,1));expect(receiver);expect(graph.context.get(target,u"first")==opts::Value(1));
        expect(!graph.context.hasOwn(target,u"gone"));expect(graph.context.get(target,u"updated")==opts::Value(5));
        expect(!graph.context.hasOwn(target,u"added"));
    };
    "math_context_bound_record_assign_snapshots_complete_keys_before_getters"_test=[] {
        struct MutatingBinding final:PropertyRecordBinding {
            std::vector<std::string> strings{"first","hidden","gone","__proto__"};
            std::vector<std::string> enumerableStrings{"first","gone","__proto__"};
            std::vector<opts::SymbolRef> symbols;
            std::vector<opts::SymbolRef> enumerableSymbols;
            std::vector<std::pair<std::string,Property>> values;
            std::vector<std::pair<opts::SymbolRef,Property>> symbolValues;
            mutable std::vector<std::string> trace;
            opts::SymbolRef flip,drop,deleted,late;
            Property prototypeValue;
            bool mutated=false;
            std::string symbolName(opts::SymbolRef key)const{
                if(key==flip)return "flip-to-enumerable";
                if(key==drop)return "flip-to-nonenumerable";
                if(key==deleted)return "deleted-during-getter";
                return "added-during-getter";
            }
            Property value(std::string_view key)const{
                for(const auto& [name,item]:values)if(name==key)return item;
                return Missing{};
            }
            Property symbolValue(opts::SymbolRef key)const{
                for(const auto& [name,item]:symbolValues)if(name==key)return item;
                return Missing{};
            }
            Property get(std::string_view key)override{
                trace.push_back("get:"+std::string(key));
                if(key=="first"&&!mutated){
                    mutated=true;
                    enumerableStrings.push_back("hidden");
                    std::erase(enumerableStrings,"gone");
                    strings.push_back("lateString");enumerableStrings.push_back("lateString");
                    enumerableSymbols.push_back(flip);
                    std::erase(enumerableSymbols,drop);
                    std::erase(symbols,deleted);
                    symbols.push_back(late);enumerableSymbols.push_back(late);
                }
                return key=="__proto__"?prototypeValue:value(key);
            }
            Property getOwn(std::string_view key)override{return value(key);}
            bool has(std::string_view key)const override{return hasOwn(key);}
            bool hasOwn(std::string_view key)const override{
                return copyContains(strings,key);
            }
            bool ownPropertyEnumerable(std::string_view key)const override{
                trace.push_back("enum:"+std::string(key));
                return copyContains(strings,key)&&copyContains(enumerableStrings,key);
            }
            void set(std::string_view key,Property item)override{setOwn(key,std::move(item));}
            void setOwn(std::string_view key,Property item)override{
                for(auto& [name,value]:values)if(name==key){value=std::move(item);return;}
                strings.emplace_back(key);enumerableStrings.emplace_back(key);values.emplace_back(key,std::move(item));
            }
            void erase(std::string_view key)override{
                std::erase(strings,std::string(key));std::erase(enumerableStrings,std::string(key));
            }
            Property getSymbol(opts::SymbolRef key)override{
                trace.push_back("getSymbol:"+symbolName(key));return symbolValue(key);
            }
            Property getOwnSymbol(opts::SymbolRef key)override{return symbolValue(key);}
            bool hasSymbol(opts::SymbolRef key)const override{return hasOwnSymbol(key);}
            bool hasOwnSymbol(opts::SymbolRef key)const override{return copyContains(symbols,key);}
            bool ownSymbolEnumerable(opts::SymbolRef key)const override{
                trace.push_back("enumSymbol:"+symbolName(key));
                return hasOwnSymbol(key)&&copyContains(enumerableSymbols,key);
            }
            void setSymbol(opts::SymbolRef key,Property item)override{setOwnSymbol(key,std::move(item));}
            void setOwnSymbol(opts::SymbolRef key,Property item)override{
                for(auto& [name,value]:symbolValues)if(name==key){value=std::move(item);return;}
                symbols.push_back(key);enumerableSymbols.push_back(key);symbolValues.emplace_back(key,std::move(item));
            }
            void eraseSymbol(opts::SymbolRef key)override{
                std::erase(symbols,key);std::erase(enumerableSymbols,key);
            }
            std::vector<opts::SymbolRef> symbolKeys()const override{return enumerableSymbols;}
            std::vector<std::string> keys()const override{return enumerableStrings;}
            std::vector<std::string> ownStringKeys()const override{return strings;}
            std::vector<opts::SymbolRef> ownSymbolKeys()const override{return symbols;}
            Property prototype()const override{return Missing{};}
            Property primitive(opts::PrimitiveHint)override{return BuiltinProperty::toString;}
        };
        Graph graph;
        auto binding=std::make_shared<MutatingBinding>();
        binding->flip=graph.context.symbol(u"flip-to-enumerable");
        binding->drop=graph.context.symbol(u"flip-to-nonenumerable");
        binding->deleted=graph.context.symbol(u"deleted-during-getter");
        binding->late=graph.context.symbol(u"added-during-getter");
        PropertyList prototype{{"marker",Property(10)}};
        binding->prototypeValue=Property(prototype);
        binding->values={{"first",Property(1)},{"hidden",Property(2)},{"gone",Property(3)}};
        binding->symbolValues={{binding->flip,Property(5)},{binding->drop,Property(6)},
            {binding->deleted,Property(7)}};
        binding->symbols={binding->flip,binding->drop,binding->deleted};
        binding->enumerableSymbols={binding->drop,binding->deleted};

        auto copied=PropertyList::live(binding).copyOwnEnumerableProperties();
        expect(binding->trace==std::vector<std::string>{
            "enum:first","get:first","enum:hidden","get:hidden","enum:gone",
            "enum:__proto__","get:__proto__","enumSymbol:flip-to-enumerable",
            "getSymbol:flip-to-enumerable","enumSymbol:flip-to-nonenumerable",
            "enumSymbol:deleted-during-getter"});
        expect(copied.keys()==std::vector<std::string>{"first","hidden"});
        expect(copied.getOwn("hidden")==Property(2));
        expect(!copied.hasOwn("gone")&&!copied.hasOwn("lateString"));
        expect(copied.get("__proto__")==Property(prototype)&&!copied.hasOwn("__proto__"));
        expect(static_cast<bool>(copied.symbolKeys()==std::vector<opts::SymbolRef>{binding->flip}));
        expect(copied.getKey(Property(binding->flip))==Property(5));
    };
    "math_context_property_assign_copy_uses_target_set_semantics"_test=[] {
        Graph graph;auto source=graph.context.object(),prototype=graph.context.object();
        graph.context.define(source,u"__proto__",opts::Descriptor::data(opts::Value(prototype)));
        const auto symbol=graph.context.symbol(u"copied-symbol");
        graph.context.define(source,symbol,opts::Descriptor::data(opts::Value(3)));
        const auto copied=graph.properties.copyOwnEnumerableProperties(graph.properties.from(source));
        const auto target=std::get<opts::ObjectRef>(graph.properties.to(copied).data);
        expect(graph.context.prototype(target)==prototype);expect(!graph.context.hasOwn(target,u"__proto__"));
        expect(graph.context.hasOwn(target,symbol));
        expect(graph.context.get(target,symbol)==opts::Value(3));
    };
    "math_context_property_assign_copy_native_root_is_new_and_retains_bound_nested_identity"_test=[] {
        Graph graph;auto nested=graph.context.object();graph.context.set(nested,u"mark",4);
        const auto marker=graph.context.symbol(u"copy-marker");
        auto nativeFunction=std::make_shared<Function>();
        nativeFunction->call=[](const FunctionReceiver&,FunctionArgs)->std::any{return Property(6);};
        OptionCallBinding binding;
        binding.receiver=[](auto&,auto receiver)->FunctionReceiver{return receiver;};
        binding.arguments=[](auto&,auto){return std::vector<std::any>{};};
        binding.result=[](auto&,const std::any& result)->opts::Value{return std::get<double>(std::any_cast<Property>(result).value);};
        const auto callableObject=graph.functions.bind(nativeFunction,std::move(binding));
        PropertyList native{{"number",Property(1)},{"nested",graph.properties.from(nested)},
            {"marker",graph.properties.from(marker)},{"callable",graph.properties.from(callableObject)}};
        const auto copied=graph.properties.copyOwnEnumerableProperties(Property(native));
        auto target=PropertyList::record(copied).value();
        expect(target.keys()==std::vector<std::string>{"number","nested","marker","callable"});
        expect(target.getOwn("nested")==graph.properties.from(nested));
        expect(target.getOwn("marker")==graph.properties.from(marker));
        expect(target.getOwn("callable")==graph.properties.from(callableObject));
        target.set("number",Property(9));
        expect(native.getOwn("number")==Property(1));expect(target.getOwn("number")==Property(9));
        expect(graph.properties.to(target.getOwn("nested"))==opts::Value(nested));
        expect(graph.properties.to(target.getOwn("marker"))==opts::Value(marker));
        expect(graph.properties.to(target.getOwn("callable"))==opts::Value(callableObject));
    };
    "math_context_property_assign_copy_retains_unbound_nested_record_identity"_test=[] {
        Graph graph;PropertyList nested{{"leaf",Property(2)}};
        PropertyList native{{"nested",Property(nested)}};
        const auto copied=graph.properties.copyOwnEnumerableProperties(Property(native));
        const auto target=PropertyList::record(copied).value();
        const Property originalNestedProperty(nested),copiedNestedProperty=target.getOwn("nested");
        const auto* originalNested=std::get_if<std::shared_ptr<PropertyStorage>>(&originalNestedProperty.value);
        const auto* copiedNested=std::get_if<std::shared_ptr<PropertyStorage>>(&copiedNestedProperty.value);
        expect(originalNested&&copiedNested&&*originalNested==*copiedNested);
        expect(target.getOwn("nested")==Property(nested));
    };
    "math_context_property_assign_copy_nullish_sources_are_empty"_test=[] {
        Graph graph;
        const auto missing=graph.properties.copyOwnEnumerableProperties(Property(Missing{}));
        const auto nullValue=graph.properties.copyOwnEnumerableProperties(Property(PropertyNull{}));
        const auto missingObject=std::get<opts::ObjectRef>(graph.properties.to(missing).data);
        const auto nullObject=std::get<opts::ObjectRef>(graph.properties.to(nullValue).data);
        expect(missingObject!=nullObject);
        expect(graph.context.enumerableStringKeys(missingObject).empty());
        expect(graph.context.enumerableStringKeys(nullObject).empty());
    };
};
}
