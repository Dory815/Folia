#include "support/folia_test.hpp"
import folia.math.core.mml_tree.attributes_bindings;
import folia.math.util.option_functions;

namespace {
using namespace boost::ut;
namespace o=folia::math::options;

struct Graph {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::ContextPropertyBindings properties{context,functions};
    std::shared_ptr<std::atomic_bool> lifetime=std::make_shared<std::atomic_bool>(true);
    folia::math::AttributesContextBindings attributes{context,functions,properties,lifetime};
};

suite attributesConstructorOrder=[] {
    "context_own_property_enumerable_does_not_read_native_field_value"_test=[] {
        o::Context context;
        int reads=0;
        auto owner=std::make_shared<int>(7);
        const auto object=context.object();
        context.bindData(object,o::Key(u"value"),std::make_shared<o::NativeDataField>(owner,
            [&]{++reads;return o::Value(*owner);},
            [&](o::Value value){*owner=static_cast<int>(std::get<double>(value.data));}));
        expect(context.ownPropertyEnumerable(o::Value(object),o::Key(u"value")));
        expect(reads==0);
        expect(context.get(o::Value(object),o::Key(u"value"))==o::Value(7));
        expect(reads==1);
    };

    "attributes_constructor_validates_global_before_getting_defaults"_test=[] {
        Graph graph;
        int reads=0;
        const auto defaults=graph.context.object();
        const auto getter=graph.context.function(u"get",[&](o::Context&,o::Value,std::span<const o::Value>){
            ++reads;return o::Value(u"unread");
        });
        graph.context.define(defaults,o::Key(u"value"),o::Descriptor::access(getter,std::nullopt,true,true));
        const std::array<o::Value,2> arguments{o::Value(defaults),o::Value(o::Undefined{})};
        expect(throws<o::Exception>([&]{(void)graph.context.construct(graph.attributes.constructor(),arguments);}));
        expect(reads==0);
        const auto globalSymbol=graph.context.symbol(u"invalid-global");
        const std::array<o::Value,2> symbolGlobalArguments{o::Value(defaults),o::Value(globalSymbol)};
        expect(throws<o::Exception>([&]{(void)graph.context.construct(graph.attributes.constructor(),symbolGlobalArguments);}));
        expect(reads==0);
    };

    "attributes_constructor_keeps_null_global_and_assigns_string_or_symbol_sources"_test=[] {
        Graph graph;
        const std::array<o::Value,2> stringArguments{o::Value(u"xy"),o::Value(o::Null{})};
        const auto stringOwner=graph.context.construct(graph.attributes.constructor(),stringArguments);
        const auto global=graph.context.get(o::Value(stringOwner),o::Key(u"global"));
        expect(std::holds_alternative<o::Null>(global.data));
        const auto defaults=std::get<o::ObjectRef>(graph.context.get(o::Value(stringOwner),o::Key(u"defaults")).data);
        expect(!graph.context.prototype(defaults).has_value());
        expect(graph.context.get(o::Value(defaults),o::Key(u"0"))==o::Value(u"x"));
        expect(graph.context.get(o::Value(defaults),o::Key(u"1"))==o::Value(u"y"));

        const auto symbol=graph.context.symbol(u"defaults-source");
        const std::array<o::Value,2> symbolArguments{o::Value(symbol),o::Value(graph.context.object())};
        const auto symbolOwner=graph.context.construct(graph.attributes.constructor(),symbolArguments);
        const auto symbolDefaults=std::get<o::ObjectRef>(graph.context.get(o::Value(symbolOwner),o::Key(u"defaults")).data);
        expect(graph.context.ownKeys(o::Value(symbolDefaults)).empty());

        const auto symbolKey=graph.context.symbol(u"enumerable-default");
        const auto symbolRecord=graph.context.object();
        graph.context.define(symbolRecord,o::Key(symbolKey),o::Descriptor::data(o::Value(u"symbol-value")));
        const std::array<o::Value,2> symbolRecordArguments{o::Value(symbolRecord),o::Value(graph.context.object())};
        const auto symbolRecordOwner=graph.context.construct(graph.attributes.constructor(),symbolRecordArguments);
        const auto symbolRecordDefaults=std::get<o::ObjectRef>(
            graph.context.get(o::Value(symbolRecordOwner),o::Key(u"defaults")).data);
        expect(static_cast<bool>(graph.context.ownSymbols(o::Value(symbolRecordDefaults))==std::vector<o::SymbolRef>{symbolKey}));
        expect(graph.context.get(o::Value(symbolRecordDefaults),o::Key(symbolKey))==o::Value(u"symbol-value"));

        const std::array<o::Value,4> boxedSources{
            o::Value(o::Undefined{}),o::Value(o::Null{}),o::Value(7.0),o::Value(false)};
        for(const auto& source:boxedSources){
            const std::array<o::Value,2> arguments{source,o::Value(graph.context.object())};
            const auto owner=graph.context.construct(graph.attributes.constructor(),arguments);
            const auto layer=std::get<o::ObjectRef>(graph.context.get(o::Value(owner),o::Key(u"defaults")).data);
            expect(graph.context.ownKeys(o::Value(layer)).empty());
        }
    };

    "attributes_constructor_get_and_set_order_uses_live_default_owner"_test=[] {
        Graph graph;
        std::vector<std::u16string> events;
        std::vector<o::Value> setterReceivers;
        const auto global=graph.context.object();
        const auto setter=graph.context.function(u"set",[&](o::Context&,o::Value receiver,std::span<const o::Value>){
            events.push_back(u"set:first");setterReceivers.push_back(std::move(receiver));return o::Value(o::Undefined{});
        });
        graph.context.define(global,o::Key(u"first"),o::Descriptor::access(std::nullopt,setter,true,true));

        const auto defaults=graph.context.object();
        const auto secondGetter=graph.context.function(u"get",[&](o::Context&,o::Value,std::span<const o::Value>){
            events.push_back(u"get:second");return o::Value(u"second-value");
        });
        const auto firstGetter=graph.context.function(u"get",[&](o::Context& context,o::Value,std::span<const o::Value>){
            events.push_back(u"get:first");
            context.define(defaults,o::Key(u"second"),o::Descriptor::access(secondGetter,std::nullopt,true,true));
            return o::Value(u"first-value");
        });
        graph.context.define(defaults,o::Key(u"first"),o::Descriptor::access(firstGetter,std::nullopt,true,true));
        graph.context.set(o::Value(defaults),o::Key(u"second"),o::Value(u"old-value"));

        const std::array<o::Value,2> arguments{o::Value(defaults),o::Value(global)};
        const auto instance=graph.context.construct(graph.attributes.constructor(),arguments);
        expect(static_cast<bool>(events==std::vector<std::u16string>{u"get:first",u"set:first",u"get:second"}));
        expect(setterReceivers.size()==1u);
        const auto target=graph.context.get(o::Value(instance),o::Key(u"defaults"));
        expect(setterReceivers[0]==target);
        expect(graph.context.get(target,o::Key(u"second"))==o::Value(u"second-value"));
    };
};
}
