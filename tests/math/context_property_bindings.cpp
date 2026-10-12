#include "support/folia_test.hpp"
import folia.math.util.context_property_bindings;
import folia.math.input.tex.node_util;
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts=folia::math::options;
struct Graph {
    opts::Context context;
    OptionFunctionBindings functions{context};
    ContextPropertyBindings properties{context,functions};
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
        const auto map=tex::TokenMapHost::create<tex::CharacterMap>(
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
};
}
