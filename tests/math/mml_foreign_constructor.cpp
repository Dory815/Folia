#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.core.mml_tree.mml_nodes.mrow;

namespace {
using namespace boost::ut;
namespace t=folia::math::tex;
namespace o=folia::math::options;
namespace m=folia::math::mml;

struct RuntimeGraph {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::regexp_host::RegExpBindings regexp{context};
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input{context};
    folia::math::core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    t::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
};

suite foreignMmlConstructor = [] {
  "foreign_factory_new_and_call_allocate_one_native_descriptor_product"_test = [] {
    RuntimeGraph graph;
    auto allocationOwner=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=graph.runtime.bindMmlFactory(allocationOwner);
    const auto nodeMap=std::get<o::ObjectRef>(graph.context.get(factoryIdentity,o::Key(u"nodeMap")).data);
    const auto mi=std::get<o::ObjectRef>(graph.runtime.maps.resolve(nodeMap)->get(o::Value(u"mi")).data);
    const auto math=std::get<o::ObjectRef>(graph.runtime.maps.resolve(nodeMap)->get(o::Value(u"math")).data);
    std::vector<std::u16string> events;
    const auto foreign=graph.context.object();
    const auto getNodeClass=graph.context.function(u"getNodeClass",
        [&](o::Context&,o::Value receiver,std::span<const o::Value> args){
          expect(receiver==o::Value(foreign));
          expect(args.size()==1u);
          const auto* name=std::get_if<std::u16string>(&args[0].data);
          expect(name!=nullptr);
          if(name){events.push_back(u"call:"+*name);return o::Value(*name==u"math"?math:mi);}
          return o::Value(o::Undefined{});
        });
    const auto create=graph.context.function(u"create",
        [&](o::Context& context,o::Value receiver,std::span<const o::Value> args){
          expect(receiver==o::Value(foreign));
          expect(args.size()>=1u);
          const auto* name=std::get_if<std::u16string>(&args[0].data);
          expect(name!=nullptr);
          if(!name)return o::Value(o::Undefined{});
          events.push_back(u"call:create:"+*name);
          const auto selected=*name==u"math"?math:mi;
          const std::array<o::Value,3> constructorArgs{
              o::Value(foreign),o::Value(o::Undefined{}),o::Value(context.array(0))};
          return o::Value(context.construct(selected,constructorArgs));
        });
    const auto getNodeClassGetter=graph.context.function(u"get",
        [&](o::Context&,o::Value,std::span<const o::Value>){
          events.push_back(u"get:getNodeClass");return o::Value(getNodeClass);
        });
    const auto createGetter=graph.context.function(u"get",
        [&](o::Context&,o::Value,std::span<const o::Value>){
          events.push_back(u"get:create");return o::Value(create);
        });
    graph.context.define(foreign,o::Key(u"getNodeClass"),
        o::Descriptor::access(getNodeClassGetter,std::nullopt,false,true));
    graph.context.define(foreign,o::Key(u"create"),
        o::Descriptor::access(createGetter,std::nullopt,false,true));
    const auto properties=graph.context.object();
    graph.context.set(o::Value(properties),o::Key(u"mathvariant"),o::Value(u"bold"));
    const auto children=graph.context.array(0);
    const std::array<o::Value,3> args{o::Value(foreign),o::Value(properties),o::Value(children)};

    const auto created=graph.context.construct(mi,args);
    expect(graph.runtime.resolveMmlNode(created).kind()=="mi");
    expect(graph.context.get(o::Value(created),o::Key(u"factory"))==o::Value(foreign));
    expect(events==std::vector<std::u16string>{
        u"get:getNodeClass",u"call:mi",u"get:getNodeClass",u"call:math"});

    const auto defaultMiPrototype=std::get<o::ObjectRef>(
        graph.context.get(o::Value(mi),o::Key(u"prototype")).data);
    const auto incompletePrototype=graph.context.object();
    graph.context.set(o::Value(mi),o::Key(u"prototype"),o::Value(incompletePrototype));
    expect(throws<o::Exception>([&]{(void)graph.context.construct(mi,args);}));
    const auto replacementPrototype=graph.context.objectWithPrototype(defaultMiPrototype);
    graph.context.set(o::Value(mi),o::Key(u"prototype"),o::Value(replacementPrototype));
    const auto replacementNode=graph.context.construct(mi,args);
    expect(graph.context.prototype(replacementNode)==std::optional<o::ObjectRef>(replacementPrototype));
    graph.context.set(o::Value(mi),o::Key(u"prototype"),o::Value(17));
    const auto objectPrototype=graph.context.intrinsics().objectPrototype;
    const auto fallbackSetChildren=graph.context.function(u"setChildren",
        [](o::Context&,o::Value,std::span<const o::Value>){return o::Value(o::Undefined{});});
    graph.context.define(objectPrototype,o::Key(u"setChildren"),
        o::Descriptor::data(o::Value(fallbackSetChildren),true,false,true));
    graph.context.define(objectPrototype,o::Key(u"kind"),
        o::Descriptor::data(o::Value(u"mi"),true,false,true));
    const auto fallbackNode=graph.context.construct(mi,args);
    expect(graph.context.prototype(fallbackNode)==std::optional<o::ObjectRef>(objectPrototype));
    graph.context.set(o::Value(mi),o::Key(u"prototype"),o::Value(defaultMiPrototype));

    events.clear();
    const auto miPrototype=defaultMiPrototype;
    const auto suppliedPrototype=graph.context.objectWithPrototype(miPrototype);
    const auto decoyFactory=graph.context.object();
    const auto decoyGetNodeClass=graph.context.function(u"getNodeClass",
        [&](o::Context&,o::Value,std::span<const o::Value> args){
          events.push_back(u"decoy:call:getNodeClass");
          const auto* name=args.empty()?nullptr:std::get_if<std::u16string>(&args[0].data);
          return o::Value(name&&*name==u"math"?math:mi);
        });
    const auto decoyGetter=graph.context.function(u"get",
        [&](o::Context&,o::Value,std::span<const o::Value>){
          events.push_back(u"decoy:get:getNodeClass");return o::Value(decoyGetNodeClass);
        });
    graph.context.define(decoyFactory,o::Key(u"getNodeClass"),
        o::Descriptor::access(decoyGetter,std::nullopt,false,true));
    o::Value assignedFactory{o::Undefined{}};
    const auto receiverFactoryGetter=graph.context.function(u"get",
        [&](o::Context&,o::Value,std::span<const o::Value>){return o::Value(decoyFactory);});
    const auto receiverFactorySetter=graph.context.function(u"set",
        [&](o::Context&,o::Value,std::span<const o::Value> values){
          assignedFactory=values.empty()?o::Value(o::Undefined{}):values.front();
          return o::Value(o::Undefined{});
        });
    graph.context.define(suppliedPrototype,o::Key(u"factory"),
        o::Descriptor::access(receiverFactoryGetter,receiverFactorySetter,false,true));
    const auto receiver=graph.context.objectWithPrototype(suppliedPrototype);
    const auto result=graph.context.call(mi,o::Value(receiver),args);
    expect(result==o::Value(receiver));
    expect(graph.runtime.resolveMmlNode(receiver).kind()=="mi");
    expect(assignedFactory==o::Value(foreign));
    expect(graph.context.get(o::Value(receiver),o::Key(u"factory"))==o::Value(decoyFactory));
    expect(graph.context.prototype(receiver)==std::optional<o::ObjectRef>(suppliedPrototype));
    expect(events==std::vector<std::u16string>{
        u"get:getNodeClass",u"call:mi",u"get:getNodeClass",u"call:math"});

    events.clear();
    const auto failingFactory=graph.context.object();
    const auto failingLookup=graph.context.function(u"getNodeClass",
        [&](o::Context&,o::Value receiverValue,std::span<const o::Value> lookupArgs)->o::Value{
          expect(receiverValue==o::Value(failingFactory));
          const auto* name=lookupArgs.empty()?nullptr:std::get_if<std::u16string>(&lookupArgs[0].data);
          if(name&&*name==u"mi")throw o::Exception("TypeError",u"lookup stopped");
          return o::Value(math);
        });
    graph.context.set(o::Value(failingFactory),o::Key(u"getNodeClass"),o::Value(failingLookup));
    graph.context.set(o::Value(failingFactory),o::Key(u"create"),o::Value(create));
    const auto failedReceiver=graph.context.objectWithPrototype(miPrototype);
    const std::array<o::Value,3> failedArgs{
        o::Value(failingFactory),o::Value(graph.context.object()),o::Value(graph.context.array(0))};
    expect(throws<o::Exception>([&]{
        (void)graph.context.call(mi,o::Value(failedReceiver),failedArgs);
    }));
    expect(graph.context.hasOwn(failedReceiver,o::Key(u"factory")));
    expect(graph.context.hasOwn(failedReceiver,o::Key(u"prevClass")));
    expect(graph.context.hasOwn(failedReceiver,o::Key(u"childNodes")));
    expect(!graph.context.hasOwn(failedReceiver,o::Key(u"attributes")));
  };
};
}
