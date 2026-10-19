#include "../../tests/support/folia_test.hpp"
import folia.math.output.svg.native_document;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::options;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

suite wrapperLengthConversion=[] {
    "common_output_jax_factory_is_a_live_canonical_owner_field"_test=[] {
        common::FontLoadHost host;
        host.asyncIsSynchronous=[] {return true;};
        host.warn=[](const std::any&){};
        svg::NativeRenderSession session(std::move(host));
        auto& context=session.context();
        auto first=session.construct();
        auto& factories=session.services().commonFactories();
        const auto firstFactory=factories.bind(first->factory);
        expect(context.get(Value(first->instanceIdentity()),Key(u"factory"))==Value(firstFactory));
        expect(factories.resolve(firstFactory)==first->factory);

        // Assigning the actual second factory through the Source field updates
        // the Jax owner and reads back through its canonical identity.
        const auto second=session.construct();
        const auto secondFactory=factories.bind(second->factory);
        context.set(Value(first->instanceIdentity()),Key(u"factory"),Value(secondFactory));
        expect(first->factory==second->factory);
        expect(context.get(Value(first->instanceIdentity()),Key(u"factory"))==Value(secondFactory));
    };

    "common_factory_repeat_binding_preserves_canonical_constructor_prototype_and_live_jax"_test=[] {
        common::FontLoadHost host;
        host.asyncIsSynchronous=[] {return true;};
        host.warn=[](const std::any&){};
        svg::NativeRenderSession session(std::move(host));
        auto& context=session.context();
        auto& bindings=session.services().commonFactories();
        const auto baseConstructor=bindings.constructor();
        const auto derivedConstructor=bindings.registerConstructor(
            u"DerivedWrapperFactory",baseConstructor,
            [](Context&,ObjectRef,std::span<const Value>){
                return std::make_shared<common::CommonWrapperFactory>();
            });

        // Constructing through the native constructor performs the initial
        // owner binding with the derived Source constructor.
        auto factory=bindings.construct(derivedConstructor);
        const auto identity=bindings.bind(factory);
        const auto expectedPrototype=std::get<ObjectRef>(context.get(
            Value(derivedConstructor),Key(u"prototype")).data);
        expect(context.prototype(identity)==expectedPrototype);
        expect(context.get(Value(identity),Key(u"constructor"))==Value(derivedConstructor));

        // The wrapper.factory getter can request the same owner repeatedly.
        // Preserve a Source-visible mutation while returning the original
        // object and its constructor chain.
        const auto mutatedJax=context.object();
        context.set(identity,Key(u"jax"),Value(mutatedJax));
        const auto repeated=bindings.bind(factory);
        expect(repeated==identity);
        expect(context.get(Value(repeated),Key(u"jax"))==Value(mutatedJax));
        expect(factory->jaxValue==Value(mutatedJax));
        expect(context.prototype(repeated)==expectedPrototype);
        expect(context.get(Value(repeated),Key(u"constructor"))==Value(derivedConstructor));
    };

    "common_wrapper_source_warning_preserves_console_argument_envelope"_test=[] {
        std::vector<std::vector<std::any>> consoleWarnings;
        std::vector<std::string> loadWarnings;
        common::FontLoadHost host;
        host.asyncIsSynchronous=[] {return true;};
        host.consoleWarn=[&](std::vector<std::any> arguments){
            consoleWarnings.push_back(std::move(arguments));
        };
        host.warn=[&](const std::any& value){
            if(const auto* text=std::any_cast<std::string>(&value))loadWarnings.push_back(*text);
        };
        host.warn(std::any(std::string("loader failure")));
        expect(loadWarnings==std::vector<std::string>{"loader failure"});
        expect(consoleWarnings.empty());
        svg::NativeRenderSession session(std::move(host));
        auto jax=session.construct();
        jax->font->sourceConsoleWarn({std::string("Invalid variant: -mhchem")});
        expect(consoleWarnings.size()==1_u);
        expect(consoleWarnings[0].size()==1_u);
        expect(std::any_cast<std::string>(consoleWarnings[0][0])=="Invalid variant: -mhchem");
        expect(loadWarnings==std::vector<std::string>{"loader failure"});
        expect(consoleWarnings.size()==1_u);
    };

    "common_wrapper_length_conversion_preserves_source_scalar_and_key_operations"_test=[] {
        common::FontLoadHost host;
        host.asyncIsSynchronous=[] {return true;};
        host.warn=[](const std::any&){};
        svg::NativeRenderSession session(std::move(host));
        auto& cx=session.context();
        const auto jax=session.construct();
        bool observed=false;
        jax->setLayoutObserver([&](const common::CommonWrapper& wrapper,const common::LayoutJob&){
            observed=true;
            expect(eq(wrapper.sourceLength2em(Property(0),1,1),0.0));
            expect(eq(wrapper.sourceLength2em(Property(-0.0),1,1),0.0));
            expect(eq(wrapper.sourceLength2em(Property(2),1,1),2.0));
            for(const auto& value:std::array<Property,6>{true,false,PropertyNull{},Missing{},
                    std::numeric_limits<double>::quiet_NaN(),""})
                expect(eq(wrapper.sourceLength2em(value,1,1),1.0));
            expect(eq(wrapper.sourceLength2em(Property(cx.symbol(u"length")),1,1),1.0));
            const auto thickness=wrapper.font->params().rule_thickness;
            expect(eq(wrapper.sourceLength2em(Property("medium"),1,1),thickness));
            expect(eq(wrapper.sourceLength2em(Property("thin"),1,1),(2.0/3.0)*thickness));
            expect(eq(wrapper.sourceLength2em(Property("2px"),1,1),2/jax->pxPerEm));
            for(const auto& keys:std::array{
                    std::array<std::u16string,2>{u"medium",u"thin"},
                    std::array<std::u16string,2>{u"other",u"2px"}}){
                const auto object=cx.object();std::size_t calls=0;
                const auto toString=cx.function(u"toString",[&](Context&,Value self,std::span<const Value>)->Value{
                    expect(self==Value(object));
                    return calls<keys.size()?Value(keys[calls++]):Value(u"unexpected conversion");
                });
                cx.set(object,Key(u"toString"),toString);
                const auto property=session.services().parser().properties.from(Value(object));
                const auto actual=wrapper.sourceLength2em(property,1,1);
                expect(eq(actual,keys[0]==u"medium"?(2.0/3.0)*thickness:2/jax->pxPerEm));
                expect(calls==2_u);
            }
        });
        const auto document=svg::createNativeSvgDocument(session);
        (void)document->convert(Value(u"x"));
        expect(observed);
        jax->setLayoutObserver({});
    };
};
}
