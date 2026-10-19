#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.core.mml_tree.mml_factory_context_values;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
namespace o=folia::math::options;
namespace t=folia::math::tex;
namespace m=folia::math::mml;
namespace tree=folia::math::tree;

struct SharedGraph {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::regexp_host::RegExpBindings regexp{context};
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input{context};
    folia::math::core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    folia::math::util::RuntimeModules modules{context};
    std::unique_ptr<t::ParserRuntime> parser(){
        return std::make_unique<t::ParserRuntime>(context,functions,regexp,unicode,input,findTeX,&modules);
    }
};

suite mmlSharedContextModule = [] {
    "shared canonical MML classes outlive first Parser and foreign receivers keep Source calls"_test = [] {
        SharedGraph graph;
        auto first=graph.parser();
        first->initializeParserRuntime();
        const auto corePrototype=first->treeNodeClasses.abstractNodePrototype();
        const auto coreWalkBefore=graph.context.get(o::Value(corePrototype),o::Key(u"walkTree"));
        const auto attrsConstructor=first->attributeBindings.constructor();
        auto firstFactory=std::make_shared<m::MmlFactory>();
        const auto firstFactoryIdentity=first->bindMmlFactory(firstFactory);
        const auto firstMap=std::get<o::ObjectRef>(graph.context.get(firstFactoryIdentity,o::Key(u"nodeMap")).data);
        const auto firstMi=std::get<o::ObjectRef>(first->maps.resolve(firstMap)->get(o::Value(u"mi")).data);
        const auto canonicalMiPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(firstMi),o::Key(u"prototype")).data);
        const auto firstTokenPrototype=graph.context.prototype(canonicalMiPrototype);
        expect(firstTokenPrototype.has_value());
        if(!firstTokenPrototype)return;
        const auto firstMmlPrototype=graph.context.prototype(*firstTokenPrototype);
        expect(firstMmlPrototype.has_value());
        if(!firstMmlPrototype)return;
        const auto firstCorePrototype=graph.context.prototype(*firstMmlPrototype);
        expect(firstCorePrototype.has_value()&&*firstCorePrototype==corePrototype)
            << "mi -> token -> AbstractMmlNode -> canonical Core AbstractNode chain";

        auto second=graph.parser();
        second->initializeParserRuntime();
        expect(second->treeNodeClasses.abstractNodePrototype()==corePrototype);
        expect(second->attributeBindings.constructor()==attrsConstructor);
        first.reset();
        expect(second->treeNodeClasses.abstractNodePrototype()==corePrototype)
            << "live second Parser continues using the first-linked Context prototype";
        expect(graph.context.get(o::Value(corePrototype),o::Key(u"walkTree"))==coreWalkBefore)
            << "first-link Core method identity remains stable after owner teardown";

        auto factory=std::make_shared<m::MmlFactory>();
        const auto factoryIdentity=second->bindMmlFactory(factory);
        const auto nodeMap=std::get<o::ObjectRef>(graph.context.get(factoryIdentity,o::Key(u"nodeMap")).data);
        const auto mi=std::get<o::ObjectRef>(second->maps.resolve(nodeMap)->get(o::Value(u"mi")).data);
        const auto tokenPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(mi),o::Key(u"prototype")).data);
        (void)nodeMap;

        const auto foreign=graph.context.objectWithPrototype(tokenPrototype);
        auto* textNative=dynamic_cast<m::TextNode*>(factory->create("text",tree::FactoryArgs{}));
        expect(textNative!=nullptr);
        if(!textNative)return;
        textNative->setText(u"x");
        const auto textIdentity=second->mmlNodeIdentities.findPointee(textNative);
        expect(textIdentity.has_value());
        if(!textIdentity)return;
        const auto childArray=graph.context.array(1);
        graph.context.set(o::Value(childArray),o::Key(u"0"),o::Value(*textIdentity));
        graph.context.set(o::Value(foreign),o::Key(u"childNodes"),o::Value(childArray));
        const auto textContentChild=graph.context.object();
        const auto convertedText=graph.context.object();
        const auto toPrimitive=graph.context.function(u"toPrimitive",[convertedText](o::Context&,o::Value self,std::span<const o::Value> args){
            expect(self==o::Value(convertedText)) << "addition converts textContent result with its own receiver";
            expect(args.size()==1u&&args.front()==o::Value(u"default")) << "Source += uses default ToPrimitive hint";
            return o::Value(std::u16string(u"y"));
        });
        graph.context.set(o::Value(convertedText),o::Key(graph.context.intrinsics().symbolToPrimitive),o::Value(toPrimitive));
        const auto textContentCalls=std::make_shared<std::size_t>(0);
        const auto textContentMethod=graph.context.function(u"textContent",[textContentChild,convertedText,textContentCalls](o::Context&,o::Value self,std::span<const o::Value>){
            expect(self==o::Value(textContentChild)) << "Source invokes textContent with the child receiver";
            ++*textContentCalls;
            return o::Value(convertedText);
        });
        graph.context.set(o::Value(textContentChild),o::Key(u"textContent"),o::Value(textContentMethod));
        graph.context.set(o::Value(childArray),o::Key(u"1"),o::Value(textContentChild));
        const auto events=std::make_shared<std::vector<o::Value>>();
        const auto callback=graph.context.function(u"visit",[events](o::Context&,o::Value self,std::span<const o::Value> args){
            events->push_back(args.empty()?o::Value(o::Undefined{}):args.front());
            expect(std::holds_alternative<o::Undefined>(self.data)) << "Source walk calls visitor as a plain function";
            return o::Value(o::Undefined{});
        });
        const auto walk=graph.context.get(o::Value(foreign),o::Key(u"walkTree"));
        const std::array<o::Value,2> args{o::Value(callback),o::Value(u"payload")};
        (void)graph.context.call(std::get<o::ObjectRef>(walk.data),o::Value(foreign),args);
        expect(events->size()==1u&&events->front()==o::Value(foreign))
            << "ordinary foreign receiver executes shared Source token walk";
        const auto getText=graph.context.get(o::Value(foreign),o::Key(u"getText"));
        const auto text=graph.context.call(std::get<o::ObjectRef>(getText.data),o::Value(foreign),{});
        expect(text==o::Value(std::u16string(u"xy")))
            << "shared Source getText reads TextNode.getText and ordinary textContent";
        expect(*textContentCalls==1u) << "ordinary textContent getter method is called once";
    };
};
}
