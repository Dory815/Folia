// Factory method checks use the production Context and module owners.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory_context_methods;
import folia.math.util.context_property_bindings;
import folia.math.util.native_identities;
import folia.math.util.option_functions;
import folia.math.input.tex.parser;

namespace {
using namespace boost::ut;
namespace opts=folia::math::options;
namespace math=folia::math;
namespace mml=folia::math::mml;

struct Fixture {
    opts::Context context;
    math::OptionFunctionBindings functions{context};
    math::regexp_host::RegExpBindings regexp{context};
    math::Unicode16Normalization unicode;
    math::core::InputJaxModule input{context};
    math::core::FindMathModule findMath{context};
    math::tex::FindTeXModule findTeX{context,findMath,regexp};
    math::tex::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
    math::ContextPropertyBindings& properties=runtime.properties;
    math::NativeIdentityBindings<mml::MmlNode*>& nodes=runtime.mmlNodeIdentities;
    std::shared_ptr<mml::MmlFactory> factory=std::make_shared<mml::MmlFactory>();
    opts::ObjectRef factoryObject=runtime.bindMmlFactory(factory);
    mml::MmlFactoryContextMethods& methods=runtime.mmlFactoryContextMethods;
};

suite mml_factory_context_methods=[] {
    "mml_factory_create_uses_actual_owner_and_canonical_node_identity"_test=[] {
        Fixture f;
        const auto create=f.context.get(f.factoryObject,opts::Key(u"create"));
        const auto immediatePrototype=f.context.prototype(f.factoryObject);
        const bool hasExpectedImmediatePrototype=immediatePrototype&&
            *immediatePrototype==f.methods.prototype();
        expect(hasExpectedImmediatePrototype);
        if(!immediatePrototype)return;
        const auto abstractPrototype=f.context.prototype(*immediatePrototype);
        const bool hasAbstractPrototype=abstractPrototype.has_value();
        expect(hasAbstractPrototype);
        if(!abstractPrototype)return;
        const auto sourceCreateDescriptor=f.context.descriptor(*abstractPrototype,opts::Key(u"create"));
        expect(sourceCreateDescriptor&&sourceCreateDescriptor->writable&&
               sourceCreateDescriptor->enumerable&&sourceCreateDescriptor->configurable);
        const auto functionLength=f.context.get(create,opts::Key(u"length"));
        const auto functionName=f.context.get(create,opts::Key(u"name"));
        expect(functionLength==opts::Value(3));
        expect(functionName==opts::Value(u""));

        const auto childRef=std::get<opts::ObjectRef>(f.context.callMember(
            create,f.factoryObject,std::array<opts::Value,1>{opts::Value(u"mi")} ).data);
        auto inputChildren=f.context.array(1);
        f.context.set(inputChildren,opts::Key(u"0"),opts::Value(childRef));
        auto inputProperties=f.context.object();
        f.context.set(inputProperties,opts::Key(u"mathvariant"),opts::Value(u"italic"));
        const std::array<opts::Value,3> args{
            opts::Value(u"mrow"),opts::Value(inputProperties),opts::Value(inputChildren)};
        const auto created=std::get<opts::ObjectRef>(f.context.callMember(
            create,f.factoryObject,args).data);
        auto* node=f.nodes.resolve(created);
        const bool hasNode=node!=nullptr;
        expect(hasNode);
        if(!node)return;
        expect(node->kind()=="mrow");
        const bool hasChildren=node->childNodes!=nullptr;
        expect(hasChildren);
        if(!node->childNodes)return;
        expect(node->childNodes->size()==1);
        if(node->childNodes->size()!=1)return;
        auto* child=f.nodes.resolve(childRef);
        const bool hasChild=child!=nullptr;
        expect(hasChild);
        if(!child)return;
        expect(node->childNodes->get(0).nodeOrNull()==f.nodes.resolve(childRef));
        expect(child->getParent().nodeOrNull()==node);
        expect(node->attributes()->get("mathvariant")==math::Property("italic"));
        expect(created!=f.methods.prototype());
    };

    "mml_factory_create_rejects_bad_kind_before_consuming_children"_test=[] {
        Fixture f;
        auto create=f.context.get(f.factoryObject,opts::Key(u"create"));
        int iteratorReads=0;
        auto input=f.context.object();
        f.context.set(input,opts::Key(u"length"),opts::Value(1));
        auto iterator=f.context.object();
        f.context.set(iterator,opts::Key(u"next"),f.context.function(u"next",
            [&iteratorReads](auto&,auto,std::span<const opts::Value>)->opts::Value{
                ++iteratorReads;return opts::Value(opts::ObjectRef{});
            }));
        auto iteratorMethod=f.context.function(u"iterator",
            [iterator](auto&,auto,std::span<const opts::Value>){return opts::Value(iterator);});
        f.context.define(input,opts::Key(f.context.intrinsics().symbolIterator),
            opts::Descriptor::data(opts::Value(iteratorMethod)));
        bool threw=false;
        try {
            const std::array<opts::Value,3> args{
                opts::Value(u"missing-kind"),opts::Value(f.context.object()),opts::Value(input)};
            (void)f.context.callMember(create,f.factoryObject,args);
        } catch(const opts::Exception& error) {threw=error.name=="TypeError";}
        expect(threw);
        expect(iteratorReads==0);
    };
};
}
