#include "support/folia_test.hpp"
import folia.math.output.common;
import folia.math.output.svg.render_session;
import folia.math.util.option_functions;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o=folia::math::options;
namespace t=folia::math::tex;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

common::FontLoadHost fontHost(){
  common::FontLoadHost host;
  host.asyncIsSynchronous=[] { return true; };
  host.warn=[](const std::any&) {};
  return host;
}

suite function_list_native_argument_values=[] {
  "function_list_context_callbacks_roundtrip_canonical_parser_arguments"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& graph=session.services();
    auto& runtime=graph.parser();
    auto& context=graph.context();
    const auto jax=t::TeX::make(runtime);
    const auto packageNames=context.array();
    const auto parserCtor=std::get<o::ObjectRef>(runtime.parserConfigurationClass.data);
    const auto parserObject=context.construct(parserCtor,std::array<o::Value,1>{o::Value(packageNames)});
    const auto parser=runtime.parserConfigurationIdentities.resolve(parserObject);

    std::vector<t::ParserConfigurationRef> seenConfigurations;
    std::vector<t::TeX*> seenJaxes;
    auto body=std::make_shared<Function>();
    body->sourceMetadata=FunctionSourceMetadata{u"probe",2};
    body->call=[&](const FunctionReceiver&,FunctionArgs args)->std::any {
      if(args.size()!=2)throw o::Exception("TypeError",u"probe expected two typed arguments");
      const auto* configuration=std::any_cast<t::ParserConfigurationRef>(&args[0]);
      const auto* jaxPointer=std::any_cast<t::TeX*>(&args[1]);
      if(!configuration||!jaxPointer)throw o::Exception("TypeError",u"probe received noncanonical argument types");
      seenConfigurations.push_back(*configuration);
      seenJaxes.push_back(*jaxPointer);
      return std::monostate{};
    };
    OptionCallBinding binding;
    binding.receiver=[](o::Context&,o::Value receiver){return FunctionReceiver(std::move(receiver));};
    binding.arguments=[&runtime](o::Context&,std::span<const o::Value> args){
      if(args.size()!=2)throw o::Exception("TypeError",u"probe expected two Source arguments");
      const auto* configuration=std::get_if<o::ObjectRef>(&args[0].data);
      const auto* jax=std::get_if<o::ObjectRef>(&args[1].data);
      if(!configuration||!jax)throw o::Exception("TypeError",u"probe expected canonical object arguments");
      return std::vector<std::any>{runtime.parserConfigurationIdentities.resolve(*configuration),
                                   runtime.texIdentities.resolve(*jax).get()};
    };
    binding.result=[](o::Context&,const std::any&){return o::Value(o::Undefined{});};
    const auto callback=runtime.functions().bind(body,std::move(binding),u"probe");
    const auto list=parser->sourceField(u"configMethod");
    const auto add=context.get(list,o::Key(u"add"));
    (void)context.callMember(add,list,std::array<o::Value,2>{callback,o::Value(3.0)});

    // The Source call and native config path must reach the same original typed callback.
    const auto jaxIdentity=runtime.texIdentities.find(jax);
    expect(jaxIdentity.has_value());
    if(!jaxIdentity)return;
    (void)context.call(std::get<o::ObjectRef>(callback.data),o::Value(o::Undefined{}),
        std::array<o::Value,2>{o::Value(parserObject),o::Value(*jaxIdentity)});
    expect(seenConfigurations.size()==1u);
    expect(seenJaxes.size()==1u);
    if(seenConfigurations.empty()||seenJaxes.empty())return;
    expect(seenConfigurations[0]==parser);
    expect(seenJaxes[0]==jax.get());

    const auto loadedBboldx=runtime.configurationHandler.get(u"bboldx");
    const auto* bboldx=std::get_if<t::ConfigurationRef>(&loadedBboldx);
    expect(bboldx!=nullptr&&*bboldx);
    if(bboldx&&*bboldx){
      const auto bboldxCallback=(*bboldx)->config();
      const auto* bboldxFunction=std::get_if<o::ObjectRef>(&bboldxCallback.data);
      expect(bboldxFunction!=nullptr);
      if(bboldxFunction)(void)context.call(*bboldxFunction,o::Value(o::Undefined{}),
          std::array<o::Value,2>{o::Value(parserObject),o::Value(*jaxIdentity)});
    }

    parser->config(*jax);
    // bboldx has a native config callback too; its registered codec must decode
    // these same canonical Source arguments to ParserConfigurationRef and TeX*.
    parser->add(u"bboldx",*jax);
    expect(seenConfigurations.size()==2u);
    expect(seenJaxes.size()==2u);
    if(seenConfigurations.size()==2u&&seenJaxes.size()==2u){
      expect(seenConfigurations[1]==seenConfigurations[0]);
      expect(seenJaxes[1]==seenJaxes[0]);
    }

    bool sawPrimitiveTypes=false;
    auto primitiveBody=std::make_shared<Function>();
    primitiveBody->call=[&](const FunctionReceiver&,FunctionArgs args)->std::any {
      if(args.size()==3){
        const auto* flag=std::any_cast<bool>(&args[0]);
        const auto* number=std::any_cast<double>(&args[1]);
        const auto* text=std::any_cast<std::u16string>(&args[2]);
        sawPrimitiveTypes=flag&&*flag&&number&&*number==7.0&&text&&*text==u"source";
      }
      return std::monostate{};
    };
    OptionCallBinding primitiveBinding;
    primitiveBinding.receiver=[](o::Context&,o::Value receiver){return FunctionReceiver(std::move(receiver));};
    primitiveBinding.arguments=[](o::Context&,std::span<const o::Value> args){
      if(args.size()!=3||!std::holds_alternative<bool>(args[0].data)||
          !std::holds_alternative<double>(args[1].data)||
          !std::holds_alternative<std::u16string>(args[2].data))
        throw o::Exception("TypeError",u"primitive callback received unexpected Source values");
      return std::vector<std::any>{std::get<bool>(args[0].data),std::get<double>(args[1].data),
                                   std::get<std::u16string>(args[2].data)};
    };
    primitiveBinding.result=[](o::Context&,const std::any&){return o::Value(o::Undefined{});};
    const auto primitiveCallback=runtime.functions().bind(primitiveBody,std::move(primitiveBinding),u"primitiveProbe");
    auto primitiveList=runtime.functionLists.bind(std::make_shared<FunctionList>());
    const o::Value primitiveListValue(primitiveList);
    const auto primitiveAdd=context.get(primitiveListValue,o::Key(u"add"));
    (void)context.callMember(primitiveAdd,primitiveListValue,
        std::array<o::Value,2>{primitiveCallback,o::Value(1.0)});
    const auto execute=context.get(primitiveListValue,o::Key(u"execute"));
    (void)context.callMember(execute,primitiveListValue,
        std::array<o::Value,3>{o::Value(true),o::Value(7.0),o::Value(u"source")});
    expect(sawPrimitiveTypes);
  };
};
} // namespace
