#include "support/folia_test.hpp"
import folia.math;
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

struct Graph {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::regexp_host::RegExpBindings regexp{context};
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input{context};
    folia::math::core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    t::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
};

suite mmlTokenWalkTree = [] {
    "native token walk filters TextNode while retaining MML descendants"_test = [] {
        auto factory=std::make_shared<m::MmlFactory>();
        auto* math=factory->create("math",tree::FactoryArgs{});
        auto* token=factory->create("mi",tree::FactoryArgs{});
        auto* text=dynamic_cast<m::TextNode*>(factory->create("text",tree::FactoryArgs{}));
        expect(math!=nullptr&&token!=nullptr&&text!=nullptr);
        if(!math||!token||!text)return;
        const bool startsWithInferredRow=math->childArrayOwner()&&
            math->childArrayOwner()->size()==1u&&math->childArrayOwner()->get(0).nodeOrNull()&&
            math->childArrayOwner()->get(0).nodeOrNull()->kind()=="inferredMrow";
        expect(startsWithInferredRow) << "math construction creates its Source inferredMrow child";
        if(!startsWithInferredRow)return;
        auto* const inferred=math->childArrayOwner()->get(0).nodeOrNull();
        text->setText(u"x");
        token->appendChild(m::NodeValue(text));
        inferred->appendChild(m::NodeValue(token));

        std::vector<std::string> visited;
        const tree::TreeVisitor<m::MmlNode> visitor=[&](m::MmlNode* node,const std::any&) {
            visited.push_back(node->kind());
        };
        (void)math->walkTree(visitor);
        expect(visited==std::vector<std::string>{"math","inferredMrow","mi"})
            << "Source math traversal includes inferredMrow and filters the token TextNode";
    };

    "Context token walk uses canonical MML membership and preserves Source call order"_test = [] {
        Graph graph;
        auto factory=std::make_shared<m::MmlFactory>();
        const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
        const auto factoryMap=std::get<o::ObjectRef>(graph.context.get(factoryIdentity,o::Key(u"nodeMap")).data);
        const auto tokenConstructor=std::get<o::ObjectRef>(graph.runtime.maps.resolve(factoryMap)->get(o::Value(u"mi")).data);
        const auto tokenPrototype=std::get<o::ObjectRef>(graph.context.get(o::Value(tokenConstructor),o::Key(u"prototype")).data);
        const auto tokenBase=graph.context.prototype(tokenPrototype);
        expect(tokenBase.has_value());
        if(!tokenBase)return;
        const auto mmlBase=graph.context.prototype(*tokenBase);
        expect(mmlBase.has_value());
        if(!mmlBase)return;

        auto* textNative=dynamic_cast<m::TextNode*>(factory->create("text",tree::FactoryArgs{}));
        expect(textNative!=nullptr);
        if(!textNative)return;
        const auto textIdentity=graph.runtime.mmlNodeIdentities.findPointee(textNative);
        expect(textIdentity.has_value());
        if(!textIdentity)return;

        const auto tokenReceiver=graph.context.objectWithPrototype(tokenPrototype);
        const auto mmlChild=graph.context.objectWithPrototype(*mmlBase);
        const auto data=graph.context.object();
        const auto childNodes=graph.context.object();
        const auto events=std::make_shared<std::vector<std::string>>();
        auto closeCalls=std::make_shared<std::size_t>(0);
        auto walkGets=std::make_shared<std::size_t>(0);
        auto childCalls=std::make_shared<std::size_t>(0);
        auto throwOnChild=std::make_shared<bool>(false);

        const auto visitor=graph.context.function(u"visitor",
            [events,closeCalls,throwOnChild,tokenReceiver,mmlChild,data](o::Context&,o::Value thisValue,
                    std::span<const o::Value> args)->o::Value {
                if(!std::holds_alternative<o::Undefined>(thisValue.data))
                    events->push_back("visitor-this-not-undefined");
                if(args.size()!=2||args[1]!=o::Value(data))
                    events->push_back("visitor-data-mismatch");
                if(!args.empty()) {
                    const auto* node=std::get_if<o::ObjectRef>(&args[0].data);
                    if(node&&*node==tokenReceiver)events->push_back("visit-token");
                    else if(node&&*node==mmlChild) {
                        events->push_back("visit-mml-child");
                        if(*throwOnChild)throw o::Exception("TypeError",u"visitor failure at child");
                    } else events->push_back("visit-unexpected-node");
                }
                return args.size()>1?args[1]:o::Value(o::Undefined{});
            });
        const auto childWalk=graph.context.function(u"walkTree",
            [events,childCalls,visitor,data,mmlChild](o::Context& context,o::Value receiver,
                    std::span<const o::Value> args)->o::Value {
                events->push_back("child.walkTree:call");
                ++*childCalls;
                if(receiver!=o::Value(mmlChild)||args.size()!=2||args[0]!=o::Value(visitor)||args[1]!=o::Value(data))
                    events->push_back("child.walkTree:receiver-or-arguments-mismatch");
                const std::array<o::Value,2> visitArgs{receiver,o::Value(data)};
                return context.call(visitor,o::Value(o::Undefined{}),visitArgs);
            });
        const auto childWalkGetter=graph.context.function(u"get walkTree",
            [events,walkGets,childWalk](o::Context&,o::Value,std::span<const o::Value>)->o::Value {
                events->push_back("child.walkTree:get");
                ++*walkGets;
                return o::Value(childWalk);
            });
        graph.context.define(mmlChild,o::Key(u"walkTree"),o::Descriptor::access(childWalkGetter,std::nullopt,true,true));

        const auto iteratorMethod=graph.context.function(u"[Symbol.iterator]",
            [events,closeCalls,text=*textIdentity,mmlChild](o::Context& context,o::Value,
                    std::span<const o::Value>)->o::Value {
                events->push_back("iterator:create");
                auto cursor=std::make_shared<std::size_t>(0);
                const auto iterator=context.nativeIterator([events,cursor,text,mmlChild](o::Context& inner)->o::Value {
                    const std::size_t index=(*cursor)++;
                    events->push_back(index==0?"iterator:next-text":index==1?"iterator:next-mml":"iterator:done");
                    const auto result=inner.object();
                    const bool done=index>=2;
                    const o::Value value=index==0?o::Value(text):index==1?o::Value(mmlChild):o::Value(o::Undefined{});
                    inner.define(result,o::Key(u"value"),o::Descriptor::data(value));
                    inner.define(result,o::Key(u"done"),o::Descriptor::data(o::Value(done)));
                    return o::Value(result);
                });
                const auto close= context.function(u"return",[events,closeCalls](o::Context& cx,o::Value,
                        std::span<const o::Value>)->o::Value {
                    events->push_back("iterator:return");
                    ++*closeCalls;
                    const auto result=cx.object();
                    cx.define(result,o::Key(u"done"),o::Descriptor::data(o::Value(true)));
                    return o::Value(result);
                });
                context.define(iterator,o::Key(u"return"),o::Descriptor::data(o::Value(close)));
                return o::Value(iterator);
            });
        graph.context.define(childNodes,o::Key(graph.context.intrinsics().symbolIterator),o::Descriptor::data(o::Value(iteratorMethod)));
        graph.context.define(tokenReceiver,o::Key(u"childNodes"),o::Descriptor::data(o::Value(childNodes)));

        const auto walk=graph.context.get(o::Value(tokenReceiver),o::Key(u"walkTree"));
        const std::array<o::Value,2> walkArgs{o::Value(visitor),o::Value(data)};
        const auto returned=graph.context.callMember(walk,o::Value(tokenReceiver),walkArgs,u"token.walkTree");
        expect(returned==o::Value(data));
        expect(*walkGets==1u&&*childCalls==1u) << "valid AbstractMmlNode child is called dynamically";
        expect(*closeCalls==0u) << "normal iteration exhausts without calling iterator.return";
        expect(*events==std::vector<std::string>{"visit-token","iterator:create","iterator:next-text",
            "iterator:next-mml","child.walkTree:get","child.walkTree:call","visit-mml-child","iterator:done"})
            << "TextNode is skipped by canonical AbstractMmlNode instance check and Source visitor order is preserved";

        events->clear();
        *throwOnChild=true;
        bool propagated=false;
        try { (void)graph.context.callMember(walk,o::Value(tokenReceiver),walkArgs,u"token.walkTree"); }
        catch(const o::Exception& error) { propagated=error.name=="TypeError"; }
        expect(propagated) << "visitor exception propagates";
        expect(*closeCalls==1u) << "for-of iterator.return runs when child traversal throws";
        expect(!events->empty()&&events->back()=="iterator:return") << "iterator closes after the callback exception";
    };

    "public Renderer renders x and returned Scene survives Renderer destruction"_test = [] {
        std::optional<folia::math::output::scene::Scene> detached;
        bool rendered=false;
        try {
            folia::math::Renderer renderer;
            detached.emplace(renderer.renderScene("x"));
            rendered=true;
        } catch(const std::exception&) {
        }
        expect(rendered) << "public Renderer completes a simple x render";
        expect(detached.has_value()) << "Renderer produced an owned Scene";
        if(detached) {
            const auto& roots=detached->roots();
            expect(!roots.empty()) << "Scene roots remain readable after Renderer destruction";
            if(!roots.empty())expect(static_cast<bool>(roots.front())) << "frozen root remains owned after Renderer destruction";
        }
    };
};
} // namespace
